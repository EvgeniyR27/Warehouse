#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Order.h"
#include "OrderObserver.h"

class OrderManager : public IOrderObserver {
public:
    static constexpr std::size_t MAX_ACTIVE_ORDERS_PER_CLIENT = 5;

private:
    int nextOrderId_;
    std::unordered_map<int, std::shared_ptr<Order>> orders_;
    std::unordered_set<std::string> uniqueClients_;

    std::int64_t grossRevenueKopecks_; // Все поступившие оплаты
    std::int64_t refundsKopecks_;      // Возвраты по отменённым оплаченным заказам

    static bool isOrderActive(OrderStatus status);

public:
    OrderManager();

    std::shared_ptr<Order> createOrder(const std::string& clientId, const std::string& region);
    std::shared_ptr<Order> findOrderById(int orderId) const;

    std::size_t getActiveOrdersCount(const std::string& clientId) const;
    std::size_t getTotalOrdersCount() const;
    std::size_t getUniqueClientsCount() const;

    // Документированное определение оборота: оплаченные заказы минус возвраты при отмене оплаченного заказа
    Price getTotalRevenue() const;
    Price getGrossRevenue() const;
    Price getTotalRefunds() const;

    // Реакция на смену статуса заказа (паттерн Наблюдатель)
    void onOrderStatusChanged(const OrderStatusChangedEvent& event) override;
};