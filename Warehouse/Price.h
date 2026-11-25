#pragma once
#include <stdexcept>

// Класс-обёртка для цены товара с валидацией данных
class Price {
private:
    int value_; // Значение цены в рублях

public:
    // Конструктор с проверкой на неотрицательность
    explicit Price(int val = 0);

    // Геттер для получения текущей цены
    int getValue() const;

    // Сеттер для изменения цены с проверкой
    void setValue(int newVal);

    // Оператор сравнения (нужен для удобного поиска по цене на складе)
    bool operator==(const Price& other) const;
};