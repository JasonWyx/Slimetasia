#pragma once
#include <cstdlib>

#include "MemoryAllocator.h"

// A basic memory allocator using default malloc and free.
struct DefaultMemAllocator : MemoryAllocator
{
    // default ctor
    DefaultMemAllocator() = default;

    // default dtor, overrides the dtor in MemoryAllocator.
    ~DefaultMemAllocator() override = default;

    // allocates memory in bytes and returns a pointer to the allocated memory.
    // uses the default malloc.
    void* Allocate(const size_t& size) override;

    // frees the allocated memory in the pointer.
    // uses the default free.
    void ReleaseMemory(void* ptr, const size_t& size) override;
};

inline void* DefaultMemAllocator::Allocate(const size_t& size)
{
    return malloc(size);
}

inline void DefaultMemAllocator::ReleaseMemory(void* ptr, const size_t& size)
{
    free(ptr);
}