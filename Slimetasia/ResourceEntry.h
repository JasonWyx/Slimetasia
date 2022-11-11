#pragma once
#include <set>

class ResourceHandleBase;
class ResourceBase;

class ResourceEntry
{
public:

    ResourceEntry()
        : m_Entry(nullptr)
    {
    }

    void InsertHandle(ResourceHandleBase* const handle);
    void RemoveHandle(ResourceHandleBase* const handle);
    void InvalidateHandles();
    void ClearHandles();

private:

    ResourceBase* m_Entry;
    std::set<ResourceHandleBase*> m_Handles;

    friend class ResourceHandleBase;
    friend class ResourceManager;
};
