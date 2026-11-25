#pragma once
#include <memory>
#include <string>
#include "Product.h"

// Абстрактная фабрика для создания товаров (паттерн Фабричный метод)
class ProductFactory {
public:
    virtual ~ProductFactory() = default;

    // Чисто виртуальный фабричный метод создания товара
    virtual std::unique_ptr<Product> createProduct(
        int id, const std::string& name, int price, int quantity, double weight) = 0;
};

// Конкретные фабрики для каждого вида товара

class SmartphoneFactory : public ProductFactory {
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name, int price, int quantity, double weight) override;
};

class LaptopFactory : public ProductFactory {
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name, int price, int quantity, double weight) override;
};

class TShirtFactory : public ProductFactory {
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name, int price, int quantity, double weight) override;
};

class JacketFactory : public ProductFactory {
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name, int price, int quantity, double weight) override;
};

class MilkFactory : public ProductFactory {
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name, int price, int quantity, double weight) override;
};

class BreadFactory : public ProductFactory {
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name, int price, int quantity, double weight) override;
};