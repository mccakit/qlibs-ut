// examples/03_assertions.cpp
//
// Every form of assertion the library supports, and the idioms that work for
// each family of types.
//
// Expected output:
//   PASSED: tests: 7 (7 passed, 0 failed, 7 compile-time), asserts: 23 (23 passed, 0 failed)
import std;
import ut;

constexpr auto answer() { return 42; }
constexpr auto pi() { return 3.14159; }
constexpr auto is_even(int i) { return i % 2 == 0; }

int main() {
  using namespace ut;

  // -------------------------------------------------------------------------
  // Four spellings of the same assertion. Pick whichever reads best; they
  // produce identical diagnostics.
  // -------------------------------------------------------------------------
  "spellings"_test = [] {
    expect(42_i == answer());       // literal on the left
    expect(answer() == 42_i);       // literal on the right
    expect(_i(42) == answer());     // explicit constructor instead of a literal
    expect(eq(42, answer()));       // the comparison type by name
  };

  // -------------------------------------------------------------------------
  // Messages are streamed only when the assertion fails, so attaching context
  // costs nothing on the happy path. `<<` chains.
  // -------------------------------------------------------------------------
  "messages"_test = [] {
    expect(42_i == answer()) << "not printed, because this passes";
    expect(42_i == answer()) << "context: " << answer() << " is the answer";
  };

  // -------------------------------------------------------------------------
  // Relational comparisons.
  // -------------------------------------------------------------------------
  "relational"_test = [] {
    expect(answer() > 41_i);
    expect(answer() >= 42_i);
    expect(answer() < 43_i);
    expect(answer() <= 42_i);
    expect(answer() != 43_i);
  };

  // -------------------------------------------------------------------------
  // Booleans.
  //
  // `expect` needs both operands, so it rejects anything that has already
  // collapsed to `bool`. That means a bare predicate has to be compared
  // against `true_b` / `false_b`, or negated with `nt`.
  //
  //   expect(is_even(4));              // will not compile: bare bool
  //   expect(true_b);                  // will not compile: _b has no operator bool
  //   expect(!false_b);                // will not compile: yields _b, see above
  // -------------------------------------------------------------------------
  "booleans"_test = [] {
    expect(true_b == is_even(4));
    expect(false_b == is_even(3));
    expect(nt(is_even(3)));             // "not even"
  };

  // -------------------------------------------------------------------------
  // Floating point.
  //
  // Comparing floats without a tolerance is a compile error. Call the
  // comparison to supply one:
  //
  //   expect(pi() == 3.14159_d);       // will not compile: Epsilon is required
  // -------------------------------------------------------------------------
  "floating point"_test = [] {
    expect((pi() == 3.14159_d)(.00001));
    expect((pi() != 3.14_d)(.001));     // differs by more than .001
    expect((4.2f == 4.2_f)(.01f));
  };

  // -------------------------------------------------------------------------
  // Strings.
  //
  // `"..."_s` produces a `_string`, compared element-wise. Both operands must
  // be `_string` — there is no comparison against a raw `const char*`, because
  // a pointer has no `.size()` to compare against:
  //
  //   constexpr const char* p = "ut";
  //   expect("ut"_s == p);             // will not compile: different types
  // -------------------------------------------------------------------------
  "strings"_test = [] {
    expect("ut"_s == "ut"_s);
    expect("ut"_s != "boost.ut"_s);
    expect(""_s == ""_s);
  };

  // -------------------------------------------------------------------------
  // `constant<...>` forces an expression to be evaluated as a constant
  // expression, and fails to compile if it cannot be. Useful for pinning down
  // that something really is constexpr-callable, independently of whether the
  // enclosing test happens to run at compile time.
  //
  //   auto i = 0;
  //   expect(constant<i == 42_i>);     // will not compile: i is not constant
  // -------------------------------------------------------------------------
  "constant"_test = [] {
    expect(constant<42_i == answer()>);
    expect(constant<42 == 42_i>);
    expect(constant<(3.14159 == 3.14159_d)(.00001)>);
  };
}
