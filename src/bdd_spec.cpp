export module ut:bdd_spec;
import std;
import :type_traits;
import :utility;
import :detail;
import :operators;
import :test;
import :literals;

namespace ut::bdd
{

    export [[maybe_unused]] constexpr auto feature = [](auto &&name) {
        return detail::test {"feature", std::forward<decltype(name)>(name)};
    };
    export [[maybe_unused]] constexpr auto scenario = [](auto &&name) {
        return detail::test {"scenario", std::forward<decltype(name)>(name)};
    };
    export [[maybe_unused]] constexpr auto given = [](auto &&name) {
        return detail::test {"given", std::forward<decltype(name)>(name)};
    };
    export [[maybe_unused]] constexpr auto when = [](auto &&name) {
        return detail::test {"when", std::forward<decltype(name)>(name)};
    };
    export [[maybe_unused]] constexpr auto then = [](auto &&name) {
        return detail::test {"then", std::forward<decltype(name)>(name)};
    };

} // namespace ut::bdd

namespace ut::bdd::gherkin
{

    export class steps
    {
            using step_t = std::string;
            using steps_t = void (*)(steps &);
            using gherkin_t = std::vector<step_t>;
            using call_step_t = utility::function<void(const std::string &)>;
            using call_steps_t = std::vector<std::pair<step_t, call_step_t>>;

            class step
            {
                public:
                    template <class TPattern>
                    step(steps &steps, const TPattern &pattern) : steps_ {steps}, pattern_ {pattern}
                    {
                    }

                    ~step()
                    {
                        steps_.next(pattern_);
                    }

                    template <class TExpr> auto operator=(const TExpr &expr) -> void
                    {
                        for (const auto &[pattern, _] : steps_.call_steps())
                        {
                            if (pattern_ == pattern)
                            {
                                return;
                            }
                        }

                        steps_.call_steps().emplace_back(pattern_, [expr, pattern = pattern_](const auto &_step) {
                            [=]<class... TArgs>(type_traits::list<TArgs...>) {
                                log << _step;
                                auto i = 0u;
                                const auto &ms = utility::match(pattern, _step);
                                expr(lexical_cast<TArgs>(ms[i++])...);
                            }(typename type_traits::function_traits<TExpr>::args {});
                        });
                    }

                private:
                    template <class T> static auto lexical_cast(const std::string &str)
                    {
                        T t {};
                        std::istringstream iss {};
                        iss.str(str);
                        if constexpr (std::is_same_v<T, std::string>)
                        {
                            t = iss.str();
                        }
                        else
                        {
                            iss >> t;
                        }
                        return t;
                    }

                    steps &steps_;
                    std::string pattern_ {};
            };

        public:
            template <class TSteps> constexpr /*explicit(false)*/ steps(const TSteps &_steps) : steps_ {_steps}
            {
            }

            template <class TGherkin> auto operator|(const TGherkin &gherkin)
            {
                gherkin_ = utility::split<std::string>(gherkin, '\n');
                for (auto &_step : gherkin_)
                {
                    _step.erase(0, _step.find_first_not_of(" \t"));
                }

                return [this] {
                    step_ = {};
                    steps_(*this);
                };
            }
            auto feature(const std::string &pattern)
            {
                return step {*this, "Feature: " + pattern};
            }
            auto scenario(const std::string &pattern)
            {
                return step {*this, "Scenario: " + pattern};
            }
            auto given(const std::string &pattern)
            {
                return step {*this, "Given " + pattern};
            }
            auto when(const std::string &pattern)
            {
                return step {*this, "When " + pattern};
            }
            auto then(const std::string &pattern)
            {
                return step {*this, "Then " + pattern};
            }

        private:
            template <class TPattern> auto next(const TPattern &pattern) -> void
            {
                const auto is_scenario = [&pattern](const auto &_step) {
                    constexpr auto scenario = "Scenario";
                    return pattern.find(scenario) == std::string::npos and _step.find(scenario) != std::string::npos;
                };

                const auto call_steps = [this, is_scenario](const auto &_step, const auto i) {
                    for (const auto &[name, call] : call_steps_)
                    {
                        if (is_scenario(_step))
                        {
                            break;
                        }

                        if (utility::is_match(_step, name) or not std::empty(utility::match(name, _step)))
                        {
                            step_ = i;
                            call(_step);
                        }
                    }
                };

                decltype(step_) i {};
                for (const auto &_step : gherkin_)
                {
                    if (i++ == step_)
                    {
                        call_steps(_step, i);
                    }
                }
            }

            auto call_steps() -> call_steps_t &
            {
                return call_steps_;
            }

            steps_t steps_ {};
            gherkin_t gherkin_ {};
            call_steps_t call_steps_ {};
            decltype(sizeof("")) step_ {};
    };

} // namespace ut::bdd::gherkin

namespace ut::spec
{

    export [[maybe_unused]] constexpr auto describe = [](auto &&name) {
        return detail::test {"describe", std::forward<decltype(name)>(name)};
    };

    export [[maybe_unused]] constexpr auto it = [](auto &&name) {
        return detail::test {"it", std::forward<decltype(name)>(name)};
    };

} // namespace ut::spec

// ---- pull operators and literals into ut for convenience ----

namespace ut
{

    export using ut::literals::operator""_test;
    export using ut::literals::operator""_b;
    export using ut::literals::operator""_i;
    export using ut::literals::operator""_s;
    export using ut::literals::operator""_c;
    export using ut::literals::operator""_sc;
    export using ut::literals::operator""_l;
    export using ut::literals::operator""_ll;
    export using ut::literals::operator""_u;
    export using ut::literals::operator""_uc;
    export using ut::literals::operator""_us;
    export using ut::literals::operator""_ul;
    export using ut::literals::operator""_i8;
    export using ut::literals::operator""_i16;
    export using ut::literals::operator""_i32;
    export using ut::literals::operator""_i64;
    export using ut::literals::operator""_u8;
    export using ut::literals::operator""_u16;
    export using ut::literals::operator""_u32;
    export using ut::literals::operator""_u64;
    export using ut::literals::operator""_f;
    export using ut::literals::operator""_d;
    export using ut::literals::operator""_ld;
    export using ut::literals::operator""_ull;

    export using ut::operators::operator==;
    export using ut::operators::operator!=;
    export using ut::operators::operator>;
    export using ut::operators::operator>=;
    export using ut::operators::operator<;
    export using ut::operators::operator<=;
    export using ut::operators::operator and;
    export using ut::operators::operator or;
    export using ut::operators::operator not;
    export using ut::operators::operator|;
    export using ut::operators::operator/;
    export using ut::operators::operator>>;

} // namespace ut
