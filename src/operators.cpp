module;

#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>)
#define UT_HAS_POSIX_FORK 1
#endif

export module ut:operators;

import std;
import :type_traits;
import :reflection;
import :events;
import :detail;
import :cfg;
import :test;

namespace ut
{

    export [[nodiscard]] constexpr auto get_ordinal_suffix(int number)
    {
        const auto last_digit = number % 10;
        const auto last_two_digits = number % 100;
        if (last_digit == 1 && last_two_digits != 11)
        {
            return "st";
        }
        if (last_digit == 2 && last_two_digits != 12)
        {
            return "nd";
        }
        if (last_digit == 3 && last_two_digits != 13)
        {
            return "rd";
        }
        return "th";
    }

    export template <class TArg> std::string format_test_parameter([[maybe_unused]] const TArg &arg, const int counter)
    {
        return std::to_string(counter) + get_ordinal_suffix(counter) + " parameter";
    }

    export template <class F>
        requires(std::integral<F> || std::floating_point<F>) && (!std::same_as<F, bool>)
    std::string format_test_parameter(const F &arg, [[maybe_unused]] const int counter)
    {
        std::ostringstream oss;
        oss << arg;
        return oss.str();
    }

    export std::string format_test_parameter(const bool &arg, [[maybe_unused]] const int counter)
    {
        return arg ? "true" : "false";
    }

    namespace operators
    {

        export [[nodiscard]] constexpr auto operator==(std::string_view lhs, std::string_view rhs)
        {
            return detail::eq_ {lhs, rhs};
        }

        export [[nodiscard]] constexpr auto operator!=(std::string_view lhs, std::string_view rhs)
        {
            return detail::neq_ {lhs, rhs};
        }

        export template <std::ranges::range T> [[nodiscard]] constexpr auto operator==(T &&lhs, T &&rhs)
        {
            return detail::eq_ {static_cast<T &&>(lhs), static_cast<T &&>(rhs)};
        }

        export template <std::ranges::range T> [[nodiscard]] constexpr auto operator!=(T &&lhs, T &&rhs)
        {
            return detail::neq_ {static_cast<T &&>(lhs), static_cast<T &&>(rhs)};
        }

        export template <class TLhs, class TRhs>
            requires type_traits::is_op<TLhs> || type_traits::is_op<TRhs>
        [[nodiscard]] constexpr auto operator==(const TLhs &lhs, const TRhs &rhs)
        {
            return detail::eq_ {lhs, rhs};
        }

        export template <class TLhs, class TRhs>
            requires type_traits::is_op<TLhs> || type_traits::is_op<TRhs>
        [[nodiscard]] constexpr auto operator!=(const TLhs &lhs, const TRhs &rhs)
        {
            return detail::neq_ {lhs, rhs};
        }

        export template <class TLhs, class TRhs>
            requires type_traits::is_op<TLhs> || type_traits::is_op<TRhs>
        [[nodiscard]] constexpr auto operator>(const TLhs &lhs, const TRhs &rhs)
        {
            return detail::gt_ {lhs, rhs};
        }

        export template <class TLhs, class TRhs>
            requires type_traits::is_op<TLhs> || type_traits::is_op<TRhs>
        [[nodiscard]] constexpr auto operator>=(const TLhs &lhs, const TRhs &rhs)
        {
            return detail::ge_ {lhs, rhs};
        }

        export template <class TLhs, class TRhs>
            requires type_traits::is_op<TLhs> || type_traits::is_op<TRhs>
        [[nodiscard]] constexpr auto operator<(const TLhs &lhs, const TRhs &rhs)
        {
            return detail::lt_ {lhs, rhs};
        }

        export template <class TLhs, class TRhs>
            requires type_traits::is_op<TLhs> || type_traits::is_op<TRhs>
        [[nodiscard]] constexpr auto operator<=(const TLhs &lhs, const TRhs &rhs)
        {
            return detail::le_ {lhs, rhs};
        }

        export template <class TLhs, class TRhs>
            requires type_traits::is_op<TLhs> || type_traits::is_op<TRhs>
        [[nodiscard]] constexpr auto operator and(const TLhs &lhs, const TRhs &rhs)
        {
            return detail::and_ {lhs, rhs};
        }

        export template <class TLhs, class TRhs>
            requires type_traits::is_op<TLhs> || type_traits::is_op<TRhs>
        [[nodiscard]] constexpr auto operator or(const TLhs &lhs, const TRhs &rhs)
        {
            return detail::or_ {lhs, rhs};
        }

        export template <class T>
            requires type_traits::is_op<T>
        [[nodiscard]] constexpr auto operator not(const T &t)
        {
            return detail::not_ {t};
        }

        export template <class T>
        [[nodiscard]] auto operator>>(const T &t, const detail::value_location<detail::fatal> &)
        {
            return detail::fatal_ {t};
        }

        export template <class Test> [[nodiscard]] auto operator/(const detail::tag &tag, Test test)
        {
            for (const auto &name : tag.name)
            {
                test.tag.push_back(name);
            }
            return test;
        }

        export [[nodiscard]] auto operator/(const detail::tag &lhs, const detail::tag &rhs)
        {
            std::vector<std::string_view> tag;
            tag.reserve(lhs.name.size() + rhs.name.size());
            for (const auto &name : lhs.name)
            {
                tag.push_back(name);
            }
            for (const auto &name : rhs.name)
            {
                tag.push_back(name);
            }
            return detail::tag {tag};
        }

        export template <class F, class T>
            requires std::ranges::range<T>
        [[nodiscard]] constexpr auto operator|(const F &f, const T &t)
        {
            return [f, t](std::string_view type, std::string_view name) {
                for (int counter = 1; const auto &arg : t)
                {
                    detail::on<F>(events::test<F, decltype(arg)> {.type = type,
                                                                  .name = std::string {name} + " (" +
                                                                          format_test_parameter(arg, counter) + ")",
                                                                  .tag = {},
                                                                  .location = {},
                                                                  .arg = arg,
                                                                  .run = f});
                    ++counter;
                }
            };
        }

        export template <class F, template <class...> class T, class... Ts>
            requires(!std::ranges::range<T<Ts...>>)
        [[nodiscard]] constexpr auto operator|(const F &f, const T<Ts...> &t)
        {
            constexpr auto unique_name = []<class TArg>(std::string_view name, const TArg &arg, int &counter) {
                auto ret = std::string {name} + " (";
                if (std::invocable<F, TArg>)
                {
                    ret += format_test_parameter(arg, counter) + ", ";
                }
                ret += std::string(reflection::type_name<TArg>()) + ")";
                ++counter;
                return ret;
            };

            return [f, t, unique_name](std::string_view type, std::string_view name) {
                int counter = 1;
                apply(
                    [=, &counter](const auto &...args) {
                        (detail::on<F>(
                             events::test<F, Ts> {.type = type,
                                                  .name = unique_name.template operator()<Ts>(name, args, counter),
                                                  .tag = {},
                                                  .location = {},
                                                  .arg = args,
                                                  .run = f}),
                         ...);
                    },
                    t);
            };
        }

        namespace terse
        {
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-comparison"
#endif

            export [[maybe_unused]] constexpr struct placeholder_gcc_t
            {
            } _t;

            export template <class T> constexpr auto operator%(const T &t, const placeholder_gcc_t &)
            {
                return detail::value<T> {t};
            }

            export template <class T> auto operator>>(const T &t, const detail::value_location<detail::fatal> &)
            {
                using fatal_t = detail::fatal_<T>;
                struct fatal_ : fatal_t, detail::log
                {
                        using type = fatal_t;
                        using fatal_t::fatal_t;
                        const detail::terse_<type> _ {*this};
                };
                return fatal_ {t};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator==(const T &lhs, const detail::value_location<typename T::value_type> &rhs)
            {
                using eq_t = detail::eq_<T, detail::value_location<typename T::value_type>>;
                struct eq_ : eq_t, detail::log
                {
                        using type = eq_t;
                        using eq_t::eq_t;
                        const detail::terse_<type> _ {*this};
                };
                return eq_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator==(const detail::value_location<typename T::value_type> &lhs, const T &rhs)
            {
                using eq_t = detail::eq_<detail::value_location<typename T::value_type>, T>;
                struct eq_ : eq_t, detail::log
                {
                        using type = eq_t;
                        using eq_t::eq_t;
                        const detail::terse_<type> _ {*this};
                };
                return eq_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator!=(const T &lhs, const detail::value_location<typename T::value_type> &rhs)
            {
                using neq_t = detail::neq_<T, detail::value_location<typename T::value_type>>;
                struct neq_ : neq_t, detail::log
                {
                        using type = neq_t;
                        using neq_t::neq_t;
                        const detail::terse_<type> _ {*this};
                };
                return neq_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator!=(const detail::value_location<typename T::value_type> &lhs, const T &rhs)
            {
                using neq_t = detail::neq_<detail::value_location<typename T::value_type>, T>;
                struct neq_ : neq_t
                {
                        using type = neq_t;
                        using neq_t::neq_t;
                        const detail::terse_<type> _ {*this};
                };
                return neq_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator>(const T &lhs, const detail::value_location<typename T::value_type> &rhs)
            {
                using gt_t = detail::gt_<T, detail::value_location<typename T::value_type>>;
                struct gt_ : gt_t, detail::log
                {
                        using type = gt_t;
                        using gt_t::gt_t;
                        const detail::terse_<type> _ {*this};
                };
                return gt_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator>(const detail::value_location<typename T::value_type> &lhs, const T &rhs)
            {
                using gt_t = detail::gt_<detail::value_location<typename T::value_type>, T>;
                struct gt_ : gt_t, detail::log
                {
                        using type = gt_t;
                        using gt_t::gt_t;
                        const detail::terse_<type> _ {*this};
                };
                return gt_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator>=(const T &lhs, const detail::value_location<typename T::value_type> &rhs)
            {
                using ge_t = detail::ge_<T, detail::value_location<typename T::value_type>>;
                struct ge_ : ge_t, detail::log
                {
                        using type = ge_t;
                        using ge_t::ge_t;
                        const detail::terse_<type> _ {*this};
                };
                return ge_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator>=(const detail::value_location<typename T::value_type> &lhs, const T &rhs)
            {
                using ge_t = detail::ge_<detail::value_location<typename T::value_type>, T>;
                struct ge_ : ge_t, detail::log
                {
                        using type = ge_t;
                        using ge_t::ge_t;
                        const detail::terse_<type> _ {*this};
                };
                return ge_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator<(const T &lhs, const detail::value_location<typename T::value_type> &rhs)
            {
                using lt_t = detail::lt_<T, detail::value_location<typename T::value_type>>;
                struct lt_ : lt_t, detail::log
                {
                        using type = lt_t;
                        using lt_t::lt_t;
                        const detail::terse_<type> _ {*this};
                };
                return lt_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator<(const detail::value_location<typename T::value_type> &lhs, const T &rhs)
            {
                using lt_t = detail::lt_<detail::value_location<typename T::value_type>, T>;
                struct lt_ : lt_t, detail::log
                {
                        using type = lt_t;
                        using lt_t::lt_t;
                        const detail::terse_<type> _ {*this};
                };
                return lt_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator<=(const T &lhs, const detail::value_location<typename T::value_type> &rhs)
            {
                using le_t = detail::le_<T, detail::value_location<typename T::value_type>>;
                struct le_ : le_t, detail::log
                {
                        using type = le_t;
                        using le_t::le_t;
                        const detail::terse_<type> _ {*this};
                };
                return le_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator<=(const detail::value_location<typename T::value_type> &lhs, const T &rhs)
            {
                using le_t = detail::le_<detail::value_location<typename T::value_type>, T>;
                struct le_ : le_t
                {
                        using type = le_t;
                        using le_t::le_t;
                        const detail::terse_<type> _ {*this};
                };
                return le_ {lhs, rhs};
            }

            export template <class TLhs, class TRhs>
                requires type_traits::is_op<TLhs> || type_traits::is_op<TRhs>
            constexpr auto operator and(const TLhs &lhs, const TRhs &rhs)
            {
                using and_t = detail::and_<typename TLhs::type, typename TRhs::type>;
                struct and_ : and_t, detail::log
                {
                        using type = and_t;
                        using and_t::and_t;
                        const detail::terse_<type> _ {*this};
                };
                return and_ {lhs, rhs};
            }

            export template <class TLhs, class TRhs>
                requires type_traits::is_op<TLhs> || type_traits::is_op<TRhs>
            constexpr auto operator or(const TLhs &lhs, const TRhs &rhs)
            {
                using or_t = detail::or_<typename TLhs::type, typename TRhs::type>;
                struct or_ : or_t, detail::log
                {
                        using type = or_t;
                        using or_t::or_t;
                        const detail::terse_<type> _ {*this};
                };
                return or_ {lhs, rhs};
            }

            export template <class T>
                requires type_traits::is_op<T>
            constexpr auto operator not(const T &t)
            {
                using not_t = detail::not_<typename T::type>;
                struct not_ : not_t, detail::log
                {
                        using type = not_t;
                        using not_t::not_t;
                        const detail::terse_<type> _ {*this};
                };
                return not_ {t};
            }

        } // namespace terse
    } // namespace operators

    // ---- top-level ut entities ----

    export template <class TExpr>
        requires type_traits::is_op<TExpr> || concepts::explicitly_convertible_to<TExpr, bool>
    constexpr auto expect(const TExpr &expr,
                          const reflection::source_location &sl = reflection::source_location::current())
    {
        return detail::expect_<TExpr> {detail::on<TExpr>(events::assertion<TExpr> {.expr = expr, .location = sl})};
    }

    export [[maybe_unused]] constexpr auto fatal = detail::fatal {};

    export template <auto Constant> constexpr auto constant = Constant;

    export template <class TException, class TExpr> [[nodiscard]] constexpr auto throws(const TExpr &expr)
    {
        return detail::throws_<TExpr, TException> {expr};
    }

    export template <class TExpr> [[nodiscard]] constexpr auto throws(const TExpr &expr)
    {
        return detail::throws_<TExpr> {expr};
    }

    export template <class TExpr> [[nodiscard]] constexpr auto nothrow(const TExpr &expr)
    {
        return detail::nothrow_ {expr};
    }

#if defined(UT_HAS_POSIX_FORK)
    export template <class TExpr> [[nodiscard]] constexpr auto aborts(const TExpr &expr)
    {
        return detail::aborts_ {expr};
    }
#endif

    export using _b = detail::value<bool>;
    export using _c = detail::value<char>;
    export using _sc = detail::value<signed char>;
    export using _s = detail::value<short>;
    export using _i = detail::value<int>;
    export using _l = detail::value<long>;
    export using _ll = detail::value<long long>;
    export using _u = detail::value<unsigned>;
    export using _uc = detail::value<unsigned char>;
    export using _us = detail::value<unsigned short>;
    export using _ul = detail::value<unsigned long>;
    export using _ull = detail::value<unsigned long long>;
    export using _i8 = detail::value<std::int8_t>;
    export using _i16 = detail::value<std::int16_t>;
    export using _i32 = detail::value<std::int32_t>;
    export using _i64 = detail::value<std::int64_t>;
    export using _u8 = detail::value<std::uint8_t>;
    export using _u16 = detail::value<std::uint16_t>;
    export using _u32 = detail::value<std::uint32_t>;
    export using _u64 = detail::value<std::uint64_t>;
    export using _f = detail::value<float>;
    export using _d = detail::value<double>;
    export using _ld = detail::value<long double>;

    export template <class T> struct _t : detail::value<T>
    {
            constexpr explicit _t(const T &t) : detail::value<T> {t}
            {
            }
    };

    export template <fixed_string suite_name = "unnamed suite"> struct suite
    {
            reflection::source_location location {};
            std::string_view name = std::string_view(suite_name);
            template <class TSuite> constexpr /*explicit(false)*/ suite(TSuite _suite)
            {
                static_assert(1 == sizeof(_suite));
                detail::on<decltype(+_suite)>(events::suite<decltype(+_suite)> {.run = +_suite, .name = name});
            }
    };

    export [[maybe_unused]] auto log = detail::log {};
    export [[maybe_unused]] auto that = detail::that_ {};
    export [[maybe_unused]] constexpr auto test = [](auto &&name) {
        return detail::test {"test", std::forward<decltype(name)>(name)};
    };
    export [[maybe_unused]] constexpr auto should = test;
    export [[maybe_unused]] auto tag = [](const auto &name) { return detail::tag {{name}}; };
    export [[maybe_unused]] auto skip = tag("skip");

    export template <class T = void> [[maybe_unused]] constexpr auto type = detail::type_<T>();

    export template <class TLhs, class TRhs>
        requires type_traits::is_stream_insertable_v<TLhs> && type_traits::is_stream_insertable_v<TRhs>
    [[nodiscard]] constexpr auto eq(const TLhs &lhs, const TRhs &rhs)
    {
        return detail::eq_ {lhs, rhs};
    }

    export template <class TLhs, class TRhs, class TEpsilon>
        requires type_traits::is_stream_insertable_v<TLhs> && type_traits::is_stream_insertable_v<TRhs>
    [[nodiscard]] constexpr auto approx(const TLhs &lhs, const TRhs &rhs, const TEpsilon &epsilon)
    {
        return detail::approx_ {lhs, rhs, epsilon};
    }

    export template <class TLhs, class TRhs>
        requires type_traits::is_stream_insertable_v<TLhs> && type_traits::is_stream_insertable_v<TRhs>
    [[nodiscard]] constexpr auto neq(const TLhs &lhs, const TRhs &rhs)
    {
        return detail::neq_ {lhs, rhs};
    }

    export template <class TLhs, class TRhs>
        requires type_traits::is_stream_insertable_v<TLhs> && type_traits::is_stream_insertable_v<TRhs>
    [[nodiscard]] constexpr auto gt(const TLhs &lhs, const TRhs &rhs)
    {
        return detail::gt_ {lhs, rhs};
    }

    export template <class TLhs, class TRhs>
        requires type_traits::is_stream_insertable_v<TLhs> && type_traits::is_stream_insertable_v<TRhs>
    [[nodiscard]] constexpr auto ge(const TLhs &lhs, const TRhs &rhs)
    {
        return detail::ge_ {lhs, rhs};
    }

    export template <class TLhs, class TRhs>
        requires type_traits::is_stream_insertable_v<TLhs> && type_traits::is_stream_insertable_v<TRhs>
    [[nodiscard]] constexpr auto lt(const TLhs &lhs, const TRhs &rhs)
    {
        return detail::lt_ {lhs, rhs};
    }

    export template <class TLhs, class TRhs>
        requires type_traits::is_stream_insertable_v<TLhs> && type_traits::is_stream_insertable_v<TRhs>
    [[nodiscard]] constexpr auto le(const TLhs &lhs, const TRhs &rhs)
    {
        return detail::le_ {lhs, rhs};
    }

    export template <class T> [[nodiscard]] constexpr auto mut(const T &t) noexcept -> T &
    {
        return const_cast<T &>(t);
    }

} // namespace ut
