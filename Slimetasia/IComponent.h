#pragma once
#include <string>

#include "Factory.h"
#include "Reflection.h"
// #include "CorePrerequisites.h"

class GameObject;

class IComponent
{
    friend class GameObject;

protected:

    std::string m_Name;
    GameObject* m_OwnerObject;

public:

    IComponent(GameObject* parentObject, std::string const& componentName);
    virtual ~IComponent() = default;

    virtual void OnActive() {}
    virtual void OnInactive() {}
    virtual void OnUpdate(float dt) {}
    virtual void Register() {}
    virtual void RevalidateResources() {};

    GameObject* GetOwner();
    std::string GetName();

    REFLECT()
};
