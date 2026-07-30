// examples/02_execution_model.cpp
//
// Which passes a test participates in is decided entirely by the shape of its
// lambda. `where()` below reports which evaluator is running it, so each test
// can assert on where it ended up.
//
// Expected output:
//   PASSED: tests: 4 (4 passed, 0 failed, 2 compile-time), asserts: 4 (4 passed, 0 failed)
//
// Four tests run at run time; only the two that are neither `mutable` nor
// capturing also ran in the compile-time pass. The static_assert test at
// namespace scope is not counted at all — it never reaches the reporter.
import std;
import ut;

using namespace ut;

constexpr auto where() {
  if consteval { return 42; } else { return 87; }
}

// ---------------------------------------------------------------------------
// Compile-time only, at a point you choose.
//
// Wrapping the registration in a static_assert is the portable way to say
// "this test must pass, and it must pass now, during compilation". Nothing is
// emitted into the binary and nothing is counted at run time.
// ---------------------------------------------------------------------------
static_assert(("compile-time only, via static_assert"_test = [] {
  expect(42_i == where());
}));

int main() {
  // -------------------------------------------------------------------------
  // Both passes. The default.
  // -------------------------------------------------------------------------
  "both passes"_test = [] {
    expect(2_i == 1 + 1);
  };

  // `constexpr` on the lambda changes nothing here — a plain lambda is already
  // implicitly constexpr when its body allows it. It is only worth writing when
  // you want the compiler to tell you that the body *cannot* be
  // constant-evaluated.
  "both passes, explicitly constexpr"_test = [] constexpr {
    expect(2_i == 1 + 1);
  };

  // -------------------------------------------------------------------------
  // Run-time only.
  //
  // `mutable` is the escape hatch: it excludes the test from the compile-time
  // pass. Reach for it when the body cannot be constant-evaluated — I/O, a C
  // API, a container your standard library has not made constexpr yet, or a
  // compiler limitation you are working around.
  // -------------------------------------------------------------------------
  "run-time only"_test = [] mutable {
    expect(87_i == where());
  };

  // -------------------------------------------------------------------------
  // A capturing lambda is also excluded from the compile-time pass, because the
  // capture is not a constant expression at the point of registration. You do
  // not need `mutable` for this — capturing is enough.
  // -------------------------------------------------------------------------
  const auto expected = 87;
  "run-time only, because it captures"_test = [expected] {
    expect(_i(expected) == where());
  };

  // -------------------------------------------------------------------------
  // Compile-time only, via a consteval lambda.
  //
  // Guarded because this relies on C++23 consteval propagation (P2564) to
  // escalate the runner's call chain to an immediate function. If your compiler
  // has not implemented that, use the static_assert form above instead — it is
  // equivalent and portable.
  // -------------------------------------------------------------------------
  #ifdef UT_EXAMPLE_CONSTEVAL_LAMBDA
  "compile-time only, via consteval lambda"_test = [] consteval {
    expect(42_i == where());
  };
  #endif
}
