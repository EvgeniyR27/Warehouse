#pragma once
#include <string>
#include <memory>
#include "Price.h"
#include "Quantity.h"
#include "Weight.h"

// Предварительное объявление класса склада во избежание циклического включения заголовков
class Warehouse;

// Абстрактный базовый класс для всех товаров
class Product {
protected:
    static int nextId_;     // Статический счётчик для генерации уникальных ID
    int id_;                // Уникальный идентификатор товара
    std::string name_;      // Название товара
    Price price_;           // Цена товара
    Quantity quantity_;     // Остаток на складе
    Weight weight_;         // Вес единицы товара
    std::string category_;  // Строковое название категории

public:
    // Конструктор (если передан id <= 0, генерируется автоматически через nextId_)
    Product(int id, const std::string& name, const Price& price,
        const Quantity& quantity, const Weight& weight, const std::string& category);

    // Виртуальный деструктор обязателен для базового полиморфного класса
    virtual ~Product() = default;

    // Чисто виртуальный метод клонирования (паттерн Прототип для глубокого копирования склада)
    virtual std::unique_ptr<Product> clone() const = 0;

    // Виртуальный метод перемещения товара на другой склад
    virtual void transferTo(Warehouse& other) = 0;

    // Геттеры
    int getId() const;
    const std::string& getName() const;
    const Price& getPrice() const;
    const Quantity& getQuantity() const;
    const Weight& getWeight() const;
    const std::string& getCategory() const;

    // Методы модификации состояния товара
    void setPrice(const Price& newPrice);
    void changeQuantity(int delta);
};

// --- Промежуточные классы категорий ---

class Electronics : public Product {
public:
    Electronics(int id, const std::string& name, const Price& price,
        const Quantity& quantity, const Weight& weight);
};

class Clothing : public Product {
public:
    Clothing(int id, const std::string& name, const Price& price,
        const Quantity& quantity, const Weight& weight);
};

class Food : public Product {
public:
    Food(int id, const std::string& name, const Price& price,
        const Quantity& quantity, const Weight& weight);
};

// --- Конкретные классы товаров (3 категории x 2 реализации) ---

class Smartphone : public Electronics {
public:
    Smartphone(int id, const std::string& name, const Price& price,
        const Quantity& quantity, const Weight& weight);
    std::unique_ptr<Product> clone() const override;
    void transferTo(Warehouse& other) override;
};

class Laptop : public Electronics {
public:
    Laptop(int id, const std::string& name, const Price& price,
        const Quantity& quantity, const Weight& weight);
    std::unique_ptr<Product> clone() const override;
    void transferTo(Warehouse& other) override;
};

class TShirt : public Clothing {
public:
    TShirt(int id, const std::string& name, const Price& price,
        const Quantity& quantity, const Weight& weight);
    std::unique_ptr<Product> clone() const override;
    void transferTo(Warehouse& other) override;
};

class Jacket : public Clothing {
public:
    Jacket(int id, const std::string& name, const Price& price,
        const Quantity& quantity, const Weight& weight);
    std::unique_ptr<Product> clone() const override;
    void transferTo(Warehouse& other) override;
};

class Milk : public Food {
public:
    Milk(int id, const std::string& name, const Price& price,
        const Quantity& quantity, const Weight& weight);
    std::unique_ptr<Product> clone() const override;
    void transferTo(Warehouse& other) override;
};

class Bread : public Food {
public:
    Bread(int id, const std::string& name, const Price& price,
        const Quantity& quantity, const Weight& weight);
    std::unique_ptr<Product> clone() const override;
    void transferTo(Warehouse& other) override;
};