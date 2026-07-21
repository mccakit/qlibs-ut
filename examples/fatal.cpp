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
    using namespace ut::literals;
    using ut::fatal;

    "fatal"_test = [] {
        using namespace ut::operators;
        using ut::expect;

        std::optional<int> o {42};
        expect(fatal(o.has_value()));
        expect(*o == 42_i);
    };

    "fatal logging"_test = [] {
        using namespace ut::operators;
        using ut::expect;

        std::optional<int> o {42};
        expect(o.has_value()) << "log messages...." << fatal;
        expect(*o == 42_i);
    };

    "fatal matcher"_test = [] {
        using namespace ut::operators;
        using ut::expect;
        using ut::that;

        std::optional<int> o {42};
        expect(fatal(that % o.has_value()) and that % *o == 42);
    };

    "fatal terse"_test = [] {
        using namespace ut::operators::terse;

        std::optional<int> o {42};
        (fatal(o.has_value()) and *o == 42_i);
    };

    using namespace ut::operators;
    using ut::expect;

    std::vector v {1u};
    expect(fatal(std::size(v) == 1_ul));
    expect(v[0] == 1_u);
}
