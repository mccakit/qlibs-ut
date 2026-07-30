// examples/01_hello_world.cpp
//
// The smallest useful test. Note that this file includes nothing at all: the
// module carries its own output dependency.
//
// Expected output:
//   PASSED: tests: 1 (1 passed, 0 failed, 1 compile-time), asserts: 4 (4 passed, 0 failed)
//
// The test appears once under `passed` and once under `compile-time` because it
// ran in both passes. Only the run-time pass reports asserts, which is why the
// assert count is 4 rather than 8.
import std;
import ut;

constexpr auto sum(auto... args) { return (0 + ... + args); }

int main() {
  using namespace ut;  // required: the module does not pollute the global namespace

  "sum"_test = [] {
    expect(0_i == sum());
    expect(1_i == sum(1));
    expect(3_i == sum(1, 2));
    expect(6_i == sum(1, 2, 3));
  };
}
