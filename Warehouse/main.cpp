#include <iostream>
#include <memory>
#include <vector>
#include "ProductFactory.h"
#include "OrderManager.h"
#include "Delivery.h"
#include "Discount.h"

int main() {
    try {
        std::cout << "========================================================\n";
        std::cout << "1. LAB 1 COMPATIBILITY: PRODUCT FACTORIES & CATALOG\n";
        std::cout << "========================================================\n";

        // Используем конкретные фабрики из вашего ProductFactory.h (Лабораторная работа №1)
        LaptopFactory laptopFactory;
        SmartphoneFactory smartphoneFactory;
        JacketFactory jacketFactory;

        // Сигнатура из Лабы 1: createProduct(int id, const std::string& name, int priceKopecks, int quantity, double weightKg)
        auto laptop = laptopFactory.createProduct(101, "Gaming Laptop", 8000000, 10, 2.5);
        auto smartphone = smartphoneFactory.createProduct(102, "Flagship Smartphone", 5000000, 25, 0.2);
        auto jacket = jacketFactory.createProduct(103, "Winter Jacket", 1500000, 15, 1.8);

        std::cout << "Products created via Lab 1 factories:\n"
            << "  1) ID " << laptop->getId() << ": " << laptop->getName() << "\n"
            << "  2) ID " << smartphone->getId() << ": " << smartphone->getName() << "\n"
            << "  3) ID " << jacket->getId() << ": " << jacket->getName() << "\n\n";

        std::cout << "========================================================\n";
        std::cout << "2. DELIVERY STRATEGIES & DELIVERY PROXY (CACHE + REJECT)\n";
        std::cout << "========================================================\n";

        OrderManager manager;
        auto order1 = manager.createOrder("client_ivan", "Moscow");
        order1->addItem(*laptop, Quantity(1));     // 2.5 кг
        order1->addItem(*smartphone, Quantity(2)); // 0.4 кг -> Итого 2.9 кг

        auto courierProxy = std::make_shared<DeliveryProxy>(std::make_shared<CourierDelivery>());
        auto pickupProxy = std::make_shared<DeliveryProxy>(std::make_shared<PickupDelivery>());
        auto postProxy = std::make_shared<DeliveryProxy>(std::make_shared<PostDelivery>());

        // Проверяем все 3 доставки на допустимой массе (2.9 кг)
        std::cout << "Pickup cost:  " << Lab1Adapter::priceToRubles(pickupProxy->getCost(*order1)) << " rub\n";
        std::cout << "Post cost:    " << Lab1Adapter::priceToRubles(postProxy->getCost(*order1)) << " rub\n";
        std::cout << "Courier cost: " << Lab1Adapter::priceToRubles(courierProxy->getCost(*order1)) << " rub\n";

        // Повторный запрос без изменения заказа -> должен сработать CACHE HIT
        std::cout << "Courier repeat request: "
            << Lab1Adapter::priceToRubles(courierProxy->getCost(*order1)) << " rub\n";

        // Добавляем ещё один смартфон -> масса изменилась (3.1 кг) -> ключ кеша изменился -> пересчёт
        order1->addItem(*smartphone, Quantity(1));
        std::cout << "Courier after weight change: "
            << Lab1Adapter::priceToRubles(courierProxy->getCost(*order1)) << " rub\n";

        // Инвалидация кеша при смене версии тарифа
        courierProxy->bumpTariffVersion();
        std::cout << "Courier after tariff version bump: "
            << Lab1Adapter::priceToRubles(courierProxy->getCost(*order1)) << " rub\n";

        // Проверка отказа по превышению массы (5 ноутбуков * 2.5 кг = 12.5 кг > лимита 10 кг для Почты)
        auto heavyOrder = manager.createOrder("client_anna", "Moscow");
        heavyOrder->addItem(*laptop, Quantity(5));
        try {
            postProxy->getCost(*heavyOrder);
        }
        catch (const std::exception& ex) {
            std::cout << "Expected rejection (Post > 10kg): " << ex.what()
                << " | Order status remained: " << orderStatusToString(heavyOrder->getStatus()) << "\n";
        }

        // Проверка отказа по неподдерживаемому региону
        auto remoteOrder = manager.createOrder("client_oleg", "Vladivostok");
        remoteOrder->addItem(*jacket, Quantity(1));
        try {
            courierProxy->getCost(*remoteOrder);
        }
        catch (const std::exception& ex) {
            std::cout << "Expected rejection (Region): " << ex.what() << "\n";
        }

        std::cout << "\n--- CourierProxy Logs ---\n";
        for (const auto& entry : courierProxy->getLogs()) {
            std::cout << entry << "\n";
        }
        std::cout << "--- PostProxy Logs ---\n";
        for (const auto& entry : postProxy->getLogs()) {
            std::cout << entry << "\n";
        }
        std::cout << "\n";

        std::cout << "========================================================\n";
        std::cout << "3. ALL 4 DISCOUNTS & RECALCULATION IDEMPOTENCY\n";
        std::cout << "========================================================\n";

        // Назначаем курьерскую доставку (500 руб.) и применяем все 4 акции:
        // Порядок: 1) 10% скидка -> 2) 5% cashback -> 3) бесплатная доставка -> 4) подарок
        order1->setDelivery(courierProxy);
        order1->applyDiscount(std::make_shared<PercentageDiscount>(10.0));
        order1->applyDiscount(std::make_shared<CashbackDiscount>(5.0));
        order1->applyDiscount(std::make_shared<FreeDeliveryDiscount>());
        order1->applyDiscount(std::make_shared<GiftDiscount>("GIFT-01", "Protective Case", Lab1Adapter::makeWeightFromKg(0.1)));

        std::cout << *order1 << "\n";

        // Проверяем, что повторный пересчёт (при добавлении куртки)
        // не дублирует подарок и корректно пересчитывает скидку и кэшбэк
        order1->addItem(*jacket, Quantity(1));
        std::cout << "After adding 1 jacket (gifts count = " << order1->getGifts().size() << "):\n";
        std::cout << *order1 << "\n";

        std::cout << "========================================================\n";
        std::cout << "4. OBSERVER, LIFECYCLE, DOUBLE PAYMENT & PRICE LOCK\n";
        std::cout << "========================================================\n";

        ConsoleOrderObserver auditLogger("AuditObserver");
        order1->subscribe(&auditLogger);
        order1->subscribe(&auditLogger); // Повторная подписка игнорируется (дублей не будет)

        order1->pay();
        Price paidAmount = order1->getFinalPrice();

        // Повторная оплата не вызывает событие Observer и не удваивает оборот
        order1->pay();
        std::cout << "Revenue after duplicate pay() call: "
            << Lab1Adapter::priceToRubles(manager.getTotalRevenue()) << " rub\n";

        // Изменяем цену товара в каталоге после оплаты — оплаченная сумма заказа не меняется
        Lab1Adapter::tryUpdateProductPrice(*laptop, Lab1Adapter::makePriceFromRubles(120000.0));
        auto updatedCatalogLaptop = laptopFactory.createProduct(101, "Gaming Laptop", 12000000, 10, 2.5);
        (void)updatedCatalogLaptop;
        std::cout << "Paid order total after catalog price change: "
            << Lab1Adapter::priceToRubles(order1->getFinalPrice())
            << " rub (was " << Lab1Adapter::priceToRubles(paidAmount) << " rub)\n";

        order1->ship();
        order1->unsubscribe(&auditLogger); // Отписываем наблюдателя перед доставкой
        order1->deliver();                 // Сообщение от AuditObserver уже не выведется

        std::cout << "\n========================================================\n";
        std::cout << "5. ORDER MANAGER LIMITS, CANCELLATION & STATISTICS\n";
        std::cout << "========================================================\n";

        // У client_ivan order1 уже Delivered (неактивен). Создаём 5 активных заказов подряд:
        std::vector<std::shared_ptr<Order>> ivanOrders;
        for (int i = 0; i < 5; ++i) {
            auto ord = manager.createOrder("client_ivan", "Moscow");
            ord->addItem(*jacket, Quantity(1)); // 15 000 руб.
            ivanOrders.push_back(ord);
        }
        std::cout << "Ivan active orders: " << manager.getActiveOrdersCount("client_ivan") << "\n";

        // Попытка создать 6-й активный заказ -> должна быть отклонена
        try {
            manager.createOrder("client_ivan", "Moscow");
        }
        catch (const std::exception& ex) {
            std::cout << "Expected limit error on 6th order: " << ex.what() << "\n";
        }

        // Оплачиваем один из новых заказов и затем отменяем его до отправки (возврат средств)
        ivanOrders[0]->pay();
        std::cout << "Revenue before refund: "
            << Lab1Adapter::priceToRubles(manager.getTotalRevenue()) << " rub\n";

        ivanOrders[0]->cancel(); // Возврат 15 000 руб. + освобождение 1 слота активных заказов
        std::cout << "Revenue after paid order cancellation (refund): "
            << Lab1Adapter::priceToRubles(manager.getTotalRevenue()) << " rub\n";
        std::cout << "Ivan active orders after cancel: "
            << manager.getActiveOrdersCount("client_ivan") << "\n";

        // Теперь слот свободен, и новый заказ снова создаётся успешно
        auto extraOrder = manager.createOrder("client_ivan", "Moscow");
        extraOrder->addItem(*smartphone, Quantity(1));
        std::cout << "Created Order #" << extraOrder->getId() << " after slot freed!\n";

        std::cout << "\n--- Final Manager Statistics ---\n";
        std::cout << "Total orders created: " << manager.getTotalOrdersCount() << "\n";
        std::cout << "Unique clients count: " << manager.getUniqueClientsCount() << "\n";
        std::cout << "Gross revenue:        " << Lab1Adapter::priceToRubles(manager.getGrossRevenue()) << " rub\n";
        std::cout << "Total refunds:        " << Lab1Adapter::priceToRubles(manager.getTotalRefunds()) << " rub\n";
        std::cout << "Net turnover:         " << Lab1Adapter::priceToRubles(manager.getTotalRevenue()) << " rub\n";

    }
    catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}