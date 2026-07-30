// examples/08_filtering.cpp
//
// UT_FILTER restricts which tests run *at run time*. The compile-time pass is
// unaffected — it already happened, at compile time, before any environment
// variable existed.
//
//   ./example_08_filtering                      # all four
//   UT_FILTER='parser'          ./example_...   # exactly "parser"
//   UT_FILTER='parser*'         ./example_...   # "parser", "parser errors", …
//   UT_FILTER='*errors'         ./example_...   # anything ending in "errors"
//   UT_FILTER='*error*'         ./example_...   # anything containing "error"
//   UT_FILTER='lexer?'          ./example_...   # "lexer" + exactly one char
//
// The pattern is a glob matched against the whole test name: `*` matches any
// run of characters, `?` matches exactly one. It is not a regular expression,
// despite what the upstream documentation says — `UT_FILTER='parser.*'` will
// match nothing, because `.` is a literal dot here.
//
// Note that filtering changes the run-time tallies but not the compile-time
// one: a filtered-out test still ran at compile time and is still counted
// there. Run with UT_FILTER='parser' and you get
//
//   PASSED: tests: 1 (1 passed, 0 failed, 4 compile-time), asserts: 1 (1 passed, 0 failed)
//
// which is the honest answer — the other three really were verified, just not
// at run time.
import std;
import ut;

constexpr auto tokens(int n) { return n; }

int main() {
  using namespace ut;

  "parser"_test = [] {
    expect(3_i == tokens(3));
  };

  "parser errors"_test = [] {
    expect(0_i == tokens(0));
  };

  "lexer"_test = [] {
    expect(1_i == tokens(1));
  };

  "lexer errors"_test = [] {
    expect(2_i == tokens(2));
  };
}
