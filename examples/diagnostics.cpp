// examples/07_diagnostics.cpp
//
// The errors the library is designed to produce. Every case below is a *compile
// error* by design, so each is behind its own flag and the file builds clean
// with none of them set.
//
// Enable one at a time to see the diagnostic:
//
//   cmake --build build --target example_07_diagnostics -- \
//     CXXFLAGS=-DUT_SHOW_MISMATCHED_TYPES
//
// or directly:
//
//   clang++ -std=c++23 -fmodule-file=ut=ut.pcm -DUT_SHOW_BARE_BOOL \
//     -c 07_diagnostics.cpp
//
// Available flags:
//   UT_SHOW_LEAK              memory leaked during constant evaluation
//   UT_SHOW_UB                undefined behaviour during constant evaluation
//   UT_SHOW_MISMATCHED_TYPES  comparison across different types
//   UT_SHOW_BARE_BOOL         expect() on something already collapsed to bool
//   UT_SHOW_MISSING_EPSILON   floating point comparison without a tolerance
//   UT_SHOW_NOT_CONSTANT      constant<> on a non-constant expression
//   UT_SHOW_FAILING_ASSERT    an ordinary assertion that does not hold
import std;
import ut;

constexpr auto answer() { return 42; }

int main() {
  using namespace ut;

  // -------------------------------------------------------------------------
  // Leak and UB detection.
  //
  // These are not features the library implements — they fall out of running
  // the test inside the constant evaluator, which refuses to leave an
  // allocation dangling or to read through a null pointer. Note that they only
  // apply to tests that actually run at compile time: mark either of these
  // `mutable` and the compiler will happily let it through.
  // -------------------------------------------------------------------------
  #ifdef UT_SHOW_LEAK
  "leak"_test = [] {
    new int;  // allocation never freed
  };
  #endif

  #ifdef UT_SHOW_UB
  "ub"_test = [] {
    int* p{};
    *p = 42;  // null dereference
  };
  #endif

  // -------------------------------------------------------------------------
  // "Comparision of different types is not allowed."
  //
  // The library will not compare an int against a short for you. Widen at the
  // call site so the conversion is visible in the source.
  // -------------------------------------------------------------------------
  #ifdef UT_SHOW_MISMATCHED_TYPES
  "mismatched types"_test = [] {
    expect(42_i == short(42));
  };
  #endif

  // -------------------------------------------------------------------------
  // "Expression required - `expect(lhs == rhs)`."
  //
  // `42 == answer()` is a bool by the time expect() sees it, and a bool has
  // nothing to print on failure beyond "false". Tag one operand.
  // -------------------------------------------------------------------------
  #ifdef UT_SHOW_BARE_BOOL
  "bare bool"_test = [] {
    expect(42 == answer());
  };
  #endif

  // -------------------------------------------------------------------------
  // "Epsilon is required - `expect((lhs == rhs)(epsilon))`."
  // -------------------------------------------------------------------------
  #ifdef UT_SHOW_MISSING_EPSILON
  "missing epsilon"_test = [] {
    expect(4.2 == 4.2_d);
  };
  #endif

  // -------------------------------------------------------------------------
  // constant<> on something that is not a constant expression.
  // -------------------------------------------------------------------------
  #ifdef UT_SHOW_NOT_CONSTANT
  "not constant"_test = [] {
    auto i = 0;
    expect(constant<i == 42_i>);
  };
  #endif

  // -------------------------------------------------------------------------
  // An ordinary failing assertion.
  //
  // Worth seeing once: because this test is eligible for the compile-time pass,
  // it fails at compile time, and you never get a run-time report at all. The
  // diagnostic points at the static_assert inside the runner, with a note
  // naming the test and the assertion. Add `mutable` to move it to run time and
  // get the usual "FAILED:" line instead.
  // -------------------------------------------------------------------------
  #ifdef UT_SHOW_FAILING_ASSERT
  "failing assert"_test = [] {
    expect(43_i == answer());
  };
  #endif

  // Something that always compiles, so the file is a valid program with no
  // flags set.
  "baseline"_test = [] {
    expect(42_i == answer());
  };
}
