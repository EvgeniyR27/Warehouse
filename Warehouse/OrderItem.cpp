#include "OrderItem.h"
#include <stdexcept>
#include <utility>

OrderItem::OrderItem(const Product& product, Quantity quantity)
    : productId_(product.getId()),
    productName_(product.getName()),
    unitPrice_(product.getPrice()),
    unitWeight_(product.getWeight()),
    quantity_(quantity) {
    if (Lab1Adapter::quantityToInt(quantity_) <= 0) {
        throw std::invalid_argument("OrderItem quantity must be positive");
    }
}

OrderItem::OrderItem(std::string productId,
    std::string productName,
    Price unitPrice,
    Weight unitWeight,
    Quantity quantity)
    : productId_(std::move(productId)),
    productName_(std::move(productName)),
    unitPrice_(unitPrice),
    unitWeight_(unitWeight),
    quantity_(quantity) {
    if (Lab1Adapter::quantityToInt(quantity_) <= 0) {
        throw std::invalid_argument("OrderItem quantity must be positive");
    }
}

const std::string& OrderItem::getProductId() const {
    return productId_;
}

const std::string& OrderItem::getProductName() const {
    return productName_;
}

Price OrderItem::getUnitPrice() const {
    return unitPrice_;
}

Weight OrderItem::getUnitWeight() const {
    return unitWeight_;
}

Quantity OrderItem::getQuantity() const {
    return quantity_;
}

void OrderItem::setQuantity(Quantity quantity) {
    if (Lab1Adapter::quantityToInt(quantity) <= 0) {
        throw std::invalid_argument("OrderItem quantity must be positive");
    }
    quantity_ = quantity;
}

Price OrderItem::getTotalPrice() const {
    std::int64_t unitKopecks = Lab1Adapter::priceToKopecks(unitPrice_);
    int count = Lab1Adapter::quantityToInt(quantity_);
    return Lab1Adapter::kopecksToPrice(unitKopecks * count);
}

Weight OrderItem::getTotalWeight() const {
    double singleKg = Lab1Adapter::weightToKg(unitWeight_);
    int count = Lab1Adapter::quantityToInt(quantity_);
    return Lab1Adapter::makeWeightFromKg(singleKg * count);
}