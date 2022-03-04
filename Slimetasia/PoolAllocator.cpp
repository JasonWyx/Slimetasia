#include "PoolAllocator.h"

#include <cassert>
#include <cstring>

#include "MemoryManager.h"

size_t PoolAllocator::m_UnitSizes[MEM_HEAPS];
int PoolAllocator::m_MapSizeToHeapIndex[MAX_UNIT_SIZE + 1];
bool PoolAllocator::m_IsMapSizeToHeapIndexInitialized = false;

PoolAllocator::PoolAllocator()
    : m_AllocatedMemBlocks(64u)
    , m_CurrentMemBlocks(0u)
{
    const size_t allocatesz = m_AllocatedMemBlocks * sizeof(MemoryBlock);
    // allocates memory to manage the memory blocks.
    m_MemBlocks = static_cast<MemoryBlock*>(MemoryManager::GetBaseAllocator().Allocate(allocatesz));
    // setting all values to 0.
    memset(m_MemBlocks, 0, allocatesz);
    // setting all pointer to nullptrs.
    memset(m_FreeMemUnits, 0, sizeof(m_FreeMemUnits));

    if (!m_IsMapSizeToHeapIndexInitialized)
    {
        for (auto i = 0u; i < MEM_HEAPS; ++i)
            m_UnitSizes[i] = (i + 1ll) * 8ll;

        auto j = 0u;
        // the first element should never be used!
        m_MapSizeToHeapIndex[0] = -1;

        for (auto i = 1u; i <= MAX_UNIT_SIZE; ++i)
        {
            j = i <= m_UnitSizes[j] ? j : j + 1;
            m_MapSizeToHeapIndex[i] = j;
        }

        m_IsMapSizeToHeapIndexInitialized = true;
    }
}

PoolAllocator::~PoolAllocator()
{
    for (auto i = 0u; i < m_CurrentMemBlocks; ++i)
        MemoryManager::GetBaseAllocator().ReleaseMemory(m_MemBlocks[i].m_MemUnit, BLOCK_SIZE);

    MemoryManager::GetBaseAllocator().ReleaseMemory(m_MemBlocks, m_AllocatedMemBlocks * sizeof(MemoryBlock));
}

void* PoolAllocator::Allocate(const size_t& size)
{
    // check for invalid input.
    if (size == 0u) return nullptr;

    // check if size specified is too large
    if (size > MAX_UNIT_SIZE) return MemoryManager::GetBaseAllocator().Allocate(size);

    // Obtains the index of the heap that will perform the allocation request.
    // asserts if the returned heap index is invalid.
    int heapindex = m_MapSizeToHeapIndex[size];
    assert(heapindex >= 0 && heapindex <= MEM_HEAPS);

    // If there are remaining free memory units in the heap pointed to by the heapindex.
    if (m_FreeMemUnits[heapindex])
    {
        MemoryUnit* memunit = m_FreeMemUnits[heapindex];
        // point to the next element.
        m_FreeMemUnits[heapindex] = memunit->m_Next;
        // return the free memory to use.
        return memunit;
    }

    // there are no remaining free mem units in the heap pointed to by the heapindex.

    // if there is a need to grow to contain more memory blocks.
    if (m_CurrentMemBlocks == m_AllocatedMemBlocks)
    {
        // holds the original chunk of allocated memory.
        auto currMemblock = m_MemBlocks;
        // grows by 64 each time.
        m_AllocatedMemBlocks += 64u;
        // allocates a new chunk of memory with the updated size.
        m_MemBlocks = static_cast<MemoryBlock*>(MemoryManager::GetBaseAllocator().Allocate(m_AllocatedMemBlocks * sizeof(MemoryBlock)));
        // copies the data over to the new memory location.
        memcpy(m_MemBlocks, currMemblock, m_CurrentMemBlocks * sizeof(MemoryBlock));
        // sets all the new data to 0.
        memset(m_MemBlocks + m_CurrentMemBlocks, 0, 64 * sizeof(MemoryBlock));
        // releases the old allocated memory.
        MemoryManager::GetBaseAllocator().ReleaseMemory(currMemblock, m_AllocatedMemBlocks - 64u);
    }

    // Allocates a new memory block and divide it into many memory units.
    auto newblock = m_MemBlocks + m_CurrentMemBlocks;
    newblock->m_MemUnit = static_cast<MemoryUnit*>(MemoryManager::GetBaseAllocator().Allocate((BLOCK_SIZE)));
    // does a check for nullptr
    assert(newblock->m_MemUnit);
    const auto unitsize = m_UnitSizes[heapindex];
    auto numunits = BLOCK_SIZE / unitsize;
    // check for out of bounds
    assert(unitsize * numunits <= BLOCK_SIZE);

    auto start = static_cast<void*>(newblock->m_MemUnit);
    auto startchar = static_cast<char*>(start);
    auto max = numunits - 1;
    // manually linking all the elements.
    for (auto i = 0u; i < max; ++i)
    {
        auto unitptr = static_cast<void*>(startchar + unitsize * i);
        auto nextptr = static_cast<void*>(startchar + unitsize * (i + 1ll));
        auto unit = static_cast<MemoryUnit*>(unitptr);
        auto nextunit = static_cast<MemoryUnit*>(nextptr);
        unit->m_Next = nextunit;
    }
    auto lastunitptr = static_cast<void*>(startchar + unitsize * max);
    auto lastunit = static_cast<MemoryUnit*>(lastunitptr);
    // there are no more memoryunits after the last memoryunit.
    lastunit->m_Next = nullptr;

    m_FreeMemUnits[heapindex] = newblock->m_MemUnit->m_Next;
    m_CurrentMemBlocks++;

    return newblock->m_MemUnit;
}

void PoolAllocator::ReleaseMemory(void* ptr, const size_t& size)
{
    // check for invalid size input
    if (size == 0u) return;

    // check if size specified is too large
    if (size > MAX_UNIT_SIZE)
    {
        MemoryManager::GetBaseAllocator().ReleaseMemory(ptr, size);
        return;
    }

    const auto heapindex = m_MapSizeToHeapIndex[size];
    // check if the heapindex is invalid.
    assert(heapindex >= 0u && heapindex < MEM_HEAPS);

    // putting the released unit back into the list.
    auto releasedunit = static_cast<MemoryUnit*>(ptr);
    releasedunit->m_Next = m_FreeMemUnits[heapindex];
    m_FreeMemUnits[heapindex] = releasedunit;
}
