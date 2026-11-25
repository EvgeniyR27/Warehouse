#pragma once
#include <stdexcept>

// Класс-обёртка для веса товара
class Weight {
private:
    double value_; // Вес в килограммах

public:
    explicit Weight(double val = 0.0);

    double getValue() const;
    void setValue(double newVal);
};