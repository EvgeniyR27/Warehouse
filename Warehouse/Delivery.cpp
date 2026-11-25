#include "Delivery.h"
#include "Order.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace {
    bool isRegionInList(const std::vector<std::string>& regions, const std::string& region) {
        return std::find(regions.begin(), regions.end(), region) != regions.end();
    }
}

// --- CourierDelivery ---

CourierDelivery::CourierDelivery()
    : supportedRegions_{ "Moscow", "SPb", "Kazan" },
    maxWeightKg_(20.0) {
}

std::string CourierDelivery::getName() const {
    return "Courier";
}

bool CourierDelivery::isAvailable(const Order& order) const {
    double weightKg = Lab1Adapter::weightToKg(order.getTotalWeight());
    return weightKg <= maxWeightKg_ && isRegionInList(supportedRegions_, order.getRegion());
}

Price CourierDelivery::getCost(const Order& order) const {
    if (!isAvailable(order)) {
        throw std::runtime_error("Courier delivery is unavailable for this order (weight > 20 kg or unsupported region)");
    }
    return Lab1Adapter::makePriceFromRubles(500.0);
}

std::string CourierDelivery::getEstimatedTime() const {
    return "1-2 days";
}

// --- PickupDelivery ---

PickupDelivery::PickupDelivery()
    : supportedRegions_{ "Moscow", "SPb", "Kazan", "Novosibirsk" } {
}

std::string PickupDelivery::getName() const {
    return "Pickup";
}

bool PickupDelivery::isAvailable(const Order& order) const {
    return isRegionInList(supportedRegions_, order.getRegion());
}

Price PickupDelivery::getCost(const Order& order) const {
    if (!isAvailable(order)) {
        throw std::runtime_error("Pickup delivery is unavailable in this region");
    }
    return Lab1Adapter::makePriceFromRubles(0.0);
}

std::string PickupDelivery::getEstimatedTime() const {
    return "1-3 days";
}

// --- PostDelivery ---

PostDelivery::PostDelivery()
    : supportedRegions_{ "Moscow", "SPb", "Kazan", "Novosibirsk", "Sochi" },
    maxWeightKg_(10.0) {
}

std::string PostDelivery::getName() const {
    return "Post";
}

bool PostDelivery::isAvailable(const Order& order) const {
    double weightKg = Lab1Adapter::weightToKg(order.getTotalWeight());
    return weightKg <= maxWeightKg_ && isRegionInList(supportedRegions_, order.getRegion());
}

Price PostDelivery::getCost(const Order& order) const {
    if (!isAvailable(order)) {
        throw std::runtime_error("Post delivery is unavailable for this order (weight > 10 kg or unsupported region)");
    }
    return Lab1Adapter::makePriceFromRubles(200.0);
}

std::string PostDelivery::getEstimatedTime() const {
    return "5-10 days";
}

// --- DeliveryProxy ---

DeliveryProxy::DeliveryProxy(std::shared_ptr<IDeliveryStrategy> strategy,
    int tariffVersion,
    std::function<void(const std::string&)> logSink)
    : realStrategy_(std::move(strategy)),
    tariffVersion_(tariffVersion),
    logSink_(std::move(logSink)) {
    if (!realStrategy_) {
        throw std::invalid_argument("DeliveryProxy requires a non-null delivery strategy");
    }
}

void DeliveryProxy::setStrategy(std::shared_ptr<IDeliveryStrategy> newStrategy) {
    if (!newStrategy) {
        throw std::invalid_argument("Delivery strategy cannot be null");
    }
    realStrategy_ = std::move(newStrategy);
}

void DeliveryProxy::bumpTariffVersion() {
    ++tariffVersion_;
}

void DeliveryProxy::clearCache() {
    cache_.clear();
}

std::string DeliveryProxy::buildCacheKey(const Order& order) const {
    // Переводим вес в целые граммы, чтобы избежать погрешностей double в ключе
    long long weightGrams = std::llround(Lab1Adapter::weightToKg(order.getTotalWeight()) * 1000.0);
    return realStrategy_->getName() +
        "|v" + std::to_string(tariffVersion_) +
        "|" + order.getRegion() +
        "|" + std::to_string(weightGrams) + "g";
}

void DeliveryProxy::recordLog(const std::string& message) const {
    logs_.push_back(message);
    if (logSink_) {
        logSink_(message);
    }
}

std::string DeliveryProxy::getName() const {
    return realStrategy_->getName();
}

bool DeliveryProxy::isAvailable(const Order& order) const {
    return realStrategy_->isAvailable(order);
}

Price DeliveryProxy::getCost(const Order& order) const {
    // 1. Всегда проверяем доступность ДО обращения к кешу
    if (!realStrategy_->isAvailable(order)) {
        recordLog("[REJECTED] Order #" + std::to_string(order.getId()) +
            " | Strategy: " + realStrategy_->getName() +
            " | Reason: unavailable (weight/region)");
        throw std::runtime_error("DeliveryProxy: delivery is unavailable for Order #" +
            std::to_string(order.getId()));
    }

    // 2. Проверяем кеш по всем значимым входам тарифа
    std::string key = buildCacheKey(order);
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        recordLog("[CACHE HIT] Order #" + std::to_string(order.getId()) +
            " | Key: " + key);
        return it->second;
    }

    // 3. При промахе кеша вызываем реальную стратегию и сохраняем результат
    Price cost = realStrategy_->getCost(order);
    cache_.emplace(key, cost);
    recordLog("[CALCULATED] Order #" + std::to_string(order.getId()) +
        " | Key: " + key +
        " | Cost: " + std::to_string(Lab1Adapter::priceToRubles(cost)) + " rub");
    return cost;
}

const std::vector<std::string>& DeliveryProxy::getLogs() const {
    return logs_;
}