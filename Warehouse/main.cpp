#include <iostream>
#include <clocale>
#include "Price.h"
#include "Quantity.h"
#include "Weight.h"
#include "Product.h"
#include "ProductFactory.h"
#include "Warehouse.h"

// Вспомогательная функция для красивого вывода содержимого склада через константный итератор
void printWarehouse(const Warehouse& warehouse, const std::string& title) {
    std::cout << "=== " << title << " (Товаров: " << warehouse.getCurrentSize()
        << " из " << warehouse.getMaxSize() << ") ===\n";
    if (warehouse.getCurrentSize() == 0) {
        std::cout << "  [Склад пуст]\n";
    }
    for (const auto& product : warehouse) {
        std::cout << "  ID: " << product.getId()
            << " | Категория: " << product.getCategory()
            << " | Название: " << product.getName()
            << " | Цена: " << product.getPrice().getValue() << " руб."
            << " | Остаток: " << product.getQuantity().getValue() << " шт."
            << " | Вес: " << product.getWeight().getValue() << " кг\n";
    }
    std::cout << "\n";
}

int main() {
    std::setlocale(LC_ALL, "Russian");

    try {
        // =========================================================================
        // СЦЕНАРИЙ 1: Создание склада, добавление товаров через разные фабрики,
        //             вывод списка через итератор
        // =========================================================================
        std::cout << ">>> СЦЕНАРИЙ 1: Создание склада и наполнение через фабрики <<<\n";

        SmartphoneFactory smartphoneFactory;
        LaptopFactory laptopFactory;
        TShirtFactory tshirtFactory;
        JacketFactory jacketFactory;
        MilkFactory milkFactory;
        BreadFactory breadFactory;

        Warehouse mainWarehouse(10); // Создаём склад вместимостью 10 позиций

        // Передаём 0 в качестве ID, чтобы сработал статический счётчик nextId_ (либо можно задать явно)
        mainWarehouse.addProduct(smartphoneFactory.createProduct(0, "iPhone 15", 85000, 5, 0.17));
        mainWarehouse.addProduct(laptopFactory.createProduct(0, "MacBook Air", 120000, 3, 1.24));
        mainWarehouse.addProduct(tshirtFactory.createProduct(0, "Футболка базовая", 1500, 20, 0.2));
        mainWarehouse.addProduct(jacketFactory.createProduct(0, "Куртка зимняя", 9500, 7, 1.5));
        mainWarehouse.addProduct(milkFactory.createProduct(0, "Молоко 3.2%", 95, 50, 1.0));
        mainWarehouse.addProduct(breadFactory.createProduct(0, "Хлеб Бородинский", 60, 30, 0.4));

        printWarehouse(mainWarehouse, "Основной склад после добавления товаров");

        // =========================================================================
        // СЦЕНАРИЙ 2: Копирование склада, изменение товаров в копии,
        //             проверка независимости оригинального склада
        // =========================================================================
        std::cout << ">>> СЦЕНАРИЙ 2: Глубокое копирование склада <<<\n";

        Warehouse copiedWarehouse = mainWarehouse; // Вызывается конструктор копирования

        // Изменяем цену и количество первого товара в копии через обычный итератор
        for (auto& product : copiedWarehouse) {
            if (product.getId() == 1) {
                product.setPrice(Price(70000));
                product.changeQuantity(10);
            }
        }

        std::cout << "Проверка товара с ID = 1 после модификации копии:\n";
        std::cout << "  В оригинале -> Цена: " << mainWarehouse.findById(1)->getPrice().getValue()
            << " руб., Остаток: " << mainWarehouse.findById(1)->getQuantity().getValue() << " шт.\n";
        std::cout << "  В копии     -> Цена: " << copiedWarehouse.findById(1)->getPrice().getValue()
            << " руб., Остаток: " << copiedWarehouse.findById(1)->getQuantity().getValue() << " шт.\n\n";

        // =========================================================================
        // СЦЕНАРИЙ 3: Перемещение склада через конструктор перемещения,
        //             проверка отсутствия копирования
        // =========================================================================
        std::cout << ">>> СЦЕНАРИЙ 3: Перемещение склада (Move semantics) <<<\n";

        // Сохраняем адрес первого товара в памяти до перемещения
        Product* addressBeforeMove = copiedWarehouse.findById(1);

        Warehouse movedWarehouse(std::move(copiedWarehouse)); // Конструктор перемещения

        Product* addressAfterMove = movedWarehouse.findById(1);

        std::cout << "Размер старого склада после std::move: " << copiedWarehouse.getCurrentSize() << "\n";
        std::cout << "Размер нового склада после std::move:  " << movedWarehouse.getCurrentSize() << "\n";
        std::cout << "Адрес товара ID=1 до перемещения:    " << addressBeforeMove << "\n";
        std::cout << "Адрес товара ID=1 после перемещения: " << addressAfterMove
            << " (адреса совпадают — глубокого копирования не было)\n\n";

        // =========================================================================
        // СЦЕНАРИЙ 4: Поиск товаров по ID, категории и цене, удаление по ID и индексу
        // =========================================================================
        std::cout << ">>> СЦЕНАРИЙ 4: Поиск и удаление товаров <<<\n";

        // 1. Поиск по ID
        Product* foundById = mainWarehouse.findById(2);
        if (foundById) {
            std::cout << "Найден по ID=2: " << foundById->getName() << "\n";
        }

        // 2. Поиск по категории
        std::vector<Product*> foodProducts = mainWarehouse.findByCategory("food");
        std::cout << "Товары категории 'food' (" << foodProducts.size() << " шт.): ";
        for (size_t i = 0; i < foodProducts.size(); ++i) {
            std::cout << foodProducts[i]->getName() << (i + 1 < foodProducts.size() ? ", " : "\n");
        }

        // 3. Поиск по цене
        std::vector<Product*> priceProducts = mainWarehouse.findByPrice(Price(1500));
        std::cout << "Товары с ценой 1500 руб. (" << priceProducts.size() << " шт.): ";
        for (size_t i = 0; i < priceProducts.size(); ++i) {
            std::cout << priceProducts[i]->getName() << "\n";
        }

        // 4. Удаление по ID и по индексу
        std::cout << "Удаляем товар с ID=6 (Хлеб) и товар по индексу 0 (iPhone 15)...\n";
        mainWarehouse.removeById(6);
        mainWarehouse.removeByIndex(0);

        printWarehouse(mainWarehouse, "Основной склад после удаления");

        // =========================================================================
        // СЦЕНАРИЙ 5: Перемещение товара между складами через transferTo,
        //             изменение остатка и цены
        // =========================================================================
        std::cout << ">>> СЦЕНАРИЙ 5: Перемещение товара между складами (transferTo) <<<\n";

        Warehouse secondaryWarehouse(5); // Второй склад
        Product* laptop = mainWarehouse.findById(2);
        if (laptop) {
            // Меняем цену и остаток перед отправкой
            laptop->setPrice(Price(115000));
            laptop->changeQuantity(2);
            // Перемещаем на второй склад
            laptop->transferTo(secondaryWarehouse);
        }

        printWarehouse(mainWarehouse, "Основной склад (остаток ноутбуков обнулён)");
        printWarehouse(secondaryWarehouse, "Второй склад (получил ноутбуки)");

        // =========================================================================
        // ТЕСТИРОВАНИЕ: Проверка выброса исключений при некорректных данных
        // =========================================================================
        std::cout << ">>> ТЕСТИРОВАНИЕ ВАЛИДАЦИИ И ОГРАНИЧЕНИЙ <<<\n";

        try {
            Price invalidPrice(-500);
        }
        catch (const std::exception& e) {
            std::cout << "[Тест 1 пройден] Перехвачена ошибка цены: " << e.what() << "\n";
        }

        try {
            Warehouse tinyWarehouse(1);
            tinyWarehouse.addProduct(milkFactory.createProduct(0, "Молоко", 90, 10, 1.0));
            tinyWarehouse.addProduct(breadFactory.createProduct(0, "Хлеб", 50, 10, 0.5));
        }
        catch (const std::exception& e) {
            std::cout << "[Тест 2 пройден] Перехвачено переполнение склада: " << e.what() << "\n";
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Непредвиденная ошибка: " << e.what() << "\n";
    }

    return 0;
}