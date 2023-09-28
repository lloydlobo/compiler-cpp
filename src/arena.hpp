#pragma once

// Created because of: Forward declare NodeExpr
// use pointer to satisfy incomplete type error tldr; we need to know size of NodeExpr to avoid
// circular definitions so we need to dynamically allocate the data and store it somewhere, but they
// are small so alloc and free will be expensive as they have small memory footprint and worsens CPU
// cache causing cache miss. so we need a contiguos memory with Area Allocator (loan of memory e.g.
// 1mb). Then we just advance a pointer by 1 unit or the pointer/elements size. so all elements are
// next to each other. |1|  3|    4| 2|1|1|. No prob with lifetimes, and can use raw pointers. Then
// free all at once after generation.

// Area of contiguous ropes of blocks

#include <cstddef>
#include <cstdlib>

class ArenaAllocator {
public:
    inline ArenaAllocator(size_t bytes)
        : m_size(bytes)
    {
        m_buffer = static_cast<std::byte*>(malloc(m_size));
        m_offset = m_buffer; // starts where buffer starts
    }

    // automatically determine the size of allocation
    template <typename T>
    inline T* alloc()
    {
        void* offset = m_offset; // starts where buffer starts
        m_offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    // make it non-copyable
    inline ArenaAllocator(const ArenaAllocator& other) = delete;

    inline ArenaAllocator operator=(const ArenaAllocator& other) = delete;

    inline ~ArenaAllocator()
    {
        free(m_buffer);
    };

private:
    size_t m_size;
    std::byte* m_buffer;
    std::byte* m_offset;
};
