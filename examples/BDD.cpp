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
    using namespace ut::literals;
    using namespace ut::operators::terse;
    using namespace ut::bdd;
    using namespace ut;

    "Scenario"_test = [] {
        given("I have...") = [] {
            when("I run...") = [] {
                then("I should have...") = [] { expect(1_u == 1u); };
                then("I should have...") = [] { expect(1u == 1_u); };
            };
        };
    };
    feature("Calculator") = [] {
        scenario("Addition") = [] {
            given("I have number 40") = [] {
                auto number = 40;
                when("I add 2 to number") = [&number] { number += 2; };
                then("I expect number to be 42") = [&number] { expect(42_i == number); };
            };
        };
    };
    // clang-format off
  scenario("Addition");
    given("I have number 40");
      auto number = 40;
    when("I add 2 to number");
      number += 2;
    then("I expect number to be 42");
      expect(42_i == number);
    // clang-format on
}
