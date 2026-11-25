#pragma once
#include <iostream>
#include <string>
#include "Lab1Adapter.h"

enum class OrderStatus {
    Pending,
    Paid,
    Shipped,
    Delivered,
    Cancelled
};

inline std::string orderStatusToString(OrderStatus status) {
    switch (status) {
    case OrderStatus::Pending:   return "Pending";
    case OrderStatus::Paid:      return "Paid";
    case OrderStatus::Shipped:   return "Shipped";
    case OrderStatus::Delivered: return "Delivered";
    case OrderStatus::Cancelled: return "Cancelled";
    default:                     return "Unknown";
    }
}

struct OrderStatusChangedEvent {
    int orderId;
    std::string clientId;
    OrderStatus oldStatus;
    OrderStatus newStatus;
    Price orderTotal;
    std::string timestamp;
};

class IOrderObserver {
public:
    virtual ~IOrderObserver() = default;
    virtual void onOrderStatusChanged(const OrderStatusChangedEvent& event) = 0;
};

// Простой консольный наблюдатель для демонстрации подписки и отписки
class ConsoleOrderObserver : public IOrderObserver {
private:
    std::string listenerName_;

public:
    explicit ConsoleOrderObserver(std::string listenerName)
        : listenerName_(std::move(listenerName)) {
    }

    void onOrderStatusChanged(const OrderStatusChangedEvent& event) override {
        std::cout << "[" << listenerName_ << "] Order #" << event.orderId
            << " (Client: " << event.clientId << "): "
            << orderStatusToString(event.oldStatus) << " -> "
            << orderStatusToString(event.newStatus)
            << " | Total: " << Lab1Adapter::priceToRubles(event.orderTotal) << " rub"
            << " | Time: " << event.timestamp << "\n";
    }
};
