#include "Quantity.h"

Quantity::Quantity(int val) : value_(0) {
    setValue(val);
}

int Quantity::getValue() const {
    return value_;
}

void Quantity::setValue(int newVal) {
    if (newVal < 0) {
        throw std::invalid_argument("Ошибка: количество товара не может быть отрицательным!");
    }
    value_ = newVal;
}

void Quantity::change(int delta) {
    if (value_ + delta < 0) {
        throw std::invalid_argument("Ошибка: итоговый остаток на складе не может стать отрицательным!");
    }
    value_ += delta;
}