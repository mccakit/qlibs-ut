//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
import std;
import ut;
int main()
{
    using ut::operator""_test;
    using ut::operator""_i;

    "using"_test = [] {
        using ut::expect;

        using ut::eq;
        expect(eq(42, 42));

        using ut::operator==;
        expect(42_i == 42);

        using ut::operator and;
        using ut::that;
        expect(that % 1 == 1 and that % 2 == 2);
    };
}
