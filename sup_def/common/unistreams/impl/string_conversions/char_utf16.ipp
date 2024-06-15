
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
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION uni::detail::string_conversions<char, utf16_char>
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING PP_STRINGIZE(::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION)

template <>
struct ::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef char       char_from;
        typedef utf16_char char_to;

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

            const size_t cu_count = ::simdutf::utf16_length_from_utf8(from.c_str(), from.size());

            const size_t tmp_to_size = sizeof(base_char_to) * (cu_count + 1);
            u_array_t<base_char_to> tmp_to;

            if (tmp_to_size > alloca_max_size)
                tmp_to = u_array_t<base_char_to>(
                    new base_char_to[tmp_to_size / sizeof(base_char_to)],
                    &array_default_deleter<base_char_to>
                );
            else
                tmp_to = u_array_t<base_char_to>(
                    static_cast<base_char_to*>(
                        alloca(tmp_to_size)
                    ),
                    &array_alloca_deleter<base_char_to>
                );
            
            using func_type = decltype(::simdutf::convert_utf8_to_utf16_with_errors);
            func_type* func = nullptr;

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
                    throw InternalError(
                        UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING ": desired endianness is undefined"
                    );
            }

            const ::simdutf::result res = func(
                from.c_str(),
                from.size(),
                tmp_to.get()
            );
            unlikely_if (res.error != ::simdutf::error_code::SUCCESS)
                throw InternalError(
                    UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING ": conversion failed"
                    " with error code " + std::to_string(res.error) +
                    "(" + magic_enum::enum_name(res.error) + ")"    +
                    " at position "     + std::to_string(res.count)
                );

            const std::vector<char_to> vec_to = traits_to::from_base_char(
                tmp_to.get(),
                res.count,
                end_to
            );

            return to_type(vec_to.cbegin(), vec_to.cend());
        }
};
                           
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION uni::detail::string_conversions<utf16_char, char>
#define UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING PP_STRINGIZE(::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION)

template <>
struct ::UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
    : protected ::uni::detail::str_conv_base
{
    private:
        typedef utf16_char char_from;
        typedef char       char_to;

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
            
            // ::uni::utf16_char stores endianness
            const ::uni::endianness end_from = from.get_endianness();
            const std::vector<base_char_from> vec_from = traits_from::to_base_char(from.c_str(), from.size());

            size_t cu_count;
            if (end_from == ::uni::endianness::big)
                cu_count = ::simdutf::utf8_length_from_utf16be(vec_from.data(), vec_from.size());
            else if (end_from == ::uni::endianness::little)
                cu_count = ::simdutf::utf8_length_from_utf16le(vec_from.data(), vec_from.size());
            unlikely_else
                throw InternalError(
                    UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING ": source endianness is undefined"
                );

            to_type to(cu_count + 1, char_to{});

            using func_type = decltype(::simdutf::convert_utf16_to_utf8_with_errors);
            func_type* func = nullptr;

            if (end_from == ::uni::endianness::big)
                func = &::simdutf::convert_utf16be_to_utf8_with_errors;
            else if (end_from == ::uni::endianness::little)
                func = &::simdutf::convert_utf16le_to_utf8_with_errors;

            const ::simdutf::result res = func(
                vec_from.data(),
                vec_from.size(),
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

#undef  CURRENT_CONVERTER_TYPE
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION
#undef  UNISTREAMS_CURRENT_STRCONV_SPECIALIZATION_STRING

