module;
#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>)
#include <sys/wait.h>
#include <unistd.h>
#define UT_HAS_POSIX_FORK 1
#endif

export module ut:detail;

import std;
import :reflection;
import :type_traits;
import :math;
import :cfg;

namespace ut::detail
{

    export struct op
    {
    };

    export template <class> struct fatal_;

    export struct fatal
    {
            template <class T>
            [[nodiscard]] auto operator()(
                const T &t, const reflection::source_location &sl = reflection::source_location::current()) const
            {
                return detail::fatal_ {t, sl};
            }
    };

    template <class T> [[nodiscard]] constexpr auto get_impl(const T &t, int) -> decltype(t.get())
    {
        return t.get();
    }

    template <class T> [[nodiscard]] constexpr auto get_impl(const T &t, ...) -> decltype(auto)
    {
        return t;
    }

    export template <class T> [[nodiscard]] constexpr auto get(const T &t)
    {
        return get_impl(t, 0);
    }

    export template <class T> struct type_ : op
    {
            template <class TOther> [[nodiscard]] constexpr auto operator()(const TOther &) const -> const type_<TOther>
            {
                return {};
            }
            [[nodiscard]] constexpr auto operator==(type_<T>) -> bool
            {
                return true;
            }
            template <class TOther> [[nodiscard]] constexpr auto operator==(type_<TOther>) -> bool
            {
                return false;
            }
            template <class TOther> [[nodiscard]] constexpr auto operator==(const TOther &) -> bool
            {
                return std::is_same_v<TOther, T>;
            }
            [[nodiscard]] constexpr auto operator!=(type_<T>) -> bool
            {
                return false;
            }
            template <class TOther> [[nodiscard]] constexpr auto operator!=(type_<TOther>) -> bool
            {
                return true;
            }
            template <class TOther> [[nodiscard]] constexpr auto operator!=(const TOther &) -> bool
            {
                return not std::is_same_v<TOther, T>;
            }
    };

    export template <class T, class = int> struct value : op
    {
            using value_type = T;

            constexpr /*explicit(false)*/ value(const T &_value) : value_ {_value}
            {
            }
            [[nodiscard]] constexpr explicit operator T() const
            {
                return value_;
            }
            [[nodiscard]] constexpr decltype(auto) get() const
            {
                return value_;
            }

            T value_ {};
    };

    template <std::floating_point T> struct value<T> : op
    {
            using value_type = T;
            static T epsilon;

            constexpr value(const T &_value, const T precision) : value_ {_value}
            {
                epsilon = precision;
            }

            constexpr /*explicit(false)*/ value(const T &val)
                : value {val, T(1) / math::pow(T(10), math::den_size<unsigned long long>(val))}
            {
            }
            [[nodiscard]] constexpr explicit operator T() const
            {
                return value_;
            }
            [[nodiscard]] constexpr decltype(auto) get() const
            {
                return value_;
            }

            T value_ {};
    };

    template <std::floating_point T> T value<T>::epsilon = T {};

    export template <class T> class value_location : public detail::value<T>
    {
        public:
            constexpr /*explicit(false)*/ value_location(
                const T &t, const reflection::source_location &sl = reflection::source_location::current())
                : detail::value<T> {t}
            {
                cfg::location = sl;
            }

            constexpr value_location(const T &t,
                                     const T precision,
                                     const reflection::source_location &sl = reflection::source_location::current())
                : detail::value<T> {t, precision}
            {
                cfg::location = sl;
            }
    };

    export template <auto N> struct integral_constant : op
    {
            using value_type = decltype(N);
            static constexpr auto value = N;

            [[nodiscard]] constexpr auto operator-() const
            {
                return integral_constant<-N> {};
            }
            [[nodiscard]] constexpr explicit operator value_type() const
            {
                return N;
            }
            [[nodiscard]] constexpr auto get() const
            {
                return N;
            }
    };

    export template <class T, auto N, auto D, auto Size, auto P = 1> struct floating_point_constant : op
    {
            using value_type = T;

            static constexpr auto epsilon = T(1) / math::pow(T(10), Size - 1);
            static constexpr auto value = T(P) * (T(N) + (T(D) / math::pow(T(10), Size)));

            [[nodiscard]] constexpr auto operator-() const
            {
                return floating_point_constant<T, N, D, Size, -1> {};
            }
            [[nodiscard]] constexpr explicit operator value_type() const
            {
                return value;
            }
            [[nodiscard]] constexpr auto get() const
            {
                return value;
            }
    };

    export template <class TLhs, class TRhs> struct eq_ : op
    {
            constexpr eq_(const TLhs &lhs = {}, const TRhs &rhs = {})
                : lhs_ {lhs}, rhs_ {rhs}, value_ {[&] {
                      using std::operator==;
                      using std::operator<;

                      if constexpr (type_traits::has_static_member_object_value_v<TLhs> and
                                    type_traits::has_static_member_object_value_v<TRhs>)
                      {
                          return lhs.value == rhs.value;
                      }
                      else if constexpr (type_traits::has_static_member_object_epsilon_v<TLhs> and
                                         type_traits::has_static_member_object_epsilon_v<TRhs>)
                      {
                          return math::abs(get(lhs) - get(rhs)) < math::min_value(TLhs::epsilon, TRhs::epsilon);
                      }
                      else if constexpr (type_traits::has_static_member_object_epsilon_v<TLhs>)
                      {
                          return math::abs(get(lhs) - get(rhs)) < TLhs::epsilon;
                      }
                      else if constexpr (type_traits::has_static_member_object_epsilon_v<TRhs>)
                      {
                          return math::abs(get(lhs) - get(rhs)) < TRhs::epsilon;
                      }
                      else
                      {
                          return get(lhs) == get(rhs);
                      }
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }
            [[nodiscard]] constexpr auto lhs() const
            {
                return get(lhs_);
            }
            [[nodiscard]] constexpr auto rhs() const
            {
                return get(rhs_);
            }

            const TLhs lhs_ {};
            const TRhs rhs_ {};
            const bool value_ {};
    };

    export template <class TLhs, class TRhs, class TEpsilon> struct approx_ : op
    {
            constexpr approx_(const TLhs &lhs = {}, const TRhs &rhs = {}, const TEpsilon &epsilon = {})
                : lhs_ {lhs}, rhs_ {rhs}, epsilon_ {epsilon}, value_ {[&] {
                      using std::operator<;

                      if constexpr (type_traits::has_static_member_object_value_v<TLhs> and
                                    type_traits::has_static_member_object_value_v<TRhs> and
                                    type_traits::has_static_member_object_value_v<TEpsilon>)
                      {
                          return math::abs_diff(TLhs::value, TRhs::value) < TEpsilon::value;
                      }
                      else
                      {
                          return math::abs_diff(get(lhs), get(rhs)) < get(epsilon);
                      }
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }
            [[nodiscard]] constexpr auto lhs() const
            {
                return get(lhs_);
            }
            [[nodiscard]] constexpr auto rhs() const
            {
                return get(rhs_);
            }
            [[nodiscard]] constexpr auto epsilon() const
            {
                return get(epsilon_);
            }

            const TLhs lhs_ {};
            const TRhs rhs_ {};
            const TEpsilon epsilon_ {};
            const bool value_ {};
    };

    export template <class TLhs, class TRhs> struct neq_ : op
    {
            constexpr neq_(const TLhs &lhs = {}, const TRhs &rhs = {})
                : lhs_ {lhs}, rhs_ {rhs}, value_ {[&] {
                      using std::operator==;
                      using std::operator!=;
                      using std::operator>;

                      if constexpr (type_traits::has_static_member_object_value_v<TLhs> and
                                    type_traits::has_static_member_object_value_v<TRhs>)
                      {
                          return lhs.value != rhs.value;
                      }
                      else if constexpr (type_traits::has_static_member_object_epsilon_v<TLhs> and
                                         type_traits::has_static_member_object_epsilon_v<TRhs>)
                      {
                          return math::abs(get(lhs_) - get(rhs_)) > math::min_value(TLhs::epsilon, TRhs::epsilon);
                      }
                      else if constexpr (type_traits::has_static_member_object_epsilon_v<TLhs>)
                      {
                          return math::abs(get(lhs_) - get(rhs_)) > TLhs::epsilon;
                      }
                      else if constexpr (type_traits::has_static_member_object_epsilon_v<TRhs>)
                      {
                          return math::abs(get(lhs_) - get(rhs_)) > TRhs::epsilon;
                      }
                      else
                      {
                          return get(lhs_) != get(rhs_);
                      }
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }
            [[nodiscard]] constexpr auto lhs() const
            {
                return get(lhs_);
            }
            [[nodiscard]] constexpr auto rhs() const
            {
                return get(rhs_);
            }

            const TLhs lhs_ {};
            const TRhs rhs_ {};
            const bool value_ {};
    };

    export template <class TLhs, class TRhs> struct gt_ : op
    {
            constexpr gt_(const TLhs &lhs = {}, const TRhs &rhs = {})
                : lhs_ {lhs}, rhs_ {rhs}, value_ {[&] {
                      using std::operator>;

                      if constexpr (type_traits::has_static_member_object_value_v<TLhs> and
                                    type_traits::has_static_member_object_value_v<TRhs>)
                      {
                          return lhs.value > rhs.value;
                      }
                      else
                      {
                          return get(lhs_) > get(rhs_);
                      }
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }
            [[nodiscard]] constexpr auto lhs() const
            {
                return get(lhs_);
            }
            [[nodiscard]] constexpr auto rhs() const
            {
                return get(rhs_);
            }

            const TLhs lhs_ {};
            const TRhs rhs_ {};
            const bool value_ {};
    };

    export template <class TLhs, class TRhs> struct ge_ : op
    {
            constexpr ge_(const TLhs &lhs = {}, const TRhs &rhs = {})
                : lhs_ {lhs}, rhs_ {rhs}, value_ {[&] {
                      using std::operator>=;

                      if constexpr (type_traits::has_static_member_object_value_v<TLhs> and
                                    type_traits::has_static_member_object_value_v<TRhs>)
                      {
                          return lhs.value >= rhs.value;
                      }
                      else
                      {
                          return get(lhs_) >= get(rhs_);
                      }
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }
            [[nodiscard]] constexpr auto lhs() const
            {
                return get(lhs_);
            }
            [[nodiscard]] constexpr auto rhs() const
            {
                return get(rhs_);
            }

            const TLhs lhs_ {};
            const TRhs rhs_ {};
            const bool value_ {};
    };

    export template <class TLhs, class TRhs> struct lt_ : op
    {
            constexpr lt_(const TLhs &lhs = {}, const TRhs &rhs = {})
                : lhs_ {lhs}, rhs_ {rhs}, value_ {[&] {
                      using std::operator<;

                      if constexpr (type_traits::has_static_member_object_value_v<TLhs> and
                                    type_traits::has_static_member_object_value_v<TRhs>)
                      {
#if defined(_MSC_VER) && !defined(__clang__)
                          return lhs.value < rhs.value;
#else
                          return TLhs::value < TRhs::value;
#endif
                      }
                      else
                      {
                          return get(lhs_) < get(rhs_);
                      }
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }
            [[nodiscard]] constexpr auto lhs() const
            {
                return get(lhs_);
            }
            [[nodiscard]] constexpr auto rhs() const
            {
                return get(rhs_);
            }

        private:
            const TLhs lhs_ {};
            const TRhs rhs_ {};
            const bool value_ {};
    };

    export template <class TLhs, class TRhs> struct le_ : op
    {
            constexpr le_(const TLhs &lhs = {}, const TRhs &rhs = {})
                : lhs_ {lhs}, rhs_ {rhs}, value_ {[&] {
                      using std::operator<=;

                      if constexpr (type_traits::has_static_member_object_value_v<TLhs> and
                                    type_traits::has_static_member_object_value_v<TRhs>)
                      {
                          return lhs.value <= rhs.value;
                      }
                      else
                      {
                          return get(lhs_) <= get(rhs_);
                      }
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }
            [[nodiscard]] constexpr auto lhs() const
            {
                return get(lhs_);
            }
            [[nodiscard]] constexpr auto rhs() const
            {
                return get(rhs_);
            }

            const TLhs lhs_ {};
            const TRhs rhs_ {};
            const bool value_ {};
    };

    export template <class TLhs, class TRhs> struct and_ : op
    {
            constexpr and_(const TLhs &lhs = {}, const TRhs &rhs = {})
                : lhs_ {lhs}, rhs_ {rhs}, value_ {static_cast<bool>(lhs) and static_cast<bool>(rhs)}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }
            [[nodiscard]] constexpr auto lhs() const
            {
                return get(lhs_);
            }
            [[nodiscard]] constexpr auto rhs() const
            {
                return get(rhs_);
            }

            const TLhs lhs_ {};
            const TRhs rhs_ {};
            const bool value_ {};
    };

    export template <class TLhs, class TRhs> struct or_ : op
    {
            constexpr or_(const TLhs &lhs = {}, const TRhs &rhs = {})
                : lhs_ {lhs}, rhs_ {rhs}, value_ {static_cast<bool>(lhs) or static_cast<bool>(rhs)}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }
            [[nodiscard]] constexpr auto lhs() const
            {
                return get(lhs_);
            }
            [[nodiscard]] constexpr auto rhs() const
            {
                return get(rhs_);
            }

            const TLhs lhs_ {};
            const TRhs rhs_ {};
            const bool value_ {};
    };

    export template <class T> struct not_ : op
    {
            explicit constexpr not_(const T &t = {}) : t_ {t}, value_ {not static_cast<bool>(t)}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }
            [[nodiscard]] constexpr auto value() const
            {
                return get(t_);
            }

            const T t_ {};
            const bool value_ {};
    };

    export template <class TExpr, class TException = void> struct throws_ : op
    {
            constexpr explicit throws_(const TExpr &expr)
                : value_ {[&expr] {
                      try
                      {
                          expr();
                          return false;
                      }
                      catch (const TException &)
                      {
                          return true;
                      }
                      catch (...)
                      {
                          return false;
                      }
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }

            const bool value_ {};
    };

    template <class TExpr> struct throws_<TExpr, void> : op
    {
            constexpr explicit throws_(const TExpr &expr)
                : value_ {[&expr] {
                      try
                      {
                          expr();
                          return false;
                      }
                      catch (...)
                      {
                          return true;
                      }
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }

            const bool value_ {};
    };

    export template <class TExpr> struct nothrow_ : op
    {
            constexpr explicit nothrow_(const TExpr &expr)
                : value_ {[&expr] {
                      try
                      {
                          expr();
                          return true;
                      }
                      catch (...)
                      {
                          return false;
                      }
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }

            const bool value_ {};
    };

#if defined(UT_HAS_POSIX_FORK)
    export template <class TExpr> struct aborts_ : op
    {
            constexpr explicit aborts_(const TExpr &expr)
                : value_ {[&expr]() -> bool {
                      if (const auto pid = fork(); not pid)
                      {
                          expr();
                          std::exit(0);
                      }
                      auto exit_status = 0;
                      wait(&exit_status);
                      return exit_status;
                  }()}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }

            const bool value_ {};
    };
#endif

} // namespace ut::detail

// type_traits additions that depend on detail
namespace ut::type_traits
{

    export template <class T>
    concept is_op = std::derived_from<T, detail::op>;

    export template <typename T, typename = void> struct is_stream_insertable : std::false_type
    {
    };

    template <typename T>
    struct is_stream_insertable<T,
                                std::void_t<decltype(std::declval<std::ostream &>() << detail::get(std::declval<T>()))>>
        : std::true_type
    {
    };

    export template <typename T> constexpr bool is_stream_insertable_v = is_stream_insertable<T>::value;

} // namespace ut::type_traits
