#pragma once
#include <string>
#include "Lab1Adapter.h"

class OrderItem {
private:
    std::string productId_;
    std::string productName_;
    Price unitPrice_;      // Зафиксированная цена за 1 шт.
    Weight unitWeight_;    // Масса 1 шт.
    Quantity quantity_;    // Заказанное количество (не связано со складом)

public:
    // Конструктор из товара каталога (фиксирует цену и вес на момент добавления)
    OrderItem(const Product& product, Quantity quantity);

    // Конструктор для ручного создания позиции (например, бесплатного подарка)
    OrderItem(std::string productId,
        std::string productName,
        Price unitPrice,
        Weight unitWeight,
        Quantity quantity);

    const std::string& getProductId() const;
    const std::string& getProductName() const;
    Price getUnitPrice() const;
    Weight getUnitWeight() const;
    Quantity getQuantity() const;

    void setQuantity(Quantity quantity);

    Price getTotalPrice() const;
    Weight getTotalWeight() const;
};
