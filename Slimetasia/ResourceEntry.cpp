#include "ResourceEntry.h"

#include "ResourceHandle.h"

void ResourceEntry::InsertHandle(ResourceHandleBase* const handle)
{
    m_Handles.insert(handle);
}

void ResourceEntry::RemoveHandle(ResourceHandleBase* const handle)
{
    m_Handles.erase(handle);
}

void ResourceEntry::InvalidateHandles()
{
    for (ResourceHandleBase* handle : m_Handles)
    {
        handle->Invalidate();
    }
}

void ResourceEntry::ClearHandles()
{
    InvalidateHandles();
    m_Handles.clear();
}
