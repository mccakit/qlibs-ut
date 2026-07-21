# UT Examples Summary

This guide provides a simple overview of the Boost.UT examples and how to run the example executables.

## Example Overview

| Executable | Purpose |
|------------|---------|
| `example_abort` | Tests code that triggers fatal aborts or exceptions. |
| `example_attr` / `example_macro` | Demonstrates custom C++ attributes (`[[expect]]`) or macros for testing. |
| `example_bdd` / `example_gherkin` | Shows BDD-style testing (`given`/`when`/`then`) and Gherkin feature files. |
| `example_benchmark` | Measures and logs execution time for code blocks. |
| `example_cli` | Shows how to configure filters, colors, and dry-runs via CLI arguments. |
| `example_exception` | Verifies expected exceptions (`throws`) or lack thereof (`nothrow`). |
| `example_expect` | Covers standard syntax for checking values, strings, types, and containers. |
| `example_fatal` | Demonstrates stopping the test suite immediately upon a critical failure. |
| `example_filter` | Shows how to run only specific tests using name patterns. |
| `example_hello_world` | A basic "Hello World" style test case. |
| `example_log` | Demonstrates logging messages during test execution. |
| `example_matcher` | Shows how to define custom logic for verifying complex conditions. |
| `example_minimal` | The absolute simplest way to write a test. |
| `example_module` | Demonstrates using the library within C++ modules. |
| `example_mut` | Shows how to safely handle mutable state within a test. |
| `example_parameterized` | Runs the same test logic against multiple inputs or types. |
| `example_run` | Demonstrates manually controlling test execution. |
| `example_report` | Demonstrates manually reporting test failures. |
| `example_skip` | Demonstrates how to temporarily disable specific tests. |
| `example_sl` | Shows how to provide explicit source location data for errors. |
| `example_spec` | Uses specification-style syntax (`describe`/`it`). |
| `example_suite` | Organizes tests into logical named suites. |
| `example_tag` | Allows filtering tests based on assigned tags. |
| `example_terse` | Uses concise operators for cleaner, shorter test code. |
| `example_test` | Compares UDL-based test syntax with function-based syntax. |
| `example_tmp` | Checks types and compile-time constants. |
| `example_using` | Demonstrates importing specific test helpers and operators. |

## Running Examples

After building, execute the desired example binary from your terminal.

### General usage

Most examples can be run as:

```bash
./example_name [filter] [options]
```

### Run a specific test by name

Use a string filter to execute only matching tests.

```bash
./example_cli pass
```

```bash
./example_filter "run.sub1"
```

### Configure CLI behavior

The CLI example demonstrates passing additional arguments to configure filtering, colored output, and dry-run mode.

```bash
./example_cli <filter> <color_mode> <dry_run>
```

Example:

```bash
./example_cli pass 0 1
```

### Manually invoke `run()`

The `example_run` executable demonstrates manually passing `argc` and `argv` to `ut::cfg<>.run()`.

```bash
./example_run "test suite"
```

### Manual error reporting

The `example_report` executable demonstrates manually reporting test failures.

```bash
./example_report
```

### Required command-line arguments

Some examples validate command-line input and terminate if required arguments are missing.

```bash
./example_main "filter_name"
```
