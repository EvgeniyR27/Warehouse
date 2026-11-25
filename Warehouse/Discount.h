#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Lab1Adapter.h"
#include "OrderItem.h"

// Контекст одного расчёта заказа. Создаётся заново при каждом пересчёте,
// поэтому повторный пересчёт не дублирует подарки и не удваивает кэшбэк.
struct OrderCalculationContext {
    Price itemsSubtotal = Lab1Adapter::makePriceFromRubles(0.0); // Сумма товаров до скидок
    Price itemsDiscount = Lab1Adapter::makePriceFromRubles(0.0); // Скидка на товары
    Price deliveryCost = Lab1Adapter::makePriceFromRubles(0.0);  // Стоимость доставки
    Price cashbackEarned = Lab1Adapter::makePriceFromRubles(0.0);// Начисленный бонусный кэшбэк
    std::vector<OrderItem> gifts;                                // Бесплатные подарочные позиции

    Price getItemsAfterDiscount() const {
        std::int64_t sub = Lab1Adapter::priceToKopecks(itemsSubtotal);
        std::int64_t disc = Lab1Adapter::priceToKopecks(itemsDiscount);
        std::int64_t res = (sub > disc) ? (sub - disc) : 0;
        return Lab1Adapter::kopecksToPrice(res);
    }

    Price getFinalPayable() const {
        std::int64_t itemsPay = Lab1Adapter::priceToKopecks(getItemsAfterDiscount());
        std::int64_t delivPay = Lab1Adapter::priceToKopecks(deliveryCost);
        return Lab1Adapter::kopecksToPrice(itemsPay + delivPay);
    }
};

// Единый контракт для всех 4 стратегий акций
class IDiscountStrategy {
public:
    virtual ~IDiscountStrategy() = default;

    virtual std::string getName() const = 0;
    // Приоритет задаёт фиксированный порядок применения акций:
    // 1 - Процентная скидка на товары
    // 2 - Cashback (считается от стоимости товаров ПОСЛЕ ценовой скидки)
    // 3 - Бесплатная доставка
    // 4 - Подарок
    virtual int getPriority() const = 0;
    virtual void apply(OrderCalculationContext& ctx) const = 0;
};

// 1. Процентная скидка только на стоимость товаров (доставку не снижает)
class PercentageDiscount : public IDiscountStrategy {
private:
    double percent_;

public:
    explicit PercentageDiscount(double percent);

    std::string getName() const override;
    int getPriority() const override;
    void apply(OrderCalculationContext& ctx) const override;
};

// 2. Cashback: начисляет % от стоимости товаров после скидки на бонусный счёт
// (не уменьшает сумму оплаты за текущий заказ)
class CashbackDiscount : public IDiscountStrategy {
private:
    double percent_;

public:
    explicit CashbackDiscount(double percent);

    std::string getName() const override;
    int getPriority() const override;
    void apply(OrderCalculationContext& ctx) const override;
};

// 3. Бесплатная доставка: обнуляет только стоимость доставки
class FreeDeliveryDiscount : public IDiscountStrategy {
public:
    std::string getName() const override;
    int getPriority() const override;
    void apply(OrderCalculationContext& ctx) const override;
};

// 4. Подарок: добавляет бесплатную подарочную позицию без второй оплаты
class GiftDiscount : public IDiscountStrategy {
private:
    std::string giftId_;
    std::string giftName_;
    Weight giftWeight_;

public:
    GiftDiscount(std::string giftId, std::string giftName, Weight giftWeight);

    std::string getName() const override;
    int getPriority() const override;
    void apply(OrderCalculationContext& ctx) const override;
};