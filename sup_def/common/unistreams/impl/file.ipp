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

namespace uni
{
    template <typename CharT>
    constexpr const char* detail::file_base<CharT>::find_next(const char* s, const char* const last)
        requires (!std::same_as<CharT, wchar_t>)
    {
        if (s == last || !s || !last)
            return nullptr;
        if constexpr (std::same_as<CharT, utf8_char> || std::same_as<CharT, char>)
            return find_next_u8(s, last);
        else if constexpr (std::same_as<CharT, utf16_char>)
        {
            if (this->file_encoding & encoding_mode::big_endian)
                return find_next_u16be(s, last);
            else if (this->file_encoding & encoding_mode::little_endian)
                return find_next_u16le(s, last);
            else
                return nullptr;
        }
        else if constexpr (std::same_as<CharT, utf32_char>)
        {
            if (this->file_encoding & encoding_mode::big_endian)
                return find_next_u32be(s, last);
            else if (this->file_encoding & encoding_mode::little_endian)
                return find_next_u32le(s, last);
            else
                return nullptr;
        }
        else
            return nullptr;
    }

    template <typename CharT>
    constexpr const char* detail::file_base<CharT>::find_prev(const char* s, const char* const first)
        requires (!std::same_as<CharT, wchar_t>)
    {
        if (s == first || !s || !first)
            return nullptr;
        if constexpr (std::same_as<CharT, utf8_char> || std::same_as<CharT, char>)
            return find_prev_u8(s, first);
        else if constexpr (std::same_as<CharT, utf16_char>)
        {
            if (this->file_encoding & encoding_mode::big_endian)
                return find_prev_u16be(s, first);
            else if (this->file_encoding & encoding_mode::little_endian)
                return find_prev_u16le(s, first);
            else
                return nullptr;
        }
        else if constexpr (std::same_as<CharT, utf32_char>)
        {
            if (this->file_encoding & encoding_mode::big_endian)
                return find_prev_u32be(s, first);
            else if (this->file_encoding & encoding_mode::little_endian)
                return find_prev_u32le(s, first);
            else
                return nullptr;
        }
        else
            return nullptr;
    }

    template <typename CharT>
    constexpr std::pair<CharT, std::optional<CharT>> detail::file_base<CharT>::create_char(const char* s, size_t byte_count)
        requires std::same_as<CharT, utf16_char>
    {
        using retvalue_t = decltype(utf16_char::value);
        CharT ret1;
        std::optional<CharT> ret2 = std::nullopt;

        if (this->file_encoding & encoding_mode::big_endian)
            ret1.endian = endianness::big;
        else if (this->file_encoding & encoding_mode::little_endian)
            ret1.endian = endianness::little;
            
        if (byte_count == 2)
            ret1.value = 
                retvalue_t( reinterpret_cast<const uint8_t*>(s)[0] ) << 8 |
                retvalue_t( reinterpret_cast<const uint8_t*>(s)[1] );
        else if (byte_count == 4)
        {
            ret1.value = 
                retvalue_t( reinterpret_cast<const uint8_t*>(s)[0] ) << 8 |
                retvalue_t( reinterpret_cast<const uint8_t*>(s)[1] );
            ret2 = utf16_char(
                retvalue_t( reinterpret_cast<const uint8_t*>(s)[2] ) << 8 |
                retvalue_t( reinterpret_cast<const uint8_t*>(s)[3] ),
                ret1.endian
            );
        }
        else
            throw InternalError("input_file::create_char(const char*, size_t): Invalid byte count");

        return std::make_pair(ret1, ret2);
    }

    template <typename CharT>
    constexpr CharT detail::file_base<CharT>::create_char(const char* s, size_t byte_count = 4)
        requires std::same_as<CharT, utf32_char>
    {
        using retvalue_t = decltype(utf32_char::value);
        CharT ret;

        if (this->file_encoding & encoding_mode::big_endian)
            ret.endian = endianness::big;
        else if (this->file_encoding & encoding_mode::little_endian)
            ret.endian = endianness::little;
            
        if (byte_count == 4)
            ret.value = 
                retvalue_t( reinterpret_cast<const uint8_t*>(s)[0] ) << 24 |
                retvalue_t( reinterpret_cast<const uint8_t*>(s)[1] ) << 16 |
                retvalue_t( reinterpret_cast<const uint8_t*>(s)[2] ) << 8 |
                retvalue_t( reinterpret_cast<const uint8_t*>(s)[3] );
        else
            throw InternalError("input_file::create_char(const char*, size_t): Invalid byte count");

        return ret;
    }

    template <typename CharT>
    warn_usage_suggest_alternative(
        "<file-type>_file(const std::filesystem::path&, endianness)",
        "<file-type>_file(const std::filesystem::path&, std::ios_base::openmode, endianness)"
    )
    constexpr bool detail::file_base<CharT>::set_endianness(endianness end)
        requires detail::has_endianness<char_type, traits_type>
    {
        if (this->file_encoding & (encoding_mode::uni_utf16 | encoding_mode::uni_utf32))
        {
            // Remove previous endianness
            this->file_encoding &= ~(encoding_mode::big_endian | encoding_mode::little_endian);

            // Add endianness to encoding
            if (end == endianness::big)
                this->file_encoding |= encoding_mode::big_endian;
            else if (end == endianness::little)
                this->file_encoding |= encoding_mode::little_endian;
            else
                return false;
        }
        this->detect_bom();
        return true;
    }
}