export module ut:test;

import std;
import :type_traits;
import :reflection;
import :events;
import :detail;
import :cfg;
import :runner;

namespace ut::detail
{

    export struct tag
    {
            std::vector<std::string_view> name {};
    };

    export template <class... Ts, class TEvent> [[nodiscard]] constexpr decltype(auto) on(TEvent &&event)
    {
        return ut::cfg<typename type_traits::identity<override, Ts...>::type>.on(static_cast<TEvent &&>(event));
    }

    export template <class Test> struct test_location
    {
            template <class T>
            constexpr test_location(const T &t,
                                    const reflection::source_location &sl = reflection::source_location::current())
                : test {t}, location {sl}
            {
            }

            Test test {};
            reflection::source_location location {};
    };

    export struct test
    {
            std::string_view type {};
            std::optional<std::string> backingName;
            std::string_view name {};
            std::vector<std::string_view> tag {};

            test(std::string_view t, std::string_view sv) : type(t), name(sv)
            {
            }
            test(std::string_view t, const std::string &s) : type(t), name(s)
            {
            }
            test(std::string_view t, const char *s) : type(t), name(s)
            {
            }
            template <std::size_t N> test(std::string_view t, const char (&s)[N]) : type(t), name(s)
            {
            }
            test(std::string_view t, std::string &&s) : type(t), backingName(std::move(s)), name(*backingName)
            {
            }

            template <class... Ts> constexpr auto operator=(test_location<void (*)()> _test)
            {
                on<Ts...>(events::test<void (*)()> {.type = type,
                                                    .name = std::string {name},
                                                    .tag = tag,
                                                    .location = _test.location,
                                                    .arg = none {},
                                                    .run = _test.test});
                return _test.test;
            }

            template <class Test>
                requires std::invocable<Test> && (!std::convertible_to<Test, void (*)()>)
            constexpr auto operator=(Test _test)
            {
                on<Test>(events::test<Test> {.type = type,
                                             .name = std::string {name},
                                             .tag = tag,
                                             .location = {},
                                             .arg = none {},
                                             .run = static_cast<Test &&>(_test)});
                return _test;
            }

            constexpr void operator=(void (*_test)(std::string_view, std::string_view)) const
            {
                _test(type, name);
            }

            template <class Test>
                requires std::invocable<Test, std::string_view, std::string_view> &&
                         (!std::convertible_to<Test, void (*)(std::string_view, std::string_view)>)
            constexpr auto operator=(Test _test)
            {
                return _test(type, name);
            }
    };

    export struct log
    {
            struct next
            {
                    template <class TMsg> auto &operator<<(const TMsg &msg)
                    {
                        on<TMsg>(events::log {' '});
                        on<TMsg>(events::log {msg});
                        return *this;
                    }
            };

            template <class TMsg> auto operator<<(const TMsg &msg) -> next
            {
                on<TMsg>(events::log {'\n'});
                on<TMsg>(events::log {msg});
                return next {};
            }

            template <class... Args> void operator()(std::format_string<Args...> fmt, Args &&...args)
            {
                on<std::string>(events::log {std::vformat(fmt.get(), std::make_format_args(args...))});
            }
    };

    export template <class TExpr> class terse_
    {
        public:
            constexpr explicit terse_(const TExpr &expr) : expr_ {expr}
            {
                cfg::wip = {};
            }

            ~terse_() noexcept(false)
            {
                if (static auto once = true; once and not cfg::wip)
                {
                    once = {};
                }
                else
                {
                    return;
                }

                cfg::wip = true;

                void(detail::on<TExpr>(events::assertion<TExpr> {.expr = expr_, .location = cfg::location}));
            }

        private:
            const TExpr &expr_;
    };

    export struct that_
    {
            template <class T> struct expr
            {
                    using type = expr;

                    constexpr explicit expr(const T &t) : t_ {t}
                    {
                    }

                    [[nodiscard]] constexpr auto operator!() const
                    {
                        return not_ {*this};
                    }

                    template <class TRhs> [[nodiscard]] constexpr auto operator==(const TRhs &rhs) const
                    {
                        return eq_ {t_, rhs};
                    }

                    template <class TRhs> [[nodiscard]] constexpr auto operator!=(const TRhs &rhs) const
                    {
                        return neq_ {t_, rhs};
                    }

                    template <class TRhs> [[nodiscard]] constexpr auto operator>(const TRhs &rhs) const
                    {
                        return gt_ {t_, rhs};
                    }

                    template <class TRhs> [[nodiscard]] constexpr auto operator>=(const TRhs &rhs) const
                    {
                        return ge_ {t_, rhs};
                    }

                    template <class TRhs> [[nodiscard]] constexpr auto operator<(const TRhs &rhs) const
                    {
                        return lt_ {t_, rhs};
                    }

                    template <class TRhs> [[nodiscard]] constexpr auto operator<=(const TRhs &rhs) const
                    {
                        return le_ {t_, rhs};
                    }

                    [[nodiscard]] constexpr operator bool() const
                    {
                        return static_cast<bool>(t_);
                    }

                    const T t_ {};
            };

            template <class T> [[nodiscard]] constexpr auto operator%(const T &t) const
            {
                return expr {t};
            }
    };

    // Full definition of fatal_ (forward-declared in :detail)
    template <class TExpr> struct fatal_ : op
    {
            using type = fatal_;

            constexpr explicit fatal_(const TExpr &expr) : expr_ {expr}
            {
            }
            constexpr explicit fatal_(const TExpr &expr, const reflection::source_location &sl)
                : expr_ {expr}, location {sl}
            {
            }

            [[nodiscard]] constexpr operator bool() const
            {
                if (static_cast<bool>(expr_))
                {
                }
                else
                {
                    cfg::wip = true;
                    void(on<TExpr>(events::assertion<TExpr> {.expr = expr_, .location = location}));
                    on<TExpr>(events::fatal_assertion {});
                }
                return static_cast<bool>(expr_);
            }

            [[nodiscard]] constexpr decltype(auto) get() const
            {
                return expr_;
            }

            TExpr expr_ {};
            reflection::source_location location {};
    };

    export template <class T> struct expect_
    {
            constexpr explicit expect_(bool value) : value_ {value}
            {
                cfg::wip = {};
            }

            template <class TMsg> auto &operator<<(const TMsg &msg)
            {
                if (not value_)
                {
                    on<T>(events::log {' '});
                    if constexpr (requires {
                                      requires std::invocable<TMsg> and not std::is_void_v<std::invoke_result_t<TMsg>>;
                                  })
                    {
                        on<T>(events::log {std::invoke(msg)});
                    }
                    else
                    {
                        on<T>(events::log {msg});
                    }
                }
                return *this;
            }

            auto &operator<<(detail::fatal)
            {
                if (not value_)
                {
                    on<T>(events::fatal_assertion {});
                }
                return *this;
            }

            [[nodiscard]] constexpr operator bool() const
            {
                return value_;
            }

            bool value_ {};
    };

} // namespace ut::detail
