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


namespace SupDef
{
    namespace Util
    {
        static_assert(STATIC_MEMPOOL_SIZE > 0, "STATIC_MEMPOOL_SIZE must be greater than 0");

#ifdef assume_aligned
    PUSH_MACRO(assume_aligned)
#endif
#undef assume_aligned
#ifdef __has_builtin
    #if __has_builtin(__builtin_assume_aligned)
        #define assume_aligned(ptr, align, ...) __builtin_assume_aligned((ptr), (align) __VA_OPT__(,) FIRST_ARG(__VA_ARGS__))
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
        #define aligned_ret(align, ...) [[__gnu__::__assume_aligned__((align) __VA_OPT__(,) FIRST_ARG(__VA_ARGS__))]]
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
        static inline bool check_alignment_requirements(aliasing_type(void)* ptr, size_t alignment)
        {
            return (CAST_PTR(uintptr_t)(ptr) % alignment) == 0;
        }

        always_inline warn_unused_result() flatten
        static inline memory_offset align_ptr(aliasing_type(void)* ptr, size_t alignment)
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

        struct memory_header;
        using memory_header_t_base = alignas(max_align_t) memory_header;
        declare_aliasing_type(memory_header_t_base, memory_header_t);

        declare_aliasing_type(std::byte, byte_t);
        static_assert(sizeof(byte_t) * 8 == __CHAR_BIT__, "byte_t must be 1 byte");

        STATIC_TODO(
            "Add `size_t wanted_align` to memory_header_t to store the wanted alignment of the data, "
            "so we can later reuse it when reallocating the data."
        )
        struct memory_header
        {
            byte_t* chnk_limits[2];
            memory_header_t* next;
            memory_header_t* prev;
            byte_t* data_start;
            size_t wanted_align;
            bool free;
        };
        static_assert(alignof(memory_header_t) == alignof(max_align_t), "memory_header_t must be aligned to max_align_t");

        template <size_t Size = STATIC_MEMPOOL_SIZE>
        class StaticMemPool
        {
            public:
                static constexpr size_t size = Size;
                using pointer_type = aliasing_type(void)*;

            private:
                always_inline flatten
                static constexpr inline std::pair<pointer_type, intptr_t> align_pointer(pointer_type ptr, size_t align) noexcept
                {
                    return { align_ptr(ptr, align).ptr, align_ptr(ptr, align).offset };
                }

                always_inline flatten
                static constexpr inline bool in_chunk(memory_header_t* hdr, pointer_type ptr) noexcept
                {
                    uintptr_t ptr_val = CAST_PTR(uintptr_t)(ptr);
                    uintptr_t chnk_start = CAST_PTR(uintptr_t)(hdr->chnk_limits[0]);
                    uintptr_t chnk_end = CAST_PTR(uintptr_t)(hdr->chnk_limits[1]);
                    hard_assert(chnk_end > chnk_start);

                    return ptr_val >= chnk_start && ptr_val < chnk_end;
                }

                always_inline flatten
                static constexpr inline size_t chunk_size(memory_header_t* hdr) noexcept
                {
                    uintptr_t start = CAST_PTR(uintptr_t)(hdr->chnk_limits[0]);
                    uintptr_t end = CAST_PTR(uintptr_t)(hdr->chnk_limits[1]);
                    hard_assert(end > start);

                    return end - start;
                }

                always_inline flatten
                static constexpr inline size_t data_room(memory_header_t* hdr) noexcept
                {
                    uintptr_t start = CAST_PTR(uintptr_t)(hdr->data_start);
                    uintptr_t end = CAST_PTR(uintptr_t)(hdr->chnk_limits[1]);
                    hard_assert(end >= start);

                    return end - start;
                }

                always_inline flatten
                static constexpr inline bool enough_room(memory_header_t* hdr, size_t size, size_t align) noexcept
                {                    
                    uintptr_t room = static_cast<uintptr_t>(data_room(hdr));
                    uintptr_t alignment_requirement = static_cast<uintptr_t>(align_pointer(hdr->data_start, align).second);

                    if (room < alignment_requirement)
                        return false;
                    return room - alignment_requirement >= static_cast<uintptr_t>(size);
                }

                always_inline flatten
                static constexpr inline bool can_split(memory_header_t* hdr, size_t size, size_t align) noexcept
                {
                    hard_assert(enough_room(hdr, size, align));


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
                    volatile memory_header_t* volatile hdr = CAST_PTR(memory_header_t*)(CAST_PTR(byte_t*)(offset.first));
                    (*hdr).chnk_limits[0] = DECAY(this->data);
                    (*hdr).chnk_limits[1] = DECAY(this->data) + Size;
                    (*hdr).next = nullptr;
                    (*hdr).prev = nullptr;
                    (*hdr).data_start = CAST_PTR(byte_t*)(CAST_PTR(uintptr_t)(offset.first) + sizeof(memory_header_t));
                    (*hdr).wanted_align = alignof(max_align_t); // We don't really care here
                    (*hdr).free = true;
                }
                
                constexpr void merge_free_chunks()
                {
                    std::lock_guard<std::recursive_mutex> lock(this->mutex);
                    memory_header_t* hdr = CAST_PTR(memory_header_t*)(DECAY(this->data));
                    while (hdr)
                    {
                        if (hdr->free)
                        {
                            memory_header_t* next_hdr = hdr->next;
                            while (next_hdr != nullptr && next_hdr->free)
                            {
                                hdr->chnk_limits[1] = next_hdr->chnk_limits[1];
                                hdr->next = next_hdr->next;
                                if (hdr->next != nullptr)
                                    hdr->next->prev = hdr;
                                if (this->safe.load(std::memory_order::acquire))
                                    ::gnulibcxx::memset_explicit(next_hdr, 0, sizeof(memory_header_t));
                                next_hdr = hdr->next;
                            }
                        }
                        hdr = hdr->next;
                    }
                }

                constexpr memory_header_t* get_header(pointer_type ptr) noexcept // ?
                {
                    TODO();
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
                
                malloc_like(1, 2)
                constexpr pointer_type allocate(size_t size, size_t requested_align = __STDCPP_DEFAULT_NEW_ALIGNMENT__)
                {
                    unlikely_if (size == 0)
                        throw InternalError("Cannot allocate 0 bytes");

                    unlikely_if (size > Size)
                        throw InternalError("Requested size is greater than the pool size");

                    unlikely_if (requested_align == 0)
                        throw InternalError("Requested alignment is 0");

                    hard_assert(std::numeric_limits<ptrdiff_t>::max() == PTRDIFF_MAX);
                    unlikely_if (size > std::numeric_limits<ptrdiff_t>::max())
                        throw InternalError("Requested size is larger than PTRDIFF_MAX");

                    std::lock_guard<std::recursive_mutex> lock(this->mutex);
                    memory_header_t* hdr = CAST_PTR(memory_header_t*)(DECAY(this->data));
                    while (hdr)
                    {
                        if (hdr->free && enough_room(hdr, size, requested_align))
                        {
                            if (can_split(hdr, size, requested_align))
                            {
                                memory_header_t new_hdr{};
                                new_hdr.chnk_limits = {
                                    CAST_PTR(byte_t*)(
                                        CAST_PTR(uintptr_t)(hdr->data_start) +
                                        align_pointer(hdr->data_start, requested_align).second +
                                        size
                                    ),
                                    hdr->chnk_limits[1]
                                };
                                new_hdr.next = hdr->next;
                                new_hdr.prev = hdr;
                                new_hdr.data_start = 
                                    CAST_PTR(byte_t*)(
                                        CAST_PTR(uintptr_t)(hdr->data_start) +
                                        align_pointer(hdr->data_start, requested_align).second +
                                        size
                                    ) +
                                    align_pointer(
                                        CAST_PTR(byte_t*)(
                                            CAST_PTR(uintptr_t)(hdr->data_start) +
                                            align_pointer(hdr->data_start, requested_align).second +
                                            size
                                        ),
                                        alignof(memory_header_t)
                                    ).second +
                                    sizeof(memory_header_t);
                                new_hdr.wanted_align = alignof(max_align_t); // We don't really care here too
                                new_hdr.free = true;
                                hdr->chnk_limits[1] = CAST_PTR(byte_t*)(
                                    CAST_PTR(uintptr_t)(hdr->data_start) +
                                    align_pointer(hdr->data_start, requested_align).second +
                                    size
                                );
                                hdr->next = CAST_PTR(memory_header_t*)(
                                    CAST_PTR(uintptr_t)(new_hdr.chnk_limits[0]) +
                                    align_pointer(new_hdr.chnk_limits[0], alignof(memory_header_t)).second
                                );
                                hard_assert(hdr->next != nullptr && CAST_PTR(uintptr_t)(hdr->next) % alignof(memory_header_t) == 0);
                                ::memcpy(hdr->next, &new_hdr, sizeof(memory_header_t));
                            }
                            hdr->free = false;
                            hdr->wanted_align = requested_align;
                            hdr->data_start = CAST_PTR(uintptr_t)(hdr->data_start) + align_pointer(hdr->data_start, requested_align).second;
                            return hdr->data_start;
                        }
                        hdr = hdr->next;
                    }
                    return nullptr;
                }

                realloc_like(2, 3)
                constexpr pointer_type reallocate(pointer_type ptr, size_t size, size_t requested_align = get_header(ptr)->wanted_align)
                {
                    TODO(
                        "Verify that get_header(ptr)->wanted_align is an ok default parameter w.r.t. the standard "
                        " (and that it compiles)\n"
                        "Idea: write two private methods `reallocate_same_align` and `reallocate_new_align`, "
                        "make `reallocate` a templated method (something like: " 
                        "`template <typename... T> requires (sizeof...(T) == 0) || (std::same_as<size_t, T> && ...) "
                        "pointer_type reallocate(pointer_type ptr, size_t size, T... requested_align)`), "
                        "and call the right method depending on the number of arguments (potentially by specializing the template "
                        "for the cases where `sizeof...(T) == 0` and `sizeof...(T) == 1`), and deleting the base templated method."
                    );
                    TODO(
                        "Implement reallocate"
                    );
                }

                free_like(1)
                constexpr void deallocate(pointer_type* ptr)
                {
                    std::lock_guard<std::recursive_mutex> lock(this->mutex);
                    memory_header_t* hdr = CAST_PTR(memory_header_t*)(DECAY(this->data));
                    while (hdr)
                    {
                        if ((hdr->data_start == ptr || in_chunk(hdr, ptr)) && !hdr->free)
                        {
                            hdr->free = true;
                            hdr->data_start = CAST_PTR(byte_t*)(
                                CAST_PTR(uintptr_t)(hdr->chnk_limits[0]) +
                                align_pointer(hdr->chnk_limits[0], alignof(memory_header_t)).second +
                                sizeof(memory_header_t)
                            );
                            if (this->safe.load(std::memory_order::acquire))
                                ::gnulibcxx::memset_explicit(hdr->data_start, arc4random_uniform(256), data_room(hdr));
                            this->merge_free_chunks();
                            return;
                        }
                        hdr = hdr->next;
                    }
                    this->merge_free_chunks();
                    throw InternalError("Pointer not found in pool");
                }

                constexpr inline bool is_safe() const noexcept
                {
                    return this->safe.load(std::memory_order::acquire);
                }

                constexpr inline void set_safe(bool safe) noexcept
                {
                    this->safe.store(safe, std::memory_order::release);
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
                alignas(memory_header_t) byte_t data[Size];
                std::recursive_mutex mutex;
                std::atomic<bool> safe = ATOMIC_VAR_INIT(false);
        };

        static StaticMemPool<STATIC_MEMPOOL_SIZE> mem_pool;
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
