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

#include <sup_def/common/util/util.hpp>
#include <cstdlib>
#include <memory>
#include <new>

STATIC_TODO(
    "Finish / fix the static allocator implementation, and improve it"
    " (see https://www.boost.org/doc/libs/1_84_0/doc/html/interprocess/memory_algorithms.html for ideas)"
);
STATIC_TODO(
    "Use stdemul::start_lifetime_as"
);

namespace SupDef
{
    namespace Util
    {
        static_assert(STATIC_MEMPOOL_SIZE > 0, "STATIC_MEMPOOL_SIZE must be greater than 0");

#ifdef assume_aligned
    PUSH_MACRO(assume_aligned)
#endif
#undef assume_aligned
#if __cpp_lib_assume_aligned && 0
    #define assume_aligned(ptr, align, ...) std::assume_aligned<(align)>((ptr) __VA_OPT__(, FIRST_ARG(__VA_ARGS__)))
#endif
#ifdef __has_builtin
    #if __has_builtin(__builtin_assume_aligned) && !defined(assume_aligned)
        #define assume_aligned(ptr, align, ...) __builtin_assume_aligned((ptr), (align) __VA_OPT__(, FIRST_ARG(__VA_ARGS__)))
    #endif
#endif
#ifndef assume_aligned
    #define assume_aligned(ptr, align, ...) (ptr)
#endif

#ifdef aligned_ret
    PUSH_MACRO(aligned_ret)
#endif
#undef aligned_ret
#ifdef __has_attribute
    #if __has_attribute(assume_aligned)
        #define aligned_ret(align, ...) [[__gnu__::__assume_aligned__((align) __VA_OPT__(, FIRST_ARG(__VA_ARGS__)))]]
    #endif
#endif
#ifndef aligned_ret
    #define aligned_ret(align, ...)
#endif

#ifdef CAST_PTR
    PUSH_MACRO(CAST_PTR)
#endif
#undef CAST_PTR
#define CAST_PTR(type) reinterpret_cast<type>

#ifdef DECAY
    PUSH_MACRO(DECAY)
#endif
#undef DECAY
#define DECAY(array) (&(*(array)) + 0)

#ifdef IS_IN_CHUNK
    PUSH_MACRO(IS_IN_CHUNK)
#endif
#undef IS_IN_CHUNK
#define IS_IN_CHUNK(hdr, ptr)       \
    (                               \
        CAST_PTR(uintptr_t)(ptr) >= \
        CAST_PTR(uintptr_t)(        \
            (hdr)->chnk_limits[0]   \
        ) &&                        \
        CAST_PTR(uintptr_t)(ptr) <  \
        CAST_PTR(uintptr_t)(        \
            (hdr)->chnk_limits[1]   \
        )                           \
    )

#ifdef CHNK_SZ
    PUSH_MACRO(CHNK_SZ)
#endif
#undef CHNK_SZ
#define CHNK_SZ(hdr) (CAST_PTR(uintptr_t)((hdr)->chnk_limits[1]) - CAST_PTR(uintptr_t)((hdr)->chnk_limits[0]))

#ifdef DATA_ROOM
    PUSH_MACRO(DATA_ROOM)
#endif
#undef DATA_ROOM
#define DATA_ROOM(hdr) ( CAST_PTR(uintptr_t)((hdr)->chnk_limits[1]) - CAST_PTR(uintptr_t)((hdr)->data_start) )

#ifdef ENOUGH_ROOM
    PUSH_MACRO(ENOUGH_ROOM)
#endif
#undef ENOUGH_ROOM
#define ENOUGH_ROOM(hdr, size, align) ( ((int64_t)DATA_ROOM((hdr))) - (int64_t)(align_pointer((hdr)->data_start, (align)).second) >= (int64_t)(size))

#ifdef CAN_SPLIT
    PUSH_MACRO(CAN_SPLIT)
#endif
#undef CAN_SPLIT
#define CAN_SPLIT(hdr, size, align)                                         \
    (                                                                       \
        /* All data space */                                                \
        ((int64_t)DATA_ROOM((hdr))) -                                       \
        /* minus the space needed to satisfy the alignment requirement */   \
        (int64_t)(align_pointer((hdr)->data_start, (align)).second) -       \
        /* minus the space needed to align the next (new) header */         \
        (int64_t)(align_pointer(                                            \
            CAST_PTR(aliasing_type(void)*)(                                 \
                CAST_PTR(uintptr_t)((hdr)->data_start) +                    \
                align_pointer((hdr)->data_start, (align)).second +          \
                (size)                                                      \
            ),                                                              \
            alignof(memory_header_t)                                        \
        ).second) -                                                         \
        /* minus the space needed for the next (new) header */              \
        (int64_t)(sizeof(memory_header_t)) > (int64_t)(size)                \
    )

#ifdef NEXT_CHNK
    PUSH_MACRO(NEXT_CHNK)
#endif
#undef NEXT_CHNK
#define NEXT_CHNK(hdr) ((hdr)->chnk_limits[1])

#ifdef PREV_CHNK
    PUSH_MACRO(PREV_CHNK)
#endif
#undef PREV_CHNK
#define PREV_CHNK(hdr) ((hdr)->prev->chnk_limits[0])

#ifdef MEM_HDR_ALIGN
    PUSH_MACRO(MEM_HDR_ALIGN)
#endif
#undef MEM_HDR_ALIGN
#define MEM_HDR_ALIGN(addr) (align_pointer((addr), alignof(memory_header_t)).second + (addr))

        struct memory_offset
        {
            aliasing_type(void)* ptr;
            intptr_t offset;
        };

        always_inline warn_unused_result()
        static constexpr inline bool check_alignment_requirements(aliasing_type(void)* ptr, size_t alignment)
        {
            return (CAST_PTR(uintptr_t)(ptr) % alignment) == 0;
        }

        always_inline warn_unused_result() flatten
        static constexpr inline memory_offset align_ptr(aliasing_type(void)* ptr, size_t alignment)
        {
            aliasing_type(void)* ptr_cpy = ptr;
            likely_if ((alignment % 2) == 0)
            {
                aliasing_type(void)* aligned_ptr = CAST_PTR(aliasing_type(void)*)(
                    (CAST_PTR(uintptr_t)(ptr) + alignment - 1) &
                    ~(alignment - 1)
                );
                hard_assert(
                    CAST_PTR(uintptr_t)(aligned_ptr) >= CAST_PTR(uintptr_t)(ptr)
                );
                return {aligned_ptr, CAST_PTR(intptr_t)(aligned_ptr) - CAST_PTR(intptr_t)(ptr)};
            }
            else
            {
                while (!check_alignment_requirements(ptr, alignment))
                {
                    ptr = CAST_PTR(aliasing_type(void)*)(
                        CAST_PTR(uintptr_t)(ptr) + 1
                    );
                }
                return {ptr, CAST_PTR(intptr_t)(ptr) - CAST_PTR(intptr_t)(ptr_cpy)};
            }
            UNREACHABLE();
        }

        struct alignas(max_align_t) memory_header;
        using memory_header_t_base = memory_header;
        declare_aliasing_type(memory_header_t_base, memory_header_t);

        declare_aliasing_type(std::byte, aligned_at(alignof(uint8_t)) byte_t);
        static_assert(sizeof(byte_t) * 8 == __CHAR_BIT__, "byte_t must be 1 byte");
        static_assert(alignof(byte_t) == alignof(uint8_t), "byte_t must be aligned to uint8_t");

        struct alignas(max_align_t) memory_header
        {
            byte_t* chnk_limits[2];
            memory_header_t* next;
            memory_header_t* prev;
            byte_t* data_start;
            size_t wanted_align;
            bool free;
        };
        static_assert(alignof(memory_header_t) == alignof(max_align_t), "memory_header_t must be aligned to max_align_t");
        static_assert(sizeof(memory_header_t) >= alignof(memory_header_t));
        static_assert(sizeof(memory_header_t) % alignof(memory_header_t) == 0);

        template <size_t Size = STATIC_MEMPOOL_SIZE>
        class StaticMemPool
        {
            STATIC_TODO(
                "To avoid U.B., use `std::start_lifetime_as` as soon as it is available in order to access "
                "the stored memory headers in the pool, instead of std::launder + std::construct_at + std::bit_cast"
            );
#if __cpp_lib_start_lifetime_as >= 202207L
    #error "Please reimplement the StaticMemPool class using `std::start_lifetime_as` when needed or possible"
#endif
            private:
                template <bool Const, bool Volatile, bool Restrict, typename T>
                using cvr_pointer_type_adaptor = std::conditional_t<
                    Const,
                    std::conditional_t<
                        Volatile,
                        std::conditional_t<
                            Restrict,
                            T* const volatile restrict,
                            T* const volatile
                        >,
                        std::conditional_t<
                            Restrict,
                            T* const restrict,
                            T* const
                        >
                    >,
                    std::conditional_t<
                        Volatile,
                        std::conditional_t<
                            Restrict,
                            T* volatile restrict,
                            T* volatile
                        >,
                        std::conditional_t<
                            Restrict,
                            T* restrict,
                            T*
                        >
                    >
                >;
            
            public:
                static constexpr size_t pool_size = Size;
                static_assert(pool_size > 0, "Pool size must be greater than 0");

                using pointer_type = aliasing_type(void)*;
                
                template <bool Const, bool Volatile, bool Restrict>
                using cvr_pointer_type = cvr_pointer_type_adaptor<Const, Volatile, Restrict, aliasing_type(void)>;

                static_assert(std::same_as<cvr_pointer_type<false, false, false>, aliasing_void*>);
                static_assert(std::same_as<cvr_pointer_type<true, false, false>, aliasing_void* const>);
                static_assert(std::same_as<cvr_pointer_type<false, true, false>, aliasing_void* volatile>);
                static_assert(std::same_as<cvr_pointer_type<true, true, false>, aliasing_void* const volatile>);

                static_assert(std::same_as<cvr_pointer_type<false, false, true>, aliasing_void* restrict>);
                static_assert(std::same_as<cvr_pointer_type<true, false, true>, aliasing_void* const restrict>);
                static_assert(std::same_as<cvr_pointer_type<false, true, true>, aliasing_void* volatile restrict>);
                static_assert(std::same_as<cvr_pointer_type<true, true, true>, aliasing_void* const volatile restrict>);

            private:
                always_inline flatten
                static constexpr inline std::pair<pointer_type, intptr_t> align_pointer(pointer_type ptr, size_t align) noexcept
                {
                    return { align_ptr(ptr, align).ptr, align_ptr(ptr, align).offset };
                }

                always_inline flatten
                static constexpr inline std::pair<pointer_type, intptr_t> align_pointer(uintptr_t ptr, size_t align) noexcept
                {
                    return { align_ptr(CAST_PTR(aliasing_type(void)*)(ptr), align).ptr, align_ptr(CAST_PTR(aliasing_type(void)*)(ptr), align).offset };
                }

                always_inline flatten
                static constexpr inline bool in_chunk(
                    cvr_pointer_type_adaptor<true, false, true, const memory_header_t> hdr,
                    pointer_type ptr
                ) noexcept
                {
                    CONTRACT(FN,,
                        [&] {
                            BOOST_CONTRACT_ASSERT(ptr != nullptr);
                            BOOST_CONTRACT_ASSERT(hdr != nullptr);
                            BOOST_CONTRACT_ASSERT(hdr->chnk_limits[0] != nullptr);
                            BOOST_CONTRACT_ASSERT(hdr->chnk_limits[1] != nullptr);
                            BOOST_CONTRACT_ASSERT(
                                CAST_PTR(uintptr_t)(hdr->chnk_limits[1]) > CAST_PTR(uintptr_t)(hdr->chnk_limits[0])
                            );
                        }
                    );

                    uintptr_t ptr_val = CAST_PTR(uintptr_t)(ptr);
                    uintptr_t chnk_start = CAST_PTR(uintptr_t)(hdr->chnk_limits[0]);
                    uintptr_t chnk_end = CAST_PTR(uintptr_t)(hdr->chnk_limits[1]);

                    return ptr_val >= chnk_start && ptr_val < chnk_end;
                }

                always_inline flatten
                static constexpr inline size_t chunk_size(
                    cvr_pointer_type_adaptor<true, false, true, const memory_header_t> hdr
                ) noexcept
                {
                    CONTRACT(FN,,
                        [&] {
                            BOOST_CONTRACT_ASSERT(hdr != nullptr);
                            BOOST_CONTRACT_ASSERT(hdr->chnk_limits[0] != nullptr);
                            BOOST_CONTRACT_ASSERT(hdr->chnk_limits[1] != nullptr);
                            BOOST_CONTRACT_ASSERT(
                                CAST_PTR(uintptr_t)(hdr->chnk_limits[1]) > CAST_PTR(uintptr_t)(hdr->chnk_limits[0])
                            );
                        }
                    );

                    uintptr_t start = CAST_PTR(uintptr_t)(hdr->chnk_limits[0]);
                    uintptr_t end = CAST_PTR(uintptr_t)(hdr->chnk_limits[1]);

                    return end - start;
                }

                always_inline flatten
                static constexpr inline size_t data_room(
                    cvr_pointer_type_adaptor<true, false, true, const memory_header_t> hdr
                ) noexcept
                {
                    CONTRACT(FN,,
                        [&] {
                            BOOST_CONTRACT_ASSERT(hdr != nullptr);
                            BOOST_CONTRACT_ASSERT(hdr->data_start != nullptr);
                            BOOST_CONTRACT_ASSERT(hdr->chnk_limits[1] != nullptr);
                            BOOST_CONTRACT_ASSERT(
                                CAST_PTR(uintptr_t)(hdr->chnk_limits[1]) >= CAST_PTR(uintptr_t)(hdr->data_start)
                            );
                        }
                    );
                    uintptr_t start = CAST_PTR(uintptr_t)(hdr->data_start);
                    uintptr_t end = CAST_PTR(uintptr_t)(hdr->chnk_limits[1]);

                    return end - start;
                }

                always_inline flatten
                static constexpr inline bool enough_room(
                    cvr_pointer_type_adaptor<true, false, false, const memory_header_t> hdr,
                    size_t size,
                    size_t align
                ) noexcept
                {
                    CONTRACT(FN,,
                        [&] {
                            BOOST_CONTRACT_ASSERT(hdr != nullptr);
                            BOOST_CONTRACT_ASSERT(hdr->data_start != nullptr);
                            BOOST_CONTRACT_ASSERT(hdr->chnk_limits[0] != nullptr);
                            BOOST_CONTRACT_ASSERT(hdr->chnk_limits[1] != nullptr);
                            BOOST_CONTRACT_ASSERT(
                                CAST_PTR(uintptr_t)(hdr->chnk_limits[1]) >= CAST_PTR(uintptr_t)(hdr->chnk_limits[0])
                            );
                            BOOST_CONTRACT_ASSERT(
                                CAST_PTR(uintptr_t)(hdr->chnk_limits[1]) >= CAST_PTR(uintptr_t)(hdr->data_start)
                            );
                            BOOST_CONTRACT_ASSERT(
                                CAST_PTR(uintptr_t)(hdr->data_start) == CAST_PTR(uintptr_t)(hdr->chnk_limits[0]) +
                                align_pointer(hdr->chnk_limits[0], alignof(memory_header_t)).second +
                                sizeof(memory_header_t)
                            );
                        }
                    );

                    uintptr_t room = static_cast<uintptr_t>(data_room(hdr));
                    uintptr_t alignment_requirement = static_cast<uintptr_t>(align_pointer(hdr->data_start, align).second);

                    if (room < alignment_requirement)
                        return false;
                    return room - alignment_requirement >= static_cast<uintptr_t>(size);
                }

                always_inline flatten
                static constexpr inline bool can_split(
                    cvr_pointer_type_adaptor<true, false, false, const memory_header_t> hdr,
                    size_t size,
                    size_t align
                ) noexcept
                {
                    CONTRACT(FN,,
                        [&] {
                            BOOST_CONTRACT_ASSERT(enough_room(hdr, size, align));
                        }
                    );

                    uintptr_t room = static_cast<uintptr_t>(data_room(hdr));
                    uintptr_t required_alignment_requirement = static_cast<uintptr_t>(align_pointer(hdr->data_start, align).second);
                    uintptr_t potential_new_chunk_end = static_cast<uintptr_t>(
                        CAST_PTR(uintptr_t)(hdr->data_start) +
                        required_alignment_requirement +
                        size
                    );
                    uintptr_t new_hdr_alignment_requirement = static_cast<uintptr_t>(
                        align_pointer(
                            CAST_PTR(aliasing_type(void)*)(potential_new_chunk_end),
                            alignof(memory_header_t)
                        ).second
                    );
                    uintptr_t new_hdr_size = static_cast<uintptr_t>(sizeof(memory_header_t));
                    uintptr_t new_chunk_min_requirements = new_hdr_alignment_requirement + new_hdr_size;

                    if (
                        room < required_alignment_requirement ||
                        room - required_alignment_requirement < new_chunk_min_requirements
                    )
                    {
                        hard_assert(!bool(CAN_SPLIT(hdr, size, align)));
                        return false;
                    }
                    hard_assert(
                        bool(CAN_SPLIT(hdr, size, align)) ==
                        bool(
                            room - required_alignment_requirement - new_chunk_min_requirements >= size
                        )
                    );
                    return room - required_alignment_requirement - new_chunk_min_requirements >= size;
                }

                always_inline flatten
                static constexpr inline aliasing_type(void)* mem_hdr_align(aliasing_type(void)* addr) noexcept
                {
                    return CAST_PTR(aliasing_type(void)*)(
                        CAST_PTR(uintptr_t)(addr) +
                        static_cast<uintptr_t>(align_pointer(addr, alignof(memory_header_t)).second)
                    );
                }
                
                constexpr inline void init_pool() noexcept
                {
                    std::lock_guard<std::recursive_mutex> lock(this->mutex);
                    auto offset = align_pointer(DECAY(this->data), alignof(memory_header_t));
                    hard_assert(offset.second == 0); // Because we aligned it
                    hard_assert(check_alignment_requirements(offset.first, alignof(memory_header_t)));
                    volatile memory_header_t* volatile hdr = std::launder(
                        std::construct_at(
                            CAST_PTR(memory_header_t*)(
                                assume_aligned(
                                    offset.first,
                                    alignof(memory_header_t)
                                )
                            )
                        )
                    );                        
                    hdr->chnk_limits[0] = DECAY(this->data);
                    hdr->chnk_limits[1] = DECAY(this->data) + pool_size;
                    hdr->next = nullptr;
                    hdr->prev = nullptr;
                    hdr->data_start = CAST_PTR(byte_t*)(CAST_PTR(uintptr_t)(offset.first) + sizeof(memory_header_t));
                    hdr->wanted_align = 0; // We don't really care here
                    hdr->free = true;
                }
                
                constexpr void merge_free_chunks()
                {
                    std::lock_guard<std::recursive_mutex> lock(this->mutex);
                    uintptr_t hdr_addr = CAST_PTR(uintptr_t)(
                        assume_aligned(DECAY(this->data), alignof(memory_header_t))
                    );
                    memory_header_t hdr = this->bit_cast<memory_header_t>(
                        CAST_PTR(byte_t*)(
                            assume_aligned(
                                CAST_PTR(byte_t*)(hdr_addr),
                                alignof(memory_header_t)
                            )
                        )
                    );
                    while (true)
                    {
                        if (hdr.next == nullptr)
                            break;
                        if (hdr.free)
                        {
                            bool keep_going = true;
                            do //while (next_hdr != nullptr && next_hdr->free)
                            {
                                uintptr_t next_hdr_addr = CAST_PTR(uintptr_t)(
                                    CAST_PTR(byte_t*)(
                                        assume_aligned(
                                            hdr.next,
                                            alignof(memory_header_t)
                                        )
                                    )
                                );
                                memory_header_t next_hdr = this->bit_cast<memory_header_t>(
                                    CAST_PTR(byte_t*)(
                                        assume_aligned(
                                            CAST_PTR(byte_t*)(next_hdr_addr),
                                            alignof(memory_header_t)
                                        )
                                    )
                                );
                                if (next_hdr.free)
                                {
                                    /* if (this->safe.load(std::memory_order::acquire))
                                        ::gnulibcxx::memset_explicit(CAST_PTR(byte_t*)(next_hdr_addr), 0, sizeof(memory_header_t)); */
                                    memory_header_t* merged = std::launder(
                                        std::construct_at(
                                            CAST_PTR(memory_header_t*)(
                                                assume_aligned(
                                                    CAST_PTR(memory_header_t*)(hdr_addr),
                                                    alignof(memory_header_t)
                                                )
                                            )
                                        )
                                    );
                                    merged->chnk_limits[0] = hdr.chnk_limits[0];
                                    merged->chnk_limits[1] = next_hdr.chnk_limits[1];
                                    merged->next = next_hdr.next;
                                    merged->prev = hdr.prev;
                                    merged->data_start = CAST_PTR(byte_t*)(
                                        hdr_addr +
                                        sizeof(memory_header_t)
                                    );
                                    merged->wanted_align = 0; // Reset it
                                    merged->free = true;
                                    if (merged->prev != nullptr)
                                        merged->prev->next = merged; // Actually useless
                                    if (merged->next != nullptr)
                                        merged->next->prev = merged;

                                    hdr = *merged;
                                    hard_assert(hdr_addr == CAST_PTR(uintptr_t)(merged));
                                    keep_going = true;
                                }
                                else
                                    keep_going = false;
                                keep_going = hdr.next == nullptr    ?
                                             false                  :
                                             keep_going;
                            } while (keep_going);
                        }
                        if (hdr.next == nullptr)
                            break;
                        hdr_addr = CAST_PTR(uintptr_t)(
                            assume_aligned(
                                hdr.next,
                                alignof(memory_header_t)
                            )
                        );
                        hdr = this->bit_cast<memory_header_t>(
                            CAST_PTR(byte_t*)(
                                assume_aligned(
                                    CAST_PTR(byte_t*)(hdr_addr),
                                    alignof(memory_header_t)
                                )
                            )
                        );
                    }
                }

                constexpr memory_header_t* get_header(pointer_type ptr) noexcept
                {
                    std::lock_guard<std::recursive_mutex> lock(this->mutex);
                    memory_header_t hdr = this->bit_cast<memory_header_t>(
                        CAST_PTR(byte_t*)(
                            assume_aligned(
                                CAST_PTR(byte_t*)(DECAY(this->data)),
                                alignof(memory_header_t)
                            )
                        )
                    );
                    uintptr_t addr = CAST_PTR(uintptr_t)(
                        assume_aligned(
                            DECAY(this->data),
                            alignof(memory_header_t)
                        )
                    );
                    while (true)
                    {
                        if ((
                                CAST_PTR(uintptr_t)(hdr.data_start) == CAST_PTR(uintptr_t)(ptr) ||
                                in_chunk(&hdr, ptr)
                            ) &&
                            !hdr.free
                        )
                        {
                            memory_header_t* volatile hdr_ptr = std::launder(
                                std::construct_at(
                                    CAST_PTR(memory_header_t*)(
                                        assume_aligned(
                                            CAST_PTR(memory_header_t*)(addr),
                                            alignof(memory_header_t)
                                        )
                                    ),
                                    std::move(hdr)
                                )
                            );
                            return hdr_ptr;
                        }
                        if (hdr.next == nullptr)
                            break;
                        addr = CAST_PTR(uintptr_t)(
                            assume_aligned(
                                hdr.next,
                                alignof(memory_header_t)
                            )
                        );
                        hdr = this->bit_cast<memory_header_t>(
                            CAST_PTR(byte_t*)(
                                assume_aligned(
                                    CAST_PTR(byte_t*)(addr),
                                    alignof(memory_header_t)
                                )
                            )
                        );
                    }
                    return nullptr;
                }

                template <typename To>
                always_inline flatten
                static constexpr inline To bit_cast(byte_t* ptr)
                {
                    no_init
                    byte_t temp[sizeof(To)];
                    std::memcpy(DECAY(temp), ptr, sizeof(To));
                    return std::bit_cast<To, byte_t[sizeof(To)]>(temp);
                }
            
            public:
                constexpr StaticMemPool()
                {
                    init_pool();
                }
                constexpr StaticMemPool(bool safe)
                {
                    this->safe.store(safe, std::memory_order::release);
                    init_pool();
                }
                
                StaticMemPool(const StaticMemPool&) = delete;
                StaticMemPool(StaticMemPool&&) = delete;
                
                StaticMemPool& operator=(const StaticMemPool&) = delete;
                StaticMemPool& operator=(StaticMemPool&&) = delete;
                
                constexpr ~StaticMemPool() = default;
                
                malloc_like(2, 3)
                constexpr pointer_type allocate(size_t size, size_t requested_align = __STDCPP_DEFAULT_NEW_ALIGNMENT__)
                {
                    unlikely_if (size == 0)
                        throw InternalError("Cannot allocate 0 bytes");

                    unlikely_if (size > pool_size)
                        throw InternalError("Requested size is greater than the pool size");

                    unlikely_if (requested_align == 0)
                        throw InternalError("Requested alignment is 0");

                    hard_assert(std::numeric_limits<ptrdiff_t>::max() == PTRDIFF_MAX);
                    unlikely_if (size > std::numeric_limits<ptrdiff_t>::max())
                        throw InternalError("Requested size is larger than PTRDIFF_MAX");

                    std::lock_guard<std::recursive_mutex> lock(this->mutex);
                    uintptr_t hdr_addr = CAST_PTR(uintptr_t)(
                        assume_aligned(
                            DECAY(this->data),
                            alignof(memory_header_t)
                        )
                    );
                    memory_header_t hdr = this->bit_cast<memory_header_t>(
                        CAST_PTR(byte_t*)(
                            assume_aligned(
                                CAST_PTR(byte_t*)(hdr_addr),
                                alignof(memory_header_t)
                            )
                        )
                    );
                    while (true)
                    {
                        if (hdr.free && enough_room(&hdr, size, requested_align))
                        {
                            if (can_split(&hdr, size, requested_align))
                            {
                                /* Base free header */

                                const uintptr_t current_chunk_start = CAST_PTR(uintptr_t)(hdr.chnk_limits[0]);
                                const uintptr_t current_chunk_end = CAST_PTR(uintptr_t)(hdr.chnk_limits[1]);

                                const uintptr_t current_data_start = CAST_PTR(uintptr_t)(hdr.data_start);

                                hard_assert(hdr_addr == current_chunk_start + align_pointer(current_chunk_start, alignof(memory_header_t)).second);
                                hard_assert(current_data_start == hdr_addr + sizeof(memory_header_t));
                                hard_assert(current_chunk_end == current_data_start + data_room(&hdr));

                                /* New free header */

                                const uintptr_t new_chunk_start = current_data_start + align_pointer(current_data_start, requested_align).second + size;
                                const uintptr_t new_chunk_end = current_chunk_end;

                                const uintptr_t new_hdr = new_chunk_start + align_pointer(new_chunk_start, alignof(memory_header_t)).second;

                                const uintptr_t new_data_start = new_hdr + sizeof(memory_header_t);

                                /* Create modified headers */

                                memory_header_t* volatile current_hdr_ptr = std::launder(
                                    std::construct_at(
                                        CAST_PTR(memory_header_t*)(
                                            assume_aligned(
                                                CAST_PTR(memory_header_t*)(hdr_addr),
                                                alignof(memory_header_t)
                                            )
                                        )
                                    )
                                );
                                memory_header_t* volatile new_hdr_ptr = std::launder(
                                    std::construct_at(
                                        CAST_PTR(memory_header_t*)(
                                            assume_aligned(
                                                CAST_PTR(memory_header_t*)(new_hdr),
                                                alignof(memory_header_t)
                                            )
                                        )
                                    )
                                );

                                current_hdr_ptr->chnk_limits[0] = CAST_PTR(byte_t*)(current_chunk_start);
                                current_hdr_ptr->chnk_limits[1] = CAST_PTR(byte_t*)(new_chunk_start);
                                current_hdr_ptr->data_start = CAST_PTR(byte_t*)(
                                    current_data_start + align_pointer(current_data_start, requested_align).second
                                );
                                current_hdr_ptr->wanted_align = requested_align;
                                current_hdr_ptr->free = false;
                                current_hdr_ptr->next = new_hdr_ptr;
                                current_hdr_ptr->prev = hdr.prev;

                                new_hdr_ptr->chnk_limits[0] = CAST_PTR(byte_t*)(new_chunk_start);
                                new_hdr_ptr->chnk_limits[1] = CAST_PTR(byte_t*)(new_chunk_end);
                                new_hdr_ptr->data_start = CAST_PTR(byte_t*)(new_data_start);
                                new_hdr_ptr->wanted_align = 0; // We don't really care here too
                                new_hdr_ptr->free = true;
                                new_hdr_ptr->next = hdr.next;
                                new_hdr_ptr->prev = current_hdr_ptr;

                                return assume_aligned(CAST_PTR(pointer_type)(current_hdr_ptr->data_start), requested_align);
                            }
                            else
                            {
                                memory_header_t* volatile current_hdr_ptr = std::launder(
                                    std::construct_at(
                                        CAST_PTR(memory_header_t*)(
                                            assume_aligned(
                                                CAST_PTR(memory_header_t*)(hdr_addr),
                                                alignof(memory_header_t)
                                            )
                                        )
                                    )
                                );
                                current_hdr_ptr->free = false;
                                current_hdr_ptr->wanted_align = requested_align;
                                current_hdr_ptr->data_start = CAST_PTR(byte_t*)(
                                    assume_aligned(
                                        CAST_PTR(byte_t*)(
                                            hdr_addr +
                                            sizeof(memory_header_t) +
                                            align_pointer(
                                                hdr_addr + sizeof(memory_header_t),
                                                requested_align
                                            ).second
                                        ),
                                        requested_align
                                    )
                                );
                                return assume_aligned(
                                    CAST_PTR(pointer_type)(hdr.data_start),
                                    requested_align
                                );                         

                            }
                        }
                        if (hdr.next == nullptr)
                            break;
                        hdr_addr = CAST_PTR(uintptr_t)(
                            assume_aligned(hdr.next, alignof(memory_header_t))
                        );
                        hdr = this->bit_cast<memory_header_t>(
                            CAST_PTR(byte_t*)(
                                assume_aligned(
                                    CAST_PTR(byte_t*)(hdr_addr),
                                    alignof(memory_header_t)
                                )
                            )
                        );
                    }
                    throw InternalError("No free chunk available");
                }

                template <typename... T>
                constexpr pointer_type reallocate(pointer_type ptr, size_t size, T... requested_align) = delete;

                realloc_like(3)
                constexpr pointer_type reallocate(pointer_type ptr, size_t size)
                {
                    STATIC_TODO(
                        "Improve reallocate methods by shrinking the chunk when possible"
                    );
                    unlikely_if (ptr == nullptr)
                        return this->allocate(size);

                    unlikely_if (size == 0)
                        throw InternalError("Cannot allocate 0 bytes");

                    unlikely_if (size > pool_size)
                        throw InternalError("Requested size is greater than the pool size");

                    hard_assert(std::numeric_limits<ptrdiff_t>::max() == PTRDIFF_MAX);
                    unlikely_if (size > std::numeric_limits<ptrdiff_t>::max())
                        throw InternalError("Requested size is larger than PTRDIFF_MAX");

                    std::lock_guard<std::recursive_mutex> lock(this->mutex);
                    memory_header_t* hdr = this->get_header(ptr);
                    unlikely_if (hdr == nullptr)
                        throw InternalError("Pointer not found in pool");
                    size_t requested_align = hdr->wanted_align;
                    pointer_type aligned_ptr = assume_aligned(hdr->data_start, requested_align);
                    if (data_room(hdr) >= size)
                        return aligned_ptr;
                    
                    pointer_type new_ptr = this->allocate(size, requested_align);
                    if (new_ptr == nullptr)
                        throw InternalError("Failed to reallocate");
                    pointer_type aligned_new_ptr = assume_aligned(new_ptr, requested_align);
                    
                    std::memmove(aligned_new_ptr, aligned_ptr, std::min(size, data_room(hdr)));
                    
                    this->deallocate(aligned_ptr);
                    
                    return aligned_new_ptr;
                }

                realloc_like(3, 4)
                constexpr pointer_type reallocate(pointer_type ptr, size_t size, size_t requested_align)
                {
                    STATIC_TODO(
                        "Improve reallocate methods by shrinking the chunk when possible"
                    );

                    unlikely_if (ptr == nullptr)
                        return this->allocate(size, requested_align);

                    unlikely_if (size == 0)
                        throw InternalError("Cannot allocate 0 bytes");

                    unlikely_if (size > pool_size)
                        throw InternalError("Requested size is greater than the pool size");

                    hard_assert(std::numeric_limits<ptrdiff_t>::max() == PTRDIFF_MAX);
                    unlikely_if (size > std::numeric_limits<ptrdiff_t>::max())
                        throw InternalError("Requested size is larger than PTRDIFF_MAX");

                    std::lock_guard<std::recursive_mutex> lock(this->mutex);
                    memory_header_t* hdr = this->get_header(ptr);
                    unlikely_if (hdr == nullptr)
                        throw InternalError("Pointer not found in pool");
                    pointer_type aligned_ptr = assume_aligned(hdr->data_start, hdr->wanted_align);
                    if (data_room(hdr) >= size && hdr->wanted_align == requested_align)
                        return aligned_ptr;

                    pointer_type new_ptr = this->allocate(size, requested_align);
                    if (new_ptr == nullptr)
                        throw InternalError("Failed to reallocate");
                    pointer_type aligned_new_ptr = assume_aligned(new_ptr, requested_align);

                    std::memmove(aligned_new_ptr, aligned_ptr, std::min(size, data_room(hdr)));

                    this->deallocate(aligned_ptr);

                    return aligned_new_ptr;
                }

                free_like(1)
                constexpr void deallocate(pointer_type ptr)
                {
                    unlikely_if (ptr == nullptr)
                        throw InternalError("Cannot deallocate nullptr");

                    std::lock_guard<std::recursive_mutex> lock(this->mutex);

                    memory_header_t* hdr = this->get_header(ptr);
                    unlikely_if (hdr == nullptr)
                        throw InternalError("Pointer not found in pool");
                    hdr->free = true;
                    if (this->safe.load(std::memory_order::acquire))
                        ::gnulibcxx::memset_explicit(hdr->data_start, ::arc4random_uniform(256), data_room(hdr));
                    hdr->data_start = CAST_PTR(byte_t*)(
                        CAST_PTR(uintptr_t)(hdr->chnk_limits[0]) +
                        align_pointer(hdr->chnk_limits[0], alignof(memory_header_t)).second +
                        sizeof(memory_header_t)
                    );
                    hdr->wanted_align = 0; // Reset the wanted alignment
                    STATIC_TODO(
                        "Potentially count deallocation and only merge after X deallocations"
                    );
                    this->merge_free_chunks();
                }

                constexpr inline bool is_safe() const noexcept
                {
                    return this->safe.load(std::memory_order::acquire);
                }

                constexpr inline void set_safe(bool safe) noexcept
                {
                    this->safe.store(safe, std::memory_order::release);
                }

                void dump_hdrs(std::ostream& s = std::cout)
                {
                    std::lock_guard<std::recursive_mutex> lock(this->mutex);
                    uintptr_t hdr_addr = CAST_PTR(uintptr_t)(
                        assume_aligned(DECAY(this->data), alignof(memory_header_t))
                    );
                    memory_header_t hdr = this->bit_cast<memory_header_t>(
                        CAST_PTR(byte_t*)(
                            assume_aligned(
                                CAST_PTR(byte_t*)(hdr_addr),
                                alignof(memory_header_t)
                            )
                        )
                    );
                    s << "\nDumping headers:\n";
                    while (true)
                    {
                        s << std::hex;
                        s << "Header at " << hdr_addr << ":\n";
                        s << "  Chunk limits: [" << CAST_PTR(uintptr_t)(hdr.chnk_limits[0]) << ", " << CAST_PTR(uintptr_t)(hdr.chnk_limits[1]) << "]\n";
                        if (hdr.prev != nullptr)
                            s << "  Previous header at " << CAST_PTR(uintptr_t)(hdr.prev) << ": [" << CAST_PTR(uintptr_t)(hdr.prev->chnk_limits[0]) << ", " << CAST_PTR(uintptr_t)(hdr.prev->chnk_limits[1]) << "]\n";
                        else
                            s << "  Previous header: nullptr\n";
                        if (hdr.next != nullptr)
                            s << "  Next header at " << CAST_PTR(uintptr_t)(hdr.next) << ": [" << CAST_PTR(uintptr_t)(hdr.next->chnk_limits[0]) << ", " << CAST_PTR(uintptr_t)(hdr.next->chnk_limits[1]) << "]\n";
                        else
                            s << "  Next header: nullptr\n";
                        s << "  Data start: " << CAST_PTR(uintptr_t)(hdr.data_start) << "\n";
                        s << std::dec;
                        s << "  Wanted alignment: " << hdr.wanted_align << "bytes\n";
                        s << "  Free: " << (hdr.free ? "yes" : "no") << "\n";
                        s << "  Chunk size: " << chunk_size(&hdr) << "bytes\n";
                        s << "  Data room: " << data_room(&hdr) << "bytes\n";
                        s << std::flush;

                        if (hdr.next == nullptr)
                            break;
                        hdr_addr = CAST_PTR(uintptr_t)(
                            assume_aligned(hdr.next, alignof(memory_header_t))
                        );
                        hdr = this->bit_cast<memory_header_t>(
                            CAST_PTR(byte_t*)(
                                assume_aligned(
                                    CAST_PTR(byte_t*)(hdr_addr),
                                    alignof(memory_header_t)
                                )
                            )
                        );
                    }
                    s << "\n";
                }

            private:
                /*
                 * Organized as follows:
                 * [
                 *     [
                 *         memory_header_t |
                 *         data ...
                 *     ]
                 *     [
                 *         potentially unused space / padding |
                 *         next memory_header_t               |
                 *         data ...
                 *    ]
                 *   ...
                 * ]
                 */
                alignas(memory_header_t) byte_t data[pool_size];
                std::recursive_mutex mutex;
                std::atomic<bool> safe = ATOMIC_VAR_INIT(false);
        };

        static StaticMemPool<STATIC_MEMPOOL_SIZE> static_mem_pool;

        malloc_like(1, 2)
        aliasing_type(void)* static_alloc(size_t size, size_t alignment)
        {
            return static_mem_pool.allocate(size, alignment);
        }

        calloc_like(1, 2, 3)
        aliasing_type(void)* static_alloc_array(size_t count, size_t size, size_t alignment)
        {
            unlikely_unless (size >= alignment)
                throw InternalError("Size must be greater than or equal to alignment");
            return static_mem_pool.allocate(count * size, alignment);
        }

        free_like(1)
        void static_dealloc(aliasing_type(void)* ptr)
        {
            static_mem_pool.deallocate(ptr);
        }

        realloc_like(2)
        aliasing_type(void)* static_realloc(aliasing_type(void)* ptr, size_t size)
        {
            return static_mem_pool.reallocate(ptr, size);
        }

        realloc_like(2, 3)
        aliasing_type(void)* static_realloc(aliasing_type(void)* ptr, size_t size, size_t alignment)
        {
            return static_mem_pool.reallocate(ptr, size, alignment);
        }

        reallocarray_like(2, 3)
        aliasing_type(void)* static_realloc_array(aliasing_type(void)* ptr, size_t count, size_t size)
        {
            return static_mem_pool.reallocate(ptr, count * size);
        }

        reallocarray_like(2, 3, 4)
        aliasing_type(void)* static_realloc_array(aliasing_type(void)* ptr, size_t count, size_t size, size_t alignment)
        {
            unlikely_unless (size >= alignment)
                throw InternalError("Size must be greater than or equal to alignment");
            return static_mem_pool.reallocate(ptr, count * size, alignment);
        }

        namespace Test
        {
            void static_dump_hdrs(std::ostream& s)
            {
                static_mem_pool.dump_hdrs(s);
            }
        }
    }
}

#undef assume_aligned
#undef aligned_ret
#undef CAST_PTR
#undef DECAY
#undef IS_IN_CHUNK
#undef CHNK_SZ
#undef DATA_ROOM
#undef ENOUGH_ROOM
#undef CAN_SPLIT
#undef NEXT_CHNK
#undef PREV_CHNK
#undef MEM_HDR_ALIGN

POP_MACRO(assume_aligned)
POP_MACRO(aligned_ret)
POP_MACRO(CAST_PTR)
POP_MACRO(DECAY)
POP_MACRO(IS_IN_CHUNK)
POP_MACRO(CHNK_SZ)
POP_MACRO(DATA_ROOM)
POP_MACRO(ENOUGH_ROOM)
POP_MACRO(CAN_SPLIT)
POP_MACRO(NEXT_CHNK)
POP_MACRO(PREV_CHNK)
POP_MACRO(MEM_HDR_ALIGN)
