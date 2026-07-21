module;

#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>)
#define UT_HAS_POSIX_FORK 1
#endif

export module ut:reporter;

import std;
import :reflection;
import :type_traits;
import :events;
import :detail;
import :cfg;

namespace ut
{

    export struct colors
    {
            std::string_view none = "\033[0m";
            std::string_view pass = "\033[32m";
            std::string_view fail = "\033[31m";
            std::string_view skip = "\033[33m";
    };

    export class printer
    {
            [[nodiscard]] auto color(const bool cond)
            {
                return cond ? colors_.pass : colors_.fail;
            }

        public:
            printer() = default;
            /*explicit(false)*/ printer(const colors colors) : colors_ {colors}
            {
            }

            template <class T> auto &operator<<(const T &t)
            {
                out_ << detail::get(t);
                return *this;
            }

            template <class T>
                requires std::ranges::range<T> && (!concepts::ostreamable<T>)
            auto &operator<<(T &&t)
            {
                *this << '{';
                auto first = true;
                for (const auto &arg : t)
                {
                    *this << (first ? "" : ", ") << arg;
                    first = false;
                }
                *this << '}';
                return *this;
            }

            auto &operator<<(std::string_view sv)
            {
                out_ << sv;
                return *this;
            }

            template <class TLhs, class TRhs> auto &operator<<(const detail::eq_<TLhs, TRhs> &op)
            {
                return (*this << color(op) << op.lhs() << " == " << op.rhs() << colors_.none);
            }

            template <class TLhs, class TRhs, class TEpsilon>
            auto &operator<<(const detail::approx_<TLhs, TRhs, TEpsilon> &op)
            {
                return (*this << color(op) << op.lhs() << " ~ (" << op.rhs() << " +/- " << op.epsilon() << ')'
                              << colors_.none);
            }

            template <class TLhs, class TRhs> auto &operator<<(const detail::neq_<TLhs, TRhs> &op)
            {
                return (*this << color(op) << op.lhs() << " != " << op.rhs() << colors_.none);
            }

            template <class TLhs, class TRhs> auto &operator<<(const detail::gt_<TLhs, TRhs> &op)
            {
                return (*this << color(op) << op.lhs() << " > " << op.rhs() << colors_.none);
            }

            template <class TLhs, class TRhs> auto &operator<<(const detail::ge_<TLhs, TRhs> &op)
            {
                return (*this << color(op) << op.lhs() << " >= " << op.rhs() << colors_.none);
            }

            template <class TLhs, class TRhs> auto &operator<<(const detail::lt_<TRhs, TLhs> &op)
            {
                return (*this << color(op) << op.lhs() << " < " << op.rhs() << colors_.none);
            }

            template <class TLhs, class TRhs> auto &operator<<(const detail::le_<TRhs, TLhs> &op)
            {
                return (*this << color(op) << op.lhs() << " <= " << op.rhs() << colors_.none);
            }

            template <class TLhs, class TRhs> auto &operator<<(const detail::and_<TLhs, TRhs> &op)
            {
                return (*this << color(op) << '(' << op.lhs() << color(op) << " and " << op.rhs() << color(op) << ')')
                       << colors_.none;
            }

            template <class TLhs, class TRhs> auto &operator<<(const detail::or_<TLhs, TRhs> &op)
            {
                return (*this << color(op) << '(' << op.lhs() << color(op) << " or " << op.rhs() << color(op) << ')')
                       << colors_.none;
            }

            template <class T> auto &operator<<(const detail::not_<T> &op)
            {
                return (*this << color(op) << "not " << op.value() << colors_.none);
            }

            template <class T> auto &operator<<(const detail::fatal_<T> &fatal)
            {
                return (*this << fatal.get());
            }

            template <class TExpr, class TException> auto &operator<<(const detail::throws_<TExpr, TException> &op)
            {
                return (*this << color(op) << "throws<" << reflection::type_name<TException>() << ">" << colors_.none);
            }

            template <class TExpr> auto &operator<<(const detail::throws_<TExpr, void> &op)
            {
                return (*this << color(op) << "throws" << colors_.none);
            }

            template <class TExpr> auto &operator<<(const detail::nothrow_<TExpr> &op)
            {
                return (*this << color(op) << "nothrow" << colors_.none);
            }

#if defined(UT_HAS_POSIX_FORK)
            template <class TExpr> auto &operator<<(const detail::aborts_<TExpr> &op)
            {
                return (*this << color(op) << "aborts" << colors_.none);
            }
#endif

            template <class T> auto &operator<<(const detail::type_<T> &)
            {
                return (*this << reflection::type_name<T>());
            }

            auto str() const
            {
                return out_.str();
            }
            const auto &colors() const
            {
                return colors_;
            }

        private:
            ut::colors colors_ {};
            std::ostringstream out_ {};
    };

    export template <class TPrinter = printer> class reporter
    {
        public:
            constexpr auto operator=(TPrinter printer)
            {
                printer_ = static_cast<TPrinter &&>(printer);
            }

            auto on(events::run_begin) -> void
            {
            }

            auto on(events::test_begin test_begin) -> void
            {
                printer_ << "Running \"" << test_begin.name << "\"...";
                fails_ = asserts_.fail;
            }

            auto on(events::test_run test_run) -> void
            {
                printer_ << "\n \"" << test_run.name << "\"...";
            }

            auto on(events::test_skip test_skip) -> void
            {
                printer_ << test_skip.name << "...SKIPPED\n";
                ++tests_.skip;
            }

            auto on(events::test_end) -> void
            {
                if (asserts_.fail > fails_)
                {
                    ++tests_.fail;
                    printer_ << '\n' << printer_.colors().fail << "FAILED" << printer_.colors().none << '\n';
                }
                else
                {
                    ++tests_.pass;
                    printer_ << printer_.colors().pass << "PASSED" << printer_.colors().none << '\n';
                }
            }

            template <class TMsg> auto on(events::log<TMsg> l) -> void
            {
                printer_ << l.msg;
            }

            auto on(events::exception exception) -> void
            {
                printer_ << "\n  " << printer_.colors().fail << "Unexpected exception with message:\n"
                         << exception.what() << printer_.colors().none;
                ++asserts_.fail;
            }

            template <class TExpr> auto on(events::assertion_pass<TExpr>) -> void
            {
                ++asserts_.pass;
            }

            template <class TExpr> auto on(events::assertion_fail<TExpr> assertion) -> void
            {
                constexpr auto short_name = [](std::string_view name) {
                    return name.rfind('/') != std::string_view::npos ? name.substr(name.rfind('/') + 1) : name;
                };
                printer_ << "\n  " << short_name(assertion.location.file_name()) << ':' << assertion.location.line()
                         << ':' << printer_.colors().fail << "FAILED" << printer_.colors().none << " ["
                         << std::boolalpha << assertion.expr << printer_.colors().none << ']';
                ++asserts_.fail;
            }

            auto on(const events::fatal_assertion &) -> void
            {
            }

            auto on(events::summary) -> void
            {
                if (tests_.fail or asserts_.fail)
                {
                    printer_ << "\n========================================================"
                                "=======================\n"
                             << "tests:   " << (tests_.pass + tests_.fail) << " | " << printer_.colors().fail
                             << tests_.fail << " failed" << printer_.colors().none << '\n'
                             << "asserts: " << (asserts_.pass + asserts_.fail) << " | " << asserts_.pass << " passed"
                             << " | " << printer_.colors().fail << asserts_.fail << " failed" << printer_.colors().none
                             << '\n';
                    std::cerr << printer_.str() << std::endl;
                }
                else
                {
                    std::cout << printer_.colors().pass << "All tests passed" << printer_.colors().none << " ("
                              << asserts_.pass << " asserts in " << tests_.pass << " tests)\n";

                    if (tests_.skip)
                    {
                        std::cout << tests_.skip << " tests skipped\n";
                    }

                    std::cout.flush();
                }
            }

        protected:
            struct
            {
                    std::size_t pass {};
                    std::size_t fail {};
                    std::size_t skip {};
            } tests_ {};

            struct
            {
                    std::size_t pass {};
                    std::size_t fail {};
            } asserts_ {};

            std::size_t fails_ {};

            TPrinter printer_ {};
    };

    export template <class TPrinter = printer> class reporter_junit
    {
            using clock_ref = std::chrono::high_resolution_clock;
            using timePoint = std::chrono::time_point<clock_ref>;
            using timeDiff = std::chrono::milliseconds;
            enum class ReportType : std::uint8_t
            {
                CONSOLE,
                JUNIT
            } report_type_;
            static constexpr ReportType CONSOLE = ReportType::CONSOLE;
            static constexpr ReportType JUNIT = ReportType::JUNIT;
            enum class StatusType : std::uint8_t
            {
                UNDEFINED,
                PASSED,
                FAILED,
                SKIPPED
            };
            static constexpr StatusType UNDEFINED = StatusType::UNDEFINED;
            static constexpr StatusType PASSED = StatusType::PASSED;
            static constexpr StatusType FAILED = StatusType::FAILED;
            static constexpr StatusType SKIPPED = StatusType::SKIPPED;
            static const std::string statusStrings[4];

            struct test_result
            {
                    std::string test_name;
                    test_result *parent = nullptr;
                    StatusType status = UNDEFINED;
                    timePoint run_start = clock_ref::now();
                    timePoint run_stop = clock_ref::now();
                    std::size_t n_tests = 0LU;
                    std::size_t fail_tests = 0LU;
                    std::size_t assertions = 0LU;
                    std::size_t skipped = 0LU;
                    std::size_t fails = 0LU;
                    std::string report_string {};
                    std::vector<std::unique_ptr<test_result>> children;

                    explicit test_result(std::string name, test_result *p = nullptr)
                        : test_name(std::move(name)), parent(p)
                    {
                    }
                    test_result(const test_result &) = delete;
                    test_result &operator=(const test_result &) = delete;
                    test_result(test_result &&) noexcept = default;
                    test_result &operator=(test_result &&) noexcept = default;
                    test_result &add_child(std::string name)
                    {
                        children.emplace_back(std::make_unique<test_result>(std::move(name), this));
                        return *children.back();
                    }
            };

            static int layer_;
            colors color_ {};
            std::vector<std::unique_ptr<test_result>> suites_results_;
            test_result *current_node_ = nullptr;

            std::streambuf *cout_save = std::cout.rdbuf();
            std::ostream lcout_;
            TPrinter printer_;
            std::stringstream ss_out_ {};

            void reset_printer()
            {
                ss_out_.str("");
                ss_out_.clear();
            }

            void add_node(std::string node_name)
            {
                if (current_node_->parent == nullptr)
                {
                    reset_printer();
                }
                layer_++;
                current_node_ = &current_node_->add_child(node_name);
            }

            void count_result()
            {
                current_node_->run_stop = clock_ref::now();
                current_node_->status = current_node_->fails > 0 ? FAILED : (current_node_->skipped ? SKIPPED : PASSED);
                auto parent = current_node_->parent;
                if (parent != nullptr)
                {
                    parent->n_tests += 1LU;
                    if ((current_node_->fails > 0 || current_node_->fail_tests > 0))
                    {
                        parent->fail_tests++;
                    }
                    parent->assertions += current_node_->assertions;
                    parent->skipped += current_node_->skipped;
                    parent->fails += current_node_->fails;
                }
                current_node_ = parent;
                layer_--;
            }

            std::string getLeadingSpace()
            {
                return layer_ > 0 ? "\n" + std::string(2 * (layer_ - 1), ' ') : "\n";
            }

        public:
            constexpr auto operator=(TPrinter printer)
            {
                printer_ = static_cast<TPrinter &&>(printer);
            }
            reporter_junit() : lcout_(std::cout.rdbuf())
            {
                suites_results_.emplace_back(std::make_unique<test_result>("global"));
                current_node_ = suites_results_.front().get();
            }
            ~reporter_junit()
            {
                std::cout.rdbuf(cout_save);
            }

            auto on(events::run_begin run)
            {
                ::ut::detail::cfg::parse_arg_with_fallback(run.argc, run.argv);

                if (detail::cfg::show_reporters)
                {
                    std::cout << "available reporter:\n";
                    std::cout << "  console (default)\n";
                    std::cout << "  junit" << std::endl;
                    std::exit(0);
                }
                if (detail::cfg::use_reporter.starts_with("junit"))
                {
                    report_type_ = JUNIT;
                }
                else
                {
                    report_type_ = CONSOLE;
                }
                if (!detail::cfg::use_colour.starts_with("yes"))
                {
                    color_ = {"", "", "", ""};
                }
                if (!detail::cfg::show_tests && !detail::cfg::show_test_names)
                {
                    std::cout.rdbuf(ss_out_.rdbuf());
                }
            }

            auto on(events::suite_begin suite) -> void
            {
                suites_results_.emplace_back(std::make_unique<test_result>((std::string)suite.name));
                current_node_ = suites_results_.back().get();
            }

            auto on(events::suite_end) -> void
            {
                current_node_ = suites_results_.front().get();
            }

            auto on(events::test_begin test_event) -> void
            {
                add_node((std::string)test_event.name);
                if (report_type_ == CONSOLE)
                {
                    ss_out_ << getLeadingSpace();
                    ss_out_ << "Running " << test_event.type << " \"" << test_event.name << "\"... ";
                }
            }

            auto on(events::test_end test_event) -> void
            {
                current_node_->report_string += ss_out_.str();
                if (report_type_ == CONSOLE)
                {
                    if (current_node_->fails > 0)
                    {
                        lcout_ << ss_out_.str();
                    }
                    else if (detail::cfg::show_successful_tests)
                    {
                        if (!current_node_->children.empty())
                        {
                            ss_out_ << getLeadingSpace();
                            ss_out_ << "Running test \"" << test_event.name << "\" ... ";
                        }
                        ss_out_ << color_.pass << "PASSED " << color_.none;
                        print_duration(ss_out_);
                        lcout_ << ss_out_.str();
                    }
                }
                reset_printer();
                count_result();
            }

            auto on(events::test_run test_event) -> void
            {
                on(events::test_begin {.type = test_event.type, .name = test_event.name});
            }

            auto on(events::test_finish test_event) -> void
            {
                on(events::test_end {.type = test_event.type, .name = test_event.name});
            }

            auto on(events::test_skip test_event) -> void
            {
                ss_out_.clear();
                add_node((std::string)test_event.name);
                current_node_->status = SKIPPED;
                current_node_->skipped += 1;
                if (report_type_ == CONSOLE)
                {
                    lcout_ << getLeadingSpace();
                    lcout_ << "Running \"" << test_event.name << "\"... ";
                    lcout_ << color_.skip << "SKIPPED" << color_.none;
                }
                reset_printer();
                count_result();
            }

            template <class TMsg> auto on(events::log<TMsg> log) -> void
            {
                ss_out_ << log.msg;
            }

            auto on(events::exception exception) -> void
            {
                current_node_->fails++;
                current_node_->report_string += color_.fail;
                current_node_->report_string += "Unexpected exception with message:\n";
                current_node_->report_string += exception.what();
                current_node_->report_string += color_.none;
                if (report_type_ == CONSOLE)
                {
                    lcout_ << getLeadingSpace();
                    lcout_ << "Running test \"" << current_node_->test_name << "\"... ";
                    lcout_ << color_.fail << "FAILED " << color_.none;
                    print_duration(lcout_);
                    lcout_ << '\n';
                    lcout_ << current_node_->report_string << '\n';
                }
                if (detail::cfg::abort_early || current_node_->fails >= detail::cfg::abort_after_n_failures)
                {
                    std::cerr << "early abort for test : " << current_node_->test_name << "after ";
                    std::cerr << current_node_->fails << " failures total." << std::endl;
                    std::exit(-1);
                }
            }

            template <class TExpr> auto on(events::assertion_pass<TExpr>) -> void
            {
                current_node_->assertions++;
            }

            template <class TExpr> auto on(events::assertion_fail<TExpr> assertion) -> void
            {
                TPrinter ss {};
                ss << ss_out_.str();
                if (report_type_ == CONSOLE)
                {
                    ss << getLeadingSpace();
                    ss << color_.fail << "FAILED " << color_.none;
                    print_duration(ss);
                }
                ss << "in: " << assertion.location.file_name() << ':' << assertion.location.line();
                ss << color_.fail << " - test condition: ";
                ss << '[' << std::boolalpha << assertion.expr;
                ss << color_.fail << ']' << color_.none;
                current_node_->report_string += ss.str();
                current_node_->fails++;
                current_node_->assertions++;
                reset_printer();
                if (report_type_ == CONSOLE)
                {
                    lcout_ << ss.str();
                }
                if (detail::cfg::abort_early || current_node_->fails >= detail::cfg::abort_after_n_failures)
                {
                    std::cerr << "early abort for test : " << current_node_->test_name << "after ";
                    std::cerr << current_node_->fails << " failures total." << std::endl;
                    std::exit(-1);
                }
            }

            auto on(const events::fatal_assertion &) -> void
            {
                TPrinter ss {};
                ss << ss_out_.str() << "\n=> " << color_.fail << "terminated for the fatal issue" << color_.none;
                current_node_->report_string += ss.str();
                reset_printer();
                if (report_type_ == CONSOLE)
                {
                    lcout_ << ss.str();
                }
                while (current_node_->parent != nullptr)
                {
                    count_result();
                }
            }

            auto on(events::summary) -> void
            {
                std::cout.flush();
                std::cout.rdbuf(cout_save);
                std::ofstream maybe_of;
                if (detail::cfg::output_filename != "")
                {
                    maybe_of = std::ofstream(detail::cfg::output_filename);
                }

                if (report_type_ == JUNIT)
                {
                    print_junit_summary(detail::cfg::output_filename != "" ? maybe_of : std::cout);
                    return;
                }
                lcout_ << ss_out_.str();
                print_console_summary(detail::cfg::output_filename != "" ? maybe_of : std::cout,
                                      detail::cfg::output_filename != "" ? maybe_of : std::cerr);
            }

        protected:
            double get_duration(test_result *test_node) const
            {
                std::int64_t time_ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(test_node->run_stop - test_node->run_start)
                        .count();
                return static_cast<double>(time_ms) / 1000.0;
            }

            void print_duration(auto &printer) const noexcept
            {
                if (detail::cfg::show_duration)
                {
                    printer << "after " << get_duration(current_node_) << " seconds ";
                }
            }

            void print_console_summary(std::ostream &out_stream, std::ostream &err_stream)
            {
                for (const auto &suite_result : suites_results_)
                {
                    if (suite_result->fails)
                    {
                        err_stream << "\n========================================================"
                                      "=======================\n"
                                   << "Suite " << suite_result->test_name << '\n'
                                   << "tests:   " << (suite_result->n_tests) << " | "
                                   << (suite_result->fail_tests > 0 ? color_.fail : color_.none)
                                   << suite_result->fail_tests << " failed" << color_.none << '\n'
                                   << "asserts: " << (suite_result->assertions) << " | "
                                   << (suite_result->assertions - suite_result->fails) << " passed"
                                   << " | " << color_.fail << suite_result->fails << " failed" << color_.none;
                    }
                    else if (suite_result->assertions || suite_result->n_tests || suite_result->skipped)
                    {
                        out_stream << color_.pass << "\nSuite '" << suite_result->test_name << "': all tests passed"
                                   << color_.none << " (" << suite_result->assertions << " asserts in "
                                   << suite_result->n_tests << " tests)";
                    }
                    if (suite_result->skipped)
                    {
                        std::cout << "; " << color_.skip << suite_result->skipped << " tests skipped" << color_.none;
                    }
                    std::cout.flush();
                }
            }

            void print_junit_summary(std::ostream &stream)
            {
                std::size_t n_tests = 0;
                std::size_t n_fails = 0;
                double total_time = 0.0;
                for (const auto &suite_result : suites_results_)
                {
                    n_tests += suite_result->assertions;
                    n_fails += suite_result->fails;
                    total_time += get_duration(suite_result.get());
                }

                stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
                stream << "<testsuites";
                stream << " name=\"all\"";
                stream << " tests=\"" << n_tests << '\"';
                stream << " failures=\"" << n_fails << '\"';
                stream << " time=\"" << total_time << '\"';
                stream << ">\n";

                for (const auto &suite_result : suites_results_)
                {
                    stream << "<testsuite";
                    stream << " classname=\"" << detail::cfg::executable_name << '\"';
                    stream << " name=\"" << suite_result->test_name << '\"';
                    stream << " tests=\"" << suite_result->assertions << '\"';
                    stream << " errors=\"" << suite_result->fails << '\"';
                    stream << " failures=\"" << suite_result->fails << '\"';
                    stream << " skipped=\"" << suite_result->skipped << '\"';
                    stream << " time=\"" << get_duration(suite_result.get()) << '\"';
                    stream << " version=\"2.3.1\">\n";
                    print_result(stream, suite_result->test_name, " ", *suite_result);
                    stream << "</testsuite>\n";
                    stream.flush();
                }
                stream << "</testsuites>";
            }

            void print_result(std::ostream &stream,
                              const std::string &suite_name,
                              const std::string &indent,
                              const test_result &test_node)
            {
                for (const auto &child_result : test_node.children)
                {
                    stream << indent;
                    stream << "<testcase classname=\"" << suite_name << '\"';
                    stream << " name=\"" << child_result->test_name << '\"';
                    stream << " tests=\"" << child_result->assertions << '\"';
                    stream << " errors=\"" << child_result->fails << '\"';
                    stream << " failures=\"" << child_result->fails << '\"';
                    stream << " skipped=\"" << child_result->skipped << '\"';
                    stream << " time=\"" << get_duration(child_result.get()) << "\"";
                    stream << " status=\"" << statusStrings[(int)child_result->status] << '\"';
                    if (child_result->report_string.empty() && child_result->children.empty())
                    {
                        stream << " />\n";
                    }
                    else if (!child_result->children.empty())
                    {
                        stream << " />\n";
                        print_result(stream, suite_name, indent + "  ", *child_result);
                        stream << indent << "</testcase>\n";
                    }
                    else if (!child_result->report_string.empty())
                    {
                        stream << ">\n";
                        stream << indent << indent << "<system-out>\n";
                        stream << child_result->report_string << "\n";
                        stream << indent << indent << "</system-out>\n";
                        stream << indent << "</testcase>\n";
                    }
                }
            }
    };

    // Out-of-class static data member definitions for reporter_junit
    template <class TPrinter>
    const std::string reporter_junit<TPrinter>::statusStrings[4] = {"UNDEFINED", "FAILED", "SKIPPED", "PASSED"};

    template <class TPrinter> int reporter_junit<TPrinter>::layer_ = 0;

} // namespace ut
