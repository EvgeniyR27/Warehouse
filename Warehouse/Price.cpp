#include "Price.h"

Price::Price(int val) : value_(0) {
    setValue(val); // Используем сеттер, чтобы не дублировать логику проверки
}

int Price::getValue() const {
    return value_;
}

void Price::setValue(int newVal) {
    if (newVal < 0) {
        throw std::invalid_argument("Ошибка: цена не может быть отрицательной!");
    }
    value_ = newVal;
}

bool Price::operator==(const Price& other) const {
    return value_ == other.value_;
}