        // Base template
#if 0
        template <typename C1, typename C2>
            requires (CharacterType<C1> || std::same_as<C1, std::filesystem::path>) && CharacterType<C2>
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */);
#else
        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */) = delete;
#endif

                /* Internal */ /* External */
        template <typename C1, typename C2>
            requires IsValidCodeCvt<C1, C2> && (!(std::same_as<C1, C2> && (std::same_as<char, C1> || std::same_as<char, C2>))) /* No `<char, char>` */ &&
                     (CharacterType<C1> && CharacterType<C2>) // double check, even if garanteed by `IsValidCodeCvt`, in case I fucked up
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            using facet_type = typename std::codecvt<C1, C2, std::mbstate_t>;
            const facet_type& f = std::use_facet<std::codecvt<C1, C2, std::mbstate_t>>(std::locale());
            std::mbstate_t mb = std::mbstate_t();
            std::codecvt_base::result res;
            std::size_t factor = 4;
            /* Internal */
            std::basic_string<C1> ret(str.size() * factor, static_cast<C1>('\0'));
            C1* internal_next = nullptr;
            const C2* external_next = nullptr;
            do
            {
                mb = std::mbstate_t();
                ret.clear();
                internal_next = nullptr;
                external_next = nullptr;
                res = f.in(
                    mb, &str[0], &str[str.size()], external_next, &ret[0], &ret[ret.size()], internal_next
                );
                factor++;
            } while (res == std::codecvt_base::partial);
            if (res == std::codecvt_base::error)
                throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "convert(const std::basic_string<C2>&): std::codecvt_base::error");
            ret.resize(internal_next - &ret[0]);
            return ret;
        }

        // Assuming UTF-8 locale and that exec encoding for `char` is UTF-8
        template <>
        inline std::basic_string<char> convert<char, char8_t>(const std::basic_string<char8_t>& str /* External */)
        {
            std::basic_string<char> ret;
            ret.reserve(str.size());
            for (auto& c : str)
                ret += reinterpret_cast<const char&>(c);
            return ret;
        }

        template <>
        inline std::basic_string<char8_t> convert<char8_t, char>(const std::basic_string<char>& str /* External */)
        {
            std::basic_string<char8_t> ret;
            ret.reserve(str.size());
            for (auto& c : str)
                ret += reinterpret_cast<const char8_t&>(c);
            return ret;
        }

        template <>
        inline std::basic_string<wchar_t> convert<wchar_t, char8_t>(const std::basic_string<char8_t>& str /* External */)
        {
            return convert<wchar_t>(convert<char>(str));
        }

        template <>
        inline std::basic_string<char8_t> convert<char8_t, wchar_t>(const std::basic_string<wchar_t>& str /* External */)
        {
            return convert<char8_t>(convert<char>(str));
        }

        template <typename C1, typename C2>
            requires CAN_CONSTRUCT_FROM((C2), (char16_t, char32_t)) && std::same_as<char, C1> && (CharacterType<C1> && CharacterType<C2>)
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            return convert<C1>(convert<char8_t>(str));
        }

        template <typename C1, typename C2>
            requires CAN_CONSTRUCT_FROM((C1), (char16_t, char32_t)) && std::same_as<char, C2> && (CharacterType<C1> && CharacterType<C2>)
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            return convert<C1>(convert<char8_t>(str));
        }

        template <typename C1, typename C2>
            requires CAN_CONSTRUCT_FROM((C2), (char16_t, char32_t)) && std::same_as<wchar_t, C1> && (CharacterType<C1> && CharacterType<C2>)
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            return convert<C1>(convert<char>(convert<char8_t>(str)));
        }

        template <typename C1, typename C2>
            requires CAN_CONSTRUCT_FROM((C1), (char16_t, char32_t)) && std::same_as<wchar_t, C2> && (CharacterType<C1> && CharacterType<C2>)
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            return convert<C1>(convert<char8_t>(convert<char>(str)));
        }

        template <typename C1, typename C2>
            requires (CharacterType<C1> && CharacterType<C2>) && std::same_as<C1, C2>
        constexpr inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            return std::basic_string<C1>(str);
        }
        
        // Base template
        template <typename C1>
            requires CharacterType<C1> || std::same_as<C1, std::filesystem::path>
        inline auto convert(const std::filesystem::path& path) = delete;
        
        template <CharacterType C1>
        inline std::basic_string<C1> convert(const std::filesystem::path& path)
        {
            return convert<C1>(path.string());
        }

        template <std::same_as<std::filesystem::path> C1>
        inline C1 convert(const std::filesystem::path& path)
        {
            return path;
        }

        // Base template
        template <typename C1, typename C2>
            requires std::same_as<C1, std::filesystem::path> && CharacterType<C2>
        inline std::filesystem::path convert(const std::basic_string<C2>& str /* External */)
        {
            return std::filesystem::path(convert<char>(str));
        }

#if 0
        // Base template
        template <typename C1, typename C2>
        inline C1 convert(const std::basic_string<C2>& str /* External */) = delete;

        // Base template
        template <typename C1, typename C2>
        inline std::basic_string<C1> convert(const C2& c) = delete;
#endif

        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline std::basic_string<C1> convert(const C2& c)
        {
            return convert<C1>(std::basic_string<C2>(1, c));
        }

        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline std::basic_string<C1> convert(const C2&& c)
        {
            return convert<C1>(std::basic_string<C2>(1, c));
        }

        // TODO: Make this a convert overload rather than a convert_num overload
        /* template <typename C1, typename C2>
            requires CharacterType<C1> && std::is_arithmetic_v<C2> && (!CharacterType<C2>)
        inline std::basic_string<C1> convert_num(const C2& c)
        {
            std::stringstream ss(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
            ss << c;
            return convert<C1>(ss.str());
        } */

        template <typename C1, typename C2>
            requires std::is_arithmetic_v<C1> && (!CharacterType<C1>) && CharacterType<C2>
        inline C1 convert_num(const std::basic_string<C2>& str /* External */)
        {
            std::stringstream ss(convert<char>(str), std::ios_base::in | std::ios_base::out | std::ios_base::binary);
            C1 ret;
            ss >> ret;
            return ret;
        }

        template <typename C1, typename C2>
            requires std::is_arithmetic_v<C1> && (!CharacterType<C1>) && CharacterType<C2>
        inline C1 convert_num(const C2& c)
        {
            return convert_num<C1>(std::basic_string<C2>(1, c));
        }

#ifdef CONVERT
    #undef CONVERT
#endif
#define CONVERT(TYPE, STR_OR_CHAR) ::SupDef::Util::convert<TYPE>(STR_OR_CHAR)