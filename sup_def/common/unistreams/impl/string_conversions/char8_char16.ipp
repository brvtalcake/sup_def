
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

#include <magic_enum/magic_enum_all.hpp>
#include <simdutf.h>

#undef  CURRENT_CONVERTER_TYPE
#define CURRENT_CONVERTER_TYPE void

#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION uni::detail::string_conversions<char8_t, char16_t>
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING PP_STRINGIZE(::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION)

template <>
struct ::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef char8_t  char_from;
        typedef char16_t char_to;

        typedef ::uni::char_traits<char_from> traits_from;
        typedef ::uni::char_traits<char_to>   traits_to;

        typedef traits_from::base_char_type base_char_from;
        typedef traits_to  ::base_char_type base_char_to;

        typedef CURRENT_CONVERTER_TYPE converter_type;

    public:
        template <typename AllocTo, typename AllocFrom>
        static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
            const ::uni::string<char_from, traits_from, AllocFrom>& from,
            const ::uni::endianness end_to
        )
        {
            using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
            using to_type   = ::uni::string<char_to  , traits_to  , AllocTo>;

            unlikely_if (from.empty())
                return to_type();

            const size_t cu_count = ::simdutf::utf16_length_from_utf8(
                reinterpret_cast<const char*>(from.c_str()),
                from.size()
            );
            to_type to(cu_count + 1, char_to{});

            using func_type = decltype(::simdutf::convert_utf8_to_utf16_with_errors);
            func_type* func = nullptr;

            if (end_to == ::uni::endianness::little)
                func = &::simdutf::convert_utf8_to_utf16le_with_errors;
            else if (end_to == ::uni::endianness::big)
                func = &::simdutf::convert_utf8_to_utf16be_with_errors;
            unlikely_else
                throw InternalError(
                    UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING ": desired endianness is undefined"
                );

            const ::simdutf::result res = func(
                reinterpret_cast<const char*>(from.c_str()),
                from.size(),
                to.data()
            );
            unlikely_if (res.error != ::simdutf::error_code::SUCCESS)
                throw InternalError(
                    UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING ": conversion failed"
                    " with error code " + std::to_string(res.error) +
                    "(" + magic_enum::enum_name(res.error) + ")"    +
                    " at position "     + std::to_string(res.count)
                );

            return to;
        }
            
};
                           
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION uni::detail::string_conversions<char16_t, char8_t>
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING PP_STRINGIZE(::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION)

template <>
struct ::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef char16_t char_from;
        typedef char8_t  char_to;

        typedef ::uni::char_traits<char_from> traits_from;
        typedef ::uni::char_traits<char_to>   traits_to;

        typedef traits_from::base_char_type base_char_from;
        typedef traits_to  ::base_char_type base_char_to;

        typedef CURRENT_CONVERTER_TYPE converter_type;

    public:
        template <typename AllocTo, typename AllocFrom>
        static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
            const ::uni::string<char_from, traits_from, AllocFrom>& from,
            const ::uni::endianness end_from
        )
        {
            using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
            using to_type   = ::uni::string<char_to  , traits_to  , AllocTo>;

            unlikely_if (from.empty())
                return to_type();
            
            using func_type = decltype(::simdutf::convert_utf16_to_utf8_with_errors);
            func_type* func = nullptr;

            size_t cu_count;
            
            if (end_from == ::uni::endianness::big)
            {
                cu_count =  ::simdutf::utf8_length_from_utf16be(from.c_str(), from.size());
                func     = &::simdutf::convert_utf16be_to_utf8_with_errors;
            }
            else if (end_from == ::uni::endianness::little)
            {
                cu_count =  ::simdutf::utf8_length_from_utf16le(from.c_str(), from.size());
                func     = &::simdutf::convert_utf16le_to_utf8_with_errors;
            }
            unlikely_else
                throw InternalError(
                    UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING ": source endianness is undefined"
                );
            
            to_type to(cu_count + 1, char_to{});

            const ::simdutf::result res = func(
                from.c_str(),
                from.size(),
                reinterpret_cast<char*>(to.data())
            );
            unlikely_if (res.error != ::simdutf::error_code::SUCCESS)
                throw InternalError(
                    UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING ": conversion failed"
                    " with error code " + std::to_string(res.error) +
                    "(" + magic_enum::enum_name(res.error) + ")"    +
                    " at position "     + std::to_string(res.count)
                );

            return to;
        }
};

#undef  CURRENT_CONVERTER_TYPE
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING

