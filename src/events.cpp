export module ut:events;

import std;
import :reflection;
import :type_traits;

namespace ut::events
{

    export struct run_begin
    {
            int argc {};
            const char **argv {};
    };

    export struct test_begin
    {
            std::string_view type {};
            std::string_view name {};
            reflection::source_location location {};
    };

    export struct suite_begin
    {
            std::string_view type {};
            std::string_view name {};
            reflection::source_location location {};
    };

    export struct suite_end
    {
            std::string_view type {};
            std::string_view name {};
            reflection::source_location location {};
    };

    export template <class Test, class TArg = none> struct test
    {
            std::string_view type {};
            std::string name {}; /// might be dynamic
            std::vector<std::string_view> tag {};
            reflection::source_location location {};
            TArg arg {};
            Test run {};

            constexpr auto operator()()
            {
                run_impl(static_cast<Test &&>(run), arg);
            }
            constexpr auto operator()() const
            {
                run_impl(static_cast<Test &&>(run), arg);
            }

        private:
            static constexpr auto run_impl(Test test, const none &)
            {
                test();
            }

            template <class T> static constexpr auto run_impl(T test, const TArg &arg) -> decltype(test(arg), void())
            {
                test(arg);
            }

            template <class T>
            static constexpr auto run_impl(T test, const TArg &) -> decltype(test.template operator()<TArg>(), void())
            {
                test.template operator()<TArg>();
            }
    };

    export template <class Test, class TArg>
    test(std::string_view, std::string_view, std::string_view, reflection::source_location, TArg, Test)
        -> test<Test, TArg>;

    export template <class TSuite> struct suite
    {
            TSuite run {};
            std::string_view name {};
            constexpr auto operator()()
            {
                run();
            }
            constexpr auto operator()() const
            {
                run();
            }
    };

    export template <class TSuite> suite(TSuite) -> suite<TSuite>;

    export struct test_run
    {
            std::string_view type {};
            std::string_view name {};
    };

    export struct test_finish
    {
            std::string_view type {};
            std::string_view name {};
    };

    export template <class TArg = none> struct skip
    {
            std::string_view type {};
            std::string_view name {};
            TArg arg {};
    };

    export template <class TArg> skip(std::string_view, std::string_view, TArg) -> skip<TArg>;

    export struct test_skip
    {
            std::string_view type {};
            std::string_view name {};
    };

    export template <class TExpr> struct assertion
    {
            TExpr expr {};
            reflection::source_location location {};
    };

    export template <class TExpr> assertion(TExpr, reflection::source_location) -> assertion<TExpr>;

    export template <class TExpr> struct assertion_pass
    {
            TExpr expr {};
            reflection::source_location location {};
    };

    export template <class TExpr> assertion_pass(TExpr) -> assertion_pass<TExpr>;

    export template <class TExpr> struct assertion_fail
    {
            TExpr expr {};
            reflection::source_location location {};
    };

    export template <class TExpr> assertion_fail(TExpr) -> assertion_fail<TExpr>;

    export struct test_end
    {
            std::string_view type {};
            std::string_view name {};
    };

    export template <class TMsg> struct log
    {
            TMsg msg {};
    };

    export template <class TMsg = std::string_view> log(TMsg) -> log<TMsg>;

    export struct fatal_assertion : std::exception
    {
    };

    export struct exception
    {
            const char *msg {};
            [[nodiscard]] auto what() const -> const char *
            {
                return msg;
            }
    };

    export struct summary
    {
    };

} // namespace ut::events
