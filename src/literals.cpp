export module ut:literals;

import std;
import :math;
import :detail;
import :test;

namespace ut::literals
{

    export [[nodiscard]] auto operator""_test(const char *name, std::size_t size)
    {
        return detail::test {"test", std::string_view {name, size}};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_i()
    {
        return detail::integral_constant<math::num<int, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_s()
    {
        return detail::integral_constant<math::num<short, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_c()
    {
        return detail::integral_constant<math::num<char, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_sc()
    {
        return detail::integral_constant<math::num<signed char, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_l()
    {
        return detail::integral_constant<math::num<long, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_ll()
    {
        return detail::integral_constant<math::num<long long, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_u()
    {
        return detail::integral_constant<math::num<unsigned, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_uc()
    {
        return detail::integral_constant<math::num<unsigned char, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_us()
    {
        return detail::integral_constant<math::num<unsigned short, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_ul()
    {
        return detail::integral_constant<math::num<unsigned long, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_ull()
    {
        return detail::integral_constant<math::num<unsigned long long, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_i8()
    {
        return detail::integral_constant<math::num<std::int8_t, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_i16()
    {
        return detail::integral_constant<math::num<std::int16_t, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_i32()
    {
        return detail::integral_constant<math::num<std::int32_t, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_i64()
    {
        return detail::integral_constant<math::num<std::int64_t, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_u8()
    {
        return detail::integral_constant<math::num<std::uint8_t, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_u16()
    {
        return detail::integral_constant<math::num<std::uint16_t, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_u32()
    {
        return detail::integral_constant<math::num<std::uint32_t, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_u64()
    {
        return detail::integral_constant<math::num<std::uint64_t, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_f()
    {
        return detail::floating_point_constant<float,
                                               math::num<unsigned long, Cs...>(),
                                               math::den<unsigned long, Cs...>(),
                                               math::den_size<unsigned long, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_d()
    {
        return detail::floating_point_constant<double,
                                               math::num<unsigned long, Cs...>(),
                                               math::den<unsigned long, Cs...>(),
                                               math::den_size<unsigned long, Cs...>()> {};
    }

    export template <char... Cs> [[nodiscard]] constexpr auto operator""_ld()
    {
        return detail::floating_point_constant<long double,
                                               math::num<unsigned long long, Cs...>(),
                                               math::den<unsigned long long, Cs...>(),
                                               math::den_size<unsigned long long, Cs...>()> {};
    }

    export constexpr auto operator""_b(const char *name, decltype(sizeof("")) size)
    {
        struct named : std::string_view, detail::op
        {
                using value_type = bool;
                [[nodiscard]] constexpr operator value_type() const
                {
                    return true;
                }
                [[nodiscard]] constexpr auto operator==(const named &) const
                {
                    return true;
                }
                [[nodiscard]] constexpr auto operator==(const bool other) const
                {
                    return other;
                }
        };
        return named {{name, size}, {}};
    }

} // namespace ut::literals
