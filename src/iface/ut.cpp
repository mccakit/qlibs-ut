//
// The MIT License (MIT)
//
// Copyright (c) 2024 Kris Jusiak <kris@jusiak.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

/**
 * @file ut.cppm
 * @brief Primary module interface unit of `ut`, a compile-time-first C++23
 *        unit-testing library.
 *
 * `ut` executes tests at compile time *and* at run time from a single source
 * of truth. A test that can be constant-evaluated is evaluated twice: once by
 * the compiler (where a failed expectation becomes a compile error, and where
 * the constant evaluator additionally catches memory leaks and undefined
 * behaviour) and once at run time.
 *
 * ### Design principles
 *
 * - **Explicit by design** - no implicit conversions, no narrowing, no
 *   epsilon-less floating point comparison.
 * - **Minimal API** - a handful of names, listed in the module groups below.
 * - **Self-contained** - the interface pulls in exactly one standard header,
 *   `<iostream>`, and only for the default output stream. Define
 *   #UT_COMPILE_TIME_ONLY when building the module for a completely
 *   freestanding, zero-dependency build.
 * - **Clean under** `-fno-exceptions -fno-rtti -Wall -Wextra -Werror -pedantic
 *   -pedantic-errors`.
 *
 * ### Usage
 *
 * @code
 * import ut;
 *
 * constexpr auto sum(auto... args) { return (0 + ... + args); }
 *
 * int main() {
 *   using namespace ut;
 *
 *   "sum"_test = [] {
 *     expect(0_i == sum());
 *     expect(2_i == sum(1, 1));
 *     expect(3_i == sum(1, 2)) << "sum(1, 2) should be 3";
 *   };
 * }
 * @endcode
 *
 * A test is executed at compile time unless it is `mutable` or captures state;
 * mark it `mutable` to force run-time-only execution:
 *
 * @code
 * "run-time only"_test = [] mutable {
 *   expect(42_i == answer());
 * };
 * @endcode
 *
 * ### Building
 *
 * @code{.sh}
 * # clang
 * clang++ -std=c++23 --precompile ut.cppm -o ut.pcm
 * clang++ -std=c++23 -c ut.pcm -o ut.pcm.o
 * clang++ -std=c++23 -fmodule-file=ut=ut.pcm -c ut.cpp -o ut.o
 * clang++ -std=c++23 -fmodule-file=ut=ut.pcm my_test.cpp ut.pcm.o ut.o -o my_test
 *
 * # gcc
 * g++ -std=c++23 -fmodules-ts -x c++ -c ut.cppm -o ut.iface.o
 * g++ -std=c++23 -fmodules-ts -c ut.cpp -o ut.o
 * g++ -std=c++23 -fmodules-ts my_test.cpp ut.iface.o ut.o -o my_test
 * @endcode
 *
 * @note Doxygen does not parse module declarations natively. Add
 *       `EXTENSION_MAPPING = cppm=C++` and `FILE_PATTERNS += *.cppm` to your
 *       `Doxyfile`, and add `UT_COMPILE_TIME_ONLY` / `UT_RUN_TIME_ONLY` /
 *       `NTEST` to `PREDEFINED` as appropriate.
 *
 * @see ut.cpp for the matching module implementation unit.
 *
 * @author Kris Jusiak <kris@jusiak.net>
 * @copyright MIT License
 */

/**
 * @def UT_COMPILE_TIME_ONLY
 * @brief Build the module without any run-time reporting machinery.
 *
 * Suppresses the `<iostream>` dependency, the run-time execution pass, the
 * summary printout and the non-`constexpr` reporter destructor. Tests are then
 * verified purely by the constant evaluator.
 *
 * @warning This is a **module build-time** option. Macros do not cross module
 *          boundaries, so defining it in an importing translation unit has no
 *          effect - it must be defined when compiling ut.cppm *and* ut.cpp.
 */

/**
 * @def UT_RUN_TIME_ONLY
 * @brief Build the module without the compile-time execution pass.
 *
 * Useful when the target standard library or compiler cannot constant-evaluate
 * the tests, or to cut build times while iterating.
 *
 * @warning Module build-time option; see #UT_COMPILE_TIME_ONLY.
 */

/**
 * @def NTEST
 * @brief Skip the library's own self-verification suite.
 *
 * The self-tests live in the module implementation unit (ut.cpp) and therefore
 * run once, when the module is built, rather than on every import.
 *
 * @warning Module build-time option; see #UT_COMPILE_TIME_ONLY.
 */

module;

#ifndef UT_COMPILE_TIME_ONLY
#include <iostream> // std::clog - the default reporting sink
#endif

export module ut;

/**
 * @defgroup ut_dsl Expression DSL
 * @brief Strongly typed literals and comparison operators.
 *
 * Comparisons are only formed when at least one operand is a `ut` typed value
 * (`42_i`, `_u{42}`, `"foo"_s`, ...). Mixing different underlying types is a
 * compile error, and floating point comparison requires an explicit epsilon.
 */

/**
 * @defgroup ut_assertions Assertions
 * @brief `expect` and the test registration machinery.
 */

/**
 * @defgroup ut_events Events
 * @brief The event types a custom reporter/outputter must handle.
 * @see ut_config
 */

/**
 * @defgroup ut_config Configuration
 * @brief The `ut::cfg` customization point and its default components.
 */

/**
 * @namespace ut
 * @brief Everything the library exports.
 *
 * @note Unlike the header this module replaces, `operator""_test` is **not**
 *       injected into the global namespace. Bring it in with
 *       `using namespace ut;` or `using ut::operator""_test;`.
 */
namespace ut
{

    /**
     * @namespace ut::type_traits
     * @brief Minimal, dependency-free replacements for `<type_traits>`.
     * @internal Not exported.
     */
    namespace type_traits
    {
        /// @brief `true` if @p T and the second parameter denote the same type.
        template <class, class> inline constexpr auto is_same_v = false;
        template <class T> inline constexpr auto is_same_v<T, T> = true;

        /// @brief `true` for `float`, `double` and `long double`.
        template <class T> inline constexpr auto is_floating_point_v = false;
        template <> inline constexpr auto is_floating_point_v<float> = true;
        template <> inline constexpr auto is_floating_point_v<double> = true;
        template <> inline constexpr auto is_floating_point_v<long double> = true;

        /// @brief `true` if the given pointer-to-`operator()` belongs to a `mutable` lambda.
        /// @details A non-`mutable` closure exposes a `const`-qualified call operator,
        ///          which does not match the partial specialization below.
        template <class> inline constexpr auto is_mutable_lambda_v = false;
        template <class R, class B, class... Ts> inline constexpr auto is_mutable_lambda_v<R (B::*)(Ts...)> = true;

        /// @brief `true` if the closure type @p Fn captures anything.
        /// @details A captureless closure is empty, so `sizeof` is 1.
        template <class Fn> inline constexpr auto has_capture_lambda_v = sizeof(Fn) > 1ul;
    } // namespace type_traits

    /**
     * @namespace ut::utility
     * @brief Small self-contained helpers.
     * @internal Not exported, but reachable through exported signatures.
     */
    namespace utility
    {

        /// @brief Unevaluated-context stand-in for `std::declval`.
        template <class T> T &&declval();

        /// @brief Yields @p T while making the enclosing lookup dependent on `Ts...`.
        /// @details Used by ut::detail::cfg so that a user specialization of ut::cfg
        ///          declared *after* this header is still found.
        template <class T, class...> struct type_identity
        {
                using type = T;
        };

        /**
         * @brief A string literal usable as a non-type template parameter.
         * @tparam Size Length of the source literal, including the terminating NUL.
         */
        template <unsigned Size> struct fixed_string
        {
                /// @brief Copies @p str into the structural storage.
                constexpr fixed_string(const char (&str)[Size])
                {
                    for (auto i = 0u; i < Size; ++i)
                    {
                        storage[i] = str[i];
                    }
                }
                /// @brief Character at index @p i (unchecked).
                [[nodiscard]] constexpr auto operator[](const auto i) const
                {
                    return storage[i];
                }
                /// @brief Pointer to the NUL-terminated storage.
                [[nodiscard]] constexpr auto data() const
                {
                    return storage;
                }
                /// @brief Length of the literal, **including** the terminating NUL.
                [[nodiscard]] static constexpr auto size()
                {
                    return Size;
                }
                /// @brief Streams the underlying characters.
                constexpr friend auto operator<<(auto &os, const fixed_string &fs) -> decltype(auto)
                {
                    return os << fs.storage;
                }
                char storage[Size] {}; ///< Structural storage; public so the type stays a valid NTTP.
        };

        /**
         * @brief Glob matcher used to honour the `UT_FILTER` environment variable.
         * @param pattern NUL-terminated glob; `*` matches any run of characters, `?`
         *                matches exactly one. A null pointer matches everything.
         * @param str     NUL-terminated candidate.
         * @return `true` if @p str matches @p pattern.
         */
        [[nodiscard]] constexpr auto match(const auto pattern, const auto str) -> bool
        {
            if (not bool(pattern))
            {
                return true;
            }
            if (not *pattern)
            {
                return not *str;
            }

            if (not *str)
            {
                return pattern[0] == '*' ? match(&pattern[1], str) : false;
            }

            if (pattern[0] != '?' and pattern[0] != '*' and pattern[0] != str[0])
            {
                return false;
            }

            if (pattern[0] == '*')
            {
                auto tmp = str;
                auto i = 0u;
                while (tmp++)
                {
                    if (match(&pattern[1], &str[i++]))
                    {
                        return true;
                    }
                }
                return false;
            }

            return match(&pattern[1], &str[1]);
        }
    } // namespace utility

    /**
     * @namespace ut::detail
     * @brief Implementation details.
     * @internal Not exported.
     */
    namespace detail
    {

        /**
         * @brief Poison pill that makes a failed expectation a compile error.
         * @internal
         *
         * Deliberately **not** `constexpr`: calling it during constant evaluation is
         * ill-formed, which is exactly how a compile-time expectation reports failure.
         * It is only ever reached from the taken branch of an `if consteval`, so it is
         * never called at run time.
         *
         * @see ut.cpp for the (empty) definition, which exists purely so that the
         *      declaration can never turn into a link error.
         */
        void failed();

        /**
         * @brief Reads the `UT_FILTER` environment variable.
         * @internal
         * @return The glob to match test names against, or `nullptr` to run everything.
         * @note Defined in ut.cpp; run-time only, so it keeps `<cstdlib>` out of the
         *       module interface entirely.
         */
        [[nodiscard]] auto filter() -> const char *;

        /**
         * @brief Terminates the process after a fatal expectation or a failed run.
         * @internal
         * @note Defined in ut.cpp.
         */
        [[noreturn]] void abort();
    } // namespace detail

    /**
     * @namespace ut::events
     * @brief Value types handed to the configured runner, reporter and outputter.
     * @ingroup ut_events
     */
    export namespace events
    {

        /// @brief Which execution pass produced an event.
        /// @ingroup ut_events
        enum class mode
        {
            run_time,    ///< The event was produced while the test ran at run time.
            compile_time ///< The event was produced during constant evaluation.
        };

        /// @brief A test that is about to be dispatched by ut::runner.
        /// @tparam T Closure type of the test body.
        /// @ingroup ut_events
        template <class T> struct run
        {
                T test {};                ///< The test body.
                const char *file_name {}; ///< Source file the test was registered in.
                int line {};              ///< Source line the test was registered on.
                const char *name {};      ///< Test name, taken from the `_test` literal.
                const char *filter {};    ///< Glob from `UT_FILTER`, or `nullptr` for "run everything".
        };

        /// @brief A test is entering execution.
        /// @ingroup ut_events
        template <mode Mode> struct test_begin
        {
                const char *file_name {}; ///< Source file.
                int line {};              ///< Source line.
                const char *name {};      ///< Test name.
        };

        /// @brief A test has finished executing.
        /// @ingroup ut_events
        template <mode Mode> struct test_end
        {
                const char *file_name {}; ///< Source file.
                int line {};              ///< Source line.
                const char *name {};      ///< Test name.
                /// @brief Outcome, doubling as an index into ut::events::summary::tests.
                enum outcome : int
                {
                    FAILED,      ///< At least one expectation failed.
                    PASSED,      ///< All expectations held.
                    COMPILE_TIME ///< The test was constant-evaluated.
                } result {};
        };

        /// @brief An expectation that held.
        /// @tparam TExpr Type of the evaluated expression, streamable for reporting.
        /// @ingroup ut_events
        template <class TExpr> struct assert_pass
        {
                const char *file_name {}; ///< Source file.
                int line {};              ///< Source line.
                TExpr expr {};            ///< The expression, retained for diagnostics.
        };

        /// @brief An expectation that did not hold.
        /// @tparam TExpr Type of the evaluated expression, streamable for reporting.
        /// @ingroup ut_events
        template <class TExpr> struct assert_fail
        {
                const char *file_name {}; ///< Source file.
                int line {};              ///< Source line.
                TExpr expr {};            ///< The expression, retained for diagnostics.
        };

        /// @brief Emitted by a fatal (`expect[...]`) expectation; terminates the run.
        /// @ingroup ut_events
        struct fatal
        {
        };

        /// @brief A user message appended to an expectation with `operator<<`.
        /// @tparam TMsg Type of the streamed message.
        /// @ingroup ut_events
        template <class TMsg> struct log
        {
                const TMsg &msg; ///< The message; not owned.
                bool result {};  ///< Outcome of the expectation the message is attached to.
        };

        /// @brief Aggregated tallies, emitted once at the end of the run.
        /// @ingroup ut_events
        struct summary
        {
                /// @brief Index names for the tally arrays below.
                enum outcome : int
                {
                    FAILED,      ///< Index of the failure tally.
                    PASSED,      ///< Index of the success tally.
                    COMPILE_TIME ///< Index of the compile-time tally (tests only).
                };
                unsigned asserts[2] {}; ///< Expectation tallies, indexed by #FAILED / #PASSED.
                unsigned tests[3] {};   ///< Test tallies, indexed by #FAILED / #PASSED / #COMPILE_TIME.
        };
    } // namespace events

    /**
     * @brief Default event sink: renders failures and the final summary as text.
     * @tparam TOStream Anything supporting `os << value`.
     * @ingroup ut_config
     *
     * Every handler is a no-op during constant evaluation, so the same code path
     * serves both execution passes.
     */
    export template <class TOStream> class outputter
    {
        public:
            /// @brief Ignores compile-time test entry.
            template <events::mode Mode> constexpr auto on(const events::test_begin<Mode> &) -> void
            {
            }
            /// @brief Remembers the running test so failures can be attributed to it.
            constexpr auto on(const events::test_begin<events::mode::run_time> &event) -> void
            {
                current_test = event;
            }
            /// @brief Ignores test completion; ut::reporter owns the tallying.
            template <events::mode Mode> constexpr auto on(const events::test_end<Mode> &) -> void
            {
            }
            /// @brief Ignores passing expectations.
            template <class TExpr> constexpr auto on(const events::assert_pass<TExpr> &) -> void
            {
            }

            /// @brief Prints `file:line:FAILED:"test": expr`.
            template <class TExpr> constexpr auto on(const events::assert_fail<TExpr> &event) -> void
            {
                if not consteval
                {
                    if (initial_new_line == '\n')
                    {
                        os << initial_new_line;
                    }
                    else
                    {
                        initial_new_line = '\n';
                    }
                    os << event.file_name << ':' << event.line << ':' << "FAILED:" << '\"' << current_test.name
                       << "\": " << event.expr;
                }
            }

            /// @brief Ignores the fatal event; ut::reporter performs the termination.
            constexpr auto on(const events::fatal &) -> void
            {
            }

            /// @brief Appends a user message, but only to a failing expectation.
            template <class TMsg> constexpr auto on(const events::log<TMsg> &event) -> void
            {
                if not consteval
                {
                    if (not event.result)
                    {
                        os << ' ' << event.msg;
                    }
                }
            }

            /// @brief Prints the closing `PASSED:`/`FAILED:` tally line.
            constexpr auto on(const events::summary &event) -> void
            {
                if not consteval
                {
                    if (event.asserts[events::summary::FAILED] || event.tests[events::summary::FAILED])
                    {
                        os << "\nFAILED: ";
                    }
                    else
                    {
                        os << "PASSED: ";
                    }
                    os << "tests: " << (event.tests[events::summary::PASSED] + event.tests[events::summary::FAILED])
                       << " (" << event.tests[events::summary::PASSED] << " passed, "
                       << event.tests[events::summary::FAILED] << " failed, "
                       << event.tests[events::summary::COMPILE_TIME] << " compile-time), "
                       << "asserts: "
                       << (event.asserts[events::summary::PASSED] + event.asserts[events::summary::FAILED]) << " ("
                       << event.asserts[events::summary::PASSED] << " passed, "
                       << event.asserts[events::summary::FAILED] << " failed)\n";
                }
            }

            TOStream &os;                                               ///< The sink.
            events::test_begin<events::mode::run_time> current_test {}; ///< Test currently executing.
            char initial_new_line {}; ///< Suppresses the leading newline of the first failure.
    };

    /**
     * @brief Tallies results, forwards them to an outputter, and terminates on failure.
     * @tparam TOutputter Type modelling the ut::outputter interface.
     * @tparam MaxDepth   Maximum nesting depth of tests.
     * @ingroup ut_config
     */
    export template <class TOutputter, auto MaxDepth = 16u> struct reporter
    {
            /// @brief Records the failure count on entry so nesting can be unwound.
            constexpr auto on(const events::test_begin<events::mode::run_time> &event) -> void
            {
                asserts_failed[current++] = summary.asserts[events::summary::FAILED];
                outputter.on(event);
            }

            /// @brief A test passed if it added no failures while it was running.
            constexpr auto on(const events::test_end<events::mode::run_time> &event) -> void
            {
                const auto result = summary.asserts[events::summary::FAILED] == asserts_failed[--current];
                ++summary.tests[result];
                events::test_end<events::mode::run_time> te {event};
                te.result = static_cast<decltype(te.result)>(result);
                outputter.on(te);
            }

            /// @brief Counts a constant-evaluated test.
            constexpr auto on(const events::test_begin<events::mode::compile_time> &) -> void
            {
                ++summary.tests[events::summary::COMPILE_TIME];
            }

            /// @brief No-op; a compile-time failure is already a compile error.
            constexpr auto on(const events::test_end<events::mode::compile_time> &) -> void
            {
            }

            /// @brief Counts a passing expectation.
            template <class TExpr> constexpr auto on(const events::assert_pass<TExpr> &event) -> void
            {
                ++summary.asserts[events::summary::PASSED];
                outputter.on(event);
            }

            /// @brief Counts a failing expectation.
            template <class TExpr> constexpr auto on(const events::assert_fail<TExpr> &event) -> void
            {
                ++summary.asserts[events::summary::FAILED];
                outputter.on(event);
            }

            /// @brief Flushes the summary and aborts the process.
            /// @warning Does not return.
            constexpr auto on(const events::fatal &event) -> void
            {
                ++summary.tests[events::summary::FAILED];
                outputter.on(event);
                outputter.on(summary);
                detail::abort();
            }

#ifndef UT_COMPILE_TIME_ONLY
            /// @brief Prints the summary at static destruction and aborts if anything failed.
            /// @note Deliberately not `constexpr`; it is the process exit hook.
            ~reporter()
            {
                outputter.on(summary);
                if (summary.asserts[events::summary::FAILED])
                {
                    detail::abort();
                }
            }
#endif

            TOutputter &outputter;                ///< Downstream sink.
            events::summary summary {};           ///< Running tallies.
            unsigned asserts_failed[MaxDepth] {}; ///< Failure counts saved per nesting level.
            unsigned current {};                  ///< Current nesting depth.
    };

    /**
     * @brief Decides whether a test runs at compile time, at run time, or both.
     * @tparam TReporter Type modelling the ut::reporter interface.
     * @ingroup ut_config
     */
    export template <class TReporter> struct runner
    {
            /**
             * @brief Dispatches a single test.
             * @tparam Test Closure type of the test body.
             * @param run   The test together with its source location and filter.
             * @return `false` only when a `mutable` test is reached during constant
             *         evaluation - which tells the caller the test is run-time only.
             *
             * A test is executed by the constant evaluator unless it is `mutable` or
             * captures state. Independently of that, it is executed at run time whenever
             * its name matches `UT_FILTER`.
             */
            template <class Test> constexpr auto on(events::run<Test> run) -> bool
            {
                if consteval
                {
                    if constexpr (!type_traits::is_mutable_lambda_v<decltype(&Test::operator())>)
                    {
                        run.test();
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
#ifndef UT_RUN_TIME_ONLY
                    if constexpr (!type_traits::is_mutable_lambda_v<decltype(&Test::operator())> &&
                                  !type_traits::has_capture_lambda_v<Test>)
                    {
                        reporter.on(events::test_begin<events::mode::compile_time> {run.file_name, run.line, run.name});
                        static_assert((
                            run.test(),
                            R"([FAILED] Compile-time expectation failed. The error below should indicate the issue. If it's not an assertion it might be that the std and/or compiler don't support running the test at compile-time. In such case mark the test as `mutable` to force run-time only execution. For example: `"run-time"_test = [] mutable { ... }`.)"));
                        reporter.on(events::test_end<events::mode::compile_time> {run.file_name, run.line, run.name});
                    }
#endif

#ifndef UT_COMPILE_TIME_ONLY
                    if (utility::match(run.filter, run.name))
                    {
                        reporter.on(events::test_begin<events::mode::run_time> {run.file_name, run.line, run.name});
                        run.test();
                        reporter.on(events::test_end<events::mode::run_time> {run.file_name, run.line, run.name});
                    }
#endif
                }
                return true;
            }

            TReporter &reporter; ///< Downstream sink.
    };

    namespace detail
    {

        /**
         * @brief Default sink, forwarding to `std::clog`.
         * @internal
         *
         * Kept as a distinct type so that the `static_assert` below can produce a
         * readable diagnostic instead of an overload-resolution failure.
         */
        struct stream
        {
                /// @brief Writes @p t to `std::clog`, or discards it under #UT_COMPILE_TIME_ONLY.
                friend constexpr decltype(auto) operator<<([[maybe_unused]] auto &os, [[maybe_unused]] const auto &t)
                {
#ifdef UT_COMPILE_TIME_ONLY
                    return os;
#else
                    static_assert(
                        requires { std::clog << t; },
                        "[ERROR] No output supported: Consider providing `operator<<` for the type | "
                        "ut::cfg<ut::override> = custom_cfg{} | building the module with -DUT_COMPILE_TIME_ONLY");
                    return (std::clog << t);
#endif
                }
        };
    } // namespace detail

    /**
     * @brief The default wiring of stream, outputter, reporter and runner.
     * @tparam Ts Unused; present so ut::cfg can be partially specialized.
     * @ingroup ut_config
     */
    export template <class...> struct default_cfg
    {
            detail::stream stream {};                               ///< Sink; forwards to `std::clog`.
            ut::outputter<decltype(stream)> outputter {stream};     ///< Text renderer.
            ut::reporter<decltype(outputter)> reporter {outputter}; ///< Tallying and termination.
            ut::runner<decltype(reporter)> runner {reporter};       ///< Execution strategy.
            const char *current_test_name {};                       ///< Name of the test currently running.
    };

    /// @brief Tag selecting a user-provided configuration.
    /// @ingroup ut_config
    export struct override
    {
    };

    /**
     * @brief Customization point holding the active configuration.
     * @tparam Ts Configuration selector; ut::override marks a user specialization.
     * @ingroup ut_config
     *
     * Replace the default by partially specializing it for ut::override:
     *
     * @code
     * struct my_cfg {
     *   // must expose `runner` and `reporter` members modelling ut::runner / ut::reporter
     * };
     * template<class... Ts> inline ut::default_cfg<Ts...> ut::cfg<ut::override, Ts...>{};
     * @endcode
     */
    export template <class... Ts> inline default_cfg<Ts...> cfg {};

    namespace detail
    {

        /**
         * @brief Two-phase-lookup shim that resolves the active ut::cfg.
         * @internal
         *
         * The arguments are only used to make the lookup dependent, so that a user
         * specialization declared after this point is still found.
         */
        template <class... Ts> [[nodiscard]] constexpr auto &cfg(Ts &&...)
        {
            return ut::cfg<typename utility::type_identity<override, Ts...>::type>;
        }
    } // namespace detail

    /**
     * @brief The assertion object; see the ut::expect instance.
     * @ingroup ut_assertions
     */
    export struct expect_t
    {
            /**
             * @brief Evaluates a non-fatal expectation.
             * @param expr      A `ut` comparison such as `42_i == answer()`.
             * @param file_name Defaulted to the caller's file.
             * @param line      Defaulted to the caller's line.
             * @return A handle accepting `<< message`, streamed only when @p expr failed.
             *
             * At compile time a failure calls ut::detail::failed(), turning the
             * enclosing constant evaluation into a diagnostic. At run time it is
             * reported and tallied, and the process aborts once the run completes.
             *
             * @code
             * expect(42_i == answer()) << "the answer changed";
             * @endcode
             *
             * @warning A bare `bool` is rejected: `expect(a == b)` on raw types would
             *          collapse to `true`/`false` and lose both operands, so the DSL
             *          types are mandatory.
             */
            static constexpr auto operator()(auto expr,
                                             const char *file_name = __builtin_FILE(),
                                             int line = __builtin_LINE())
            {
                if constexpr (constexpr auto unsupported = type_traits::is_same_v<bool, decltype(expr)> || !requires {
                                  static_cast<bool>(expr);
                              }; unsupported)
                {
                    static_assert(
                        !unsupported,
                        "[ERROR] Expression required - `expect(lhs == rhs)`. For example: `expect(3_i == sum(1, 2))`.");
                }
                else
                {
                    bool result {};
                    if consteval
                    {
                        if (result = static_cast<bool>(expr); !result)
                        {
                            detail::failed();
                        }
                    }
                    else
                    {
                        if (result = static_cast<bool>(expr); result)
                        {
                            detail::cfg(expr).reporter.on(events::assert_pass {file_name, line, expr});
                        }
                        else
                        {
                            detail::cfg(expr).reporter.on(events::assert_fail {file_name, line, expr});
                        }
                    }
                    return log {result};
                }
            }

            /**
             * @brief Evaluates a fatal expectation.
             * @param expr      A `ut` comparison such as `42_i == answer()`.
             * @param file_name Defaulted to the caller's file.
             * @param line      Defaulted to the caller's line.
             * @return A handle accepting `<< message`; its destructor raises
             *         ut::events::fatal.
             *
             * @code
             * expect[ptr != nullptr_v] << "cannot continue";
             * @endcode
             *
             * @warning **Inherited behaviour, preserved verbatim from the header this
             *          module replaces:** the returned handle raises ut::events::fatal
             *          from its destructor *unconditionally*, and ut::reporter::on()
             *          responds by aborting - so `expect[...]` currently terminates the
             *          run whether the expectation held or not. Guarding the destructor
             *          with `if (not result)` is the one-line fix, but that is a
             *          behavioural change and is therefore left to the maintainer.
             *
             * @note This overload is what the original header's trailing comment
             *       ("multiple and/or default parameters requires C++23") was waiting
             *       for: P2128 lifts the one-parameter restriction on `operator[]` and
             *       allows default arguments, so the `fatal_expr` conversion helper that
             *       used to smuggle the source location in is no longer needed.
             */
            static constexpr auto operator[](auto expr,
                                             const char *file_name = __builtin_FILE(),
                                             int line = __builtin_LINE())
            {
                if constexpr (constexpr auto unsupported = type_traits::is_same_v<bool, decltype(expr)> || !requires {
                                  static_cast<bool>(expr);
                              }; unsupported)
                {
                    static_assert(
                        !unsupported,
                        "[ERROR] Expression required - `expect[lhs == rhs]`. For example: `expect[3_i == sum(1, 2)]`.");
                }
                else
                {
                    bool result {};
                    if consteval
                    {
                        if (result = static_cast<bool>(expr); !result)
                        {
                            detail::failed();
                        }
                    }
                    else
                    {
                        if (result = static_cast<bool>(expr); result)
                        {
                            detail::cfg(expr).reporter.on(events::assert_pass {file_name, line, expr});
                        }
                        else
                        {
                            detail::cfg(expr).reporter.on(events::assert_fail {file_name, line, expr});
                        }
                    }
                    return fatal_log {result};
                }
            }

        private:
            /// @brief Handle returned by expect_t::operator(); streams messages on failure.
            struct log
            {
                    /// @brief Forwards @p msg to the configured outputter.
                    template <class TMsg> constexpr const auto &operator<<(const TMsg &msg) const
                    {
                        detail::cfg(msg).outputter.on(events::log<TMsg> {msg, result});
                        return *this;
                    }
                    bool result {}; ///< Outcome of the expectation.
            };

            /**
             * @brief Handle returned by expect_t::operator[]; raises ut::events::fatal.
             * @tparam tag Defaulted to a fresh closure type, giving every call site a
             *             distinct instantiation so that ut::detail::cfg() lookup stays
             *             dependent.
             */
            template <auto tag = [] {}> struct fatal_log
            {
                    /// @brief Forwards @p msg to the configured outputter.
                    template <class TMsg> constexpr const auto &operator<<(const TMsg &msg) const
                    {
                        detail::cfg(msg).outputter.on(events::log<TMsg> {msg, result});
                        return *this;
                    }
                    /// @brief Raises ut::events::fatal. @warning See expect_t::operator[].
                    constexpr ~fatal_log()
                    {
                        detail::cfg(tag).reporter.on(events::fatal {});
                    }
                    bool result {}; ///< Outcome of the expectation.
            };
    };

    /// @brief The assertion entry point.
    /// @ingroup ut_assertions
    export inline constexpr expect_t expect {};

    /**
     * @brief Registers a group of tests that runs during static initialization.
     * @ingroup ut_assertions
     *
     * @code
     * const ut::suite errors = [] {
     *   "out of range"_test = [] { expect(throws([]{ at(-1); })); };
     * };
     * @endcode
     */
    export struct suite
    {
            /// @brief Immediately invokes @p test.
            /// @note Not `constexpr`: suites exist to run at static initialization time.
            template <class Test> suite(Test test)
            {
                test();
            }
    };

    namespace detail
    {
        /**
         * @brief Carrier produced by ut::operator""_test.
         * @tparam Name The test name, captured from the literal.
         * @internal
         */
        template <utility::fixed_string Name> struct test
        {
                /// @brief Runs the test body as a side effect of construction.
                struct run
                {
                        /// @brief Dispatches @p test through the configured runner.
                        /// @param test      The test body.
                        /// @param file_name Defaulted to the caller's file.
                        /// @param line      Defaulted to the caller's line.
                        template <class T>
                        constexpr run(T test, const char *file_name = __builtin_FILE(), int line = __builtin_LINE())
                            : result {[&] {
                                  if consteval
                                  {
                                      return cfg(test).runner.on(events::run {test, file_name, line, Name.data()});
                                  }
                                  else
                                  {
                                      if constexpr (requires { cfg(test).current_test_name; })
                                      {
                                          cfg(test).current_test_name = Name.data();
                                      }
                                      return cfg(test).runner.on(
                                          events::run {test, file_name, line, Name.data(), filter()});
                                  }
                              }()}
                        {
                        }
                        bool result {}; ///< `false` if the test was skipped during constant evaluation.
                };
                /// @brief `"name"_test = []{...}` - assignment is the registration syntax.
                constexpr auto operator=(run test) const
                {
                    return test.result;
                }
        };
    } // namespace detail

    /**
     * @brief Names a test.
     * @tparam Str The test name.
     * @return A carrier to be assigned the test body.
     * @ingroup ut_assertions
     *
     * @code
     * "addition"_test = [] { expect(2_i == 1 + 1); };
     * @endcode
     */
    export template <utility::fixed_string Str> [[nodiscard]] constexpr auto operator""_test()
    {
        return detail::test<Str> {};
    }

    /**
     * @brief Forces @p Expr to be evaluated as a constant expression.
     * @tparam Expr The expression.
     * @ingroup ut_assertions
     *
     * @code
     * expect(constant<42_i == answer()>);
     * @endcode
     */
    export template <auto Expr> inline constexpr auto constant = Expr;

    /**
     * @brief Casts away constness so a captured copy can be mutated in a constexpr test.
     * @tparam T Type of @p t.
     * @param t  The object to strip `const` from.
     * @return A mutable reference to @p t.
     * @ingroup ut_assertions
     *
     * @warning Undefined behaviour if @p t is genuinely `const`. Intended for
     *          by-value lambda captures, which are only `const` because the
     *          closure's call operator is.
     */
    export template <class T> [[nodiscard]] constexpr auto &mut(const T &t)
    {
        return const_cast<T &>(t);
    }

    /**
     * @brief Equality comparison.
     * @tparam TLhs Left operand type.
     * @tparam TRhs Right operand type.
     * @ingroup ut_dsl
     *
     * The primary template exists only to reject mismatched operand types with a
     * readable diagnostic; the partial specializations below implement the
     * comparison for same-type, floating point and container-like operands.
     */
    export template <class TLhs, class TRhs> struct eq
    {
            constexpr eq(const TLhs &lhs, const TRhs &rhs) : lhs {lhs}, rhs {rhs}
            {
            }
            static_assert(type_traits::is_same_v<TLhs, TRhs>, "[ERROR] Comparision of different types is not allowed.");
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return false;
            }
            const TLhs &lhs; ///< Left operand.
            const TRhs &rhs; ///< Right operand.
    };

    /// @brief Equality of two values of the same type.
    template <class T> struct eq<T, T>
    {
            constexpr eq(const T &lhs, const T &rhs) : lhs {lhs}, rhs {rhs}, result {lhs == rhs}
            {
            }
            constexpr friend auto operator<<(auto &os, const eq &expr) -> decltype(auto)
            {
                return (os << expr.lhs << " == " << expr.rhs);
            }
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return result;
            }
            T lhs;
            T rhs;
            bool result {};
    };

    /// @brief Equality of floating point values; requires an explicit epsilon.
    template <class T>
        requires type_traits::is_floating_point_v<T>
    struct eq<T, T>
    {
            constexpr eq(const T &lhs, const T &rhs) : lhs {lhs}, rhs {rhs}
            {
            }
            constexpr friend auto operator<<(auto &os, const eq &expr) -> decltype(auto)
            {
                return (os << expr.lhs << " == " << expr.rhs);
            }
            [[nodiscard]] constexpr explicit operator bool() const
            {
                static_assert(!type_traits::is_floating_point_v<T>,
                              "[ERROR] Epsilon is required - `expect((lhs == rhs)(epsilon))`. For example: "
                              "`expect((4.2_f == sum(4.f, .2f))(.01f))`.");
                return {};
            }
            /// @brief The comparison, closed over a tolerance.
            struct epsilon
            {
                    constexpr epsilon(const T &lhs, const T &rhs, const T e)
                        : lhs {lhs}, rhs {rhs}, e {e}, result {(lhs < rhs ? rhs - lhs : lhs - rhs) < e}
                    {
                    }
                    [[nodiscard]] constexpr explicit operator bool() const
                    {
                        return result;
                    }
                    constexpr friend auto operator<<(auto &os, const epsilon &expr) -> decltype(auto)
                    {
                        return (os << "(" << expr.lhs << " == " << expr.rhs << ")(" << expr.e << ")");
                    }
                    T lhs;
                    T rhs;
                    T e;
                    bool result {};
            };
            /// @brief Supplies the tolerance: `(4.2_d == x)(.01)`.
            [[nodiscard]] constexpr auto operator()(T e) const
            {
                return epsilon {lhs, rhs, e};
            }
            T lhs;
            T rhs;
    };

    /// @brief Element-wise equality for indexable, sized operands.
    template <class TLhs, class TRhs>
        requires requires(TLhs lhs, TRhs rhs) {
            lhs[0];
            rhs[0];
            lhs.size();
            rhs.size();
        }
    struct eq<TLhs, TRhs>
    {
            static_assert(
                type_traits::is_same_v<decltype(utility::declval<TLhs>()[0]), decltype(utility::declval<TLhs>()[0])>,
                "[ERROR] Comparision of different underlying types is not allowed.");
            constexpr eq(const TLhs &lhs, const TRhs &rhs)
                : lhs {lhs}, rhs {rhs}, result {[](const auto &lhs, const auto &rhs) {
                      if (lhs.size() != rhs.size())
                      {
                          return false;
                      }
                      for (decltype(lhs.size()) i {}; i < lhs.size(); ++i)
                      {
                          if (lhs[i] != rhs[i])
                          {
                              return false;
                          }
                      }
                      return true;
                  }(lhs, rhs)}
            {
            }
            constexpr friend auto operator<<(auto &os, const eq &expr) -> decltype(auto)
            {
                return (os << expr.lhs << " == " << expr.rhs);
            }
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return result;
            }
            TLhs lhs;
            TRhs rhs;
            bool result {};
    };

    /**
     * @brief Inequality comparison.
     * @tparam TLhs Left operand type.
     * @tparam TRhs Right operand type.
     * @ingroup ut_dsl
     * @see eq for the specialization structure.
     */
    export template <class TLhs, class TRhs> struct neq
    {
            constexpr neq(const TLhs &lhs, const TRhs &rhs) : lhs {lhs}, rhs {rhs}
            {
            }
            static_assert(type_traits::is_same_v<TLhs, TRhs>, "[ERROR] Comparision of different types is not allowed.");
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return false;
            }
            const TLhs &lhs; ///< Left operand.
            const TRhs &rhs; ///< Right operand.
    };

    /// @brief Inequality of two values of the same type.
    template <class T> struct neq<T, T>
    {
            constexpr neq(const T &lhs, const T &rhs) : lhs {lhs}, rhs {rhs}, result {lhs != rhs}
            {
            }
            constexpr friend auto operator<<(auto &os, const neq &expr) -> decltype(auto)
            {
                return (os << expr.lhs << " != " << expr.rhs);
            }
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return result;
            }
            T lhs;
            T rhs;
            bool result {};
    };

    /// @brief Inequality of floating point values; requires an explicit epsilon.
    template <class T>
        requires type_traits::is_floating_point_v<T>
    struct neq<T, T>
    {
            constexpr neq(const T &lhs, const T &rhs) : lhs {lhs}, rhs {rhs}
            {
            }
            constexpr friend auto operator<<(auto &os, const neq &expr) -> decltype(auto)
            {
                return (os << expr.lhs << " != " << expr.rhs);
            }
            [[nodiscard]] constexpr explicit operator bool() const
            {
                static_assert(!type_traits::is_floating_point_v<T>,
                              "[ERROR] Epsilon is required - `expect((lhs != rhs)(epsilon))`. For example: "
                              "`expect((4.2_f != sum(4.f, .3f))(.01f))`.");
                return {};
            }
            /// @brief The comparison, closed over a tolerance.
            struct epsilon
            {
                    constexpr epsilon(const T &lhs, const T &rhs, const T e)
                        : lhs {lhs}, rhs {rhs}, e {e}, result {(lhs < rhs ? rhs - lhs : lhs - rhs) >= e}
                    {
                    }
                    [[nodiscard]] constexpr explicit operator bool() const
                    {
                        return result;
                    }
                    constexpr friend auto operator<<(auto &os, const epsilon &expr) -> decltype(auto)
                    {
                        return (os << "(" << expr.lhs << " != " << expr.rhs << ")(" << expr.e << ")");
                    }
                    T lhs;
                    T rhs;
                    T e;
                    bool result {};
            };
            /// @brief Supplies the tolerance: `(4.2_d != x)(.01)`.
            [[nodiscard]] constexpr auto operator()(T e) const
            {
                return epsilon {lhs, rhs, e};
            }
            T lhs;
            T rhs;
    };

    /// @brief Greater-than comparison.
    /// @ingroup ut_dsl
    export template <class TLhs, class TRhs> struct gt
    {
            constexpr gt(const TLhs &lhs, const TRhs &rhs) : lhs {lhs}, rhs {rhs}
            {
            }
            static_assert(type_traits::is_same_v<TLhs, TRhs>, "[ERROR] Comparision of different types is not allowed.");
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return false;
            }
            const TLhs &lhs; ///< Left operand.
            const TLhs &rhs; ///< Right operand.
    };

    /// @brief Greater-than of two values of the same type.
    template <class T> struct gt<T, T>
    {
            constexpr gt(const T &lhs, const T &rhs) : lhs {lhs}, rhs {rhs}, result {lhs > rhs}
            {
            }
            constexpr friend auto operator<<(auto &os, const gt &expr) -> decltype(auto)
            {
                return (os << expr.lhs << " > " << expr.rhs);
            }
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return result;
            }
            T lhs;
            T rhs;
            bool result {};
    };

    /// @brief Greater-or-equal comparison.
    /// @ingroup ut_dsl
    export template <class TLhs, class TRhs> struct ge
    {
            constexpr ge(const TLhs &lhs, const TRhs &rhs) : lhs {lhs}, rhs {rhs}
            {
            }
            static_assert(type_traits::is_same_v<TLhs, TRhs>, "[ERROR] Comparision of different types is not allowed.");
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return false;
            }
            const TLhs &lhs; ///< Left operand.
            const TLhs &rhs; ///< Right operand.
    };

    /// @brief Greater-or-equal of two values of the same type.
    template <class T> struct ge<T, T>
    {
            constexpr ge(const T &lhs, const T &rhs) : lhs {lhs}, rhs {rhs}, result {lhs >= rhs}
            {
            }
            constexpr friend auto operator<<(auto &os, const ge &expr) -> decltype(auto)
            {
                return (os << expr.lhs << " >= " << expr.rhs);
            }
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return result;
            }
            T lhs;
            T rhs;
            bool result {};
    };

    /// @brief Less-than comparison.
    /// @ingroup ut_dsl
    export template <class TLhs, class TRhs> struct lt
    {
            constexpr lt(const TLhs &lhs, const TRhs &rhs) : lhs {lhs}, rhs {rhs}
            {
            }
            static_assert(type_traits::is_same_v<TLhs, TRhs>, "[ERROR] Comparision of different types is not allowed.");
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return false;
            }
            const TLhs &lhs; ///< Left operand.
            const TLhs &rhs; ///< Right operand.
    };

    /// @brief Less-than of two values of the same type.
    template <class T> struct lt<T, T>
    {
            constexpr lt(const T &lhs, const T &rhs) : lhs {lhs}, rhs {rhs}, result {lhs < rhs}
            {
            }
            constexpr friend auto operator<<(auto &os, const lt &expr) -> decltype(auto)
            {
                return (os << expr.lhs << " < " << expr.rhs);
            }
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return result;
            }
            T lhs;
            T rhs;
            bool result {};
    };

    /// @brief Less-or-equal comparison.
    /// @ingroup ut_dsl
    export template <class TLhs, class TRhs> struct le
    {
            constexpr le(const TLhs &lhs, const TRhs &rhs) : lhs {lhs}, rhs {rhs}
            {
            }
            static_assert(type_traits::is_same_v<TLhs, TRhs>, "[ERROR] Comparision of different types is not allowed.");
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return false;
            }
            const TLhs &lhs; ///< Left operand.
            const TLhs &rhs; ///< Right operand.
    };

    /// @brief Less-or-equal of two values of the same type.
    template <class T> struct le<T, T>
    {
            constexpr le(const T &lhs, const T &rhs) : lhs {lhs}, rhs {rhs}, result {lhs <= rhs}
            {
            }
            constexpr friend auto operator<<(auto &os, const le &expr) -> decltype(auto)
            {
                return (os << expr.lhs << " <= " << expr.rhs);
            }
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return result;
            }
            T lhs;
            T rhs;
            bool result {};
    };

    /// @brief Logical negation of a single operand.
    /// @tparam T Operand type.
    /// @ingroup ut_dsl
    export template <class T> struct nt
    {
            constexpr nt(const T &t) : t {t}, result {!t}
            {
            }
            constexpr friend auto operator<<(auto &os, const nt &expr) -> decltype(auto)
            {
                return (os << "!" << expr.t);
            }
            [[nodiscard]] constexpr explicit operator bool() const
            {
                return result;
            }
            T t;            ///< The operand.
            bool result {}; ///< The negated value.
    };

    namespace detail
    {
        /// @brief Unwraps a `ut` typed value, or passes a raw value straight through.
        /// @internal
        constexpr decltype(auto) get(const auto &t)
        {
            if constexpr (requires { t.VALUE; })
            {
                return t.VALUE;
            }
            else
            {
                return t;
            }
        }
    } // namespace detail

    /**
     * @namespace ut::dsl
     * @brief Inline namespace holding the operators and typed literals.
     * @ingroup ut_dsl
     *
     * Each operator is constrained on at least one operand exposing `VALUE`, so
     * these overloads never participate in unrelated comparisons.
     */
    inline namespace dsl
    {
        export {

            /// @brief Builds an ut::eq comparison. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator==(const auto &lhs, const auto &rhs)
                requires(requires { lhs.VALUE; } || requires { rhs.VALUE; })
            {
                return eq {detail::get(lhs), detail::get(rhs)};
            }
            /// @brief Builds an ut::neq comparison. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator!=(const auto &lhs, const auto &rhs)
                requires(requires { lhs.VALUE; } || requires { rhs.VALUE; })
            {
                return neq {detail::get(lhs), detail::get(rhs)};
            }
            /// @brief Builds a ut::gt comparison. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator>(const auto &lhs, const auto &rhs)
                requires(requires { lhs.VALUE; } || requires { rhs.VALUE; })
            {
                return gt {detail::get(lhs), detail::get(rhs)};
            }
            /// @brief Builds a ut::ge comparison. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator>=(const auto &lhs, const auto &rhs)
                requires(requires { lhs.VALUE; } || requires { rhs.VALUE; })
            {
                return ge {detail::get(lhs), detail::get(rhs)};
            }
            /// @brief Builds a ut::lt comparison. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator<(const auto &lhs, const auto &rhs)
                requires(requires { lhs.VALUE; } || requires { rhs.VALUE; })
            {
                return lt {detail::get(lhs), detail::get(rhs)};
            }
            /// @brief Builds a ut::le comparison. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator<=(const auto &lhs, const auto &rhs)
                requires(requires { lhs.VALUE; } || requires { rhs.VALUE; })
            {
                return le {detail::get(lhs), detail::get(rhs)};
            }
            /// @brief Negates a typed value in place, preserving its type. @ingroup ut_dsl
            template <class T>
            [[nodiscard]] constexpr auto operator!(const T &t)
                requires requires { t.VALUE; }
            {
                return T {!t.VALUE};
            }

            /// @brief Strongly typed `bool`. @ingroup ut_dsl
            struct _b
            {
                    bool VALUE;
            };
            /// @brief `_b{true}`. @ingroup ut_dsl
            inline constexpr auto true_b = _b {true};
            /// @brief `_b{false}`. @ingroup ut_dsl
            inline constexpr auto false_b = _b {false};
            /// @brief Strongly typed `char`. @ingroup ut_dsl
            struct _c
            {
                    char VALUE {};
            };
            /// @brief Strongly typed `signed char`. @ingroup ut_dsl
            struct _sc
            {
                    signed char VALUE {};
            };
            /// @brief Strongly typed `short`. @ingroup ut_dsl
            struct _s
            {
                    short VALUE {};
                    constexpr auto operator-() const
                    {
                        return _s(-VALUE);
                    }
            };
            /// @brief Strongly typed `int`. @ingroup ut_dsl
            struct _i
            {
                    int VALUE {};
                    constexpr auto operator-() const
                    {
                        return _i(-VALUE);
                    }
            };
            /// @brief Strongly typed `long`. @ingroup ut_dsl
            struct _l
            {
                    long VALUE {};
                    constexpr auto operator-() const
                    {
                        return _l(-VALUE);
                    }
            };
            /// @brief Strongly typed `long long`. @ingroup ut_dsl
            struct _ll
            {
                    long long VALUE {};
                    constexpr auto operator-() const
                    {
                        return _ll(-VALUE);
                    }
            };
            /// @brief Strongly typed `unsigned`. @ingroup ut_dsl
            struct _u
            {
                    unsigned VALUE {};
            };
            /// @brief Strongly typed `unsigned char`. @ingroup ut_dsl
            struct _uc
            {
                    unsigned char VALUE {};
            };
            /// @brief Strongly typed `unsigned short`. @ingroup ut_dsl
            struct _us
            {
                    unsigned short VALUE {};
            };
            /// @brief Strongly typed `unsigned long`. @ingroup ut_dsl
            struct _ul
            {
                    unsigned long VALUE {};
            };
            /// @brief Strongly typed `unsigned long long`. @ingroup ut_dsl
            struct _ull
            {
                    unsigned long long VALUE {};
            };
            /// @brief Strongly typed `float`; comparisons require an epsilon. @ingroup ut_dsl
            struct _f
            {
                    float VALUE {};
                    constexpr auto operator-() const
                    {
                        return _f(-VALUE);
                    }
            };
            /// @brief Strongly typed `double`; comparisons require an epsilon. @ingroup ut_dsl
            struct _d
            {
                    double VALUE {};
                    constexpr auto operator-() const
                    {
                        return _d(-VALUE);
                    }
            };
            /// @brief Strongly typed `long double`; comparisons require an epsilon. @ingroup ut_dsl
            struct _ld
            {
                    long double VALUE {};
                    constexpr auto operator-() const
                    {
                        return _ld(-VALUE);
                    }
            };
            /// @brief Strongly typed `int8_t`. @ingroup ut_dsl
            struct _i8
            {
                    __INT8_TYPE__ VALUE {};
                    constexpr auto operator-() const
                    {
                        return _i8(-VALUE);
                    }
            };
            /// @brief Strongly typed `int16_t`. @ingroup ut_dsl
            struct _i16
            {
                    __INT16_TYPE__ VALUE {};
                    constexpr auto operator-() const
                    {
                        return _i16(-VALUE);
                    }
            };
            /// @brief Strongly typed `int32_t`. @ingroup ut_dsl
            struct _i32
            {
                    __INT32_TYPE__ VALUE {};
                    constexpr auto operator-() const
                    {
                        return _i32(-VALUE);
                    }
            };
            /// @brief Strongly typed `int64_t`. @ingroup ut_dsl
            struct _i64
            {
                    __INT64_TYPE__ VALUE {};
                    constexpr auto operator-() const
                    {
                        return _i64(-VALUE);
                    }
            };
            /// @brief Strongly typed `uint8_t`. @ingroup ut_dsl
            struct _u8
            {
                    __UINT8_TYPE__ VALUE {};
            };
            /// @brief Strongly typed `uint16_t`. @ingroup ut_dsl
            struct _u16
            {
                    __UINT16_TYPE__ VALUE {};
            };
            /// @brief Strongly typed `uint32_t`. @ingroup ut_dsl
            struct _u32
            {
                    __UINT32_TYPE__ VALUE {};
            };
            /// @brief Strongly typed `uint64_t`. @ingroup ut_dsl
            struct _u64
            {
                    __UINT64_TYPE__ VALUE {};
            };

            /// @brief Strongly typed string, compared element-wise.
            /// @ingroup ut_dsl
            struct _string
            {
                    /// @brief Non-owning character range.
                    struct view
                    {
                            /// @brief Character at index @p i (unchecked).
                            [[nodiscard]] constexpr auto operator[](auto i) const
                            {
                                return data_[i];
                            }
                            /// @brief Number of characters, excluding the terminating NUL.
                            [[nodiscard]] constexpr auto size() const
                            {
                                return size_;
                            }
                            /// @brief Streams the underlying characters.
                            constexpr friend auto operator<<(auto &os, const view &v) -> decltype(auto)
                            {
                                return (os << v.data_);
                            }
                            /// @brief Element-wise equality.
                            [[nodiscard]] constexpr auto operator==(const view &other) const -> bool
                            {
                                if (size() != other.size())
                                {
                                    return false;
                                }
                                for (decltype(size()) i {}; i < size(); ++i)
                                {
                                    if ((*this)[i] != other[i])
                                    {
                                        return false;
                                    }
                                }
                                return true;
                            }
                            const char *data_ {}; ///< First character; not owned.
                            unsigned size_ {};    ///< Character count.
                    } VALUE {};
            };

            /// @brief `42_i` -> ut::_i. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_i(unsigned long long int value)
            {
                return _i(value);
            }
            /// @brief `42_s` -> ut::_s. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_s(unsigned long long int value)
            {
                return _s(value);
            }
            /// @brief `42_c` -> ut::_c. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_c(unsigned long long int value)
            {
                return _c(value);
            }
            /// @brief `42_sc` -> ut::_sc. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_sc(unsigned long long int value)
            {
                return _sc(value);
            }
            /// @brief `42_l` -> ut::_l. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_l(unsigned long long int value)
            {
                return _l(value);
            }
            /// @brief `42_ll` -> ut::_ll. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_ll(unsigned long long int value)
            {
                return _ll(value);
            }
            /// @brief `42_u` -> ut::_u. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_u(unsigned long long int value)
            {
                return _u(value);
            }
            /// @brief `42_uc` -> ut::_uc. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_uc(unsigned long long int value)
            {
                return _uc(value);
            }
            /// @brief `42_us` -> ut::_us. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_us(unsigned long long int value)
            {
                return _us(value);
            }
            /// @brief `42_ul` -> ut::_ul. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_ul(unsigned long long int value)
            {
                return _ul(value);
            }
            /// @brief `42_ull` -> ut::_ull. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_ull(unsigned long long int value)
            {
                return _ull(value);
            }
            /// @brief `4.2_f` -> ut::_f. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_f(long double value)
            {
                return _f(value);
            }
            /// @brief `4.2_d` -> ut::_d. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_d(long double value)
            {
                return _d(value);
            }
            /// @brief `4.2_ld` -> ut::_ld. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_ld(long double value)
            {
                return _ld(value);
            }
            /// @brief `42_i8` -> ut::_i8. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_i8(unsigned long long int value)
            {
                return _i8(value);
            }
            /// @brief `42_i16` -> ut::_i16. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_i16(unsigned long long int value)
            {
                return _i16(value);
            }
            /// @brief `42_i32` -> ut::_i32. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_i32(unsigned long long int value)
            {
                return _i32(value);
            }
            /// @brief `42_i64` -> ut::_i64. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_i64(unsigned long long int value)
            {
                return _i64(value);
            }
            /// @brief `42_u8` -> ut::_u8. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_u8(unsigned long long int value)
            {
                return _u8(value);
            }
            /// @brief `42_u16` -> ut::_u16. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_u16(unsigned long long int value)
            {
                return _u16(value);
            }
            /// @brief `42_u32` -> ut::_u32. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_u32(unsigned long long int value)
            {
                return _u32(value);
            }
            /// @brief `42_u64` -> ut::_u64. @ingroup ut_dsl
            [[nodiscard]] constexpr auto operator""_u64(unsigned long long int value)
            {
                return _u64(value);
            }
            /// @brief `"foo"_s` -> ut::_string. @ingroup ut_dsl
            template <utility::fixed_string Str> [[nodiscard]] constexpr auto operator""_s()
            {
                return _string {Str.data(), Str.size() - 1u};
            }

        } // export
    } // namespace dsl
} // namespace ut

// -*- mode: c++; -*-
// vim: set filetype=cpp:
