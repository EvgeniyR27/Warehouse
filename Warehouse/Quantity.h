#pragma once
#include <stdexcept>

//  ласс-обЄртка дл€ количества товара на складе
class Quantity {
private:
    int value_; //  оличество единиц товара

public:
    explicit Quantity(int val = 0);

    int getValue() const;
    void setValue(int newVal);

    // ћетод дл€ изменени€ остатка на величину delta (может быть положительной или отрицательной)
    void change(int delta);
};