export module ut:utility;

import std;

namespace ut::utility
{

    export template <class> class function;

    export template <class R, class... TArgs> class function<R(TArgs...)>
    {
        public:
            constexpr function() = default;
            template <class T>
            constexpr /*explicit(false)*/ function(T data)
                : invoke_ {invoke_impl<T>}, destroy_ {destroy_impl<T>}, data_ {new T {static_cast<T &&>(data)}}
            {
            }
            constexpr function(function &&other) noexcept
                : invoke_ {static_cast<decltype(other.invoke_) &&>(other.invoke_)},
                  destroy_ {static_cast<decltype(other.destroy_) &&>(other.destroy_)},
                  data_ {static_cast<decltype(other.data_) &&>(other.data_)}
            {
                other.data_ = {};
            }
            constexpr function(const function &) = delete;
            ~function()
            {
                destroy_(data_);
            }

            constexpr function &operator=(const function &) = delete;
            constexpr function &operator=(function &&) = delete;
            [[nodiscard]] constexpr auto operator()(TArgs... args) -> R
            {
                return invoke_(data_, args...);
            }
            [[nodiscard]] constexpr auto operator()(TArgs... args) const -> R
            {
                return invoke_(data_, args...);
            }

        private:
            template <class T> [[nodiscard]] static auto invoke_impl(void *data, TArgs... args) -> R
            {
                return (*static_cast<T *>(data))(args...);
            }

            template <class T> static auto destroy_impl(void *data) -> void
            {
                delete static_cast<T *>(data);
            }

            R (*invoke_)(void *, TArgs...) {};
            void (*destroy_)(void *) {};
            void *data_ {};
    };

    export [[nodiscard]] auto is_match(std::string_view input, std::string_view pattern) -> bool
    {
        if (std::empty(pattern))
        {
            return std::empty(input);
        }

        if (std::empty(input))
        {
            return pattern[0] == '*' ? is_match(input, pattern.substr(1)) : false;
        }

        if (pattern[0] != '?' and pattern[0] != '*' and pattern[0] != input[0])
        {
            return false;
        }

        if (pattern[0] == '*')
        {
            for (decltype(std::size(input)) i = 0u; i <= std::size(input); ++i)
            {
                if (is_match(input.substr(i), pattern.substr(1)))
                {
                    return true;
                }
            }
            return false;
        }

        return is_match(input.substr(1), pattern.substr(1));
    }

    export template <class TPattern, class TStr>
    [[nodiscard]] constexpr auto match(const TPattern &pattern, const TStr &str) -> std::vector<TStr>
    {
        std::vector<TStr> groups {};
        auto pi = 0u;
        auto si = 0u;

        const auto matcher = [&](char b, char e, char c = 0) {
            const auto match = si;
            while (str[si] and str[si] != b and str[si] != c)
            {
                ++si;
            }
            groups.emplace_back(str.substr(match, si - match));
            while (pattern[pi] and pattern[pi] != e)
            {
                ++pi;
            }
            pi++;
        };

        while (pi < std::size(pattern) && si < std::size(str))
        {
            if (pattern[pi] == '\'' and str[si] == '\'' and pattern[pi + 1] == '{')
            {
                ++si;
                matcher('\'', '}');
            }
            else if (pattern[pi] == '{')
            {
                matcher(' ', '}', ',');
            }
            else if (pattern[pi] != str[si])
            {
                return {};
            }
            ++pi;
            ++si;
        }

        if (si < str.size() or pi < std::size(pattern))
        {
            return {};
        }

        return groups;
    }

    export template <class T = std::string_view, class TDelim>
    [[nodiscard]] auto split(T input, TDelim delim) -> std::vector<T>
    {
        std::vector<T> output {};
        std::size_t first {};
        while (first < std::size(input))
        {
            const auto second = input.find_first_of(delim, first);
            if (first != second)
            {
                output.emplace_back(input.substr(first, second - first));
            }
            if (second == T::npos)
            {
                break;
            }
            first = second + 1;
        }
        return output;
    }

    export constexpr auto regex_match(const char *str, const char *pattern) -> bool
    {
        if (*pattern == '\0' && *str == '\0')
        {
            return true;
        }
        if (*pattern == '\0' && *str != '\0')
        {
            return false;
        }
        if (*str == '\0' && *pattern != '\0')
        {
            return false;
        }
        if (*pattern == '.')
        {
            return regex_match(str + 1, pattern + 1);
        }
        if (*pattern == *str)
        {
            return regex_match(str + 1, pattern + 1);
        }
        return false;
    }

} // namespace ut::utility
