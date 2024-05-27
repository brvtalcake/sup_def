
/* 
 * MIT License
 * 
 * Copyright (c) 2023-2024 Axel PASCON
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#undef CURRENT_CONVERTER_TYPE
#define CURRENT_CONVERTER_TYPE std::codecvt<wchar_t, char, std::mbstate_t>

template <>
struct ::uni::detail::string_conversions<char, wchar_t>
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef char    char_from;
        typedef wchar_t char_to;

        typedef ::uni::char_traits<char_from> traits_from;
        typedef ::uni::char_traits<char_to>   traits_to;

        typedef traits_from::base_char_type base_char_from;
        typedef traits_to  ::base_char_type base_char_to;

        typedef CURRENT_CONVERTER_TYPE converter_type;

    public:
        template <typename AllocFrom, typename AllocTo>
        static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
            const ::uni::string<char_from, traits_from, AllocFrom>& from
        )
        {
            using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
            using to_type   = ::uni::string<char_to  , traits_to  , AllocTo>;

            unlikely_if (f.size() == 0)
                return to_type();

            to_type to(f.size(), base_char_to{0});

            std::mbstate_t state{0};
            std::locale loc = getloc();
            const converter_type& converter = std::use_facet<converter_type>(loc);
            std::codecvt_base::result res;

            const base_char_from* from_start = from.c_str();
            const base_char_from* from_end = from_start + from.length();
            const base_char_from* from_next = nullptr;

            base_char_to* to_start = to.data();
            base_char_to* to_end = to_start + to.size();
            base_char_to* to_next = nullptr;

            do
            {    
                res = converter.in(
                    state,
                    from_start,
                    from_end,
                    from_next,
                    to_start,
                    to_end,
                    to_next
                );
                        
                if (res == std::codecvt_base::partial)
                {
                    to.resize(to.size() * 2);
                    to_start = to.data();
                    to_end = to_start + to.size();
                    from_start = from.c_str();
                    from_end = from_start + from.size();
                }
            } while (res == std::codecvt_base::partial);

            if (res == std::codecvt_base::error)
                throw InternalError("Error while converting string");
            unlikely_if (res == std::codecvt_base::noconv)
                throw InternalError("No conversion needed");

            to.shrink_to_fit();
            return to;
        }
};

template <>
struct ::uni::detail::string_conversions<wchar_t, char>
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef wchar_t char_from;
        typedef char    char_to;

        typedef ::uni::char_traits<char_from> traits_from;
        typedef ::uni::char_traits<char_to>   traits_to;

        typedef traits_from::base_char_type base_char_from;
        typedef traits_to  ::base_char_type base_char_to;

        typedef CURRENT_CONVERTER_TYPE converter_type;

    public:
        template <typename AllocFrom, typename AllocTo>
        static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
            const ::uni::string<char_from, traits_from, AllocFrom>& from
        )
        {
            using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
            using to_type   = ::uni::string<char_to  , traits_to  , AllocTo>;

            unlikely_if (f.size() == 0)
                return to_type();

            to_type to(f.size(), base_char_to{0});

            std::mbstate_t state{0};
            std::locale loc = getloc();
            const converter_type& converter = std::use_facet<converter_type>(loc);
            std::codecvt_base::result res;

            const base_char_from* from_start = from.c_str();
            const base_char_from* from_end = from_start + from.length();
            const base_char_from* from_next = nullptr;

            base_char_to* to_start = to.data();
            base_char_to* to_end = to_start + to.size();
            base_char_to* to_next = nullptr;

            do
            {    
                res = converter.out(
                    state,
                    from_start,
                    from_end,
                    from_next,
                    to_start,
                    to_end,
                    to_next
                );
                        
                if (res == std::codecvt_base::partial)
                {
                    to.resize(to.size() * 2);
                    to_start = to.data();
                    to_end = to_start + to.size();
                    from_start = from.c_str();
                    from_end = from_start + from.size();
                }
            } while (res == std::codecvt_base::partial);

            if (res == std::codecvt_base::error)
                throw InternalError("Error while converting string");
            unlikely_if (res == std::codecvt_base::noconv)
                throw InternalError("No conversion needed");

            to.shrink_to_fit();
            return to;
        }

};

#undef CURRENT_CONVERTER_TYPE