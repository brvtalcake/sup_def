
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

#undef CURRENT_CONVERTER_TYPE
#define CURRENT_CONVERTER_TYPE void

template <>
struct ::uni::detail::string_conversions<char, char16_t>
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef char    char_from;
        typedef char16_t char_to;

        typedef ::uni::char_traits<char_from> traits_from;
        typedef ::uni::char_traits<char_to>   traits_to;

        typedef traits_from::base_char_type base_char_from;
        typedef traits_to  ::base_char_type base_char_to;

        typedef CURRENT_CONVERTER_TYPE converter_type;

    public:
        template <typename AllocFrom, typename AllocTo>
        static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
            const ::uni::string<char_from, traits_from, AllocFrom>& from,
            const ::uni::endianness end_to
        )
        {
            using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
            using to_type   = ::uni::string<char_to  , traits_to  , AllocTo>;

            unlikely_if (from.size() == 0)
                return to_type();

            const size_t cp_count = ::simdutf::count_utf8(from.c_str(), from.size());
            to_type to(cp_count * 2, base_char_to{0});

            using func_type = decltype(&::simdutf::convert_utf8_to_utf16_with_errors);
            func_type func = nullptr;

            switch (end_to)
            {
                case ::uni::endianness::little:
                    func = &::simdutf::convert_utf8_to_utf16le_with_errors;
                    break;
                case ::uni::endianness::big:
                    func = &::simdutf::convert_utf8_to_utf16be_with_errors;
                    break;
#if SUPDEF_HAVE_CPP_ATTRIBUTE_UNLIKELY
                [[unlikely]]
#endif
                default:
                    throw InternalError("uni::detail::string_conversions<char, char16_t>: desired endianness is undefined");
            }

            const ::simdutf::result res = func(from.c_str(), from.size(), to.data(), to.size());
            unlikely_if (res.error != ::simdutf::error_code::SUCCESS)
                throw InternalError(
                    "uni::detail::string_conversions<char, char16_t>: conversion failed"
                    " with error code " + std::to_string(res.error) +
                    " at position " + std::to_string(res.count)
                );

            to.shrink_to_fit();
            return to;
        }
};

template <>
struct ::uni::detail::string_conversions<char16_t, char>
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef char16_t char_from;
        typedef char     char_to;

        typedef ::uni::char_traits<char_from> traits_from;
        typedef ::uni::char_traits<char_to>   traits_to;

        typedef traits_from::base_char_type base_char_from;
        typedef traits_to  ::base_char_type base_char_to;

        typedef CURRENT_CONVERTER_TYPE converter_type;

    public:
        template <typename AllocFrom, typename AllocTo>
        static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
            const ::uni::string<char_from, traits_from, AllocFrom>& from,
            const ::uni::endianness end_from
        )
        {
            using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
            using to_type   = ::uni::string<char_to  , traits_to  , AllocTo>;

            unlikely_if (from.size() == 0)
                return to_type();

            const size_t cp_count = ::simdutf::count_utf16(from.c_str(), from.size());
            to_type to(cp_count, base_char_to{0});

            using func_type = decltype(&::simdutf::convert_utf16_to_utf8_with_errors);
            func_type func = nullptr;

            switch (end_from)
            {
                case ::uni::endianness::little:
                    func = &::simdutf::convert_utf16le_to_utf8_with_errors;
                    break;
                case ::uni::endianness::big:
                    func = &::simdutf::convert_utf16be_to_utf8_with_errors;
                    break;
#if SUPDEF_HAVE_CPP_ATTRIBUTE_UNLIKELY
                [[unlikely]]
#endif
                default:
                    throw InternalError("uni::detail::string_conversions<char16_t, char>: source endianness is undefined");
            }

            const ::simdutf::result res = func(from.c_str(), from.size(), to.data(), to.size());
            unlikely_if (res.error != ::simdutf::error_code::SUCCESS)
                throw InternalError(
                    "uni::detail::string_conversions<char16_t, char>: conversion failed"
                    " with error code " + std::to_string(res.error) +
                    " at position " + std::to_string(res.count)
                );

            to.shrink_to_fit();
            return to;
        }
};

