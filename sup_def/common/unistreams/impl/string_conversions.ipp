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

// List TODO:
//
//  char <--> wchar_t          : DONE, NEED_TEST
//  char <--> utf8_char        : DONE, NEED_TEST
//  char <--> utf16_char       : DONE, NEED_TEST
//  char <--> utf32_char       : DONE, NEED_TEST
//
//  wchar_t <--> utf8_char     : DONE, NEED_TEST
//  wchar_t <--> utf16_char    : DONE, NEED_TEST
//  wchar_t <--> utf32_char    : DONE, NEED_TEST
//
//  utf8_char <--> utf16_char  : DONE, NEED_TEST
//  utf8_char <--> utf32_char  : DONE, NEED_TEST
//
//  utf16_char <--> utf32_char : DONE, NEED_TEST
//
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
        };
        // char <--> wchar_t
        // Specializations :
        //  string_conversions<char, wchar_t>
        //  string_conversions<wchar_t, char>
        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        > requires CAN_CONSTRUCT_FROM(
            (CharTFrom, CharTTo),
            (char, wchar_t)
        )
        struct string_conversions<CharTFrom, CharTTo> : public str_conv_base
        {
            public:
                typedef CharTFrom char_from;
                typedef CharTTo char_to;

                typedef ::uni::char_traits<char_from> traits_from;
                typedef ::uni::char_traits<char_to> traits_to;

                typedef traits_from::base_char_type base_char_from;
                typedef traits_to::base_char_type base_char_to;

            private:
                typedef std::codecvt<wchar_t, char, std::mbstate_t> convertor_type;

                static constexpr bool from_to_do_out = std::same_as<base_char_from, wchar_t> && std::same_as<base_char_to, char>;
                static constexpr bool from_to_do_in  = std::same_as<base_char_from, char>    && std::same_as<base_char_to, wchar_t>;

                static_assert(from_to_do_out != from_to_do_in);

            public:
                template <typename AllocFrom, typename AllocTo = priv::rebind_alloc<AllocFrom, char_to>>
                static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
                    const ::uni::string<char_from, traits_from, AllocFrom>& f
                )
                {
                    using to_type = ::uni::string<char_to, traits_to, AllocTo>;
                    using from_type = ::uni::string<char_from, traits_from, AllocFrom>;

                    probably_if (f.size() == 0, 5, CHAOS)
                        return to_type();
                    
                    to_type to;
                    from_type from = f;
                    
                    std::basic_string<base_char_from> tmp_from(from.c_str(), from.length());
                    std::basic_string<base_char_to> tmp_to(from.size(), base_char_to());

                    std::mbstate_t state{0};
                    auto loc = getloc();
                    const convertor_type& convertor = std::use_facet<convertor_type>(loc);
                    std::codecvt_base::result res;

                    const base_char_from* from_start = tmp_from.c_str();
                    const base_char_from* from_end = from_start + tmp_from.size();
                    const base_char_from* from_next = nullptr;
                    base_char_to* to_start = tmp_to.data();
                    base_char_to* to_end = to_start + tmp_to.size();
                    base_char_to* to_next = nullptr;

                    do
                    {
                        if constexpr (from_to_do_out)
                        {
                            res = convertor.out(
                                state,
                                from_start,
                                from_end,
                                from_next,
                                to_start,
                                to_end,
                                to_next
                            );
                        }
                        else
                        {
                            res = convertor.in(
                                state,
                                from_start,
                                from_end,
                                from_next,
                                to_start,
                                to_end,
                                to_next
                            );
                        }
                        if (res == std::codecvt_base::partial)
                        {
                            tmp_to.resize(tmp_to.size() * 2);
                            to_start = tmp_to.data();
                            to_end = to_start + tmp_to.size();
                            from_start = tmp_from.c_str();
                            from_end = from_start + tmp_from.size();
                        }
                    } while (res == std::codecvt_base::partial);
                    if (res == std::codecvt_base::error)
                        throw InternalError("Error while converting string");
                    unlikely_if (res == std::codecvt_base::noconv)
                        throw InternalError("No conversion needed");

                    to = to_type(tmp_to.c_str());

                    return to;
                }
        };

        // char <--> utf8_char
        // Specializations :
        //  string_conversions<char, utf8_char>
        //  string_conversions<utf8_char, char>
        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        > requires CAN_CONSTRUCT_FROM(
            (CharTFrom, CharTTo),
            (char, utf8_char)
        )
        struct string_conversions<CharTFrom, CharTTo> : public str_conv_base
        {
            typedef CharTFrom char_from;
            typedef CharTTo char_to;

            typedef ::uni::char_traits<char_from> traits_from;
            typedef ::uni::char_traits<char_to> traits_to;

            typedef traits_from::base_char_type base_char_from;
            typedef traits_to::base_char_type base_char_to;

            template <typename AllocFrom, typename AllocTo = priv::rebind_alloc<AllocFrom, char_to>>
            static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
                const ::uni::string<char_from, traits_from, AllocFrom>& f
            )
            {
                using to_type = ::uni::string<char_to, traits_to, AllocTo>;
                using from_type = ::uni::string<char_from, traits_from, AllocFrom>;

                probably_if (f.size() == 0, 5, CHAOS)
                    return to_type();
                
                to_type to;
                from_type from = f;
                
                const char_to* tmp = reinterpret_cast<const char_to*>(from.c_str());
                to = to_type(tmp);

                return to;
            }
        };

        // char <--> utf16_char
        // Specializations :
        //  string_conversions<char, utf16_char>
        //  string_conversions<utf16_char, char>
        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        > requires CAN_CONSTRUCT_FROM(
            (CharTFrom, CharTTo),
            (char, utf16_char)
        )
        struct string_conversions<CharTFrom, CharTTo> : public str_conv_base
        {
            typedef CharTFrom char_from;
            typedef CharTTo char_to;

            typedef ::uni::char_traits<char_from> traits_from;
            typedef ::uni::char_traits<char_to> traits_to;

            typedef traits_from::base_char_type base_char_from;
            typedef traits_to::base_char_type base_char_to;

            template <typename AllocFrom, typename AllocTo = priv::rebind_alloc<AllocFrom, char_to>>
            static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
                const ::uni::string<char_from, traits_from, AllocFrom>& f,
                [[maybe_unused]] const endianness to_endianness = endianness::big // Used when char_to is utf16_char
            )
            {
                using to_type = ::uni::string<char_to, traits_to, AllocTo>;
                using from_type = ::uni::string<char_from, traits_from, AllocFrom>;

                probably_if (f.size() == 0, 5, CHAOS)
                    return to_type();

                if constexpr (detail::has_endianness<char_to>)
                {
                    return ::uni::detail::string_conversions<utf8_char, char_to>()(
                        ::uni::detail::string_conversions<char_from, utf8_char>()(f),
                        to_endianness
                    );
                }
                else
                {
                    return ::uni::detail::string_conversions<utf8_char, char_to>()(
                        ::uni::detail::string_conversions<char_from, utf8_char>()(f)
                    );
                }
            }
        };

        // char <--> utf32_char
        // Specializations :
        //  string_conversions<char, utf32_char>
        //  string_conversions<utf32_char, char>
        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        > requires CAN_CONSTRUCT_FROM(
            (CharTFrom, CharTTo),
            (char, utf32_char)
        )
        struct string_conversions<CharTFrom, CharTTo> : public str_conv_base
        {
            typedef CharTFrom char_from;
            typedef CharTTo char_to;

            typedef ::uni::char_traits<char_from> traits_from;
            typedef ::uni::char_traits<char_to> traits_to;

            typedef traits_from::base_char_type base_char_from;
            typedef traits_to::base_char_type base_char_to;

            template <typename AllocFrom, typename AllocTo = priv::rebind_alloc<AllocFrom, char_to>>
            static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
                const ::uni::string<char_from, traits_from, AllocFrom>& f,
                [[maybe_unused]] const endianness to_endianness = endianness::big // Used when char_to is utf32_char
            )
            {
                using to_type = ::uni::string<char_to, traits_to, AllocTo>;
                using from_type = ::uni::string<char_from, traits_from, AllocFrom>;

                probably_if (f.size() == 0, 5, CHAOS)
                    return to_type();
                
                if constexpr (detail::has_endianness<char_to>)
                {
                    return ::uni::detail::string_conversions<utf8_char, char_to>()(
                        ::uni::detail::string_conversions<char_from, utf8_char>()(f),
                        to_endianness
                    );
                }
                else
                {
                    return ::uni::detail::string_conversions<utf8_char, char_to>()(
                        ::uni::detail::string_conversions<char_from, utf8_char>()(f)
                    );
                }
            }
        };

        // wchar_t <--> utf8_char
        // Specializations :
        //  string_conversions<wchar_t, utf8_char>
        //  string_conversions<utf8_char, wchar_t>
        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        > requires CAN_CONSTRUCT_FROM(
            (CharTFrom, CharTTo),
            (wchar_t, utf8_char)
        )
        struct string_conversions<CharTFrom, CharTTo> : public str_conv_base
        {
            typedef CharTFrom char_from;
            typedef CharTTo char_to;

            typedef ::uni::char_traits<char_from> traits_from;
            typedef ::uni::char_traits<char_to> traits_to;

            typedef traits_from::base_char_type base_char_from;
            typedef traits_to::base_char_type base_char_to;

            template <typename AllocFrom, typename AllocTo = priv::rebind_alloc<AllocFrom, char_to>>
            static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
                const ::uni::string<char_from, traits_from, AllocFrom>& f
            )
            {
                using to_type = ::uni::string<char_to, traits_to, AllocTo>;
                using from_type = ::uni::string<char_from, traits_from, AllocFrom>;

                probably_if (f.size() == 0, 5, CHAOS)
                    return to_type();
                
                return ::uni::detail::string_conversions<char, char_to>()(
                    ::uni::detail::string_conversions<char_from, char>()(f)
                );
            }
        };

        // wchar_t <--> utf16_char
        // Specializations :
        //  string_conversions<wchar_t, utf16_char>
        //  string_conversions<utf16_char, wchar_t>
        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        > requires CAN_CONSTRUCT_FROM(
            (CharTFrom, CharTTo),
            (wchar_t, utf16_char)
        )
        struct string_conversions<CharTFrom, CharTTo> : public str_conv_base
        {
            typedef CharTFrom char_from;
            typedef CharTTo char_to;

            typedef ::uni::char_traits<char_from> traits_from;
            typedef ::uni::char_traits<char_to> traits_to;

            typedef traits_from::base_char_type base_char_from;
            typedef traits_to::base_char_type base_char_to;

            template <typename AllocFrom, typename AllocTo = priv::rebind_alloc<AllocFrom, char_to>>
            static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
                const ::uni::string<char_from, traits_from, AllocFrom>& f,
                [[maybe_unused]] const endianness to_endianness = endianness::big // Used when char_to is utf16_char
            )
            {
                using to_type = ::uni::string<char_to, traits_to, AllocTo>;
                using from_type = ::uni::string<char_from, traits_from, AllocFrom>;

                probably_if (f.size() == 0, 5, CHAOS)
                    return to_type();
                
                if constexpr (detail::has_endianness<char_to>)
                {
                    return ::uni::detail::string_conversions<char, char_to>()(
                        ::uni::detail::string_conversions<char_from, char>()(f),
                        to_endianness
                    );
                }
                else
                {
                    return ::uni::detail::string_conversions<char, char_to>()(
                        ::uni::detail::string_conversions<char_from, char>()(f)
                    );
                }
            }
        };

        // wchar_t <--> utf32_char
        // Specializations :
        //  string_conversions<wchar_t, utf32_char>
        //  string_conversions<utf32_char, wchar_t>
        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        > requires CAN_CONSTRUCT_FROM(
            (CharTFrom, CharTTo),
            (wchar_t, utf32_char)
        )
        struct string_conversions<CharTFrom, CharTTo> : public str_conv_base
        {
            typedef CharTFrom char_from;
            typedef CharTTo char_to;

            typedef ::uni::char_traits<char_from> traits_from;
            typedef ::uni::char_traits<char_to> traits_to;

            typedef traits_from::base_char_type base_char_from;
            typedef traits_to::base_char_type base_char_to;

            template <typename AllocFrom, typename AllocTo = priv::rebind_alloc<AllocFrom, char_to>>
            static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
                const ::uni::string<char_from, traits_from, AllocFrom>& f,
                [[maybe_unused]] const endianness to_endianness = endianness::big // Used when char_to is utf32_char
            )
            {
                using to_type = ::uni::string<char_to, traits_to, AllocTo>;
                using from_type = ::uni::string<char_from, traits_from, AllocFrom>;

                probably_if (f.size() == 0, 5, CHAOS)
                    return to_type();
                
                if constexpr (detail::has_endianness<char_to>)
                {
                    return ::uni::detail::string_conversions<char, char_to>()(
                        ::uni::detail::string_conversions<char_from, char>()(f),
                        to_endianness
                    );
                }
                else
                {
                    return ::uni::detail::string_conversions<char, char_to>()(
                        ::uni::detail::string_conversions<char_from, char>()(f)
                    );
                }
            }
        };

        // utf8_char <--> utf16_char
        // Specializations :
        //  string_conversions<utf8_char, utf16_char>
        //  string_conversions<utf16_char, utf8_char>
        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        > requires CAN_CONSTRUCT_FROM(
            (CharTFrom, CharTTo),
            (utf8_char, utf16_char)
        )
        struct string_conversions<CharTFrom, CharTTo> : public str_conv_base
        {
            public:
                typedef CharTFrom char_from;
                typedef CharTTo char_to;

                typedef ::uni::char_traits<char_from> traits_from;
                typedef ::uni::char_traits<char_to> traits_to;

                typedef traits_from::base_char_type base_char_from;
                typedef traits_to::base_char_type base_char_to;

            private:
                using convertor_type = std::codecvt<char16_t, char8_t, std::mbstate_t>;

                static constexpr bool from_has_endianness = detail::has_endianness<char_from>;
                static constexpr bool to_has_endianness = detail::has_endianness<char_to>;
                
                static_assert(from_has_endianness != to_has_endianness);

                static constexpr bool from_to_do_out = std::same_as<base_char_from, char16_t> && std::same_as<base_char_to, char8_t>;
                static constexpr bool from_to_do_in  = std::same_as<base_char_from, char8_t>  && std::same_as<base_char_to, char16_t>;

                static_assert(from_to_do_out != from_to_do_in);

                static_assert(from_to_do_out == from_has_endianness);
                static_assert(
                    std::same_as<
                        base_char_from,
                        std::conditional_t<
                            from_to_do_out,
                            char16_t,
                            char8_t
                        >
                    >
                );
            
            public:
                template <typename AllocFrom, typename AllocTo = priv::rebind_alloc<AllocFrom, char_to>>
                static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
                    const ::uni::string<char_from, traits_from, AllocFrom>& f,
                    [[maybe_unused]] const endianness to_endianness = endianness::big // Used when char_to is utf16_char
                )
                {
                    using to_type = ::uni::string<char_to, traits_to, AllocTo>;
                    using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
                    
                    probably_if (f.size() == 0, 5, CHAOS)
                        return to_type();
                    
                    to_type to;
                    from_type from = f;
                    
                    if constexpr (from_has_endianness)
                    {
                        if (f.get_endianness() == endianness::little)
                            from.set_endianness(endianness::big);
                        else if (f.get_endianness() == endianness::undefined)
                            throw InternalError("Invalid endianness");
                        
                        std::unique_ptr<base_char_from[]> buffer_from = traits_from::to_base_char(from.c_str(), from.size());
                        
                        std::basic_string<base_char_from> tmp_from(buffer_from.get());
                        std::basic_string<base_char_to> tmp_to(from.size(), base_char_to());
                        
                        std::mbstate_t state{0};
                        auto loc = getloc();
                        const convertor_type& convertor = std::use_facet<convertor_type>(loc);
                        std::codecvt_base::result res;
                        
                        const base_char_from* from_start = tmp_from.c_str();
                        const base_char_from* from_end = from_start + tmp_from.size();
                        const base_char_from* from_next = nullptr;
                        base_char_to* to_start = tmp_to.data();
                        base_char_to* to_end = to_start + tmp_to.size();
                        base_char_to* to_next = nullptr;

                        do
                        {
                            if constexpr (from_to_do_out)
                            {
                                res = convertor.out(
                                    state,
                                    from_start,
                                    from_end,
                                    from_next,
                                    to_start,
                                    to_end,
                                    to_next
                                );
                            }
                            else
                            {
                                res = convertor.in(
                                    state,
                                    from_start,
                                    from_end,
                                    from_next,
                                    to_start,
                                    to_end,
                                    to_next
                                );
                            }
                            if (res == std::codecvt_base::partial)
                            {
                                tmp_to.resize(tmp_to.size() * 2);
                                to_start = tmp_to.data();
                                to_end = to_start + tmp_to.size();
                                from_start = tmp_from.c_str();
                                from_end = from_start + tmp_from.size();
                            }
                        } while (res == std::codecvt_base::partial);
                        if (res == std::codecvt_base::error)
                            throw InternalError("Error while converting string");
                        unlikely_if (res == std::codecvt_base::noconv)
                            throw InternalError("No conversion needed");
                        
                        std::unique_ptr<char_to[]> buffer_to = traits_to::from_base_char(tmp_to.c_str(), tmp_to.size());
                        to = to_type(buffer_to.get());
                    }
                    else // !from_has_endianness <==> to_has_endianness
                    {
                        if (to_endianness == endianness::undefined)
                            throw InternalError("Invalid endianness");

                        std::unique_ptr<base_char_from[]> buffer_from = traits_from::to_base_char(from.c_str(), from.size());
                        
                        std::basic_string<base_char_from> tmp_from(buffer_from.get());
                        std::basic_string<base_char_to> tmp_to(from.size(), base_char_to());

                        std::mbstate_t state{0};
                        auto loc = getloc();
                        const convertor_type& convertor = std::use_facet<convertor_type>(loc);
                        std::codecvt_base::result res;

                        const base_char_from* from_start = tmp_from.c_str();
                        const base_char_from* from_end = from_start + tmp_from.size();
                        const base_char_from* from_next = nullptr;
                        base_char_to* to_start = tmp_to.data();
                        base_char_to* to_end = to_start + tmp_to.size();
                        base_char_to* to_next = nullptr;

                        do
                        {
                            if constexpr (from_to_do_out)
                            {
                                res = convertor.out(
                                    state,
                                    from_start,
                                    from_end,
                                    from_next,
                                    to_start,
                                    to_end,
                                    to_next
                                );
                            }
                            else
                            {
                                res = convertor.in(
                                    state,
                                    from_start,
                                    from_end,
                                    from_next,
                                    to_start,
                                    to_end,
                                    to_next
                                );
                            }
                            if (res == std::codecvt_base::partial)
                            {
                                tmp_to.resize(tmp_to.size() * 2);
                                to_start = tmp_to.data();
                                to_end = to_start + tmp_to.size();
                                from_start = tmp_from.c_str();
                                from_end = from_start + tmp_from.size();
                            }
                        } while (res == std::codecvt_base::partial);
                        if (res == std::codecvt_base::error)
                            throw InternalError("Error while converting string");
                        unlikely_if (res == std::codecvt_base::noconv)
                            throw InternalError("No conversion needed");

                        std::unique_ptr<char_to[]> buffer_to = traits_to::from_base_char(tmp_to.c_str(), tmp_to.size(), get_literal_endianness<base_char_to>());
                        to = to_type(buffer_to.get());
                        to.set_endianness(to_endianness);
                    }

                    return to;
                }
        };

        // utf8_char <--> utf32_char
        // Specializations :
        //  string_conversions<utf8_char, utf32_char>
        //  string_conversions<utf32_char, utf8_char>
        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        > requires CAN_CONSTRUCT_FROM(
            (CharTFrom, CharTTo),
            (utf8_char, utf32_char)
        )
        struct string_conversions<CharTFrom, CharTTo> : public str_conv_base
        {
            public:
                typedef CharTFrom char_from;
                typedef CharTTo char_to;

                typedef ::uni::char_traits<char_from> traits_from;
                typedef ::uni::char_traits<char_to> traits_to;

                typedef traits_from::base_char_type base_char_from;
                typedef traits_to::base_char_type base_char_to;

            private:
                using convertor_type = std::codecvt<char32_t, char8_t, std::mbstate_t>;

                static constexpr bool from_has_endianness = detail::has_endianness<char_from>;
                static constexpr bool to_has_endianness = detail::has_endianness<char_to>;
                
                static_assert(from_has_endianness != to_has_endianness);

                static constexpr bool from_to_do_out = std::same_as<base_char_from, char32_t> && std::same_as<base_char_to, char8_t>;
                static constexpr bool from_to_do_in  = std::same_as<base_char_from, char8_t>  && std::same_as<base_char_to, char32_t>;

                static_assert(from_to_do_out != from_to_do_in);

                static_assert(from_to_do_out == from_has_endianness);
                static_assert(
                    std::same_as<
                        base_char_from,
                        std::conditional_t<
                            from_to_do_out,
                            char32_t,
                            char8_t
                        >
                    >
                );
            
            public:
                template <typename AllocFrom, typename AllocTo = priv::rebind_alloc<AllocFrom, char_to>>
                static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
                    const ::uni::string<char_from, traits_from, AllocFrom>& f,
                    [[maybe_unused]] const endianness to_endianness = endianness::big // Used when char_to is utf32_char
                )
                {
                    using to_type = ::uni::string<char_to, traits_to, AllocTo>;
                    using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
                    
                    probably_if (f.size() == 0, 5, CHAOS)
                        return to_type();
                    
                    to_type to;
                    from_type from = f;
                    
                    if constexpr (from_has_endianness)
                    {
                        if (f.get_endianness() == endianness::little)
                            from.set_endianness(endianness::big);
                        else if (f.get_endianness() == endianness::undefined)
                            throw InternalError("Invalid endianness");

                        std::unique_ptr<base_char_from[]> buffer_from = traits_from::to_base_char(from.c_str(), from.size());

                        std::basic_string<base_char_from> tmp_from(buffer_from.get());
                        std::basic_string<base_char_to> tmp_to(from.size(), base_char_to());

                        std::mbstate_t state{0};
                        auto loc = getloc();
                        const convertor_type& convertor = std::use_facet<convertor_type>(loc);
                        std::codecvt_base::result res;

                        const base_char_from* from_start = tmp_from.c_str();
                        const base_char_from* from_end = from_start + tmp_from.size();
                        const base_char_from* from_next = nullptr;
                        base_char_to* to_start = tmp_to.data();
                        base_char_to* to_end = to_start + tmp_to.size();
                        base_char_to* to_next = nullptr;

                        do
                        {
                            if constexpr (from_to_do_out)
                            {
                                res = convertor.out(
                                    state,
                                    from_start,
                                    from_end,
                                    from_next,
                                    to_start,
                                    to_end,
                                    to_next
                                );
                            }
                            else
                            {
                                res = convertor.in(
                                    state,
                                    from_start,
                                    from_end,
                                    from_next,
                                    to_start,
                                    to_end,
                                    to_next
                                );
                            }
                            if (res == std::codecvt_base::partial)
                            {
                                tmp_to.resize(tmp_to.size() * 2);
                                to_start = tmp_to.data();
                                to_end = to_start + tmp_to.size();
                                from_start = tmp_from.c_str();
                                from_end = from_start + tmp_from.size();
                            }
                        } while (res == std::codecvt_base::partial);
                        if (res == std::codecvt_base::error)
                            throw InternalError("Error while converting string");
                        unlikely_if (res == std::codecvt_base::noconv)
                            throw InternalError("No conversion needed");

                        std::unique_ptr<char_to[]> buffer_to = traits_to::from_base_char(tmp_to.c_str(), tmp_to.size());
                        to = to_type(buffer_to.get());
                    }
                    else // !from_has_endianness <==> to_has_endianness
                    {
                        if (to_endianness == endianness::undefined)
                            throw InternalError("Invalid endianness");

                        std::unique_ptr<base_char_from[]> buffer_from = traits_from::to_base_char(from.c_str(), from.size());

                        std::basic_string<base_char_from> tmp_from(buffer_from.get());
                        std::basic_string<base_char_to> tmp_to(from.size(), base_char_to());

                        std::mbstate_t state{0};
                        auto loc = getloc();
                        const convertor_type& convertor = std::use_facet<convertor_type>(loc);
                        std::codecvt_base::result res;

                        const base_char_from* from_start = tmp_from.c_str();
                        const base_char_from* from_end = from_start + tmp_from.size();
                        const base_char_from* from_next = nullptr;
                        base_char_to* to_start = tmp_to.data();
                        base_char_to* to_end = to_start + tmp_to.size();
                        base_char_to* to_next = nullptr;

                        do
                        {
                            if constexpr (from_to_do_out)
                            {
                                res = convertor.out(
                                    state,
                                    from_start,
                                    from_end,
                                    from_next,
                                    to_start,
                                    to_end,
                                    to_next
                                );
                            }
                            else
                            {
                                res = convertor.in(
                                    state,
                                    from_start,
                                    from_end,
                                    from_next,
                                    to_start,
                                    to_end,
                                    to_next
                                );
                            }
                            if (res == std::codecvt_base::partial)
                            {
                                tmp_to.resize(tmp_to.size() * 2);
                                to_start = tmp_to.data();
                                to_end = to_start + tmp_to.size();
                                from_start = tmp_from.c_str();
                                from_end = from_start + tmp_from.size();
                            }
                        } while (res == std::codecvt_base::partial);
                        if (res == std::codecvt_base::error)
                            throw InternalError("Error while converting string");
                        unlikely_if (res == std::codecvt_base::noconv)
                            throw InternalError("No conversion needed");

                        std::unique_ptr<char_to[]> buffer_to = traits_to::from_base_char(tmp_to.c_str(), tmp_to.size(), get_literal_endianness<base_char_to>());
                        to = to_type(buffer_to.get());
                        to.set_endianness(to_endianness);
                    }

                    return to;
                }
        };

        // utf16_char <--> utf32_char
        // Specializations :
        //  string_conversions<utf16_char, utf32_char>
        //  string_conversions<utf32_char, utf16_char>
        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        > requires CAN_CONSTRUCT_FROM(
            (CharTFrom, CharTTo),
            (utf16_char, utf32_char)
        )
        struct string_conversions<CharTFrom, CharTTo> : public str_conv_base
        {
            public:
                typedef CharTFrom char_from;
                typedef CharTTo char_to;

                typedef ::uni::char_traits<char_from> traits_from;
                typedef ::uni::char_traits<char_to> traits_to;

                typedef traits_from::base_char_type base_char_from;
                typedef traits_to::base_char_type base_char_to;

            private:
#if 0
                using convertor_type = std::codecvt<char32_t, char16_t, std::mbstate_t>;

                static constexpr bool from_has_endianness = detail::has_endianness<char_from>;
                static constexpr bool to_has_endianness = detail::has_endianness<char_to>;
                
                static_assert(from_has_endianness && to_has_endianness);

                static constexpr bool from_to_do_out = std::same_as<base_char_from, char32_t> && std::same_as<base_char_to, char16_t>;
                static constexpr bool from_to_do_in  = std::same_as<base_char_from, char16_t> && std::same_as<base_char_to, char32_t>;

                static_assert(from_to_do_out != from_to_do_in);
#endif

            public:
                template <typename AllocFrom, typename AllocTo = priv::rebind_alloc<AllocFrom, char_to>>
                static constexpr ::uni::string<char_to, traits_to, AllocTo> operator()(
                    const ::uni::string<char_from, traits_from, AllocFrom>& f,
                    const endianness to_endianness = endianness::big // Always used
                )
                {
                    using to_type = ::uni::string<char_to, traits_to, AllocTo>;
                    using from_type = ::uni::string<char_from, traits_from, AllocFrom>;
                    
                    probably_if (f.size() == 0, 5, CHAOS)
                        return to_type();
                    
#if 0
                    to_type to;
                    from_type from = f;
                    
                    if (f.get_endianness() == endianness::little)
                        from.set_endianness(endianness::big);
                    else if (f.get_endianness() == endianness::undefined)
                        throw InternalError("Invalid endianness");

                    std::unique_ptr<base_char_from[]> buffer_from = traits_from::to_base_char(from.c_str(), from.size());

                    std::basic_string<base_char_from> tmp_from(buffer_from.get());
                    std::basic_string<base_char_to> tmp_to(from.size(), base_char_to());

                    std::mbstate_t state{0};
                    auto loc = getloc();
                    const convertor_type& convertor = std::use_facet<convertor_type>(loc);
                    std::codecvt_base::result res;

                    const base_char_from* from_start = tmp_from.c_str();
                    const base_char_from* from_end = from_start + tmp_from.size();
                    const base_char_from* from_next = nullptr;
                    base_char_to* to_start = tmp_to.data();
                    base_char_to* to_end = to_start + tmp_to.size();
                    base_char_to* to_next = nullptr;

                    do
                    {
                        if constexpr (from_to_do_out)
                        {
                            res = convertor.out(
                                state,
                                from_start,
                                from_end,
                                from_next,
                                to_start,
                                to_end,
                                to_next
                            );
                        }
                        else
                        {
                            res = convertor.in(
                                state,
                                from_start,
                                from_end,
                                from_next,
                                to_start,
                                to_end,
                                to_next
                            );
                        }
                        if (res == std::codecvt_base::partial)
                        {
                            tmp_to.resize(tmp_to.size() * 2);
                            to_start = tmp_to.data();
                            to_end = to_start + tmp_to.size();
                            from_start = tmp_from.c_str();
                            from_end = from_start + tmp_from.size();
                        }
                    } while (res == std::codecvt_base::partial);
                    if (res == std::codecvt_base::error)
                        throw InternalError("Error while converting string");
                    unlikely_if (res == std::codecvt_base::noconv)
                        throw InternalError("No conversion needed");

                    std::unique_ptr<char_to[]> buffer_to = traits_to::from_base_char(tmp_to.c_str(), tmp_to.size(), get_literal_endianness<base_char_to>());
                    to = to_type(buffer_to.get());
                    to.set_endianness(to_endianness);

                    return to;
#else
                    return ::uni::detail::string_conversions<utf8_char, char_to>()(
                        ::uni::detail::string_conversions<char_from, utf8_char>()(f),
                        to_endianness
                    );
#endif
                }
        };
    }
}