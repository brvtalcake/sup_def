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
    template <::uni::supported_uni_char CharT>
    constexpr const char* input_file<CharT>::find_next(const char* s, const char* const last)
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

    template <::uni::supported_uni_char CharT>
    constexpr const char* input_file<CharT>::find_prev(const char* s, const char* const first)
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

    template <::uni::supported_uni_char CharT>
    constexpr std::pair<CharT, std::optional<CharT>> input_file<CharT>::create_char(const char* s, size_t byte_count)
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

    template <::uni::supported_uni_char CharT>
    void input_file<CharT>::detect_bom()
    {
        unlikely_if (!this->is_open())
            return;
        if (this->tried_detecting_bom)
            return;
        this->has_bom = false;

        if (!this->file.pubsync())
            throw InternalError("input_file::detect_bom(): Error syncing file");
        auto curr_pos = this->get_raw_pos();
        this->file.pubseekoff(0, std::ios_base::beg, std::ios_base::in);
        char bom[4] = { 0 };
        this->file.sgetn(bom, 4);
        if (this->file_encoding & (encoding_mode::uni_utf8 | encoding_mode::char_utf8))
        {
            if (
                bom[0] == '\xEF' &&
                bom[1] == '\xBB' &&
                bom[2] == '\xBF'
            )
                this->has_bom = true;
        }
        else if (this->file_encoding & encoding_mode::uni_utf16)
        {
            if (
                bom[0] == '\xFE' &&
                bom[1] == '\xFF' &&
                (this->file_encoding & encoding_mode::big_endian)
            )
                this->has_bom = true;
            else if (
                bom[0] == '\xFF' &&
                bom[1] == '\xFE' &&
                (this->file_encoding & encoding_mode::little_endian)
            )
                this->has_bom = true;
        }
        else if (this->file_encoding & encoding_mode::uni_utf32)
        {
            if (
                bom[0] == '\x00' &&
                bom[1] == '\x00' &&
                bom[2] == '\xFE' &&
                bom[3] == '\xFF' &&
                (this->file_encoding & encoding_mode::big_endian)
            )
                this->has_bom = true;
            else if (
                bom[0] == '\xFF' &&
                bom[1] == '\xFE' &&
                bom[2] == '\x00' &&
                bom[3] == '\x00' &&
                (this->file_encoding & encoding_mode::little_endian)
            )
                this->has_bom = true;
        }
        this->file.pubseekoff(curr_pos, std::ios_base::beg, std::ios_base::in);
        this->tried_detecting_bom = true;
    }

    template <::uni::supported_uni_char CharT>
    void input_file<CharT>::skip_bom()
    {
        if (this->tried_detecting_bom && this->has_bom)
        {
            try
            {
                auto raw_pos = this->get_raw_pos();
                if (
                    (this->file_encoding & (encoding_mode::uni_utf8 | encoding_mode::char_utf8)) &&
                    raw_pos < 3
                )
                    this->file.pubseekoff(3, std::ios_base::beg, std::ios_base::in);
                else if (
                    (this->file_encoding & encoding_mode::uni_utf16) &&
                    raw_pos < 2
                )
                    this->file.pubseekoff(2, std::ios_base::beg, std::ios_base::in);
                else if (
                    (this->file_encoding & encoding_mode::uni_utf32) &&
                    raw_pos < 4
                )
                    this->file.pubseekoff(4, std::ios_base::beg, std::ios_base::in);
                this->current_pos = 0;
            }
            catch (...)
            {
                return;
            }
        }
    }

    template <::uni::supported_uni_char CharT>
    void input_file<CharT>::seek_next()
    {
        if constexpr (std::same_as<CharT, wchar_t>)
        {
            this->file.pubseekoff(1, std::ios_base::cur, std::ios_base::in);
            ++(this->current_pos);
        }
        else
        {
            if (this->current_pos == 0)
                this->skip_bom();
            constexpr size_t max_bytes = 4;
            char buff[max_bytes] = { 0 };
            char* const last = buff + max_bytes;

            for (size_t i = 0; i < max_bytes; ++i)
            {
                std::char_traits<char>::int_type c = this->file.sbumpc();
                unlikely_if (c == std::char_traits<char>::eof())
                    throw InternalError("input_file::seek_next(): Error seeking to next character");
                buff[i] = std::char_traits<char>::to_char_type(c);
                if (find_next(buff, buff + i) != nullptr)
                {
                    ++(this->current_pos);
                    return;
                }
            }

            throw InternalError("input_file::seek_next(): Error seeking to next character");
        }
    }

    template <::uni::supported_uni_char CharT>
    void input_file<CharT>::seek_prev()
    {
        if constexpr (std::same_as<CharT, wchar_t>)
        {
            this->file.pubseekoff(-1, std::ios_base::cur, std::ios_base::in);
            --(this->current_pos);
        }
        else
        {
            if (this->current_pos <= 1)
            {
                this->file.pubseekoff(0, std::ios_base::beg, std::ios_base::in);
                this->current_pos = 0;
                this->skip_bom();
                return;
            }
            constexpr size_t max_bytes = 4;
            char buff[max_bytes] = { 0 };
            char* const last = buff + max_bytes;

            for (size_t i = max_bytes - 1; i <= 0; --i)
            {
                std::char_traits<char>::int_type c = this->file.sungetc();
                unlikely_if (c == std::char_traits<char>::eof())
                    throw InternalError("input_file::seek_prev(): Error seeking to previous character");
                buff[i] = std::char_traits<char>::to_char_type(c);
                if (find_prev(last, buff + i) != nullptr)
                {
                    --(this->current_pos);
                    return;
                }
            }

            throw InternalError("input_file::seek_prev(): Error seeking to previous character");
        }
    }

    template <::uni::supported_uni_char CharT>
    virtual void input_file<CharT>::make_ready_for_io()
    {
        if (!this->is_open())
            throw InternalError("input_file::make_ready_for_read(): File is not open");
        if (!this->file.pubsync())
            throw InternalError("input_file::make_ready_for_read(): Error syncing file");
        this->detect_bom();
        this->skip_bom();
    }

    template <::uni::supported_uni_char CharT>
    std::streamsize input_file<CharT>::read(char_type* s, std::streamsize n)
    {
#if 0
        std::memset(s, 0, n * sizeof(char_type));
#endif
        if constexpr (std::same_as<char_type, wchar_t>)
        {
            std::streamsize ret = this->file.sgetn(s, n);
            this->current_pos += ret;
            return ret;
        }
        else
        {
            static_assert(
                one_char_max_bytes > 0 && std::same_as<raw_char_type, char>
            );

            this->make_ready_for_io();

            std::streamsize ret = 0;
            std::streamsize tmp = 0;

            const size_t buff_size = one_char_max_bytes * n;
            constexpr size_t max_alloca_size = 1024 * 4; // 4KB
            bool stack_allocated = false;
            raw_char_type* buf = nullptr;

            if (buff_size <= max_alloca_size)
            {
                buf = (raw_char_type*)(alloca(buff_size));
                stack_allocated = true;
            }
            else
                buf = new raw_char_type[buff_size];
            if (!buf)
                throw InternalError("input_file::read(char_type*, std::streamsize): Error allocating buffer");

#if !SUPDEF_USE_DEFER
            auto on_exit_do = [this, &buf, &stack_allocated, &ret](void) -> void
            {
                if (buf && !stack_allocated)
                    delete[] buf;
                this->current_pos += ret;
            }; // TODO: Use a DEFER macro
#else
            auto lambda1 = [&buf, &stack_allocated](void) -> void
            {
                if (buf && !stack_allocated)
                    delete[] buf;
            };
            auto lambda2 = [this, &ret](void) -> void
            {
                this->current_pos += ret;
            };
            DEFER(on_exit_do, SCOPE_EXIT,
                std::move(lambda1),
                std::move(lambda2)
            );
#endif
            try
            {
                tmp = this->file.sgetn(buf, buff_size);
                if (tmp <= 0)
                {
                    if (buf && !stack_allocated)
                        delete[] buf;
                    return tmp;
                }

                raw_char_type* const last = buf + size_t(tmp);
                raw_char_type* next = buf;
                raw_char_type* curr = buf;

                while (
                    (next = this->find_next(curr, last)) != nullptr &&
                    ret < n
                )
                {
                    if (next - curr > one_char_max_bytes)
                        throw InternalError("input_file::read(char_type*, std::streamsize): Invalid character size");
                    if constexpr (std::same_as<char_type, utf16_char>)
                    {
                        auto [c1, c2] = this->create_char(curr, next - curr);
                        traits_type::assign(*s, c1);
                        ++s;
                        ++ret;
                        if (c2.has_value())
                        {
                            traits_type::assign(*s, c2.value());
                            ++s;
                            ++ret;
                        }
                    }
                    else if constexpr (std::same_as<char_type, utf32_char>)
                    {
                        traits_type::assign(*s, this->create_char(curr, next - curr));
                        ++s;
                        ++ret;
                    }
                    else if constexpr (std::same_as<char_type, utf8_char> || std::same_as<char_type, char>)
                    {
                        traits_type::copy(s, reinterpret_cast<char_type*>(curr), next - curr);
                        s += next - curr;
                        ret += next - curr;
                    }
                    else
                        throw InternalError("input_file::read(char_type*, std::streamsize): Invalid character type");
                    curr = next;
                }
            }
            catch (...)
            {
#if !SUPDEF_USE_DEFER
                on_exit_do();
#endif
                throw;
            }

#if !SUPDEF_USE_DEFER
            on_exit_do();
#endif

            if (ret < n)
                traits_type::assign(scpy + ret, n - ret, char_type());
            return ret;
        }
    }

    template <::uni::supported_uni_char CharT>
    void input_file<CharT>::seekoffg(off_type off, std::ios_base::seekdir dir)
    {
        using namespace mpark::patterns;

        this->make_ready_for_io();
        match(dir)(
            pattern(std::ios_base::beg) = [this, off]()
            {
                if (off < 0)
                    throw InternalError("input_file::seekg(off_type, std::ios_base::seekdir): Invalid offset");
                this->file.pubseekoff(0, std::ios_base::beg, std::ios_base::in);
                this->current_pos = 0;
                this->skip_bom();
                while (off > 0)
                {
                    this->seek_next();
                    --off;
                }
            },
            pattern(std::ios_base::cur) = [this, off]()
            {
                if (off == 0)
                    return;
                else if (off > 0)
                {
                    while (off > 0)
                    {
                        this->seek_next();
                        --off;
                    }
                }
                else
                {
                    while (off < 0)
                    {
                        this->seek_prev();
                        ++off;
                    }
                }
            },
            pattern(std::ios_base::end) = [this, off]()
            {
                if (off > 0)
                    throw InternalError("input_file::seekg(off_type, std::ios_base::seekdir): Invalid offset");
                auto&& [max_pos, max_raw_pos] = this->get_max_pos();
                this->file.pubseekpos(max_raw_pos, std::ios_base::in);
                this->current_pos = max_pos;
                while (off < 0)
                {
                    this->seek_prev();
                    ++off;
                }
            },
            pattern(mpark::patterns::_) = []()
            { throw InternalError("input_file::seekg(off_type, std::ios_base::seekdir): Invalid seekdir"); }
        );
    }

    template <::uni::supported_uni_char CharT>
    constexpr CharT input_file<CharT>::create_char(const char* s, size_t byte_count = 4)
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

    template <::uni::supported_uni_char CharT>
    warn_usage_suggest_alternative(
        "input_file(const std::filesystem::path&, endianness)",
        "input_file(const std::filesystem::path&, std::ios_base::openmode, endianness)"
    )
    constexpr bool input_file<CharT>::set_endianness(endianness end)
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