export module ut:math;

import std;

namespace ut::math
{

    export template <class T> [[nodiscard]] constexpr auto abs(const T t) -> T
    {
        return t < T {} ? -t : t;
    }

    export template <class T, class U>
    [[nodiscard]] constexpr auto abs_diff(const T t, const U u) -> decltype(t < u ? u - t : t - u)
    {
        return t < u ? u - t : t - u;
    }

    export template <class T> [[nodiscard]] constexpr auto min_value(const T &lhs, const T &rhs) -> const T &
    {
        return (rhs < lhs) ? rhs : lhs;
    }

    export template <class T, class TExp> [[nodiscard]] constexpr auto pow(const T base, const TExp exp) -> T
    {
        return exp ? T(base * pow(base, exp - TExp(1))) : T(1);
    }

    export template <class T, char... Cs> [[nodiscard]] constexpr auto num() -> T
    {
        static_assert(((Cs == '.' or Cs == '\'' or (Cs >= '0' and Cs <= '9')) and ...));
        T result {};
        for (const char c : std::array {Cs...})
        {
            if (c == '.')
            {
                break;
            }
            if (c >= '0' and c <= '9')
            {
                result = result * T(10) + T(c - '0');
            }
        }
        return result;
    }

    export template <class T, char... Cs> [[nodiscard]] constexpr auto den() -> T
    {
        constexpr std::array cs {Cs...};
        T result {};
        auto i = 0u;
        while (cs[i++] != '.')
        {
        }

        for (auto j = i; j < sizeof...(Cs); ++j)
        {
            result += pow(T(10), sizeof...(Cs) - j) * T(cs[j] - '0');
        }
        return result;
    }

    export template <class T, char... Cs> [[nodiscard]] constexpr auto den_size() -> T
    {
        constexpr std::array cs {Cs...};
        T i {};
        while (cs[i++] != '.')
        {
        }

        return T(sizeof...(Cs)) - i + T(1);
    }

    export template <class T, class TValue> [[nodiscard]] constexpr auto den_size(TValue value) -> T
    {
        constexpr auto precision = TValue(1e-7);
        T result {};
        TValue tmp {};
        do
        {
            value *= 10;
            tmp = value - T(value);
            ++result;
        } while (tmp > precision);

        return result;
    }

} // namespace ut::math
