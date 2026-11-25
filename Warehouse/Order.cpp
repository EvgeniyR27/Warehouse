#include "Order.h"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

Order::Order(int id, std::string clientId, std::string region)
    : id_(id),
    clientId_(std::move(clientId)),
    region_(std::move(region)),
    status_(OrderStatus::Pending),
    fixedPaidTotal_(Lab1Adapter::makePriceFromRubles(0.0)) {
    if (clientId_.empty()) {
        throw std::invalid_argument("Client ID cannot be empty");
    }
    if (region_.empty()) {
        throw std::invalid_argument("Delivery region cannot be empty");
    }
}

void Order::ensureEditable() const {
    if (status_ != OrderStatus::Pending) {
        throw std::logic_error("Order #" + std::to_string(id_) +
            " cannot be modified in status " + orderStatusToString(status_));
    }
}

void Order::recalculate() {
    // Создаём чистый контекст расчёта с нуля
    OrderCalculationContext fresh;

    std::int64_t subtotalKopecks = 0;
    for (const auto& item : items_) {
        subtotalKopecks += Lab1Adapter::priceToKopecks(item.getTotalPrice());
    }
    fresh.itemsSubtotal = Lab1Adapter::kopecksToPrice(subtotalKopecks);

    // Если доставка уже назначена, запрашиваем актуальную стоимость
    if (deliveryStrategy_) {
        fresh.deliveryCost = deliveryStrategy_->getCost(*this);
    }

    // Применяем акции строго по их приоритету (1 -> 2 -> 3 -> 4)
    for (const auto& discount : discounts_) {
        discount->apply(fresh);
    }

    calc_ = std::move(fresh);
}

std::string Order::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm tmBuf{};
#ifdef _WIN32
    localtime_s(&tmBuf, &now);
#else
    localtime_r(&now, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Order::notifyObservers(OrderStatus oldStatus, OrderStatus newStatus) {
    OrderStatusChangedEvent event{
        id_,
        clientId_,
        oldStatus,
        newStatus,
        getFinalPrice(),
        getCurrentTimestamp()
    };

    // Делаем копию списка на случай, если наблюдатель отпишется во время уведомления
    auto snapshot = observers_;
    for (IOrderObserver* obs : snapshot) {
        if (obs != nullptr) {
            obs->onOrderStatusChanged(event);
        }
    }
}

int Order::getId() const {
    return id_;
}

const std::string& Order::getClientId() const {
    return clientId_;
}

const std::string& Order::getRegion() const {
    return region_;
}

OrderStatus Order::getStatus() const {
    return status_;
}

const std::vector<OrderItem>& Order::getItems() const {
    return items_;
}

const std::vector<OrderItem>& Order::getGifts() const {
    return calc_.gifts;
}

Weight Order::getTotalWeight() const {
    double totalKg = 0.0;
    for (const auto& item : items_) {
        totalKg += Lab1Adapter::weightToKg(item.getTotalWeight());
    }
    return Lab1Adapter::makeWeightFromKg(totalKg);
}

Price Order::getItemsSubtotal() const {
    return calc_.itemsSubtotal;
}

Price Order::getItemsDiscount() const {
    return calc_.itemsDiscount;
}

Price Order::getDeliveryCost() const {
    return calc_.deliveryCost;
}

Price Order::getCashbackEarned() const {
    return calc_.cashbackEarned;
}

Price Order::getFinalPrice() const {
    // После оплаты сумма зафиксирована и не зависит от дальнейших изменений каталога
    if (status_ == OrderStatus::Paid ||
        status_ == OrderStatus::Shipped ||
        status_ == OrderStatus::Delivered) {
        return fixedPaidTotal_;
    }
    return calc_.getFinalPayable();
}

void Order::addItem(const Product& product, Quantity quantity) {
    ensureEditable();
    int addCount = Lab1Adapter::quantityToInt(quantity);
    if (addCount <= 0) {
        throw std::invalid_argument("Quantity to add must be positive");
    }

    std::string targetId = Lab1Adapter::productIdToString(product.getId());
    auto it = std::find_if(items_.begin(), items_.end(), [&](const OrderItem& item) {
        return item.getProductId() == targetId;
        });

    if (it != items_.end()) {
        int currentCount = Lab1Adapter::quantityToInt(it->getQuantity());
        it->setQuantity(Quantity(currentCount + addCount));
    }
    else {
        items_.emplace_back(product, quantity);
    }

    recalculate();
}

void Order::removeItem(const std::string& productId) {
    ensureEditable();
    auto it = std::remove_if(items_.begin(), items_.end(), [&](const OrderItem& item) {
        return item.getProductId() == productId;
        });
    if (it == items_.end()) {
        throw std::invalid_argument("Product not found in order: " + productId);
    }
    items_.erase(it, items_.end());
    recalculate();
}

void Order::applyDiscount(std::shared_ptr<IDiscountStrategy> discount) {
    ensureEditable();
    if (!discount) {
        throw std::invalid_argument("Discount strategy cannot be null");
    }

    // Не добавляем один и тот же экземпляр стратегии дважды
    if (std::find(discounts_.begin(), discounts_.end(), discount) == discounts_.end()) {
        discounts_.push_back(std::move(discount));
        std::sort(discounts_.begin(), discounts_.end(),
            [](const std::shared_ptr<IDiscountStrategy>& a,
                const std::shared_ptr<IDiscountStrategy>& b) {
                    return a->getPriority() < b->getPriority();
            });
    }

    recalculate();
}

void Order::setDelivery(std::shared_ptr<IDeliveryStrategy> delivery) {
    ensureEditable();
    if (!delivery) {
        throw std::invalid_argument("Delivery strategy cannot be null");
    }

    // Сначала проверяем расчёт стоимости: если доставка недоступна (вес/регион),
    // getCost выбросит исключение, и состояние заказа не испортится.
    Price cost = delivery->getCost(*this);
    (void)cost;

    deliveryStrategy_ = std::move(delivery);
    recalculate();
}

void Order::setStatus(OrderStatus newStatus) {
    // Повторная установка того же статуса не вызывает событий и не меняет состояние
    if (status_ == newStatus) {
        return;
    }

    bool valid = false;
    switch (status_) {
    case OrderStatus::Pending:
        valid = (newStatus == OrderStatus::Paid || newStatus == OrderStatus::Cancelled);
        break;
    case OrderStatus::Paid:
        valid = (newStatus == OrderStatus::Shipped || newStatus == OrderStatus::Cancelled);
        break;
    case OrderStatus::Shipped:
        valid = (newStatus == OrderStatus::Delivered);
        break;
    case OrderStatus::Delivered:
    case OrderStatus::Cancelled:
        valid = false;
        break;
    }

    if (!valid) {
        throw std::logic_error("Invalid status transition: " +
            orderStatusToString(status_) + " -> " +
            orderStatusToString(newStatus));
    }

    if (newStatus == OrderStatus::Paid) {
        if (items_.empty()) {
            throw std::logic_error("Cannot pay for an empty order");
        }
        recalculate();
        fixedPaidTotal_ = calc_.getFinalPayable();
    }

    OrderStatus oldStatus = status_;
    status_ = newStatus;
    notifyObservers(oldStatus, newStatus);
}

void Order::pay() {
    setStatus(OrderStatus::Paid);
}

void Order::ship() {
    setStatus(OrderStatus::Shipped);
}

void Order::deliver() {
    setStatus(OrderStatus::Delivered);
}

void Order::cancel() {
    setStatus(OrderStatus::Cancelled);
}

void Order::subscribe(IOrderObserver* observer) {
    if (observer == nullptr) {
        return;
    }
    if (std::find(observers_.begin(), observers_.end(), observer) == observers_.end()) {
        observers_.push_back(observer);
    }
}

void Order::unsubscribe(IOrderObserver* observer) {
    observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

std::string Order::toString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "=== Order #" << id_ << " [" << orderStatusToString(status_) << "] ===\n"
        << "Client: " << clientId_ << " | Region: " << region_
        << " | Total weight: " << Lab1Adapter::weightToKg(getTotalWeight()) << " kg\n"
        << "Items:\n";

    for (const auto& item : items_) {
        oss << "  - " << item.getProductName()
            << " x" << Lab1Adapter::quantityToInt(item.getQuantity())
            << " (" << Lab1Adapter::priceToRubles(item.getUnitPrice()) << " rub/pc) = "
            << Lab1Adapter::priceToRubles(item.getTotalPrice()) << " rub\n";
    }

    if (!calc_.gifts.empty()) {
        oss << "Gifts:\n";
        for (const auto& gift : calc_.gifts) {
            oss << "  + " << gift.getProductName() << " x1 (0.00 rub)\n";
        }
    }

    oss << "Delivery: "
        << (deliveryStrategy_ ? deliveryStrategy_->getName() + " (" + deliveryStrategy_->getEstimatedTime() + ")" : "Not selected")
        << "\n"
        << "Subtotal: " << Lab1Adapter::priceToRubles(calc_.itemsSubtotal) << " rub"
        << " | Discount: -" << Lab1Adapter::priceToRubles(calc_.itemsDiscount) << " rub"
        << " | Delivery cost: " << Lab1Adapter::priceToRubles(calc_.deliveryCost) << " rub\n"
        << "Cashback bonus: " << Lab1Adapter::priceToRubles(calc_.cashbackEarned) << " rub"
        << " | FINAL TO PAY: " << Lab1Adapter::priceToRubles(getFinalPrice()) << " rub\n";

    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Order& order) {
    os << order.toString();
    return os;
}