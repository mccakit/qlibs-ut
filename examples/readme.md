# Examples

Eight self-contained programs, each exercising one part of the library. The filenames are not
ordered, but the table below is: read it top to bottom if you are new to `ut`. The first four
cover everything you need to write tests, and the last four cover configuration, failure modes
and diagnostics.

## At a glance

| Example | Demonstrates | Exits |
|---|---|---|
| [hello_world](#hello_world) | The smallest useful test | 0 |
| [execution_model](#execution_model) | Which lambda shapes run in which pass | 0 |
| [assertions](#assertions) | Every working `expect` form | 0 |
| [suites_and_subtests](#suites_and_subtests) | `suite`, nesting, `mut` | 0 |
| [custom_config](#custom_config) | A TAP outputter via `ut::cfg<ut::override>` | `SIGABRT` |
| [fatal](#fatal) | `expect[...]`, and its unconditional-abort defect | `SIGABRT` |
| [diagnostics](#diagnostics) | The compile errors, one flag each | 0 |
| [filtering](#filtering) | `UT_FILTER` glob matching | 0 |

---

## Reading the summary line

Every example prints a line like this, and it is worth understanding before you compare any of
them against their documented output:

```text
PASSED: tests: 4 (4 passed, 0 failed, 2 compile-time), asserts: 7 (7 passed, 0 failed)
```

Three rules produce those numbers:

1. **A test that runs in both passes is counted twice, in different columns.** Once under
   `passed`, once under `compile-time`. So `tests: 4 … 2 compile-time` means four tests ran at
   run time and two of those four also ran at compile time. The parenthesised numbers do not
   sum to the total.

2. **Only the run-time pass reports asserts.** During constant evaluation a failed expectation
   is a compile error, so there is nothing to tally — the reporter is never called. An assert
   count is therefore always the run-time count, never double.

3. **A test excluded from the compile-time pass contributes nothing to `compile-time`.** That
   happens when the lambda is `mutable` or captures anything. This is why `suites_and_subtests`
   reports only 2 compile-time tests out of 4: both of its nested tests capture.

One consequence worth internalising: a test registered *inside* another test's body during the
compile-time pass runs immediately, without being reported. Only the outer test is counted as
compile-time. That is why nesting does not inflate the compile-time column.

---

## hello_world

The minimum. A `constexpr` fold, one test, four expectations.

```cpp
"sum"_test = [] {
  expect(0_i == sum());
  expect(3_i == sum(1, 2));
};
```

Two things to notice. The file includes **nothing** — no `<iostream>`, no `<cstdio>` — because
the module carries its own output dependency and importers pay nothing for it. And
`using namespace ut;` is mandatory: unlike the single header this module replaces, nothing is
injected into the global namespace.

```text
PASSED: tests: 1 (1 passed, 0 failed, 1 compile-time), asserts: 4 (4 passed, 0 failed)
```

## execution_model

The one example that repays careful reading. Which passes a test participates in is decided
entirely by the shape of its lambda, and this file puts all five cases side by side against a
helper that reports which evaluator is running it:

```cpp
constexpr auto where() {
  if consteval { return 42; } else { return 87; }
}
```

| Written as | Compile-time | Run-time |
|---|---|---|
| `[] { … }` | yes | yes |
| `[] constexpr { … }` | yes | yes |
| `[] mutable { … }` | no | yes |
| `[capture] { … }` | no | yes |
| `static_assert(("n"_test = [] { … }))` | yes | no |
| `[] consteval { … }` | yes | no |

`mutable` is the escape hatch for bodies that cannot be constant-evaluated — I/O, a C API, a
container your standard library has not made `constexpr` yet. Capturing has the same effect
without you asking for it, because the capture is not a constant expression at the point of
registration.

`constexpr` on the lambda changes nothing on its own; a plain lambda is already implicitly
`constexpr` when its body allows. It is worth writing only when you want the compiler to tell
you that the body *cannot* be constant-evaluated.

```text
PASSED: tests: 4 (4 passed, 0 failed, 2 compile-time), asserts: 4 (4 passed, 0 failed)
```

**Flag:** `-DUT_EXAMPLE_CONSTEVAL_LAMBDA` enables the `[] consteval` test, which is off by
default. Calling a `consteval` lambda from the runner's run-time branch needs C++23 consteval
propagation (P2564) to escalate the whole call chain to an immediate function; whether that
resolves cleanly is compiler-dependent. The `static_assert` form is equivalent and portable, so
that is what the file uses as its primary demonstration.

## assertions

A catalogue of every assertion form that works, organised by type family, with the forms that
*don't* work sitting next to them as commented-out counterexamples. That pairing is the point of
the file — several natural-looking spellings are rejected by design, and it is faster to see
them than to discover them.

Four equivalent spellings:

```cpp
expect(42_i == answer());     // literal on the left
expect(answer() == 42_i);     // literal on the right
expect(_i(42) == answer());   // explicit constructor
expect(eq(42, answer()));     // comparison type by name
```

Messages stream only on failure, and chain:

```cpp
expect(42_i == answer()) << "context: " << answer() << " is the answer";
```

The three rejections worth knowing about up front:

```cpp
expect(is_even(4));        // no: already a bool, both operands lost
expect(true_b);            // no: _b has no operator bool
expect(!(42_i == 43));     // no: eq has no .VALUE, so this collapses to bool
```

Booleans go through a comparison or through `nt`:

```cpp
expect(true_b == is_even(4));
expect(nt(is_even(3)));
```

Floating point requires an explicit tolerance, supplied by calling the comparison:

```cpp
expect((pi() == 3.14159_d)(.00001));
```

Strings compare element-wise, and **both** operands must be `_string`. There is no comparison
against a raw `const char*`, because a pointer has no `.size()` to compare against:

```cpp
expect("ut"_s == "ut"_s);    // yes
expect("ut"_s == some_ptr);  // no: different types
```

`constant<…>` forces evaluation as a constant expression and fails to compile if that is not
possible — useful for pinning down that something really is `constexpr`-callable, independently
of whether the enclosing test happens to run at compile time.

```text
PASSED: tests: 7 (7 passed, 0 failed, 7 compile-time), asserts: 23 (23 passed, 0 failed)
```

## suites_and_subtests

Three things that go together in practice.

**Suites** run during static initialisation, before `main`. This is how you spread tests across
translation units without touching `main`:

```cpp
const suite vector_suite = [] {
  "vector"_test = [] { … };
};
```

**Nested tests** are just registrations inside a test body. Each gets its own name in the report
and its own pass/fail accounting. Capturing by copy gives each sub-test an independent starting
state, so siblings cannot interfere — at the cost of excluding them from the compile-time pass.

**`mut`** exists because a lambda's `operator()` is `const`, which makes everything captured by
copy `const` when viewed from inside the body. `mut` casts that away:

```cpp
"outer"_test = [] {
  auto i = 0;
  "inner"_test = [i] { mut(i) = 42; expect(42_i == i); };
  expect(0_i == i);   // the outer i is untouched
};
```

It is a `const_cast`, so applying it to something genuinely `const` — rather than merely
observed through a closure — is undefined behaviour.

This is also the first example to include a standard header, and it does so **before**
`import ut;`. Including after an import can be rejected by Clang when the header and the module
declare overlapping names.

```text
PASSED: tests: 4 (4 passed, 0 failed, 2 compile-time), asserts: 7 (7 passed, 0 failed)
```

## custom_config

Replacing the default text output with a TAP emitter, by specialising `ut::cfg` for
`ut::override`.

Only the *outputter* is replaced. The stock `ut::reporter` and `ut::runner` are kept, which is
the usual case: the reporter already knows how to tally and the runner already knows the
compile-time/run-time policy, so all that is left is deciding what the output looks like.

```cpp
struct config {
  tap::outputter outputter{};
  ut::reporter<decltype(outputter)> reporter{outputter};
  ut::runner<decltype(reporter)> runner{reporter};
  const char* current_test_name{};   // optional
};

template<class... Ts> auto ut::cfg<ut::override, Ts...> = tap::config{};
```

The interesting problem the example has to solve is **ordering**. TAP wants diagnostics after
the `not ok` line, but the library reports an assertion failure — and any message attached with
`<<` — while the test is still running. So the outputter buffers diagnostics and flushes them on
`test_end`. Any custom outputter that reorders output has to do something like this.

Note also that every handler guards its side effects with `if not consteval`. Handlers are
called during both passes, and `printf` is not available at compile time.

```text
1..3
ok 1 - addition
not ok 2 - subtraction
#   custom_config.cpp:118 expected 1, got 0
ok 3 - multiplication
# 2/3 passed
```

Then `SIGABRT` — see [Why two examples abort](#why-two-examples-abort).

## fatal

`expect[...]` is the fatal form: it stops the run rather than recording a failure and
continuing, for when carrying on would crash or produce cascading noise.

```cpp
"guard against empty"_test = [] mutable {
  std::vector<int> v{};
  expect[v.size() > 0_ul] << "cannot index into an empty vector";
  expect(v[0] == 42_i);   // not reached
};
```

> ### ⚠️ This example documents a defect
>
> `expect[...]` **currently aborts whether the expectation passed or failed.** The handle
> returned by `operator[]` raises `events::fatal` from its destructor unconditionally, and
> `reporter::on(events::fatal&)` responds by calling `abort()`.
>
> This is inherited verbatim from the upstream single header and was preserved rather than
> silently changed during the C++23 port. The second test in the file exists to show it: its
> expectation holds, and the process still aborts before reaching the next line.
>
> The fix is one line in `ut.cppm`:
>
> ```cpp
> constexpr ~fatal_log() {
>   if (not result) { detail::cfg(tag).reporter.on(events::fatal{}); }
> }
> ```
>
> Applying it is a behavioural change, so it is left as a deliberate decision rather than
> something the modernisation did on its own.

## diagnostics

The errors the library is *designed* to produce. Every case is a compile error, so each sits
behind its own flag and the file builds clean with none set.

| Flag | Diagnostic |
|---|---|
| `UT_SHOW_LEAK` | Allocation not freed during constant evaluation |
| `UT_SHOW_UB` | Null dereference during constant evaluation |
| `UT_SHOW_MISMATCHED_TYPES` | `Comparision of different types is not allowed` |
| `UT_SHOW_BARE_BOOL` | `Expression required - expect(lhs == rhs)` |
| `UT_SHOW_MISSING_EPSILON` | `Epsilon is required` |
| `UT_SHOW_NOT_CONSTANT` | `constant<…>` on a non-constant expression |
| `UT_SHOW_FAILING_ASSERT` | An ordinary assertion that does not hold |

```sh
clang++ -std=c++23 -fmodule-file=ut=ut.pcm -DUT_SHOW_BARE_BOOL -c diagnostics.cpp
```

The leak and UB cases are not features the library implements — they fall out of running the
test inside the constant evaluator, which refuses to leave an allocation dangling or read
through a null pointer. They apply **only** to tests that actually run at compile time; mark
either one `mutable` and the compiler will let it through.

`UT_SHOW_FAILING_ASSERT` is worth enabling once. Because the test is eligible for the
compile-time pass, it fails at compile time and you never get a run-time report at all — the
diagnostic points at the `static_assert` inside the runner, with a note naming the test and the
assertion. Add `mutable` to move it to run time and get the familiar `FAILED:` line instead.
That difference is the library's central design decision in one observation.

With no flags set:

```text
PASSED: tests: 1 (1 passed, 0 failed, 1 compile-time), asserts: 1 (1 passed, 0 failed)
```

## filtering

`UT_FILTER` restricts which tests run **at run time**. The compile-time pass is unaffected — it
already happened, before any environment variable existed.

```sh
./example_filtering                          # all four
UT_FILTER='parser'       ./example_filtering # exactly "parser"
UT_FILTER='parser*'      ./example_filtering # "parser", "parser errors"
UT_FILTER='*errors'      ./example_filtering # anything ending in "errors"
UT_FILTER='*error*'      ./example_filtering # anything containing "error"
UT_FILTER='lexer?'       ./example_filtering # "lexer" plus exactly one character
```

The pattern is a **glob** matched against the whole test name: `*` matches any run of
characters, `?` matches exactly one. It is **not** a regular expression, despite what the
upstream documentation says — `UT_FILTER='parser.*'` matches nothing, because `.` is a literal
dot here.

Filtering changes the run-time tallies but not the compile-time one. With `UT_FILTER=parser`:

```text
PASSED: tests: 1 (1 passed, 0 failed, 4 compile-time), asserts: 1 (1 passed, 0 failed)
```

Which is the honest answer: the other three really were verified, just not at run time.

---

## Why two examples abort

`custom_config` and `fatal` terminate with `SIGABRT` by design, and this is a property of the
library rather than of the examples.

The stock `ut::reporter` calls `abort()` from its destructor if any expectation failed, and
`reporter::on(events::fatal&)` calls it immediately. There is no path that sets a non-zero exit
code and returns normally. Any harness that distinguishes "test failed" from "test crashed" will
report the former as the latter.

`CMakeLists.txt` works around this by matching on expected output rather than exit status for
those two targets. If you would rather have conventional exit codes, that is a reporter change —
and `custom_config` already shows the shape of a replacement.
