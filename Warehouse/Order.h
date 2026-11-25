#pragma once
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "Delivery.h"
#include "Discount.h"
#include "Lab1Adapter.h"
#include "OrderItem.h"
#include "OrderObserver.h"

class Order {
private:
    int id_;
    std::string clientId_;
    std::string region_;
    OrderStatus status_;

    std::vector<OrderItem> items_;
    std::vector<std::shared_ptr<IDiscountStrategy>> discounts_;
    std::shared_ptr<IDeliveryStrategy> deliveryStrategy_;

    OrderCalculationContext calc_;
    Price fixedPaidTotal_; // Сумма, зафиксированная при оплате

    std::vector<IOrderObserver*> observers_;

    void ensureEditable() const;
    void recalculate();
    void notifyObservers(OrderStatus oldStatus, OrderStatus newStatus);
    static std::string getCurrentTimestamp();

public:
    Order(int id, std::string clientId, std::string region);

    // Геттеры (безопасный просмотр без выдачи изменяемых внутренних ссылок)
    int getId() const;
    const std::string& getClientId() const;
    const std::string& getRegion() const;
    OrderStatus getStatus() const;

    const std::vector<OrderItem>& getItems() const;
    const std::vector<OrderItem>& getGifts() const;

    Weight getTotalWeight() const;
    Price getItemsSubtotal() const;
    Price getItemsDiscount() const;
    Price getDeliveryCost() const;
    Price getCashbackEarned() const;
    Price getFinalPrice() const;

    // Три операции взаимодействия с заказом (доступны только в статусе Pending)
    void addItem(const Product& product, Quantity quantity);
    void removeItem(const std::string& productId);
    void applyDiscount(std::shared_ptr<IDiscountStrategy> discount);
    void setDelivery(std::shared_ptr<IDeliveryStrategy> delivery);

    // Управление жизненным циклом заказа
    void pay();
    void ship();
    void deliver();
    void cancel();
    void setStatus(OrderStatus newStatus);

    // Паттерн Наблюдатель (подписка и отписка без дубликатов)
    void subscribe(IOrderObserver* observer);
    void unsubscribe(IOrderObserver* observer);

    // Строковое представление заказа
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Order& order);
};
