#pragma once
#include <cmath>
#include <cstdint>
#include <string>

// Подключаем ваши готовые заголовки из 1-й лабораторной работы
#include "Price.h"
#include "Quantity.h"
#include "Weight.h"
#include "Product.h"

// Вспомогательные функции приведения типов, чтобы во всей 2-й работе
// было единое правило округления до копеек и не путались рубли с копейками.
// Если в вашей Лабе 1 Price хранит double (рубли), этот код работает сразу.
// Если ваш Price хранит int64_t (копейки), просто раскомментируйте варианты для копеек.
namespace Lab1Adapter {

    inline Price makePriceFromRubles(double rubles) {
        if (rubles < 0.0) {
            rubles = 0.0;
        }
        // Округляем до целых копеек (2 знака после запятой)
        double roundedRubles = std::round(rubles * 100.0) / 100.0;
        return Price(roundedRubles);
        // Если ваш Price хранит копейки (int64_t):
        // return Price(static_cast<std::int64_t>(std::llround(rubles * 100.0)));
    }

    inline double priceToRubles(const Price& price) {
        return price.getValue(); // или price.getAmount(), если метод называется так
        // Если ваш Price хранит копейки (int64_t):
        // return static_cast<double>(price.getValue()) / 100.0;
    }

    inline std::int64_t priceToKopecks(const Price& price) {
        return static_cast<std::int64_t>(std::llround(priceToRubles(price) * 100.0));
    }

    inline Price kopecksToPrice(std::int64_t kopecks) {
        return makePriceFromRubles(static_cast<double>(kopecks) / 100.0);
    }

    inline int quantityToInt(const Quantity& qty) {
        return qty.getValue();
    }

    inline double weightToKg(const Weight& weight) {
        return weight.getValue();
    }

    inline Weight makeWeightFromKg(double kg) {
        return Weight(kg < 0.0 ? 0.0 : kg);
    }

} // namespace Lab1Adapter