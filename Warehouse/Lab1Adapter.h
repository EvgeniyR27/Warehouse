#pragma once
#include <cmath>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

// Подключаем ваши готовые заголовки из 1-й лабораторной работы
#include "Price.h"
#include "Quantity.h"
#include "Weight.h"
#include "Product.h"

namespace Lab1Adapter {

    // Определяем базовый тип внутри Price (int / int64_t копеек или double рублей)
    using PriceRawType = std::decay_t<decltype(std::declval<Price>().getValue())>;

    // Универсальное приведение ID товара из Лабы 1 (int, size_t или std::string) к std::string
    template <typename T>
    inline std::string productIdToString(const T& id) {
        if constexpr (std::is_convertible_v<T, std::string>) {
            return std::string(id);
        }
        else {
            return std::to_string(id);
        }
    }

    inline Price makePriceFromRubles(double rubles) {
        if (rubles < 0.0) {
            rubles = 0.0;
        }
        if constexpr (std::is_integral_v<PriceRawType>) {
            // Если Price в Лабе 1 хранит копейки в int/int64_t (500 руб. = 50000 коп.):
            auto kopecks = static_cast<PriceRawType>(std::llround(rubles * 100.0));
            return Price(kopecks);
        }
        else {
            double roundedRubles = std::round(rubles * 100.0) / 100.0;
            return Price(static_cast<PriceRawType>(roundedRubles));
        }
    }

    inline double priceToRubles(const Price& price) {
        if constexpr (std::is_integral_v<PriceRawType>) {
            return static_cast<double>(price.getValue()) / 100.0;
        }
        else {
            return static_cast<double>(price.getValue());
        }
    }

    inline std::int64_t priceToKopecks(const Price& price) {
        if constexpr (std::is_integral_v<PriceRawType>) {
            return static_cast<std::int64_t>(price.getValue());
        }
        else {
            return static_cast<std::int64_t>(std::llround(price.getValue() * 100.0));
        }
    }

    inline Price kopecksToPrice(std::int64_t kopecks) {
        if constexpr (std::is_integral_v<PriceRawType>) {
            return Price(static_cast<PriceRawType>(kopecks));
        }
        else {
            return makePriceFromRubles(static_cast<double>(kopecks) / 100.0);
        }
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

    namespace detail {
        template <typename T, typename = void>
        struct has_set_price_object : std::false_type {};

        template <typename T>
        struct has_set_price_object<T, std::void_t<decltype(std::declval<T&>().setPrice(std::declval<Price>()))>>
            : std::true_type {
        };

        template <typename T, typename = void>
        struct has_set_price_int : std::false_type {};

        template <typename T>
        struct has_set_price_int<T, std::void_t<decltype(std::declval<T&>().setPrice(std::declval<int>()))>>
            : std::true_type {
        };
    }

    // Безопасно обновляет цену товара в каталоге, если в Product есть метод setPrice
    template <typename TProduct>
    inline bool tryUpdateProductPrice(TProduct& product, const Price& newPrice) {
        if constexpr (detail::has_set_price_object<TProduct>::value) {
            product.setPrice(newPrice);
            return true;
        }
        else if constexpr (detail::has_set_price_int<TProduct>::value) {
            product.setPrice(static_cast<int>(priceToKopecks(newPrice)));
            return true;
        }
        else {
            (void)product;
            (void)newPrice;
            return false;
        }
    }

} // namespace Lab1Adapter