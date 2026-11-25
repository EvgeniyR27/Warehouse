#include "OrderManager.h"
#include <stdexcept>

OrderManager::OrderManager()
    : nextOrderId_(1),
    grossRevenueKopecks_(0),
    refundsKopecks_(0) {
}

bool OrderManager::isOrderActive(OrderStatus status) {
    // Активными считаются заказы в обработке и доставке; Delivered и Cancelled освобождают слот
    return status == OrderStatus::Pending ||
        status == OrderStatus::Paid ||
        status == OrderStatus::Shipped;
}

std::shared_ptr<Order> OrderManager::createOrder(const std::string& clientId, const std::string& region) {
    if (clientId.empty()) {
        throw std::invalid_argument("OrderManager: clientId cannot be empty");
    }
    if (region.empty()) {
        throw std::invalid_argument("OrderManager: region cannot be empty");
    }

    if (getActiveOrdersCount(clientId) >= MAX_ACTIVE_ORDERS_PER_CLIENT) {
        throw std::runtime_error("OrderManager: client '" + clientId +
            "' reached active orders limit (" +
            std::to_string(MAX_ACTIVE_ORDERS_PER_CLIENT) + ")");
    }

    int id = nextOrderId_++;
    auto order = std::make_shared<Order>(id, clientId, region);
    order->subscribe(this); // Менеджер автоматически подписывается на события заказа

    orders_.emplace(id, order);
    uniqueClients_.insert(clientId);

    return order;
}

std::shared_ptr<Order> OrderManager::findOrderById(int orderId) const {
    auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        return nullptr;
    }
    return it->second;
}

std::size_t OrderManager::getActiveOrdersCount(const std::string& clientId) const {
    std::size_t count = 0;
    for (const auto& pair : orders_) {
        const auto& order = pair.second;
        if (order->getClientId() == clientId && isOrderActive(order->getStatus())) {
            ++count;
        }
    }
    return count;
}

std::size_t OrderManager::getTotalOrdersCount() const {
    return orders_.size();
}

std::size_t OrderManager::getUniqueClientsCount() const {
    return uniqueClients_.size();
}

Price OrderManager::getTotalRevenue() const {
    return Lab1Adapter::kopecksToPrice(grossRevenueKopecks_ - refundsKopecks_);
}

Price OrderManager::getGrossRevenue() const {
    return Lab1Adapter::kopecksToPrice(grossRevenueKopecks_);
}

Price OrderManager::getTotalRefunds() const {
    return Lab1Adapter::kopecksToPrice(refundsKopecks_);
}

void OrderManager::onOrderStatusChanged(const OrderStatusChangedEvent& event) {
    std::int64_t amountKopecks = Lab1Adapter::priceToKopecks(event.orderTotal);

    if (event.newStatus == OrderStatus::Paid) {
        grossRevenueKopecks_ += amountKopecks;
    }
    else if (event.oldStatus == OrderStatus::Paid && event.newStatus == OrderStatus::Cancelled) {
        // Возврат средств при отмене оплаченного заказа (до отправки)
        refundsKopecks_ += amountKopecks;
    }
    // Отмена неоплаченного заказа (Pending -> Cancelled) оборот не меняет
}