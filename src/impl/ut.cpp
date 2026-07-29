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
 * @file ut.cpp
 * @brief Module implementation unit of `ut`.
 *
 * `ut` is a compile-time-first library, so nearly everything it offers is a
 * template or a `constexpr` entity and must be *reachable* to importers -
 * which means it has to live in the interface unit (ut.cppm). Definitions
 * placed here are visible only to other translation units of module `ut`, so
 * this file deliberately holds just two kinds of thing:
 *
 * 1. The handful of genuinely non-`constexpr`, run-time-only helpers declared
 *    in `ut::detail`. Keeping them here is what allows the interface to be
 *    written without `<cstdlib>` and without compiler builtins for process
 *    termination.
 * 2. The library's own self-verification suite. In the single-header original
 *    these `static_assert`s were re-checked by every translation unit that
 *    included the header; as part of a module they are checked exactly once,
 *    when the module is built.
 *
 * @note This unit implicitly imports `ut`, which is why it can reach the
 *       non-exported `ut::type_traits`, `ut::utility` and `ut::detail`
 *       namespaces exercised below.
 *
 * @see ut.cppm for the module interface and the full documentation.
 *
 * @author Kris Jusiak <kris@jusiak.net>
 * @copyright MIT License
 */

module;

#include <cstdlib> // std::getenv, std::abort

module ut;

namespace ut::detail
{

    /**
     * @copydoc ut::detail::failed
     *
     * The definition is intentionally empty. What makes a compile-time
     * expectation fail is that this function is not `constexpr`, so calling it
     * during constant evaluation is ill-formed and the compiler emits a
     * diagnostic. The body exists only so that the declaration can never become
     * an unresolved symbol if a compiler ever emits the immediate-function branch
     * of an `if consteval` into run-time code.
     */
    void failed()
    {
    }

    /**
     * @copydoc ut::detail::filter
     */
    [[nodiscard]] auto filter() -> const char *
    {
        return std::getenv("UT_FILTER");
    }

    /**
     * @copydoc ut::detail::abort
     */
    [[noreturn]] void abort()
    {
        std::abort();
    }

} // namespace ut::detail

#ifndef NTEST
/**
 * @cond UT_SELF_TEST
 * Self-verification of the library's own internals. Excluded from the
 * generated documentation; disable the check entirely with `-DNTEST`.
 */
static_assert((
    [] {
        // ut::type_traits::is_same_v
        {
            static_assert(!ut::type_traits::is_same_v<int, void>);
            static_assert(!ut::type_traits::is_same_v<void, int>);
            static_assert(!ut::type_traits::is_same_v<void *, int>);
            static_assert(!ut::type_traits::is_same_v<int, const int>);
            static_assert(ut::type_traits::is_same_v<void, void>);
            static_assert(ut::type_traits::is_same_v<int, int>);
        }

        // ut::type_traits::is_mutable_lambda_v
        {
            auto l1 = []() {};
            auto l2 = []() constexpr {};
            auto l3 = []() mutable {};
            auto l4 = []() mutable constexpr {};
            static_assert(!ut::type_traits::is_mutable_lambda_v<decltype(&decltype(l1)::operator())>);
            static_assert(!ut::type_traits::is_mutable_lambda_v<decltype(&decltype(l2)::operator())>);
            static_assert(ut::type_traits::is_mutable_lambda_v<decltype(&decltype(l3)::operator())>);
            static_assert(ut::type_traits::is_mutable_lambda_v<decltype(&decltype(l4)::operator())>);
        }

        // ut::type_traits::has_capture_lambda_v
        {
            int i {};
            auto l1 = []() { return 42; };
            auto l2 = []() constexpr { return 42; };
            auto l3 = []() mutable { return 42; };
            auto l4 = [&i]() constexpr { return i; };
            auto l5 = [i]() mutable { return i; };
            auto l6 = [=]() mutable constexpr { return i; };
            static_assert(!ut::type_traits::has_capture_lambda_v<decltype(l1)>);
            static_assert(!ut::type_traits::has_capture_lambda_v<decltype(l2)>);
            static_assert(!ut::type_traits::has_capture_lambda_v<decltype(l3)>);
            static_assert(ut::type_traits::has_capture_lambda_v<decltype(l4)>);
            static_assert(ut::type_traits::has_capture_lambda_v<decltype(l5)>);
            static_assert(ut::type_traits::has_capture_lambda_v<decltype(l6)>);
        }

        // ut::type_traits::is_floating_point_v
        {
            static_assert(!ut::type_traits::is_floating_point_v<int>);
            static_assert(!ut::type_traits::is_floating_point_v<bool>);
            static_assert(!ut::type_traits::is_floating_point_v<char>);
            static_assert(!ut::type_traits::is_floating_point_v<void>);
            static_assert(ut::type_traits::is_floating_point_v<float>);
            static_assert(ut::type_traits::is_floating_point_v<double>);
            static_assert(ut::type_traits::is_floating_point_v<long double>);
        }

        // ut::utility::fixed_string
        {
            static_assert(sizeof("") == ut::utility::fixed_string {""}.size());
            static_assert(sizeof("foo") == ut::utility::fixed_string {"foo"}.size());
            static_assert('f' == ut::utility::fixed_string {"foo"}[0]);
            static_assert('o' == ut::utility::fixed_string {"foo"}[1]);
            static_assert('o' == ut::utility::fixed_string {"foo"}[2]);
            static_assert('\0' == ut::utility::fixed_string {"foo"}[3]);
        }

        // ut::utility::match
        {
            static_assert(ut::utility::match("", ""));
            static_assert(not ut::utility::match("", "foo"));
            static_assert(ut::utility::match("*", ""));
            static_assert(ut::utility::match("*", "foo"));
            static_assert(ut::utility::match("*", "bar"));
            static_assert(ut::utility::match("foo", "foo"));
            static_assert(not ut::utility::match("foo", "bar"));
            static_assert(not ut::utility::match("fo", "foo"));
            static_assert(ut::utility::match("foo*", "foo"));
            static_assert(ut::utility::match("foo*", "foo1"));
            static_assert(ut::utility::match("foo*", "foo2"));
            static_assert(ut::utility::match("foo*", "foo23"));
            static_assert(ut::utility::match("foo?", "foo2"));
            static_assert(ut::utility::match("foo?", "foo1"));
            static_assert(ut::utility::match("foo??", "foo12"));
            static_assert(ut::utility::match("foo?x", "foo1x"));
            static_assert(ut::utility::match("foo?x", "foo2x"));
            static_assert(not ut::utility::match("fo?xx", "fooxxx"));
        }

        // ut::detail::get
        {
            static_assert(42 == ut::detail::get(42));
            static_assert(42u == ut::detail::get(ut::_u(42)));
        }

        // ut::eq
        {
            using namespace ut;
            static_assert(eq(42, 42));
            static_assert(!eq(43, 42));
            static_assert(!eq(42, 43));

            static_assert((eq(4.2, 4.2))(.1));
            static_assert((eq(4.24, 4.23))(.01));
            static_assert(!(eq(4.24, 4.23))(.001));

            static_assert(eq("foo"_s, "foo"_s));
            static_assert(!eq("foo"_s, "bar"_s));
            static_assert(!eq(""_s, "foo"_s));
            static_assert(!eq("bar"_s, ""_s));
        }

        // ut::neq
        {
            using namespace ut;
            static_assert(neq(42, 43));
            static_assert(neq(43, 42));
            static_assert(!neq(42, 42));

            static_assert(!(neq(4.2, 4.2))(.1));
            static_assert(!(neq(4.24, 4.23))(.01));
            static_assert((neq(4.24, 4.23))(.001));
        }

        // ut::gt
        {
            using namespace ut;
            static_assert(gt(43, 42));
            static_assert(!gt(42, 43));
            static_assert(!gt(42, 42));
        }

        // ut::ge
        {
            using namespace ut;
            static_assert(ge(43, 42));
            static_assert(ge(43, 43));
            static_assert(!ge(42, 43));
        }

        // ut::lt
        {
            using namespace ut;
            static_assert(lt(42, 43));
            static_assert(!lt(43, 42));
            static_assert(!lt(42, 42));
        }

        // ut::le
        {
            using namespace ut;
            static_assert(le(42, 43));
            static_assert(le(42, 42));
            static_assert(!le(43, 42));
        }

        // ut::nt
        {
            using namespace ut;
            static_assert(nt(false));
            static_assert(!nt(true));
        }

        using sc = signed char;
        using uc = unsigned char;
        using ul = unsigned long;
        using us = unsigned short;
        using ull = unsigned long long;
        using ll = long long;
        using ld = long double;

        // ut::_
        {
            using namespace ut;

            static_assert(_b {true}.VALUE);
            static_assert(!_b {false}.VALUE);
            static_assert(true_b.VALUE);
            static_assert(!false_b.VALUE);
            static_assert(char('0') == _c {'0'}.VALUE);
            static_assert(sc(42) == _sc {42}.VALUE);
            static_assert(short(-42) == -_s {42}.VALUE);
            static_assert(int(-42) == -_i {42}.VALUE);
            static_assert(long(-42) == -_l {42}.VALUE);
            static_assert(ll(-42) == -_ll {42}.VALUE);
            static_assert(unsigned(42) == _u {42}.VALUE);
            static_assert(uc(42) == _uc {42}.VALUE);
            static_assert(us(42) == _us {42}.VALUE);
            static_assert(ul(42) == _ul {42}.VALUE);
            static_assert(ull(42) == _ull {42}.VALUE);
            static_assert(-_f {4.2}.VALUE < float(0));
            static_assert(-_d {4.2}.VALUE < double(0));
            static_assert(-_ld {4.2}.VALUE < ld(0));

            static_assert((__INT8_TYPE__)(-42) == -_i8 {42}.VALUE);
            static_assert((__INT16_TYPE__)(-42) == -_i16 {42}.VALUE);
            static_assert((__INT32_TYPE__)(-42) == -_i32 {42}.VALUE);
            static_assert((__INT64_TYPE__)(-42) == -_i64 {42}.VALUE);
            static_assert((__UINT8_TYPE__)(42) == _u8 {42}.VALUE);
            static_assert((__UINT16_TYPE__)(42) == _u16 {42}.VALUE);
            static_assert((__UINT32_TYPE__)(42) == _u32 {42}.VALUE);
            static_assert((__UINT64_TYPE__)(42) == _u64 {42}.VALUE);

            static_assert(sizeof("foo") == _string::view {"foo", sizeof("foo")}.size());
            static_assert('f' == _string::view {"foo", sizeof("foo")}[0]);
            static_assert('o' == _string::view {"foo", sizeof("foo")}[1]);
            static_assert('o' == _string::view {"foo", sizeof("foo")}[2]);
        }

        // ut::operator""_*
        {
            using namespace ut;
            static_assert(int(-42) == -42_i);
            static_assert(short(-42) == -42_s);
            static_assert(char(0xA) == 0xA_c);
            static_assert(sc(0xA) == 0xA_sc);
            static_assert(long(-42) == -42_l);
            static_assert(ll(-42) == -42_ll);
            static_assert(unsigned(42) == 42_u);
            static_assert(uc(42) == 42_uc);
            static_assert(us(42) == 42_us);
            static_assert(ul(42) == 42_ul);
            static_assert(ull(42) == 42_ull);
            static_assert(-4.2_f < float(0));
            static_assert(-4.2_d < double(0));
            static_assert(-4.2_ld < ld(0));
            static_assert((-4.2f == -4.2_f)(.1));
            static_assert((.1234f == .1234_f)(.0001));
            static_assert((.13f == .12_f)(.1));
            static_assert(!(.13f == .12_f)(.001));
            static_assert((-9.12345678 == -9.12345678_d)(.00001));

            static_assert((__INT8_TYPE__)(-42) == -42_i8);
            static_assert((__INT16_TYPE__)(-42) == -42_i16);
            static_assert((__INT32_TYPE__)(-42) == -42_i32);
            static_assert((__INT64_TYPE__)(-42) == -42_i64);
            static_assert((__UINT8_TYPE__)(42) == 42_u8);
            static_assert((__UINT16_TYPE__)(42) == 42_u16);
            static_assert((__UINT32_TYPE__)(42) == 42_u32);
            static_assert((__UINT64_TYPE__)(42) == 42_u64);

            static_assert(sizeof("") - 1u == ""_s.VALUE.size());
            static_assert(sizeof("foo") - 1u == "foo"_s.VALUE.size());
            static_assert('f' == "foo"_s.VALUE[0]);
            static_assert('o' == "foo"_s.VALUE[1]);
            static_assert('o' == "foo"_s.VALUE[2]);
        }

        // ut::constant
        {
            using namespace ut;
            static_assert(constant<42 == 42>);
            static_assert(constant<42 == 42_i>);
            static_assert(constant<42_i == 42>);
        }

        // ut::mut
        {
            static_assert(43 == [] {
                using namespace ut;
                auto i = 42;
                return [=] {
                    mut(i) = 43;
                    return i;
                }();
            }());
        }
    }(),
    true));
/// @endcond
#endif // NTEST

// -*- mode: c++; -*-
// vim: set filetype=cpp:
