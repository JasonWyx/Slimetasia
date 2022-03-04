#include "IComponent.h"

#include "GameObject.h"

IComponent::IComponent(GameObject* parentObject, std::string const& componentName)
    : m_OwnerObject(parentObject)
    , m_Name(componentName)
{
}

GameObject* IComponent::GetOwner()
{
    return m_OwnerObject;
}

std::string IComponent::GetName()
{
    return m_Name;
}

REFLECT_VIRTUAL(IComponent)
REFLECT_PROPERTY(m_Name)
REFLECT_PROPERTY(m_OwnerObject)
REFLECT_END()
