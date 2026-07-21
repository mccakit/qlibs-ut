export module ut:reflection;

import std;

namespace ut::reflection
{

    export using source_location = std::source_location;

    namespace detail
    {

        struct sentinel
        {
        };

        template <typename TargetType>
        [[nodiscard]] constexpr auto get_template_function_name_use_type() -> std::string_view
        {
#if defined(_MSC_VER) && !defined(__clang__)
            return {&__FUNCSIG__[0], sizeof(__FUNCSIG__)};
#else
            return {&__PRETTY_FUNCTION__[0], sizeof(__PRETTY_FUNCTION__)};
#endif
        }

        template <typename TargetType>
        [[nodiscard]] constexpr auto get_template_function_name_use_decay_type() -> std::string_view
        {
            return get_template_function_name_use_type<std::decay_t<TargetType>>();
        }

        constexpr std::string_view raw_type_name = get_template_function_name_use_decay_type<sentinel>();

        constexpr std::size_t raw_length = raw_type_name.length();
        constexpr std::string_view need_name =
#if defined(_MSC_VER) and not defined(__clang__)
            "struct ut::reflection::detail::sentinel";
#else
            "ut::reflection::detail::sentinel";
#endif
        constexpr std::size_t need_length = need_name.length();
        static_assert(need_length <= raw_length, "Auto find prefix and suffix length broken error 1");
        constexpr std::size_t prefix_length = raw_type_name.find(need_name);
        static_assert(prefix_length != std::string_view::npos, "Auto find prefix and suffix length broken error 2");
        static_assert(prefix_length <= raw_length, "Auto find prefix and suffix length broken error 3");
        constexpr std::size_t tail_length = raw_length - prefix_length;
        static_assert(need_length <= tail_length, "Auto find prefix and suffix length broken error 4");
        constexpr std::size_t suffix_length = tail_length - need_length;

    } // namespace detail

    export template <typename TargetType> [[nodiscard]] constexpr auto type_name() -> std::string_view
    {
        const std::string_view raw_type_name = detail::get_template_function_name_use_type<TargetType>();
        const std::size_t end = raw_type_name.length() - detail::suffix_length;
        const std::size_t len = end - detail::prefix_length;
        return raw_type_name.substr(detail::prefix_length, len);
    }

    export template <typename TargetType> [[nodiscard]] constexpr auto decay_type_name() -> std::string_view
    {
        const std::string_view raw_type_name = detail::get_template_function_name_use_decay_type<TargetType>();
        const std::size_t end = raw_type_name.length() - detail::suffix_length;
        const std::size_t len = end - detail::prefix_length;
        return raw_type_name.substr(detail::prefix_length, len);
    }

} // namespace ut::reflection
