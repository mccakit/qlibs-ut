// examples/04_suites_and_subtests.cpp
//
// Suites run before main; nested tests are just registrations inside a test
// body; `mut` lets a captured copy be modified.
//
// Expected output:
//   PASSED: tests: 4 (4 passed, 0 failed, 2 compile-time), asserts: 7 (7 passed, 0 failed)

// Standard headers go *before* `import ut;`. Including after an import can be
// rejected by Clang when the module and the header declare overlapping names —
// see "Known issues" in the manual.
import std;
import ut;

using namespace ut;

// ---------------------------------------------------------------------------
// A suite runs its body during static initialization, before main. This is how
// you spread tests across translation units without touching main.
// ---------------------------------------------------------------------------
const suite vector_suite = [] {
  "vector"_test = [] {
    std::vector<int> v(5);
    expect(v.size() == 5_ul);       // _ul assumes size_t is unsigned long (LP64)
    expect(v.capacity() >= 5_ul);

    // -----------------------------------------------------------------------
    // A nested test is just a registration inside the enclosing body. It gets
    // its own name in the report and its own pass/fail accounting.
    //
    // Capturing by copy gives each sub-test an independent starting state, so
    // sibling sub-tests cannot interfere with each other. The cost is that a
    // capturing lambda is excluded from the compile-time pass.
    // -----------------------------------------------------------------------
    "resizing bigger changes size and capacity"_test = [=] {
      mut(v).resize(10);
      expect(v.size() == 10_ul);
      expect(v.capacity() >= 10_ul);
    };

    // The outer `v` is untouched by the sub-test above, because the sub-test
    // mutated its own copy.
    expect(v.size() == 5_ul);
  };
};

int main() {
  // -------------------------------------------------------------------------
  // What `mut` is for.
  //
  // A lambda's `operator()` is const, so anything captured by copy is const
  // when you look at it from inside the body. `mut` casts that away. It is a
  // const_cast, so applying it to something that is genuinely const — rather
  // than merely observed through a closure — is undefined behaviour.
  // -------------------------------------------------------------------------
  "mut"_test = [] {
    auto i = 0;

    "sub-test mutates its own copy"_test = [i] {
      mut(i) = 42;
      expect(42_i == i);
    };

    expect(0_i == i);
  };
}
