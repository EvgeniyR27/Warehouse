#include "Warehouse.h"
#include <stdexcept>

// --- Реализация WarehouseIterator ---

WarehouseIterator::WarehouseIterator(std::vector<std::unique_ptr<Product>>::iterator current,
    std::vector<std::unique_ptr<Product>>::iterator end)
    : current_(current), end_(end) {
}

WarehouseIterator& WarehouseIterator::operator++() {
    if (current_ != end_) {
        ++current_;
    }
    return *this;
}

Product& WarehouseIterator::operator*() const {
    return **current_; // Двойное разыменование: итератор -> unique_ptr -> Product
}

Product* WarehouseIterator::operator->() const {
    return current_->get();
}

bool WarehouseIterator::operator!=(const WarehouseIterator& other) const {
    return current_ != other.current_;
}

bool WarehouseIterator::operator==(const WarehouseIterator& other) const {
    return current_ == other.current_;
}

// --- Реализация ConstWarehouseIterator ---

ConstWarehouseIterator::ConstWarehouseIterator(std::vector<std::unique_ptr<Product>>::const_iterator current,
    std::vector<std::unique_ptr<Product>>::const_iterator end)
    : current_(current), end_(end) {
}

ConstWarehouseIterator& ConstWarehouseIterator::operator++() {
    if (current_ != end_) {
        ++current_;
    }
    return *this;
}

const Product& ConstWarehouseIterator::operator*() const {
    return **current_;
}

const Product* ConstWarehouseIterator::operator->() const {
    return current_->get();
}

bool ConstWarehouseIterator::operator!=(const ConstWarehouseIterator& other) const {
    return current_ != other.current_;
}

bool ConstWarehouseIterator::operator==(const ConstWarehouseIterator& other) const {
    return current_ == other.current_;
}

// --- Реализация класса Warehouse ---

Warehouse::Warehouse(size_t maxSize) : maxSize_(maxSize), currentSize_(0) {
    // Проверка на положительный размер склада
    if (maxSize == 0) {
        throw std::invalid_argument("Ошибка: максимальный размер склада должен быть больше нуля!");
    }
}

// Конструктор глубокого копирования: клонируем каждый товар через виртуальный метод clone()
Warehouse::Warehouse(const Warehouse& other)
    : maxSize_(other.maxSize_), currentSize_(other.currentSize_) {
    products_.reserve(other.products_.size());
    for (size_t i = 0; i < other.products_.size(); ++i) {
        products_.push_back(other.products_[i]->clone());
    }
}

// Оператор присваивания копированием
Warehouse& Warehouse::operator=(const Warehouse& other) {
    if (this == &other) {
        return *this; // Защита от самоприсваивания (w = w)
    }
    maxSize_ = other.maxSize_;
    currentSize_ = other.currentSize_;
    products_.clear();
    products_.reserve(other.products_.size());
    for (size_t i = 0; i < other.products_.size(); ++i) {
        products_.push_back(other.products_[i]->clone());
    }
    return *this;
}

// Конструктор перемещения: переносит внутренний буфер вектора без клонирования объектов
Warehouse::Warehouse(Warehouse&& other) noexcept
    : products_(std::move(other.products_)),
    maxSize_(other.maxSize_),
    currentSize_(other.currentSize_) {
    other.maxSize_ = 0;
    other.currentSize_ = 0;
}

// Оператор присваивания перемещением
Warehouse& Warehouse::operator=(Warehouse&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    products_ = std::move(other.products_);
    maxSize_ = other.maxSize_;
    currentSize_ = other.currentSize_;
    other.maxSize_ = 0;
    other.currentSize_ = 0;
    return *this;
}

void Warehouse::addProduct(std::unique_ptr<Product> product) {
    if (!product) {
        throw std::invalid_argument("Ошибка: попытка добавить пустой указатель на товар!");
    }
    if (products_.size() >= maxSize_) {
        throw std::overflow_error("Ошибка: склад переполнен, превышен лимит вместимости!");
    }
    products_.push_back(std::move(product));
    currentSize_ = products_.size();
}

void Warehouse::removeById(int id) {
    for (auto it = products_.begin(); it != products_.end(); ++it) {
        if ((*it)->getId() == id) {
            products_.erase(it);
            currentSize_ = products_.size();
            return;
        }
    }
    throw std::runtime_error("Ошибка: товар с указанным ID не найден на складе!");
}

void Warehouse::removeByIndex(size_t index) {
    if (index >= products_.size()) {
        throw std::out_of_range("Ошибка: индекс выходит за пределы списка товаров!");
    }
    products_.erase(products_.begin() + index);
    currentSize_ = products_.size();
}

Product* Warehouse::findById(int id) const {
    for (size_t i = 0; i < products_.size(); ++i) {
        if (products_[i]->getId() == id) {
            return products_[i].get();
        }
    }
    return nullptr; // Если товар не найден, возвращаем нулевой указатель
}

std::vector<Product*> Warehouse::findByCategory(const std::string& category) const {
    std::vector<Product*> result;
    for (size_t i = 0; i < products_.size(); ++i) {
        if (products_[i]->getCategory() == category) {
            result.push_back(products_[i].get());
        }
    }
    return result;
}

std::vector<Product*> Warehouse::findByPrice(const Price& price) const {
    std::vector<Product*> result;
    for (size_t i = 0; i < products_.size(); ++i) {
        if (products_[i]->getPrice() == price) {
            result.push_back(products_[i].get());
        }
    }
    return result;
}

size_t Warehouse::getMaxSize() const { return maxSize_; }
size_t Warehouse::getCurrentSize() const { return currentSize_; }

WarehouseIterator Warehouse::begin() {
    return WarehouseIterator(products_.begin(), products_.end());
}

WarehouseIterator Warehouse::end() {
    return WarehouseIterator(products_.end(), products_.end());
}

ConstWarehouseIterator Warehouse::begin() const {
    return ConstWarehouseIterator(products_.begin(), products_.end());
}

ConstWarehouseIterator Warehouse::end() const {
    return ConstWarehouseIterator(products_.end(), products_.end());
}