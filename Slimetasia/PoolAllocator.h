#pragma once
#include "MemoryAllocator.h"
#include "Utility.h"

class PoolAllocator : public MemoryAllocator
{
public:

    PoolAllocator();
    ~PoolAllocator() override;

    PoolAllocator& operator=(const PoolAllocator& rhs) = default;

    // to-do. Add move constructor and assignment.

    // allocates memory in bytes and returns a pointer to the allocated memory.
    // uses the default malloc.
    void* Allocate(const size_t& size) override;
    // frees the allocated memory in the pointer.
    // uses the default free.
    void ReleaseMemory(void* ptr, const size_t& size) override;

private:

    struct MemoryUnit
    {
        MemoryUnit* m_Next;
    };

    struct MemoryBlock
    {
        MemoryUnit* m_MemUnit;
    };

    static const int MEM_HEAPS = 128;
    static const size_t MAX_UNIT_SIZE = 1024u;
    static const size_t BLOCK_SIZE = 16u * MAX_UNIT_SIZE;

    static size_t m_UnitSizes[MEM_HEAPS];
    static int m_MapSizeToHeapIndex[MAX_UNIT_SIZE + 1];
    static bool m_IsMapSizeToHeapIndexInitialized;

    MemoryUnit* m_FreeMemUnits[MEM_HEAPS];
    MemoryBlock* m_MemBlocks;
    uint m_AllocatedMemBlocks;
    uint m_CurrentMemBlocks;
};