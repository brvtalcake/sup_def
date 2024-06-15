
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


#undef  CURRENT_CONVERTER_TYPE
#define CURRENT_CONVERTER_TYPE void

#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION uni::detail::string_conversions<char8_t, utf8_char>
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING PP_STRINGIZE(::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION)

template <>
struct ::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef char8_t   char_from;
        typedef utf8_char char_to;

        typedef ::uni::char_traits<char_from> traits_from;
        typedef ::uni::char_traits<char_to>   traits_to;

        typedef traits_from::base_char_type base_char_from;
        typedef traits_to  ::base_char_type base_char_to;

        typedef CURRENT_CONVERTER_TYPE converter_type;

    public:
        template <typename AllocTo, typename AllocFrom>
        static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
            const ::uni::string<char_from, traits_from, AllocFrom>& from
        )
        {
            using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
            using to_type   = ::uni::string<char_to  , traits_to  , AllocTo>;

            unlikely_if (from.empty())
                return to_type();

            const std::vector<char_to> vec_to = traits_to::from_base_char(
                from.c_str(),
                from.size()
            );

            return to_type(vec_to.cbegin(), vec_to.cend());
        }
};
                           
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION uni::detail::string_conversions<utf8_char, char8_t>
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING PP_STRINGIZE(::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION)

template <>
struct ::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef utf8_char char_from;
        typedef char8_t   char_to;

        typedef ::uni::char_traits<char_from> traits_from;
        typedef ::uni::char_traits<char_to>   traits_to;

        typedef traits_from::base_char_type base_char_from;
        typedef traits_to  ::base_char_type base_char_to;

        typedef CURRENT_CONVERTER_TYPE converter_type;

    public:
        template <typename AllocTo, typename AllocFrom>
        static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
            const ::uni::string<char_from, traits_from, AllocFrom>& from
        )
        {
            using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
            using to_type   = ::uni::string<char_to  , traits_to  , AllocTo>;

            unlikely_if (from.empty())
                return to_type();

            const std::vector<base_char_from> vec_from = traits_from::to_base_char(
                from.c_str(),
                from.size()
            );

            return to_type(
                vec_from.data(),
                vec_from.size()
            );
        }
};

#undef  CURRENT_CONVERTER_TYPE
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING

