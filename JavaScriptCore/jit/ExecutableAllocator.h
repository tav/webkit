/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef ExecutableAllocator_h
#define ExecutableAllocator_h

#include <limits>
#include <wtf/Assertions.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/UnusedParam.h>
#include <wtf/Vector.h>

#if PLATFORM(IPHONE)
#include <libkern/OSCacheControl.h>
#include <sys/mman.h>
#endif

#if PLATFORM(SYMBIAN)
#include <e32std.h>
#endif

#define JIT_ALLOCATOR_PAGE_SIZE (ExecutableAllocator::pageSize)
#define JIT_ALLOCATOR_LARGE_ALLOC_SIZE (ExecutableAllocator::pageSize * 4)

#if ENABLE(ASSEMBLER_WX_EXCLUSIVE)
#define PROTECTION_FLAGS_RW (PROT_READ | PROT_WRITE)
#define PROTECTION_FLAGS_RX (PROT_READ | PROT_EXEC)
#define INITIAL_PROTECTION_FLAGS PROTECTION_FLAGS_RX
#else
#define INITIAL_PROTECTION_FLAGS (PROT_READ | PROT_WRITE | PROT_EXEC)
#endif

namespace JSC {

inline size_t roundUpAllocationSize(size_t request, size_t granularity)
{
    if ((std::numeric_limits<size_t>::max() - granularity) <= request)
        CRASH(); // Allocation is too large
    
    // Round up to next page boundary
    size_t size = request + (granularity - 1);
    size = size & ~(granularity - 1);
    ASSERT(size >= request);
    return size;
}

}

#if ENABLE(ASSEMBLER)

namespace JSC {

class ExecutablePool : public RefCounted<ExecutablePool> {
private:
    struct Allocation {
        char* pages;
        size_t size;
#if PLATFORM(SYMBIAN)
        RChunk* chunk;
#endif
    };
    typedef Vector<Allocation, 2> AllocationList;

public:
    static PassRefPtr<ExecutablePool> create(size_t n)
    {
        return adoptRef(new ExecutablePool(n));
    }

    void* alloc(size_t n)
    {
        ASSERT(m_freePtr <= m_end);

        // Round 'n' up to a multiple of word size; if all allocations are of
        // word sized quantities, then all subsequent allocations will be aligned.
        n = roundUpAllocationSize(n, sizeof(void*));

        if (static_cast<ptrdiff_t>(n) < (m_end - m_freePtr)) {
            void* result = m_freePtr;
            m_freePtr += n;
            return result;
        }

        // Insufficient space to allocate in the existing pool
        // so we need allocate into a new pool
        return poolAllocate(n);
    }
    
    ~ExecutablePool()
    {
        AllocationList::const_iterator end = m_pools.end();
        for (AllocationList::const_iterator ptr = m_pools.begin(); ptr != end; ++ptr)
            ExecutablePool::systemRelease(*ptr);
    }

    size_t available() const { return (m_pools.size() > 1) ? 0 : m_end - m_freePtr; }

private:
    static Allocation systemAlloc(size_t n);
    static void systemRelease(const Allocation& alloc);

    ExecutablePool(size_t n);

    void* poolAllocate(size_t n);

    char* m_freePtr;
    char* m_end;
    AllocationList m_pools;
};

class ExecutableAllocator {
    enum ProtectionSeting { Writable, Executable };

public:
    static size_t pageSize;
    ExecutableAllocator()
    {
        if (!pageSize)
            intializePageSize();
        m_smallAllocationPool = ExecutablePool::create(JIT_ALLOCATOR_LARGE_ALLOC_SIZE);
    }

    PassRefPtr<ExecutablePool> poolForSize(size_t n)
    {
        // Try to fit in the existing small allocator
        if (n < m_smallAllocationPool->available())
            return m_smallAllocationPool;

        // If the request is large, we just provide a unshared allocator
        if (n > JIT_ALLOCATOR_LARGE_ALLOC_SIZE)
            return ExecutablePool::create(n);

        // Create a new allocator
        RefPtr<ExecutablePool> pool = ExecutablePool::create(JIT_ALLOCATOR_LARGE_ALLOC_SIZE);

        // If the new allocator will result in more free space than in
        // the current small allocator, then we will use it instead
        if ((pool->available() - n) > m_smallAllocationPool->available())
            m_smallAllocationPool = pool;
        return pool.release();
    }

#if ENABLE(ASSEMBLER_WX_EXCLUSIVE)
    static void makeWritable(void* start, size_t size)
    {
        reprotectRegion(start, size, Writable);
    }

    static void makeExecutable(void* start, size_t size)
    {
        reprotectRegion(start, size, Executable);
    }
#else
    static void makeWritable(void*, size_t) {}
    static void makeExecutable(void*, size_t) {}
#endif


#if PLATFORM(X86) || PLATFORM(X86_64)
    static void cacheFlush(void*, size_t)
    {
    }
#elif PLATFORM(ARM_THUMB2) && PLATFORM(IPHONE)
    static void cacheFlush(void* code, size_t size)
    {
        sys_dcache_flush(code, size);
        sys_icache_invalidate(code, size);
    }
#elif PLATFORM(SYMBIAN)
    static void cacheFlush(void* code, size_t size)
    {
        User::IMB_Range(code, static_cast<char*>(code) + size);
    }
#elif PLATFORM(ARM_TRADITIONAL) && PLATFORM(LINUX)
    static void cacheFlush(void* code, size_t size)
    {
        asm volatile (
            "push    {r7}\n"
            "mov     r0, %0\n"
            "mov     r1, %1\n"
            "mov     r7, #0xf0000\n"
            "add     r7, r7, #0x2\n"
            "mov     r2, #0x0\n"
            "svc     0x0\n"
            "pop     {r7}\n"
            :
            : "r" (code), "r" (reinterpret_cast<char*>(code) + size)
            : "r0", "r1");
    }
#else
    #error "The cacheFlush support is missing on this platform."
#endif

private:

#if ENABLE(ASSEMBLER_WX_EXCLUSIVE)
    static void reprotectRegion(void*, size_t, ProtectionSeting);
#endif

    RefPtr<ExecutablePool> m_smallAllocationPool;
    static void intializePageSize();
};

inline ExecutablePool::ExecutablePool(size_t n)
{
    size_t allocSize = roundUpAllocationSize(n, JIT_ALLOCATOR_PAGE_SIZE);
    Allocation mem = systemAlloc(allocSize);
    m_pools.append(mem);
    m_freePtr = mem.pages;
    if (!m_freePtr)
        CRASH(); // Failed to allocate
    m_end = m_freePtr + allocSize;
}

inline void* ExecutablePool::poolAllocate(size_t n)
{
    size_t allocSize = roundUpAllocationSize(n, JIT_ALLOCATOR_PAGE_SIZE);
    
    Allocation result = systemAlloc(allocSize);
    if (!result.pages)
        CRASH(); // Failed to allocate
    
    ASSERT(m_end >= m_freePtr);
    if ((allocSize - n) > static_cast<size_t>(m_end - m_freePtr)) {
        // Replace allocation pool
        m_freePtr = result.pages + n;
        m_end = result.pages + allocSize;
    }

    m_pools.append(result);
    return result.pages;
}

}

#endif // ENABLE(ASSEMBLER)

#endif // !defined(ExecutableAllocator)
