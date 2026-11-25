#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Product.h"

// Собственный итератор для обхода товаров на складе (с возможностью изменения)
class WarehouseIterator {
private:
    std::vector<std::unique_ptr<Product>>::iterator current_; // Текущая позиция
    std::vector<std::unique_ptr<Product>>::iterator end_;     // Конец контейнера

public:
    WarehouseIterator(std::vector<std::unique_ptr<Product>>::iterator current,
        std::vector<std::unique_ptr<Product>>::iterator end);

    // Переход к следующему элементу
    WarehouseIterator& operator++();

    // Разыменование итератора (возвращает ссылку на сам товар, скрывая unique_ptr)
    Product& operator*() const;
    Product* operator->() const;

    // Операторы сравнения для работы в циклах
    bool operator!=(const WarehouseIterator& other) const;
    bool operator==(const WarehouseIterator& other) const;
};

// Константный итератор для обхода константного склада без изменения товаров
class ConstWarehouseIterator {
private:
    std::vector<std::unique_ptr<Product>>::const_iterator current_;
    std::vector<std::unique_ptr<Product>>::const_iterator end_;

public:
    ConstWarehouseIterator(std::vector<std::unique_ptr<Product>>::const_iterator current,
        std::vector<std::unique_ptr<Product>>::const_iterator end);

    ConstWarehouseIterator& operator++();
    const Product& operator*() const;
    const Product* operator->() const;
    bool operator!=(const ConstWarehouseIterator& other) const;
    bool operator==(const ConstWarehouseIterator& other) const;
};

// Класс склада — контейнер для хранения товаров
class Warehouse {
private:
    std::vector<std::unique_ptr<Product>> products_; // Вектор умных указателей на товары
    size_t maxSize_;                                 // Максимальная вместимость склада
    size_t currentSize_;                             // Текущее количество товаров

public:
    // Конструктор с заданием максимальной вместимости
    explicit Warehouse(size_t maxSize);

    // --- Правило пяти ---
    ~Warehouse() = default;                                 // 1. Деструктор
    Warehouse(const Warehouse& other);                      // 2. Конструктор копирования (глубокое)
    Warehouse& operator=(const Warehouse& other);           // 3. Оператор присваивания копированием
    Warehouse(Warehouse&& other) noexcept;                  // 4. Конструктор перемещения
    Warehouse& operator=(Warehouse&& other) noexcept;       // 5. Оператор присваивания перемещением

    // Добавление и удаление товаров
    void addProduct(std::unique_ptr<Product> product);
    void removeById(int id);
    void removeByIndex(size_t index);

    // Методы поиска товаров
    Product* findById(int id) const;
    std::vector<Product*> findByCategory(const std::string& category) const;
    std::vector<Product*> findByPrice(const Price& price) const;

    // Геттеры состояния склада
    size_t getMaxSize() const;
    size_t getCurrentSize() const;

    // Методы для получения итераторов (поддержка range-based for)
    WarehouseIterator begin();
    WarehouseIterator end();
    ConstWarehouseIterator begin() const;
    ConstWarehouseIterator end() const;
};