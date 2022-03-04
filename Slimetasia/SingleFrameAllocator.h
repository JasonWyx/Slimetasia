#pragma once
#include "MemoryAllocator.h"
#include "Utility.h"

class SingleFrameAllocator : public MemoryAllocator
{
public:
    SingleFrameAllocator();

    ~SingleFrameAllocator() override;

    SingleFrameAllocator(const SingleFrameAllocator& allocator) = default;

    SingleFrameAllocator(SingleFrameAllocator&& allocator) = default;

    SingleFrameAllocator& operator=(const SingleFrameAllocator& allocator) = default;

    SingleFrameAllocator& operator=(SingleFrameAllocator&& allocator) = default;

    // allocates memory in bytes and returns a pointer to the allocated memory.
    // uses the default malloc.
    void* Allocate(const size_t& size) override;

    // frees the allocated memory in the pointer.
    // uses the default free.
    void ReleaseMemory(void* ptr, const size_t& size) override;

    virtual void ResetMemory();

private:
    // frames to wait before shrinking the allocated memory if it is overallocated.
    static const uint FRAMES_UNTIL_SHRINK = 120u;

    // the initial size (in bytes) of the single frame allocator.
    size_t INIT_SINGLE_FRAME_ALLOCATOR_BYTES = 1048576;  // 1Mb

    // total size of the memory(in bytes) of the allocator.
    size_t m_TotalSize;

    // the start of the allocated memory block.
    char* m_Start;

    // the next avaliable mem location.
    size_t m_CurrOffset;

    // number of frames since we detected that too much memory was allocated.
    uint m_FramesTooMuchAllocated;

    // true if we need to allocate more memory.
    bool m_NeedToAllocate;
};
