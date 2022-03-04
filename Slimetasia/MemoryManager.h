#pragma once
#include "DefaultMemAllocator.h"
#include "PoolAllocator.h"
#include "SingleFrameAllocator.h"

struct MemoryAllocator;

class MemoryManager
{
public:
    // using an enum class instead of a normal enum.
    // ensures that the values do not convert to any other types.
    enum class AllocationType
    {
        Base = 0,  // Base memory allocator.
        Pool,      // Memory pool allocator.
        Frame      // Single Frame memory allocator.
    };

    // uses the default ctor.
    MemoryManager() = default;

    // uses the default dtor.
    virtual ~MemoryManager() = default;

    // getters
    PoolAllocator& GetPoolAllocator() { return m_PoolMemAllocator; }
    SingleFrameAllocator& GetSingleFrameAllocator() { return m_SingleFrameMemAllocator; }
    static MemoryAllocator& GetBaseAllocator() { return *m_BaseMemAllocator; }

    // setters
    void SetBaseAllocator(MemoryAllocator* allocator) { m_BaseMemAllocator = allocator; }

    // funcs

    void* Allocate(AllocationType allocatetype, const size_t& size);
    void ReleaseMemory(AllocationType allocatetype, void* ptr, const size_t size);
    void ResetFrameAllocator();

    static MemoryAllocator* m_BaseMemAllocator;

private:
    // default memory allocator
    static DefaultMemAllocator m_DefaultAllocator;
    PoolAllocator m_PoolMemAllocator;
    SingleFrameAllocator m_SingleFrameMemAllocator;
};

inline void* MemoryManager::Allocate(AllocationType allocatetype, const size_t& size)
{
    switch (allocatetype)
    {
        case AllocationType::Base: return m_BaseMemAllocator->Allocate(size);

        case AllocationType::Frame: return m_SingleFrameMemAllocator.Allocate(size);

        case AllocationType::Pool: return m_PoolMemAllocator.Allocate(size);

        default: return nullptr;
    }
}

inline void MemoryManager::ReleaseMemory(AllocationType allocatetype, void* ptr, const size_t size)
{
    switch (allocatetype)
    {
        case AllocationType::Base: m_BaseMemAllocator->ReleaseMemory(ptr, size); break;
        case AllocationType::Frame: m_SingleFrameMemAllocator.ReleaseMemory(ptr, size); break;
        case AllocationType::Pool: m_PoolMemAllocator.ReleaseMemory(ptr, size); break;
    }
}

inline void MemoryManager::ResetFrameAllocator()
{
    m_SingleFrameMemAllocator.ResetMemory();
}
