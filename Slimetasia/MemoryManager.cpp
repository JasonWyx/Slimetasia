#include "MemoryManager.h"

#include "DefaultMemAllocator.h"

DefaultMemAllocator MemoryManager::m_DefaultAllocator = DefaultMemAllocator();
MemoryAllocator* MemoryManager::m_BaseMemAllocator = &m_DefaultAllocator;
