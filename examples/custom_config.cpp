// examples/05_custom_config.cpp
//
// Replacing the default output with a TAP (Test Anything Protocol) emitter, by
// specializing ut::cfg for ut::override.
//
// This example replaces only the *outputter* and keeps the stock ut::reporter
// and ut::runner, which is the usual case: the reporter already knows how to
// tally, and the runner already knows the compile-time/run-time policy, so all
// that is left is deciding what the output looks like.
//
// Note the ordering problem this has to solve. TAP wants diagnostics *after*
// the "not ok" line, but the library reports an assertion failure — and any
// message attached to it with `<<` — while the test is still running. So the
// outputter buffers diagnostics and flushes them on test_end.
//
// Expected output (path and line number will differ):
//   1..3
//   ok 1 - addition
//   not ok 2 - subtraction
//   #   05_custom_config.cpp:118 expected 1, got 0
//   ok 3 - multiplication
//   # 2/3 passed
//
// …followed by SIGABRT, because a test failed. The stock reporter aborts from
// its destructor rather than setting an exit code — see the manual's
// "Known issues".

import std;
import ut;

namespace tap {

struct outputter {
  // The reporter only ever forwards run_time events to the outputter, but the
  // templated overloads must exist so that compile_time instantiations resolve.
  template<ut::events::mode Mode>
  constexpr auto on(const ut::events::test_begin<Mode>&) -> void { }
  template<ut::events::mode Mode>
  constexpr auto on(const ut::events::test_end<Mode>&) -> void { }

  constexpr auto on(const ut::events::test_begin<ut::events::mode::run_time>&) -> void { }

  constexpr auto on(const ut::events::test_end<ut::events::mode::run_time>& event) -> void {
    if not consteval {
      using te = ut::events::test_end<ut::events::mode::run_time>;
      std::printf("%s %u - %s\n", event.result == te::PASSED ? "ok" : "not ok", ++index, event.name);
      flush();
    }
  }

  template<class TExpr>
  constexpr auto on(const ut::events::assert_pass<TExpr>&) -> void { }

  template<class TExpr>
  constexpr auto on(const ut::events::assert_fail<TExpr>& event) -> void {
    if not consteval { append("%s:%d ", event.file_name, event.line); }
  }

  constexpr auto on(const ut::events::fatal&) -> void { }

  // Messages attached with `<<`. `result` is false when the assertion failed,
  // which is the only case worth reporting.
  template<class TMsg>
  constexpr auto on(const ut::events::log<TMsg>& event) -> void {
    if not consteval {
      if (not event.result) { append_msg(event.msg); }
    }
  }

  constexpr auto on(const ut::events::summary& event) -> void {
    if not consteval {
      const auto passed = event.tests[ut::events::summary::PASSED];
      const auto failed = event.tests[ut::events::summary::FAILED];
      std::printf("# %u/%u passed\n", passed, passed + failed);
    }
  }

 private:
  // Not constexpr, and that is fine: every caller sits inside `if not consteval`.
  template<class... TArgs>
  auto append(const char* fmt, TArgs... args) -> void {
    const auto room = sizeof(pending) - len;
    if (room <= 1) { return; }
    const auto n = std::snprintf(pending + len, room, fmt, args...);
    if (n > 0) { len += unsigned(n) < room - 1 ? unsigned(n) : unsigned(room - 1); }
  }

  // Overloaded per message type so `<< 0` renders as a number, not a pointer.
  auto append_msg(const char* msg) -> void { append("%s", msg); }
  auto append_msg(int msg) -> void { append("%d", msg); }
  auto append_msg(unsigned msg) -> void { append("%u", msg); }
  auto append_msg(double msg) -> void { append("%g", msg); }

  auto flush() -> void {
    if (len) { std::printf("#   %s\n", pending); }
    len = 0;
    pending[0] = '\0';
  }

  unsigned index{};
  unsigned len{};
  char pending[256]{};
};

struct config {
  tap::outputter outputter{};
  ut::reporter<decltype(outputter)> reporter{outputter};
  ut::runner<decltype(reporter)> runner{reporter};
  const char* current_test_name{};  // optional; the runner sets it if present
};

}  // namespace tap

// The customization point. `ut::override` is the tag meaning "a user supplied
// this"; the trailing pack keeps the specialization findable from the dependent
// lookup the library performs internally.
template<class... Ts> auto ut::cfg<ut::override, Ts...> = tap::config{};

int main() {
  using namespace ut;

  std::printf("1..3\n");

  // Deliberately run-time only. A compile-time failure would be a compile
  // error, and this example is about the run-time reporting path.
  "addition"_test = [] mutable { expect(2_i == 1 + 1); };

  "subtraction"_test = [] mutable {
    expect(1_i == 1 - 1) << "expected 1, got " << (1 - 1);
  };

  "multiplication"_test = [] mutable { expect(6_i == 2 * 3); };
}
