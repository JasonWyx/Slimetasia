#pragma once
#include <string>

#include "CorePrerequisites.h"
#include "luascript.h"

class IComponent;
class Layer;

using GameObjectList = std::list<GameObject*>;
using ChildrenList = std::list<unsigned>;

class GameObject
{
    friend class Layer;

    std::string m_Name;
    std::string m_Archetype;
    std::string m_Tag;
    unsigned m_Id;
    bool m_IsActive;
    bool m_IsActiveToggled;
    bool m_IsInitialized;
    bool m_IsFlaggedForDestroy;
    bool m_IsChildren;
    Layer* m_ParentLayer;
    unsigned m_ParentObject;
    ChildrenList m_ChildrenObjects;
    IComponentList m_ActiveComponents;
    IComponentList m_UnwantedComponents;

    void ClearComponents();
    void CloneParentComponents(unsigned char* curr, unsigned char* rhs, std::string comp);
    void CloneStructComponents(unsigned char* curr, unsigned char* rhs, std::string prop);
    void ClearComponentsXceptTransform();

public:
    // For storing a list of objects in the same node for octree
    GameObject* m_NextObj;

    GameObject(Layer* parentLayer, unsigned id);
    ~GameObject();

    void Update(float dt);
    void PostFrameUpdate();       // Any other updates done post frame
    void Clone(GameObject* rhs);  // Clone Object
    void ArchetypeClone(GameObject* rhs);
    void Destroy();  // Flags object for destruction

    std::string const& GetName() const;
    const unsigned& GetID() const;
    bool GetIsChildren() const { return m_IsChildren; }
    void SetIsChildren(bool b) { m_IsChildren = b; }
    void SetActive(bool active);
    void SetName(std::string n) { m_Name = n; }
    void SetID(unsigned id) { m_Id = id; }
    void SetArchetype(const std::string& a) { m_Archetype = a; }
    const std::string& GetArchetype() const { return m_Archetype; }
    const std::string& GetTag() const { return m_Tag; }
    void SetTag(std::string t) { m_Tag = t; }
    void SetParentObject(unsigned p) { m_ParentObject = p; }
    void AttachChild(unsigned childObject);
    void RemoveChild(unsigned childObject);
    Layer* GetParentLayer() const;
    unsigned GetParentObject() const;
    unsigned GetChildObjectByName(std::string const& objectName) const;
    unsigned GetChildObjectByNameCascade(std::string const& objectName) const;
    ChildrenList GetChildObjectsByName(std::string const& objectName) const;
    ChildrenList GetChildrenObjects() const;
    IComponentList GetComponentList();
    bool GetActive() const;
    void SetChildrenObjects(ChildrenList list) { m_ChildrenObjects = list; }

    void UpdateComponents();

    void DeleteAllChildren(GameObject* go);

    template <typename ComponentType> ComponentType* AddComponent()
    {
        // TODO: Do check for scripts
        for (IComponent* component : m_ActiveComponents)
        {
            // Component already exists
            if (ComponentType* dynamicType = dynamic_cast<ComponentType*>(component))
            {
                if (component->GetName() != "LuaScript")
                {
                    ASSERT(true);
                }
            }
        }
        ComponentType* newComponent = new ComponentType(this);
        if (m_IsActive && m_ParentLayer) newComponent->OnActive();
        m_ActiveComponents.push_back(newComponent);
        return newComponent;
    }

    // used for dependancy checks. Adds the component if the component specified is not found.
    template <typename ComponentType> ComponentType* AddIfDoesntExist()
    {
        for (IComponent* component : m_ActiveComponents)
        {
            if (ComponentType* dynamicType = dynamic_cast<ComponentType*>(component))
            {
                return dynamicType;
            }
        }
        return AddComponent<ComponentType>();
    }

    template <typename ComponentType> ComponentType* GetComponent() const
    {
        for (IComponent* component : m_ActiveComponents)
        {
            if (ComponentType* dynamicType = dynamic_cast<ComponentType*>(component)) return dynamicType;
        }
        return nullptr;
    }

    template <typename ComponentType> void RemoveComponent()
    {
        for (IComponent* component : m_ActiveComponents)
        {
            if (ComponentType* dynamicType = dynamic_cast<ComponentType*>(component))
            {
                dynamicType->OnInactive();
                m_UnwantedComponents.push_back(dynamicType);
                m_ActiveComponents.remove(component);
                return;
            }
        }
    }

    template <typename ComponentType> std::vector<ComponentType*> GetAllComponentWithThisBase() const
    {
        std::vector<ComponentType*> vec;

        for (IComponent* component : m_ActiveComponents)
        {
            if (ComponentType* dynamicType = dynamic_cast<ComponentType*>(component)) vec.push_back(dynamicType);
        }

        return vec;
    }

    LuaScript* GetScript(const std::string& script_name)
    {
        for (IComponent* component : m_ActiveComponents)
        {
            if (LuaScript* dynamicType = dynamic_cast<LuaScript*>(component))
            {
                if (dynamicType->GetScript() == script_name)
                    return dynamicType;
                else
                    continue;
            }
        }
        return nullptr;
    }

    void RemoveScript(const std::string& script_name)
    {
        for (IComponent* component : m_ActiveComponents)
        {
            if (LuaScript* dynamicType = dynamic_cast<LuaScript*>(component))
            {
                if (dynamicType->GetScript() == script_name)
                {
                    m_UnwantedComponents.push_back(dynamicType);
                    m_ActiveComponents.remove(component);
                    return;
                }
                else
                    continue;
            }
        }
    }

    std::vector<std::string> GetScripts()
    {
        std::vector<std::string> scripts;
        for (IComponent* component : m_ActiveComponents)
        {
            if (LuaScript* dynamicType = dynamic_cast<LuaScript*>(component))
            {
                scripts.push_back(dynamicType->GetScript());
            }
        }
        return scripts;
    }

    std::vector<LuaScript*> GetLuaScripts()
    {
        std::vector<LuaScript*> scripts;

        for (IComponent* component : m_ActiveComponents)
            if (LuaScript* dynamicType = dynamic_cast<LuaScript*>(component)) scripts.push_back(dynamicType);

        return scripts;
    }
};
