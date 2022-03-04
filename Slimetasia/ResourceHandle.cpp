#include "ResourceHandle.h"

#include <typeinfo>

#include "ResourceEntry.h"
#include "ResourceManager.h"

ResourceHandleBase::ResourceHandleBase()
    : m_ResourceGUID(0)
{
}

ResourceHandleBase::ResourceHandleBase(ResourceEntry* const entry)
    : m_ResourceGUID(entry->m_Entry->GetGUID())
{
}

ResourceHandleBase::~ResourceHandleBase() {}

void ResourceHandleBase::Invalidate()
{
    m_ResourceGUID = 0;
}

bool ResourceHandleBase::Validate()
{
    ResourceBase* resource = GetResource();

    if (!resource)
    {
        m_ResourceGUID = 0;
    }

    return m_ResourceGUID != 0;
}

ResourceHandleBase::operator ResourceGUID()
{
    return m_ResourceGUID;
}

ResourceBase* ResourceHandleBase::GetResource()
{
    return ResourceManager::Instance().GetResourceInstance(*this);
}
