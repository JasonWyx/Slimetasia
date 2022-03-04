#pragma once
#include "Logger.h"
#include "ResourceBase.h"
#include "ResourceEntry.h"

class ResourceHandleBase
{
public:
    ResourceHandleBase();  // Create null handle
    explicit ResourceHandleBase(ResourceEntry* const entry);
    ~ResourceHandleBase();

    void Invalidate();
    bool Validate();
    operator ResourceGUID();

protected:  // Members
    unsigned m_Unused;
    ResourceGUID m_ResourceGUID;

protected:  // Functions
    ResourceBase* GetResource();
};

// Templated handles to check validity and handle resource access
template <typename ResourceType> class ResourceHandle : public ResourceHandleBase
{
public:
    ResourceHandle()
        : ResourceHandleBase()
    {
    }
    explicit ResourceHandle(ResourceEntry* const entry)
        : ResourceHandleBase(entry)
    {
    }

    ResourceType* operator->() { return dynamic_cast<ResourceType*>(GetResource()); }

    ResourceType& operator*() { return *(operator->()); }
};
