#include "Product.h"
#include "Warehouse.h"
#include <iostream>

// Инициализация статического счётчика идентификаторов
int Product::nextId_ = 1;

Product::Product(int id, const std::string& name, const Price& price,
    const Quantity& quantity, const Weight& weight, const std::string& category)
    : name_(name), price_(price), quantity_(quantity), weight_(weight), category_(category) {
    // Если передан неположительный ID, используем автогенерацию
    if (id <= 0) {
        id_ = nextId_++;
    }
    else {
        id_ = id;
        // Обновляем счётчик, чтобы будущие автоматические ID не пересекались с ручными
        if (id >= nextId_) {
            nextId_ = id + 1;
        }
    }
}

int Product::getId() const { return id_; }
const std::string& Product::getName() const { return name_; }
const Price& Product::getPrice() const { return price_; }
const Quantity& Product::getQuantity() const { return quantity_; }
const Weight& Product::getWeight() const { return weight_; }
const std::string& Product::getCategory() const { return category_; }

void Product::setPrice(const Price& newPrice) {
    price_ = newPrice;
}

void Product::changeQuantity(int delta) {
    quantity_.change(delta);
}

// --- Реализация промежуточных категорий ---

Electronics::Electronics(int id, const std::string& name, const Price& price,
    const Quantity& quantity, const Weight& weight)
    : Product(id, name, price, quantity, weight, "electronics") {
}

Clothing::Clothing(int id, const std::string& name, const Price& price,
    const Quantity& quantity, const Weight& weight)
    : Product(id, name, price, quantity, weight, "clothing") {
}

Food::Food(int id, const std::string& name, const Price& price,
    const Quantity& quantity, const Weight& weight)
    : Product(id, name, price, quantity, weight, "food") {
}

// --- Реализация конкретных товаров ---

Smartphone::Smartphone(int id, const std::string& name, const Price& price,
    const Quantity& quantity, const Weight& weight)
    : Electronics(id, name, price, quantity, weight) {
}

std::unique_ptr<Product> Smartphone::clone() const {
    return std::make_unique<Smartphone>(*this);
}

void Smartphone::transferTo(Warehouse& other) {
    // Клонируем товар на целевой склад, а на текущем обнуляем остаток
    other.addProduct(this->clone());
    this->quantity_.setValue(0);
    std::cout << "[Smartphone] Смартфон \"" << name_ << "\" перемещён на другой склад.\n";
}

Laptop::Laptop(int id, const std::string& name, const Price& price,
    const Quantity& quantity, const Weight& weight)
    : Electronics(id, name, price, quantity, weight) {
}

std::unique_ptr<Product> Laptop::clone() const {
    return std::make_unique<Laptop>(*this);
}

void Laptop::transferTo(Warehouse& other) {
    other.addProduct(this->clone());
    this->quantity_.setValue(0);
    std::cout << "[Laptop] Ноутбук \"" << name_ << "\" перемещён на другой склад.\n";
}

TShirt::TShirt(int id, const std::string& name, const Price& price,
    const Quantity& quantity, const Weight& weight)
    : Clothing(id, name, price, quantity, weight) {
}

std::unique_ptr<Product> TShirt::clone() const {
    return std::make_unique<TShirt>(*this);
}

void TShirt::transferTo(Warehouse& other) {
    other.addProduct(this->clone());
    this->quantity_.setValue(0);
    std::cout << "[TShirt] Футболка \"" << name_ << "\" перемещена на другой склад.\n";
}

Jacket::Jacket(int id, const std::string& name, const Price& price,
    const Quantity& quantity, const Weight& weight)
    : Clothing(id, name, price, quantity, weight) {
}

std::unique_ptr<Product> Jacket::clone() const {
    return std::make_unique<Jacket>(*this);
}

void Jacket::transferTo(Warehouse& other) {
    other.addProduct(this->clone());
    this->quantity_.setValue(0);
    std::cout << "[Jacket] Куртка \"" << name_ << "\" перемещена на другой склад.\n";
}

Milk::Milk(int id, const std::string& name, const Price& price,
    const Quantity& quantity, const Weight& weight)
    : Food(id, name, price, quantity, weight) {
}

std::unique_ptr<Product> Milk::clone() const {
    return std::make_unique<Milk>(*this);
}

void Milk::transferTo(Warehouse& other) {
    other.addProduct(this->clone());
    this->quantity_.setValue(0);
    std::cout << "[Milk] Молоко \"" << name_ << "\" перемещено на другой склад.\n";
}

Bread::Bread(int id, const std::string& name, const Price& price,
    const Quantity& quantity, const Weight& weight)
    : Food(id, name, price, quantity, weight) {
}

std::unique_ptr<Product> Bread::clone() const {
    return std::make_unique<Bread>(*this);
}

void Bread::transferTo(Warehouse& other) {
    other.addProduct(this->clone());
    this->quantity_.setValue(0);
    std::cout << "[Bread] Хлеб \"" << name_ << "\" перемещён на другой склад.\n";
}