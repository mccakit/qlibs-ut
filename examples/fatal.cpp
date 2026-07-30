// examples/06_fatal.cpp
//
// `expect[...]` is the fatal form: it is meant to stop the run rather than
// record a failure and continue, for when carrying on would crash or produce
// meaningless cascading failures.
//
// ---------------------------------------------------------------------------
// KNOWN DEFECT — read before using expect[...]
//
// It currently aborts whether the expectation passed or failed.
//
// `expect_t::operator[]` returns a handle whose destructor raises
// events::fatal unconditionally, and reporter::on(events::fatal&) responds by
// calling abort(). The result is inherited verbatim from the upstream single
// header and was preserved rather than silently changed during the C++23 port.
//
// The fix is one line in ut.cppm:
//
//   constexpr ~fatal_log() {
//     if (not result) { detail::cfg(tag).reporter.on(events::fatal{}); }
//   }
//
// Until then, treat `expect[...]` as "report this expectation, then abort".
// ---------------------------------------------------------------------------
//
// Expected output as written:
//   06_fatal.cpp:NN:FAILED:"guard against empty": 0 > 0 cannot index into an empty vector
//   FAILED: tests: 1 (0 passed, 1 failed, 0 compile-time), asserts: 1 (0 passed, 1 failed)
//   (SIGABRT)
//
// The second test never runs, because the first one aborted the process.

import std;
import ut;

int main() {
  using namespace ut;

  // -------------------------------------------------------------------------
  // The intended use: bail out before an operation that would be undefined.
  // `mutable` because std::vector indexing past the end is exactly the kind of
  // thing you want checked at run time.
  // -------------------------------------------------------------------------
  "guard against empty"_test = [] mutable {
    std::vector<int> v{};
    expect[v.size() > 0_ul] << "cannot index into an empty vector";
    expect(v[0] == 42_i);  // not reached — v is empty
  };

  // -------------------------------------------------------------------------
  // This test is never reached, and would not be even if the vector above were
  // non-empty, because of the defect described at the top of this file.
  // -------------------------------------------------------------------------
  "never runs"_test = [] mutable {
    std::vector<int> v{42};
    expect[v.size() > 0_ul] << "this expectation passes, and still aborts";
    expect(v[0] == 42_i);
  };
}
