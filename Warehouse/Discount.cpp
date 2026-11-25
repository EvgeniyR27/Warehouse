#include "Discount.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

// --- PercentageDiscount ---

PercentageDiscount::PercentageDiscount(double percent) : percent_(percent) {
    if (percent_ < 0.0 || percent_ > 100.0) {
        throw std::invalid_argument("PercentageDiscount must be between 0 and 100");
    }
}

std::string PercentageDiscount::getName() const {
    return "PercentageDiscount (" + std::to_string(static_cast<int>(percent_)) + "%)";
}

int PercentageDiscount::getPriority() const {
    return 1;
}

void PercentageDiscount::apply(OrderCalculationContext& ctx) const {
    std::int64_t subtotalKopecks = Lab1Adapter::priceToKopecks(ctx.itemsSubtotal);
    std::int64_t currentDiscKopecks = Lab1Adapter::priceToKopecks(ctx.itemsDiscount);
    std::int64_t addedDiscKopecks = std::llround(subtotalKopecks * (percent_ / 100.0));

    std::int64_t totalDisc = std::min(subtotalKopecks, currentDiscKopecks + addedDiscKopecks);
    ctx.itemsDiscount = Lab1Adapter::kopecksToPrice(totalDisc);
}

// --- CashbackDiscount ---

CashbackDiscount::CashbackDiscount(double percent) : percent_(percent) {
    if (percent_ < 0.0 || percent_ > 100.0) {
        throw std::invalid_argument("CashbackDiscount must be between 0 and 100");
    }
}

std::string CashbackDiscount::getName() const {
    return "CashbackDiscount (" + std::to_string(static_cast<int>(percent_)) + "%)";
}

int CashbackDiscount::getPriority() const {
    return 2;
}

void CashbackDiscount::apply(OrderCalculationContext& ctx) const {
    // Считаем кэшбэк от стоимости товаров ПОСЛЕ применения ценовой скидки
    std::int64_t afterDiscountKopecks = Lab1Adapter::priceToKopecks(ctx.getItemsAfterDiscount());
    std::int64_t currentCashback = Lab1Adapter::priceToKopecks(ctx.cashbackEarned);
    std::int64_t addedCashback = std::llround(afterDiscountKopecks * (percent_ / 100.0));

    ctx.cashbackEarned = Lab1Adapter::kopecksToPrice(currentCashback + addedCashback);
}

// --- FreeDeliveryDiscount ---

std::string FreeDeliveryDiscount::getName() const {
    return "FreeDeliveryDiscount";
}

int FreeDeliveryDiscount::getPriority() const {
    return 3;
}

void FreeDeliveryDiscount::apply(OrderCalculationContext& ctx) const {
    ctx.deliveryCost = Lab1Adapter::makePriceFromRubles(0.0);
}

// --- GiftDiscount ---

GiftDiscount::GiftDiscount(std::string giftId, std::string giftName, Weight giftWeight)
    : giftId_(std::move(giftId)),
    giftName_(std::move(giftName)),
    giftWeight_(giftWeight) {
}

std::string GiftDiscount::getName() const {
    return "GiftDiscount (" + giftName_ + ")";
}

int GiftDiscount::getPriority() const {
    return 4;
}

void GiftDiscount::apply(OrderCalculationContext& ctx) const {
    // Контекст создаётся заново при каждом recalculate(), поэтому подарок не дублируется
    ctx.gifts.emplace_back(
        giftId_,
        giftName_ + " [GIFT]",
        Lab1Adapter::makePriceFromRubles(0.0),
        giftWeight_,
        Quantity(1)
    );
}