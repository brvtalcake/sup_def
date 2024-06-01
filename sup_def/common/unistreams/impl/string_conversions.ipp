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

#if !INCLUDED_FROM_UNISTREAMS_SOURCE
    #error "This file shall not be included from outside the unistreams library"
#endif

#include <simdutf.h>

// List TODO:
//
// char <--> wchar_t                 : DONE, NEEDS TESTING
// char <--> char8_t                 : DONE, NEEDS TESTING
// char <--> char16_t                : DONE, NEEDS TESTING
// char <--> char32_t                : DONE, NEEDS TESTING
// char <--> utf8_char               : TODO
// char <--> utf16_char              : TODO
// char <--> utf32_char              : TODO
//
// wchar_t <--> char8_t              : TODO
// wchar_t <--> char16_t             : TODO
// wchar_t <--> char32_t             : TODO
// wchar_t <--> utf8_char            : TODO
// wchar_t <--> utf16_char           : TODO
// wchar_t <--> utf32_char           : TODO
//
// char8_t <--> char16_t             : TODO
// char8_t <--> char32_t             : TODO
// char8_t <--> utf8_char            : TODO
// char8_t <--> utf16_char           : TODO
// char8_t <--> utf32_char           : TODO
//
// char16_t <--> char32_t            : TODO
// char16_t <--> utf8_char           : TODO
// char16_t <--> utf16_char          : TODO
// char16_t <--> utf32_char          : TODO
//
// char32_t <--> utf8_char           : TODO
// char32_t <--> utf16_char          : TODO
// char32_t <--> utf32_char          : TODO
//
// utf8_char <--> utf16_char         : TODO
// utf8_char <--> utf32_char         : TODO
//
// utf16_char <--> utf32_char        : TODO

namespace uni
{
    namespace detail
    {
        namespace priv
        {
            template <typename Alloc, typename Target>
            using rebind_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Target>;
        }
        static std::locale user_preferred_unicode_aware_locale();
        template <typename CharT>
            requires std::same_as<CharT, char16_t> || std::same_as<CharT, char32_t>
        consteval endianness get_literal_endianness();

        struct str_conv_base
        {
            protected:
                static constexpr std::locale getloc()
                {
                    if (user_preferred_unicode_aware_locale() != std::locale())
                        ::SupDef::set_app_locale();
                    return std::locale();
                }

                template <typename T>
                using u_ptr_deleter_t = void(*)(T*);

                template <typename T>
                using u_array_deleter_t = void(*)(T*);

                template <typename T>
                using u_ptr_t = std::unique_ptr<T, u_ptr_deleter_t<T>>;

                template <typename T>
                using u_array_t = std::unique_ptr<T[], u_array_deleter_t<T>>;

                template <typename T>
                static constexpr void alloca_deleter(T* ptr)
                { }
                template <typename T>
                static constexpr void default_deleter(T* ptr)
                {
                    static const std::default_delete<T> del;
                    del(ptr);
                }

                template <typename T>
                static constexpr void array_alloca_deleter(T* ptr)
                { }
                template <typename T>
                static constexpr void array_default_deleter(T* ptr)
                {
                    static const std::default_delete<T[]> del;
                    del(ptr);
                }

                static constexpr size_t alloca_max_size = 4 * 1024;
        };
    }
}

#include <sup_def/common/unistreams/impl/string_conversions/char_wchar.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char_char8.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char_char16.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char_char32.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char_utf8.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char_utf16.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char_utf32.ipp>

#include <sup_def/common/unistreams/impl/string_conversions/wchar_char8.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/wchar_char16.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/wchar_char32.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/wchar_utf8.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/wchar_utf16.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/wchar_utf32.ipp>

#include <sup_def/common/unistreams/impl/string_conversions/char8_char16.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char8_char32.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char8_utf8.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char8_utf16.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char8_utf32.ipp>

#include <sup_def/common/unistreams/impl/string_conversions/char16_char32.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char16_utf8.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char16_utf16.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char16_utf32.ipp>

#include <sup_def/common/unistreams/impl/string_conversions/char32_utf8.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char32_utf16.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/char32_utf32.ipp>

#include <sup_def/common/unistreams/impl/string_conversions/utf8_utf16.ipp>
#include <sup_def/common/unistreams/impl/string_conversions/utf8_utf32.ipp>

#include <sup_def/common/unistreams/impl/string_conversions/utf16_utf32.ipp>
