#include "Weight.h"

Weight::Weight(double val) : value_(0.0) {
    setValue(val);
}

double Weight::getValue() const {
    return value_;
}

void Weight::setValue(double newVal) {
    if (newVal < 0.0) {
        throw std::invalid_argument("Ошибка: вес товара не может быть отрицательным!");
    }
    value_ = newVal;
}