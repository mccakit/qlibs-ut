export module ut:type_traits;

import std;

namespace ut
{

    namespace type_traits
    {

        export template <class...> struct list
        {
        };

        export template <class T, class...> struct identity
        {
                using type = T;
        };

        export template <class T> struct function_traits : function_traits<decltype(&T::operator())>
        {
        };

        template <class R, class... TArgs> struct function_traits<R (*)(TArgs...)>
        {
                using result_type = R;
                using args = list<TArgs...>;
        };

        template <class R, class... TArgs> struct function_traits<R(TArgs...)>
        {
                using result_type = R;
                using args = list<TArgs...>;
        };

        template <class R, class T, class... TArgs> struct function_traits<R (T::*)(TArgs...)>
        {
                using result_type = R;
                using args = list<TArgs...>;
        };

        template <class R, class T, class... TArgs> struct function_traits<R (T::*)(TArgs...) const>
        {
                using result_type = R;
                using args = list<TArgs...>;
        };

        export template <class T, class = void> struct has_static_member_object_value : std::false_type
        {
        };

        template <class T>
        struct has_static_member_object_value<T, std::void_t<decltype(std::declval<T>().value)>>
            : std::bool_constant<!std::is_member_pointer_v<decltype(&T::value)> &&
                                 !std::is_function_v<decltype(T::value)>>
        {
        };

        export template <class T>
        constexpr bool has_static_member_object_value_v = has_static_member_object_value<T>::value;

        export template <class T, class = void> struct has_static_member_object_epsilon : std::false_type
        {
        };

        template <class T>
        struct has_static_member_object_epsilon<T, std::void_t<decltype(std::declval<T>().epsilon)>>
            : std::bool_constant<!std::is_member_pointer_v<decltype(&T::epsilon)> &&
                                 !std::is_function_v<decltype(T::epsilon)>>
        {
        };

        export template <class T>
        constexpr bool has_static_member_object_epsilon_v = has_static_member_object_epsilon<T>::value;

    } // namespace type_traits

    namespace concepts
    {

        // std::convertible_to also requires implicit conversion to work
        // See https://stackoverflow.com/a/76547623
        export template <class From, class To>
        concept explicitly_convertible_to = requires { static_cast<To>(std::declval<From>()); };

        export template <class T>
        concept ostreamable = requires(std::ostringstream &os, T t) { os << t; };

    } // namespace concepts

    export template <typename CharT, std::size_t SIZE> struct fixed_string
    {
            constexpr static std::size_t N = SIZE;
            CharT _data[N + 1] = {};

            constexpr explicit(false) fixed_string(const CharT (&str)[N + 1]) noexcept
            {
                if constexpr (N != 0)
                {
                    for (std::size_t i = 0; i < N; ++i)
                    {
                        _data[i] = str[i];
                    }
                }
            }

            [[nodiscard]] constexpr std::size_t size() const noexcept
            {
                return N;
            }
            [[nodiscard]] constexpr bool empty() const noexcept
            {
                return N == 0;
            }
            [[nodiscard]] constexpr explicit operator std::string_view() const noexcept
            {
                return {_data, N};
            }
            [[nodiscard]] explicit operator std::string() const noexcept
            {
                return {_data, N};
            }
            [[nodiscard]] operator const char *() const noexcept
            {
                return _data;
            }
            [[nodiscard]] constexpr bool operator==(const fixed_string &other) const noexcept
            {
                return std::string_view {_data, N} == std::string_view(other);
            }

            template <std::size_t N2>
            [[nodiscard]] friend constexpr bool operator==(const fixed_string &, const fixed_string<CharT, N2> &)
            {
                return false;
            }
    };

    export template <typename CharT, std::size_t N> fixed_string(const CharT (&str)[N]) -> fixed_string<CharT, N - 1>;

    export struct none
    {
    };

} // namespace ut
