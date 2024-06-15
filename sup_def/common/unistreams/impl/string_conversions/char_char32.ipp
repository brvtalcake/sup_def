
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

#include <simdutf.h>
#include <magic_enum/magic_enum_all.hpp>

#undef  CURRENT_CONVERTER_TYPE
#define CURRENT_CONVERTER_TYPE void

#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION uni::detail::string_conversions<char, char32_t>
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING PP_STRINGIZE(::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION)

template <>
struct ::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef char     char_from;
        typedef char32_t char_to;

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

            const size_t cu_count = ::simdutf::utf32_length_from_utf8(from.c_str(), from.size());
            to_type to(cu_count + 1, char_to{});

            decltype(auto) func = &::simdutf::convert_utf8_to_utf32_with_errors;

            switch (bool need_bswap = false; end_to)
            {
                case ::uni::endianness::little:
                    need_bswap = true;
                    case_fallthrough;
                case ::uni::endianness::big: {
                        const ::simdutf::result res = func(from.c_str(), from.size(), to.data());
                        unlikely_if (res.error != ::simdutf::error_code::SUCCESS)
                            throw InternalError(
                                UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING ": conversion failed"
                                " with error code " + std::to_string(res.error) +
                                "(" + magic_enum::enum_name(res.error) + ")"    +
                                " at position "     + std::to_string(res.count)
                            );
                        if (need_bswap)
                            for (char_to& c : to)
                                c = std::byteswap(c);
                    }
                    break;
#if SUPDEF_HAVE_CPP_ATTRIBUTE_UNLIKELY
                [[unlikely]]
#endif
                default:
                    throw InternalError(
                        UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING ": desired endianness is undefined"
                    );
            }

            return to;
        }
};
                           
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION uni::detail::string_conversions<char32_t, char>
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING PP_STRINGIZE(::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION)

template <>
struct ::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef char32_t char_from;
        typedef char     char_to;

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

            from_type from_swapped;
            const from_type* maybe_swapped_from;

            switch (end_from)
            {
                case ::uni::endianness::little:
                    maybe_swapped_from = std::addressof(from_swapped);
                    for (const char_from& c : from)
                        from_swapped.push_back(std::byteswap(c));
                    break;
                case ::uni::endianness::big:
                    maybe_swapped_from = std::addressof(from);
                    break;
#if SUPDEF_HAVE_CPP_ATTRIBUTE_UNLIKELY
                [[unlikely]]
#endif
                default:
                    throw InternalError(
                        UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING ": source endianness is undefined"
                    );
            }

            const size_t cu_count = ::simdutf::utf8_length_from_utf32(maybe_swapped_from->c_str(), maybe_swapped_from->size());
            to_type to(cu_count + 1, char_to{});

            decltype(auto) func = &::simdutf::convert_utf32_to_utf8_with_errors;

            const ::simdutf::result res = func(maybe_swapped_from->c_str(), maybe_swapped_from->size(), to.data());
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

