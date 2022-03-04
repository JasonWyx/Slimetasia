#include "SingleFrameAllocator.h"

#include <cassert>

#include "MemoryManager.h"

SingleFrameAllocator::SingleFrameAllocator()
    : m_TotalSize(INIT_SINGLE_FRAME_ALLOCATOR_BYTES)
    , m_CurrOffset(0u)
    , m_FramesTooMuchAllocated(0u)
    , m_NeedToAllocate(false)
{
    m_Start = static_cast<char*>(MemoryManager::GetBaseAllocator().Allocate(m_TotalSize));
    // checks if m_start is nullptr.
    assert(m_Start);
}

SingleFrameAllocator::~SingleFrameAllocator()
{
    MemoryManager::GetBaseAllocator().ReleaseMemory(m_Start, m_TotalSize);
}

void* SingleFrameAllocator::Allocate(const size_t& size)
{
    if (m_CurrOffset + size > m_TotalSize)
    {
        m_NeedToAllocate = true;
        return MemoryManager::GetBaseAllocator().Allocate(size);
    }
    void* nextmem = m_Start + m_CurrOffset;

    m_CurrOffset += size;

    return nextmem;
}

void SingleFrameAllocator::ReleaseMemory(void* ptr, const size_t& size)
{
    auto p = static_cast<char*>(ptr);
    if (p < m_Start || p > m_Start + m_TotalSize) MemoryManager::GetBaseAllocator().ReleaseMemory(ptr, size);
}

void SingleFrameAllocator::ResetMemory()
{
    // if there was too much memory allocated.
    if (m_CurrOffset < m_TotalSize / 2u)
    {
        m_FramesTooMuchAllocated++;
        if (m_FramesTooMuchAllocated > FRAMES_UNTIL_SHRINK)
        {
            MemoryManager::GetBaseAllocator().ReleaseMemory(m_Start, m_TotalSize);

            m_TotalSize /= 2;
            if (m_TotalSize == 0) m_TotalSize = 1;

            m_Start = static_cast<char*>(MemoryManager::GetBaseAllocator().Allocate(m_TotalSize));
            // checks if m_start is nullptr.
            assert(m_Start);

            m_FramesTooMuchAllocated = 0;
        }
    }
    else
        m_FramesTooMuchAllocated = 0;

    if (m_NeedToAllocate)
    {
        MemoryManager::GetBaseAllocator().ReleaseMemory(m_Start, m_TotalSize);

        m_TotalSize *= 2;

        m_Start = static_cast<char*>(MemoryManager::GetBaseAllocator().Allocate(m_TotalSize));
        assert(m_Start);

        m_NeedToAllocate = false;
        m_FramesTooMuchAllocated = 0;
    }

    m_CurrOffset = 0;
}
