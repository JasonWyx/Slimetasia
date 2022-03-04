#pragma once

// abstract class, to be inherited from.
struct MemoryAllocator
{
    // default ctor
    MemoryAllocator() = default;

    // default dtor
    virtual ~MemoryAllocator() = default;

    // pure virtual function
    // allocates memory in bytes and returns a pointer to the allocated memory.
    virtual void* Allocate(const size_t& size) = 0;

    // frees the allocated memory in the pointer.
    virtual void ReleaseMemory(void* ptr, const size_t& size) = 0;
};