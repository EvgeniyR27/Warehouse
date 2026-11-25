#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Lab1Adapter.h"

class Order; // Предварительное объявление, чтобы избежать циклического include

// Единый контракт способов доставки
class IDeliveryStrategy {
public:
    virtual ~IDeliveryStrategy() = default;

    virtual std::string getName() const = 0;
    virtual bool isAvailable(const Order& order) const = 0;
    virtual Price getCost(const Order& order) const = 0;
    virtual std::string getEstimatedTime() const = 0;
};

// 1. Курьерская доставка: 500 руб., 1-2 дня, до 20 кг, заданные зоны
class CourierDelivery : public IDeliveryStrategy {
private:
    std::vector<std::string> supportedRegions_;
    double maxWeightKg_;

public:
    CourierDelivery();

    std::string getName() const override;
    bool isAvailable(const Order& order) const override;
    Price getCost(const Order& order) const override;
    std::string getEstimatedTime() const override;
};

// 2. Самовывоз: 0 руб., 1-3 дня, без ограничения массы в учебном примере
class PickupDelivery : public IDeliveryStrategy {
private:
    std::vector<std::string> supportedRegions_;

public:
    PickupDelivery();

    std::string getName() const override;
    bool isAvailable(const Order& order) const override;
    Price getCost(const Order& order) const override;
    std::string getEstimatedTime() const override;
};

// 3. Почта: 200 руб., 5-10 дней, до 10 кг, заданные зоны
class PostDelivery : public IDeliveryStrategy {
private:
    std::vector<std::string> supportedRegions_;
    double maxWeightKg_;

public:
    PostDelivery();

    std::string getName() const override;
    bool isAvailable(const Order& order) const override;
    Price getCost(const Order& order) const override;
    std::string getEstimatedTime() const override;
};

// Заместитель (Proxy) доставки: проверка доступности, кеш и журнал обращений
class DeliveryProxy : public IDeliveryStrategy {
private:
    std::shared_ptr<IDeliveryStrategy> realStrategy_;
    int tariffVersion_;
    mutable std::unordered_map<std::string, Price> cache_;
    mutable std::vector<std::string> logs_;
    std::function<void(const std::string&)> logSink_;

    std::string buildCacheKey(const Order& order) const;
    void recordLog(const std::string& message) const;

public:
    explicit DeliveryProxy(std::shared_ptr<IDeliveryStrategy> strategy,
        int tariffVersion = 1,
        std::function<void(const std::string&)> logSink = nullptr);

    // Позволяет подменить стратегию доставки
    void setStrategy(std::shared_ptr<IDeliveryStrategy> newStrategy);

    // Изменение версии тарифа инвалидирует старые записи в кеше
    void bumpTariffVersion();
    void clearCache();

    std::string getName() const override;
    bool isAvailable(const Order& order) const override;
    Price getCost(const Order& order) const override;
    std::string getEstimatedTime() const override;

    const std::vector<std::string>& getLogs() const;
}; 
