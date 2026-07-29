# ut — user manual

A compile-time-first unit-testing library for C++23, distributed as a named module.

> *"If you liked it then you `"should have put a"_test` on it"* — Beyoncé rule

Based on [qlibs/ut](https://github.com/qlibs/ut) v2.1.6 by Kris Jusiak, restructured as a
module interface/implementation pair. See [Migrating from the single header](#migrating-from-the-single-header)
if you are coming from `#include <ut>`.

---

## Contents

- [What it does](#what-it-does)
- [Requirements](#requirements)
- [Building](#building)
- [Quick start](#quick-start)
- [Execution model](#execution-model)
- [Assertions](#assertions)
- [Tests and suites](#tests-and-suites)
- [Compile-time errors the library catches](#compile-time-errors-the-library-catches)
- [Filtering](#filtering)
- [Build-time options](#build-time-options)
- [Custom configuration](#custom-configuration)
- [API reference](#api-reference)
- [Migrating from the single header](#migrating-from-the-single-header)
- [Known issues](#known-issues)
- [FAQ](#faq)
- [License](#license)

---

## What it does

`ut` runs your tests **twice** from one source of truth: once inside the compiler's constant
evaluator, and once at run time. A test that fails during constant evaluation is a compile
error. Because the constant evaluator refuses to leak memory or commit undefined behaviour,
tests that run at compile time get leak and UB detection for free.

Design properties:

- **Explicit by design** — no implicit conversions, no narrowing, no epsilon-less floating
  point comparison, no comparison across different types.
- **Minimal API** — roughly thirty names, all listed under [API reference](#api-reference).
- **Nearly dependency-free** — the module interface includes exactly one standard header,
  `<iostream>`, and only for the default output sink. Build with `UT_COMPILE_TIME_ONLY` and
  it includes nothing at all.
- **Clean under** `-fno-exceptions -fno-rtti -Wall -Wextra -Werror -pedantic -pedantic-errors`.
- **Cheap to import** — the module is compiled once; importers pay for a BMI load, not for
  re-parsing a 1,400-line header.

## Requirements

- A C++23 compiler with named-module support. The library uses `if consteval` (P1938),
  multi-parameter `operator[]` with default arguments (P2128), and `static operator()` (P1169).
- Clang 17+ or GCC 14+ are the practical baselines. GCC 13 compiles the language features but
  its module support is `-fmodules-ts` only.
- CMake 3.28+ if you want CMake to drive module scanning.

## Building

The library is two files:

| File | Kind | Contents |
|---|---|---|
| `ut.cppm` | primary module interface unit | Everything importers can see |
| `ut.cpp` | module implementation unit | Run-time helpers + the library's self-tests |

Both must be compiled, and both must be compiled with the *same* set of
[build-time options](#build-time-options).

### CMake

```cmake
cmake_minimum_required(VERSION 3.28)
project(ut CXX)

add_library(ut)
target_compile_features(ut PUBLIC cxx_std_23)
target_sources(ut
  PUBLIC
    FILE_SET CXX_MODULES FILES src/iface/ut.cppm
  PRIVATE
    src/ut.cpp
)

# consumers
add_executable(my_test test/my_test.cpp)
target_link_libraries(my_test PRIVATE ut)
```

The interface unit's file extension does not matter to CMake — `FILE_SET CXX_MODULES` marks it
regardless — so naming it `iface/ut.cpp` works as well as `ut.cppm`.

### Clang, by hand

```sh
clang++ -std=c++23 --precompile ut.cppm -o ut.pcm
clang++ -std=c++23 -c ut.pcm -o ut.pcm.o
clang++ -std=c++23 -fmodule-file=ut=ut.pcm -c ut.cpp -o ut.o
clang++ -std=c++23 -fmodule-file=ut=ut.pcm my_test.cpp ut.pcm.o ut.o -o my_test
```

### GCC, by hand

```sh
g++ -std=c++23 -fmodules-ts -x c++ -c ut.cppm -o ut.iface.o
g++ -std=c++23 -fmodules-ts -c ut.cpp -o ut.o
g++ -std=c++23 -fmodules-ts my_test.cpp ut.iface.o ut.o -o my_test
```

## Quick start

```cpp
import ut;

constexpr auto sum(auto... args) { return (0 + ... + args); }

int main() {
  using namespace ut;

  "sum"_test = [] {
    expect(sum(1) == 1_i);
    expect(sum(1, 2) == 3_i);
    expect(sum(1, 2, 3) == 6_i);
  };
}
```

```console
$ ./my_test
PASSED: tests: 1 (1 passed, 0 failed, 1 compile-time), asserts: 3 (3 passed, 0 failed)
```

> **`using namespace ut;` is required.** Unlike the single header, the module does not inject
> `operator""_test` into the global namespace. `using ut::operator""_test;` works too if you
> want just the one name.

A failure looks like this:

```console
$ ./my_test
my_test.cpp:8:FAILED:"sum": 6 == 5
FAILED: tests: 1 (0 passed, 1 failed, 1 compile-time), asserts: 3 (2 passed, 1 failed)
```

…except that if the test is eligible for compile-time execution you will never get that far,
because the failure is a compile error first. That is the point of the library.

## Execution model

Which passes a test participates in is decided by the shape of its lambda:

```cpp
"a"_test = []           { /* compile-time AND run-time */ };
"b"_test = [] constexpr { /* compile-time AND run-time */ };
"c"_test = [] mutable   { /* run-time only              */ };
"d"_test = [] consteval { /* compile-time only          */ };
"e"_test = [v]          { /* run-time only (captures)   */ };
```

The rules the runner applies:

- A **`mutable`** lambda is excluded from the compile-time pass. This is the escape hatch for
  tests that cannot be constant-evaluated — because they touch `std::vector`, do I/O, call into
  a C library, or hit a compiler limitation.
- A **capturing** lambda is excluded from the compile-time pass, because the capture itself is
  not a constant expression at the point the test is registered.
- Everything else is constant-evaluated *and* executed at run time.

You can force the compile-time pass to be the only thing that happens, and to happen at a
point you choose, by wrapping the registration in a `static_assert`:

```cpp
static_assert(("sum"_test = [] {
  expect(sum(1, 2, 3) == 6_i);
}));
```

The two passes are also observable from inside a test, which is occasionally what you want to
assert about:

```cpp
constexpr auto where() {
  if consteval { return 42; } else { return 87; }
}

int main() {
  using namespace ut;
  "compile-time"_test = [] consteval { expect(42_i == where()); };
  "run-time"_test     = [] mutable   { expect(87_i == where()); };
}
```

Note that the summary counts compile-time tests separately, so a single `_test` that ran in
both passes contributes to both the total and the `compile-time` tally.

## Assertions

### `expect(...)`

```cpp
expect(42_i == answer());
expect(eq(42, answer()))    << "same thing, spelled out";
expect(_i(42) == answer())  << "also the same thing";
```

The message after `<<` is only streamed when the expectation **fails**, so you can attach
context liberally without polluting passing output.

`expect` requires a `ut` comparison object, not a `bool`. This is deliberate:

```cpp
expect(answer() == 42);    // ERROR: collapses to bool, both operands lost
expect(answer() == 42_i);  // OK: eq<int, int>, both operands retained for the message
```

That is what lets a failure print `6 == 5` instead of `false`.

### `expect[...]` — fatal

```cpp
expect[v.size() > 1_ul] << "cannot continue";
expect(v[1] == 42_i);   // intended: never reached if the above failed
```

> ### ⚠️ Known defect
>
> **`expect[...]` currently aborts the run whether the expectation passed or failed.**
>
> The handle returned by `operator[]` raises `events::fatal` from its destructor
> unconditionally, and `reporter::on(events::fatal&)` responds by calling `abort()`. This is
> inherited verbatim from the upstream single header and was preserved rather than silently
> changed. Until it is fixed, treat `expect[...]` as "abort here, reporting the result of this
> expectation on the way out".
>
> The fix is one line in `expect_t::fatal_log::~fatal_log()`:
>
> ```cpp
> constexpr ~fatal_log() {
>   if (not result) { detail::cfg(tag).reporter.on(events::fatal{}); }
> }
> ```
>
> Applying it is a behavioural change, so it is left as a deliberate decision for the
> maintainer rather than something the modernisation did on its own.

### Floating point

Floating point comparison without a tolerance is a compile error. Supply one by calling the
comparison:

```cpp
expect((4.2 == 4.2_d)(.01));    // |lhs - rhs| < .01
expect((4.24 != 4.23_d)(.001)); // |lhs - rhs| >= .001
```

### Strings and containers

`"foo"_s` produces a `_string`, compared element-wise:

```cpp
expect("foo"_s == name());
```

Any pair of operands that both support `operator[]` and `.size()` is compared element-wise
too, provided the element types match.

### `constant<...>`

Forces an expression to be evaluated as a constant expression, and fails to compile if it
cannot be:

```cpp
expect(constant<42_i == answer()>);  // answer() must be constexpr-callable

auto i = 0;
expect(constant<i == 42_i>);         // ERROR: i is not a constant expression
```

## Tests and suites

### Naming and nesting

```cpp
"vector"_test = [] {
  std::vector<int> v(5);
  expect(v.size() == 5_ul);
  expect(v.capacity() >= 5_ul);

  "resizing bigger changes size and capacity"_test = [=] {
    mut(v).resize(10);
    expect(v.size() == 10_ul);
    expect(v.capacity() >= 10_ul);
  };
};
```

Nesting is just registration inside a test body. Nesting depth is bounded by the reporter's
`MaxDepth` template parameter, which defaults to 16.

### `mut(...)`

Lambdas capture by value as `const`, because the closure's `operator()` is `const`. `mut`
casts that away so a captured copy can be modified:

```cpp
"outer"_test = [] {
  int i = 0;
  "inner"_test = [i] { mut(i) = 42; };  // mutates the inner copy
  expect(i == 0_i);                     // outer i is untouched
};
```

> `mut` is a `const_cast`. Applying it to an object that is genuinely `const` — as opposed to
> one that is only `const` because you are looking at it through a closure — is undefined
> behaviour.

### `suite`

A `suite` runs its body during static initialisation, before `main`:

```cpp
const ut::suite vector_tests = [] {
  "empty"_test = [] { expect(std::vector<int>{}.empty()); };
};

int main() { }
```

Useful for splitting tests across translation units without an explicit registration call in
`main`. Note that suites are *not* `constexpr` — the constructor exists precisely to run at
static-init time.

## Compile-time errors the library catches

These are all diagnostics, not runtime failures:

```cpp
"leak"_test = [] {
  new int;                    // ERROR: allocation not deallocated during constant evaluation
};

"ub"_test = [] {
  int* p{};
  *p = 42;                    // ERROR: null dereference during constant evaluation
};

"mismatched types"_test = [] {
  expect(42_i == short(42));  // ERROR: Comparision of different types is not allowed
};

"bare bool"_test = [] {
  expect(42 == 42);           // ERROR: Expression required - `expect(lhs == rhs)`
};

"no epsilon"_test = [] {
  expect(4.2 == 4.2_d);       // ERROR: Epsilon is required
};
```

The leak and UB checks only apply to tests that actually run at compile time — a `mutable`
test gets neither.

## Filtering

Set `UT_FILTER` to restrict which tests run **at run time**. The compile-time pass is not
affected, since it already happened.

```sh
UT_FILTER='sum'   ./my_test   # exactly the test named "sum"
UT_FILTER='sum*'  ./my_test   # every test whose name starts with "sum"
UT_FILTER='*sum*' ./my_test   # every test whose name contains "sum"
UT_FILTER='foo?'  ./my_test   # "foo" plus exactly one more character
```

The pattern is a **glob**, matched against the whole test name — `*` matches any run of
characters, `?` matches exactly one. It is not a regular expression, despite what older
documentation says. An unset `UT_FILTER` runs everything.

## Build-time options

> **These are macros, and macros do not cross module boundaries.** Defining them in a file that
> says `import ut;` does nothing. They must be defined when compiling `ut.cppm` **and**
> `ut.cpp`, and the two must agree.

| Macro | Effect |
|---|---|
| `NTEST` | Skip the library's own self-verification suite |
| `UT_COMPILE_TIME_ONLY` | Drop the run-time pass, the summary output, and the `<iostream>` dependency |
| `UT_RUN_TIME_ONLY` | Drop the compile-time pass |

In CMake:

```cmake
target_compile_definitions(ut PRIVATE UT_RUN_TIME_ONLY)
```

`PRIVATE` is correct here — these affect how the module is built, not how it is consumed.

`UT_RUN_TIME_ONLY` is the option to reach for when your compiler or standard library cannot
constant-evaluate something and you want to get moving. `UT_COMPILE_TIME_ONLY` gives you a
build with no standard library dependency whatsoever, suitable for freestanding targets.

`NTEST` is worth noting for a reason that changed with modularisation: the self-tests used to
be re-checked by *every* translation unit that included the header. They now live in `ut.cpp`
and are checked once, when the module is built. The cost of leaving them on is much lower than
it used to be.

## Custom configuration

Everything the library does at run time goes through `ut::cfg`. Replace it by specialising for
`ut::override`:

```cpp
import ut;

struct silent_outputter {
  template<ut::events::mode Mode>
  constexpr auto on(const ut::events::test_begin<Mode>&) -> void { }
  template<ut::events::mode Mode>
  constexpr auto on(const ut::events::test_end<Mode>&) -> void { }
  template<class TExpr>
  constexpr auto on(const ut::events::assert_pass<TExpr>&) -> void { }
  template<class TExpr>
  constexpr auto on(const ut::events::assert_fail<TExpr>&) -> void { }
  constexpr auto on(const ut::events::fatal&) -> void { }
  constexpr auto on(const ut::events::summary&) -> void { }
  template<class TMsg>
  constexpr auto on(const ut::events::log<TMsg>&) -> void { }
};

struct my_cfg {
  silent_outputter outputter{};
  ut::reporter<decltype(outputter)> reporter{outputter};
  ut::runner<decltype(reporter)> runner{reporter};
  const char* current_test_name{};   // optional
};

template<class... Ts> auto ut::cfg<ut::override, Ts...> = my_cfg{};
```

A configuration must expose:

| Member | Used by | Required |
|---|---|---|
| `runner` | test dispatch | yes |
| `reporter` | `expect`, `expect[...]` | yes |
| `outputter` | `<< message` | yes |
| `current_test_name` | test dispatch | optional, set if present |

You can replace any layer independently: keep `ut::reporter` and `ut::runner` and supply only
your own outputter (as above), or replace the reporter to change what counts as failure, or
replace the runner to change the compile-time/run-time policy.

`ut::outputter`, `ut::reporter`, `ut::runner`, `ut::default_cfg` and the whole `ut::events`
namespace are exported for exactly this purpose. (The pre-modularisation `ut.cppm` wrapper did
not export them, which made custom configurations impossible to write against `import ut;` —
this is fixed.)

### Events

| Event | Raised when |
|---|---|
| `events::run<T>` | A test is dispatched to the runner |
| `events::test_begin<Mode>` | A test starts |
| `events::test_end<Mode>` | A test finishes; carries `FAILED`/`PASSED`/`COMPILE_TIME` |
| `events::assert_pass<TExpr>` | An expectation held |
| `events::assert_fail<TExpr>` | An expectation did not hold |
| `events::fatal` | A fatal expectation completed; the run terminates |
| `events::log<TMsg>` | A message was attached with `<<` |
| `events::summary` | Once, at the end of the run |

`events::mode` distinguishes `run_time` from `compile_time`. Handlers are called during both
passes, so anything with a side effect needs an `if not consteval` guard — see how
`ut::outputter` does it.

## API reference

Everything below is in namespace `ut`.

### Test registration

```cpp
template<fixed_string Str>
[[nodiscard]] constexpr auto operator""_test();

struct suite;
```

### Assertions

```cpp
struct expect_t {
  static constexpr auto operator()(auto expr, const char* file = __builtin_FILE(),
                                              int line = __builtin_LINE());
  static constexpr auto operator[](auto expr, const char* file = __builtin_FILE(),
                                              int line = __builtin_LINE());
};
inline constexpr expect_t expect{};

template<auto Expr> inline constexpr auto constant;
template<class T> [[nodiscard]] constexpr auto& mut(const T&);
```

### Comparisons

```cpp
template<class TLhs, class TRhs> struct eq;   // ==
template<class TLhs, class TRhs> struct neq;  // !=
template<class TLhs, class TRhs> struct gt;   // >
template<class TLhs, class TRhs> struct ge;   // >=
template<class TLhs, class TRhs> struct lt;   // <
template<class TLhs, class TRhs> struct le;   // <=
template<class T>                struct nt;   // !
```

The operators `== != > >= < <= !` build these, and only participate when at least one operand
is a `ut` typed value.

### Typed values

| Type | Underlying | | Type | Underlying |
|---|---|---|---|---|
| `_b` | `bool` | | `_f` | `float` |
| `_c` | `char` | | `_d` | `double` |
| `_sc` | `signed char` | | `_ld` | `long double` |
| `_s` | `short` | | `_i8` | `int8_t` |
| `_i` | `int` | | `_i16` | `int16_t` |
| `_l` | `long` | | `_i32` | `int32_t` |
| `_ll` | `long long` | | `_i64` | `int64_t` |
| `_u` | `unsigned` | | `_u8` | `uint8_t` |
| `_uc` | `unsigned char` | | `_u16` | `uint16_t` |
| `_us` | `unsigned short` | | `_u32` | `uint32_t` |
| `_ul` | `unsigned long` | | `_u64` | `uint64_t` |
| `_ull` | `unsigned long long` | | `_string` | `const char*` |

`true_b` and `false_b` are predefined `_b` constants.

Each has a matching literal suffix: `42_i`, `42_ul`, `4.2_d`, `"foo"_s`, and so on.

### Configuration

```cpp
namespace events { /* mode, run, test_begin, test_end,
                      assert_pass, assert_fail, fatal, log, summary */ }

template<class TOStream>                     class  outputter;
template<class TOutputter, auto MaxDepth=16> struct reporter;
template<class TReporter>                    struct runner;
template<class...>                           struct default_cfg;

struct override;
template<class... Ts> inline default_cfg<Ts...> cfg;
```

## Migrating from the single header

| Was | Now |
|---|---|
| `#include <ut>` | `import ut;` |
| `#include <iostream>` for output | not needed; the module handles it |
| `operator""_test` available at global scope | `using namespace ut;` required |
| `UT_RUN_TIME` | write `mutable` |
| `UT_COMPILE_TIME` | write `consteval` |
| `-DNTEST` on your test TU | define it when building the module |
| `-DUT_RUN_TIME_ONLY` on your test TU | define it when building the module |
| `-DUT_COMPILE_TIME_ONLY` on your test TU | define it when building the module |
| `ut::inline v2_1_6::…` | `ut::…` |

Notes on each of the breaking changes:

**`UT_RUN_TIME` / `UT_COMPILE_TIME` are gone.** They expanded to `mutable` and `consteval`.
Macros are not exported from modules, so they could not be made to work through `import ut;`
under any spelling. Write the keywords.

**The three configuration macros moved to module build time.** A module is compiled once, so
per-TU configuration is no longer meaningful. This is a real loss of flexibility if you were
mixing `-DUT_RUN_TIME_ONLY` across translation units; the workaround is to build two module
variants under different names.

**The global `using ut::operator""_test;` is gone.** The header injected it into the global
namespace unconditionally. Note that the *old* `ut.cppm` wrapper did not re-export it either,
so if you were already on the module this is not a change.

**The version inline namespace is gone.** Module names participate in mangling, so `ut::` is
already distinct from anything else. If you were relying on `v2_1_6` for side-by-side
installation, use distinct module names instead.

**`std::clog` is now reached via a real `#include <iostream>`.** The header forward-declared
`std::basic_ostream` and `std::clog` itself to avoid the include. That is undefined behaviour
(you may not add declarations to namespace `std`), and it cannot survive modularisation anyway
— the module is compiled in isolation and cannot see what its importers included. The include
now sits in the global module fragment, so importers pay nothing for it.

## Known issues

- **`expect[...]` aborts unconditionally.** See the [warning above](#expect---fatal).
- **A failing run terminates with `SIGABRT`,** not a clean non-zero exit. The reporter's
  destructor calls `abort()` if anything failed. Test harnesses that distinguish "failed" from
  "crashed" will report the former as the latter.
- **`#include` after `import ut;` can be rejected by Clang.** If a header you include declares
  something the module also declares — most likely `<iostream>` — put the `#include` *before*
  the `import`. This is [llvm#61465](https://github.com/llvm/llvm-project/issues/61465), not a
  property of this library.
- **Reflection integration is not wired up.** Upstream supports comparing aggregates via
  [qlibs/reflect](https://github.com/qlibs/reflect). That integration is orthogonal to `ut`
  and would need `reflect` to be available as a module too.

## FAQ

**How do I turn off compile-time testing entirely?**
Build the module with `UT_RUN_TIME_ONLY`. To also skip the library's self-verification, add
`NTEST`. Both are module-build-time options.

**Why does my test not run at compile time?**
It is `mutable`, or it captures something. Both exclude a test from the compile-time pass. If
neither applies and it still does not, the body is doing something the constant evaluator
rejects — the diagnostic will point at it.

**Why can't I write `expect(a == b)` with plain ints?**
Because the result would be a `bool` and both operands would be lost, leaving nothing useful
to print on failure. Tag one side: `expect(a == b_i)`, or use `expect(eq(a, b))`.

**Can I use it with exceptions and RTTI disabled?**
Yes. That is a supported configuration and the library uses neither.

**Similar projects?**
[boost.ut](https://github.com/boost-ext/ut), [Catch2](https://github.com/catchorg/Catch2),
[GoogleTest](https://github.com/google/googletest), [doctest](https://github.com/doctest/doctest).

## Further reading

- [*"unit"_test: Implementing a Macro-free Unit Testing Framework from Scratch in C++20*](https://www.youtube.com/watch?v=-qAXShy1xiE)
- [*Future of Testing With C++20*](https://www.youtube.com/watch?v=KlU0cb_tbuw)
- [*Towards Painless Testing*](https://www.youtube.com/watch?v=NVrZjT5lW5o)

## License

MIT. Copyright © 2024 Kris Jusiak &lt;<kris@jusiak.net>&gt;.
