export module ut:cfg;
import std;
import :reflection;

namespace ut::detail
{

    export struct cfg
    {
            using value_ref = std::variant<std::monostate,
                                           std::reference_wrapper<bool>,
                                           std::reference_wrapper<std::size_t>,
                                           std::reference_wrapper<std::string>>;
            using option = std::tuple<std::string, std::string, value_ref, std::string>;

            static reflection::source_location location;
            static bool wip;
            static int largc;
            static const char **largv;
            static std::string executable_name;
            static std::string query_pattern;
            static bool invert_query_pattern;
            static std::string query_regex_pattern;
            static bool show_help;
            static bool show_tests;
            static bool list_tags;
            static bool show_successful_tests;
            static std::string output_filename;
            static std::string use_reporter;
            static std::string suite_name;
            static bool abort_early;
            static std::size_t abort_after_n_failures;
            static bool show_duration;
            static std::size_t show_min_duration;
            static std::string input_filename;
            static bool show_test_names;
            static bool show_reporters;
            static std::string sort_order;
            static std::size_t rnd_seed;
            static std::string use_colour;
            static bool show_lib_identity;
            static std::string wait_for_keypress;

            static const std::vector<option> options;

            static std::optional<option> find_arg(std::string_view arg);
            static void print_usage();
            static void print_identity();
            static void parse_arg_with_fallback(int argc, const char *argv[]);
            static void parse(int argc, const char *argv[]);
    };

    // ---- static data member definitions ----

    reflection::source_location cfg::location {};
    bool cfg::wip {};

#if defined(_MSC_VER)
    int cfg::largc = __argc;
    const char **cfg::largv = const_cast<const char **>(__argv);
#else
    int cfg::largc = 0;
    const char **cfg::largv = nullptr;
#endif

    std::string cfg::executable_name = "unknown executable";
    std::string cfg::query_pattern;
    bool cfg::invert_query_pattern = false;
    std::string cfg::query_regex_pattern;
    bool cfg::show_help = false;
    bool cfg::show_tests = false;
    bool cfg::list_tags = false;
    bool cfg::show_successful_tests = false;
    std::string cfg::output_filename;
    std::string cfg::use_reporter = "console";
    std::string cfg::suite_name;
    bool cfg::abort_early = false;
    std::size_t cfg::abort_after_n_failures = std::numeric_limits<std::size_t>::max();
    bool cfg::show_duration = false;
    std::size_t cfg::show_min_duration = 0;
    std::string cfg::input_filename;
    bool cfg::show_test_names = false;
    bool cfg::show_reporters = false;
    std::string cfg::sort_order = "decl";
    std::size_t cfg::rnd_seed = 0;
    std::string cfg::use_colour = "yes";
    bool cfg::show_lib_identity = false;
    std::string cfg::wait_for_keypress = "never";

    const std::vector<cfg::option> cfg::options = {
        // clang-format off
{"-? -h --help", "", std::ref(show_help), "display usage information"},
{"-l --list-tests", "", std::ref(show_tests), "list all/matching test cases"},
{"-t, --list-tags", "", std::ref(list_tags), "list all/matching tags"},
{"-s, --success", "", std::ref(show_successful_tests), "include successful tests in output"},
{"-o, --out", "<filename>", std::ref(output_filename), "output filename"},
{"-r, --reporter", "<name>", std::ref(use_reporter), "reporter to use (defaults to console)"},
{"-n, --name", "<name>", std::ref(suite_name), "suite name"},
{"-a, --abort", "", std::ref(abort_early), "abort at first failure"},
{"-x, --abortx", "<no. failures>", std::ref(abort_after_n_failures), "abort after x failures"},
{"-d, --durations", "", std::ref(show_duration), "show test durations"},
{"-D, --min-duration", "<seconds>", std::ref(show_min_duration), "show test durations for [...]"},
{"-f, --input-file", "<filename>", std::ref(input_filename), "load test names to run from a file"},
{"--list-test-names-only", "", std::ref(show_test_names), "list all/matching test cases names only"},
{"--list-reporters", "", std::ref(show_reporters), "list all reporters"},
{"--order <decl|lex|rand>", "", std::ref(sort_order), "test case order (defaults to decl)"},
{"--rng-seed", "<'time'|number>", std::ref(rnd_seed), "set a specific seed for random numbers"},
{"--use-colour", "<yes|no>", std::ref(use_colour), "should output be colourised"},
{"--libidentify", "", std::ref(show_lib_identity), "report name and version according to libidentify standard"},
{"--wait-for-keypress", "<never|start|exit|both>", std::ref(wait_for_keypress), "waits for a keypress before exiting"}
        // clang-format on
    };

    // ---- member function definitions ----

    std::optional<cfg::option> cfg::find_arg(std::string_view arg)
    {
        for (const auto &option : cfg::options)
        {
            if (std::get<0>(option).find(arg) != std::string::npos)
            {
                return option;
            }
        }
        return std::nullopt;
    }

    void cfg::print_usage()
    {
        std::size_t opt_width = 30;
        std::cout << cfg::executable_name << " [<test name|pattern|tags> ... ] options\n\nwith options:\n";
        for (const auto &[cmd, arg, val, description] : cfg::options)
        {
            std::string s = cmd;
            s.append(" ");
            s.append(arg);
            const auto pad_by = (s.size() <= opt_width) ? opt_width - s.size() : 0;
            s.insert(s.end(), pad_by, ' ');
            std::cout << "  " << s << description << std::endl;
        }
    }

    void cfg::print_identity()
    {
        // according to: https://github.com/janwilmans/LibIdentify
        std::cout << "description:    A UT / \u03bct test executable\n";
        std::cout << "category:       testframework\n";
        std::cout << "framework:      UT: C++20 \u03bc(micro)/Unit Testing Framework\n";
        std::cout << "version:        2.3.1" << std::endl;
    }

    void cfg::parse_arg_with_fallback(int argc, const char *argv[])
    {
        if (argc > 0 && argv != nullptr)
        {
            cfg::largc = argc;
            cfg::largv = argv;
        }
        else
        {
            cfg::largc = 0;
            cfg::largv = nullptr;
        }
        parse(cfg::largc, cfg::largv);
    }

    void cfg::parse(int argc, const char *argv[])
    {
        const std::size_t n_args = argc > 0 ? static_cast<std::size_t>(argc) : 0U;
        if (n_args > 0 && argv != nullptr)
        {
            executable_name = argv[0];
        }
        query_pattern = "";
        bool found_first_option = false;
        for (auto i = 1U; i < n_args && argv != nullptr; i++)
        {
            std::string cmd(argv[i]);
            auto cmd_option = find_arg(cmd);
            if (!cmd_option.has_value())
            {
                if (found_first_option)
                {
                    std::cerr << "unknown option: '" << cmd << "' run:" << std::endl;
                    std::cerr << "'" << executable_name << " --help'" << std::endl;
                    std::cerr << "for additional help" << std::endl;
                    std::exit(-1);
                }
                else
                {
                    if (i > 1U)
                    {
                        query_pattern.append(" ");
                    }
                    query_pattern.append(cmd);
                }
                continue;
            }
            found_first_option = true;
            auto var = std::get<value_ref>(*cmd_option);
            const bool has_option_arg = !std::get<1>(*cmd_option).empty();
            if (!has_option_arg && std::holds_alternative<std::reference_wrapper<bool>>(var))
            {
                std::get<std::reference_wrapper<bool>>(var).get() = true;
                continue;
            }
            if ((i + 1) >= n_args)
            {
                std::cerr << "missing argument for option " << argv[i] << std::endl;
                std::exit(-1);
            }
            i += 1;
            if (std::holds_alternative<std::reference_wrapper<std::size_t>>(var))
            {
                std::size_t last;
                std::string argument(argv[i]);
                auto val = static_cast<std::size_t>(std::stoull(argument, &last));
                if (last != argument.length())
                {
                    std::cerr << "cannot parse option of " << argv[i - 1] << " " << argv[i] << std::endl;
                    std::exit(-1);
                }
                std::get<std::reference_wrapper<std::size_t>>(var).get() = val;
            }
            if (std::holds_alternative<std::reference_wrapper<std::string>>(var))
            {
                std::get<std::reference_wrapper<std::string>>(var).get() = argv[i];
                continue;
            }
        }

        if (show_help)
        {
            print_usage();
            std::exit(0);
        }

        if (show_lib_identity)
        {
            print_identity();
            std::exit(0);
        }

        if (!query_pattern.empty())
        {
            query_regex_pattern = "";
            for (const char c : query_pattern)
            {
                if (c == '!')
                {
                    invert_query_pattern = true;
                }
                else if (c == '*')
                {
                    query_regex_pattern += ".*";
                }
                else if (c == '?')
                {
                    query_regex_pattern += '.';
                }
                else if (c == '.')
                {
                    query_regex_pattern += "\\.";
                }
                else if (c == '\\')
                {
                    query_regex_pattern += "\\\\";
                }
                else
                {
                    query_regex_pattern += c;
                }
            }
        }
    }

} // namespace ut::detail
