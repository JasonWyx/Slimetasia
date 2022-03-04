#include "GameObject.h"

#include "AudioEmitter.h"
#include "AudioListener.h"
#include "CollisionMesh_3D.h"
#include "IComponent.h"
#include "MeshAnimator.h"
#include "MeshRenderer.h"
#include "ParticleSystem.h"
#include "ResourceManager.h"

GameObject::GameObject(Layer* parentLayer, unsigned id)
    : m_Name()
    , m_Id(id)
    , m_IsActive(false)
    , m_IsActiveToggled(false)
    , m_IsInitialized(false)
    , m_IsFlaggedForDestroy(false)
    , m_ParentLayer(parentLayer)
    , m_ParentObject(0)
    , m_ChildrenObjects()
    , m_ActiveComponents()
    , m_UnwantedComponents()
    , m_NextObj(nullptr)
    , m_IsChildren(false)
{
}

GameObject::~GameObject()
{
    ClearComponents();
}

void GameObject::ClearComponents()
{
    for (IComponent* component : m_ActiveComponents)
    {
        component->OnInactive();
        delete component;
    }

    for (IComponent* component : m_UnwantedComponents)
    {
        component->OnInactive();
        delete component;
    }

    m_ActiveComponents.clear();
    m_UnwantedComponents.clear();
}

void GameObject::CloneParentComponents(unsigned char* curr, unsigned char* rhs, std::string comp)
{
    if (!Factory::m_Reflection->at(comp)->getParents().back().get_parents().empty()) CloneParentComponents(curr, rhs, Factory::m_Reflection->at(comp)->getParents().back().key);
    auto properties = Factory::m_Reflection->at(comp)->getParents().back().get_properties();
    for (auto& property : properties)
    {
        if (property.type == typeid(std::string).name())
        {
            std::string s = reinterpret_cast<std::string*>(curr + property.offset)->c_str();
            *reinterpret_cast<std::string*>(rhs + property.offset) = s;
        }
        else if (property.type.find("ptr") != std::string::npos)
            continue;
        else if (property.type.find("std::") != std::string::npos)
            continue;
        else if (property.type.find("*") != std::string::npos)
            continue;
        else
        {
            if ((property.type.find("struct") != std::string::npos || property.type.find("class") != std::string::npos) && property.type.find("std::") == std::string::npos)
            {
                try
                {
                    CloneStructComponents(curr + property.offset, rhs + property.offset, property.type);
                }
                catch (...)
                {
                    // std::cout << property.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                    for (unsigned i = 0; i < property.size; ++i)
                        *(rhs + property.offset + i) = *(curr + property.offset + i);
                }
            }
            else
            {
                for (unsigned i = 0; i < property.size; ++i)
                    *(rhs + property.offset + i) = *(curr + property.offset + i);
            }
        }
    }
}

void GameObject::CloneStructComponents(unsigned char* curr, unsigned char* rhs, std::string prop)
{
    if (!Factory::m_Reflection->at(prop)->getParents().back().get_parents().empty()) CloneParentComponents(curr, rhs, Factory::m_Reflection->at(prop)->getParents().back().key);
    auto properties = Factory::m_Reflection->at(prop)->getProperties();
    for (auto& property : properties)
    {
        if (property.type == typeid(std::string).name())
        {
            std::string s = reinterpret_cast<std::string*>(curr + property.offset)->c_str();
            *reinterpret_cast<std::string*>(rhs + property.offset) = s;
        }
        else if (property.type.find("ptr") != std::string::npos)
            continue;
        else if (property.type.find("std::") != std::string::npos)
            continue;
        else if (property.type.find("*") != std::string::npos)
            continue;
        else
        {
            if ((property.type.find("struct") != std::string::npos || property.type.find("class") != std::string::npos) && property.type.find("std::") == std::string::npos)
            {
                try
                {
                    CloneStructComponents(curr + property.offset, rhs + property.offset, property.type);
                }
                catch (...)
                {
                    // std::cout << property.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                    for (unsigned i = 0; i < property.size; ++i)
                        *(rhs + property.offset + i) = *(curr + property.offset + i);
                }
            }
            else
            {
                for (unsigned i = 0; i < property.size; ++i)
                    *(rhs + property.offset + i) = *(curr + property.offset + i);
            }
        }
    }
}

void GameObject::ClearComponentsXceptTransform()
{
    while (true)
    {
        for (IComponent* component : m_ActiveComponents)
        {
            if (component->GetName() != "Transform")
            {
                component->OnInactive();
                delete component;
                m_ActiveComponents.remove(component);
                break;
            }
        }
        if (m_ActiveComponents.size() == 0) break;
        if (m_ActiveComponents.size() == 1 && m_ActiveComponents.front()->GetName() == "Transform") break;
    }

    for (IComponent* component : m_UnwantedComponents)
    {
        /* if(component->m_Registered)
           component->Unregister();*/
        component->OnInactive();
        delete component;
    }

    m_UnwantedComponents.clear();
}

void GameObject::Update(float dt)
{
    for (auto& Comp : m_ActiveComponents)
        Comp->OnUpdate(dt);
}

void GameObject::PostFrameUpdate()
{
    // Clear components flagged to be destroyed in this frame
    auto start = m_UnwantedComponents.begin();
    while (start != m_UnwantedComponents.end())
    {
        delete *start;
        ++start;
    }

    m_UnwantedComponents.clear();

    if (m_IsActiveToggled)
    {
        m_IsActiveToggled = false;
        m_IsActive = !m_IsActive;

        for (IComponent* component : m_ActiveComponents)
        {
            // Call appropriate toggle function
            if (m_IsActive)
                component->OnActive();
            else
                component->OnInactive();
        }
    }
}

void GameObject::Clone(GameObject* rhs)
{
    // may need the parent and struct thing like the serializer
    // TODO: Clone rhs to current Object
    rhs->m_Tag = m_Tag;
    rhs->m_ParentObject = m_ParentObject;
    rhs->m_IsChildren = m_IsChildren;
    if (rhs->GetParentLayer() && rhs->m_IsChildren)
    {
        rhs->GetParentLayer()->GetObjectById(rhs->m_ParentObject)->AttachChild(rhs->GetID());
    }
    rhs->ClearComponents();
    for (auto& component : m_ActiveComponents)
    {
        unsigned char* currAddr = reinterpret_cast<unsigned char*>(component);
        unsigned char* compAddr = reinterpret_cast<unsigned char*>(Factory::m_Factories->at(component->GetName())->create(rhs));
        if (component->GetName() == "BoxParticleEmitter" || component->GetName() == "CircleParticleEmitter")
        {
            IComponent* com = reinterpret_cast<IComponent*>(compAddr);
            dynamic_cast<ParticleEmitter*>(com)->m_Attractors = dynamic_cast<ParticleEmitter*>(component)->m_Attractors;
        }
        if (!Factory::m_Reflection->at(component->GetName())->getParents().empty()) CloneParentComponents(currAddr, compAddr, component->GetName());
        auto properties = Factory::m_Reflection->at(component->GetName())->getProperties();
        for (auto& property : properties)
        {
            if (property.type == typeid(std::string).name())
            {
                std::string s = reinterpret_cast<std::string*>(currAddr + property.offset)->c_str();
                *reinterpret_cast<std::string*>(compAddr + property.offset) = s;
            }
            else if (property.type.find("ptr") != std::string::npos)
                continue;
            else if (property.type.find("std::") != std::string::npos)
                continue;
            else if (property.type.find("*") != std::string::npos)
                continue;
            else
            {
                if ((property.type.find("struct") != std::string::npos || property.type.find("class") != std::string::npos) && property.type.find("std::") == std::string::npos)
                {
                    try
                    {
                        CloneStructComponents(currAddr + property.offset, compAddr + property.offset, property.type);
                    }
                    catch (...)
                    {
                        // std::cout << property.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                        for (unsigned i = 0; i < property.size; ++i)
                            *(compAddr + property.offset + i) = *(currAddr + property.offset + i);
                    }
                }
                else
                {
                    for (unsigned i = 0; i < property.size; ++i)
                        *(compAddr + property.offset + i) = *(currAddr + property.offset + i);
                }
            }
        }
    }
    rhs->UpdateComponents();
}

void GameObject::ArchetypeClone(GameObject* rhs)
{
    rhs->m_Tag = m_Tag;
    rhs->m_ParentObject = m_ParentObject;
    rhs->m_IsChildren = m_IsChildren;
    if (rhs->GetParentLayer() && rhs->m_IsChildren)
    {
        rhs->GetParentLayer()->GetObjectById(rhs->m_ParentObject)->AttachChild(rhs->GetID());
    }
    rhs->ClearComponentsXceptTransform();
    for (auto& component : m_ActiveComponents)
    {
        if (component->GetName() == "Transform") continue;
        unsigned char* currAddr = reinterpret_cast<unsigned char*>(component);
        unsigned char* compAddr = reinterpret_cast<unsigned char*>(Factory::m_Factories->at(component->GetName())->create(rhs));
        if (!Factory::m_Reflection->at(component->GetName())->getParents().empty()) CloneParentComponents(currAddr, compAddr, component->GetName());
        auto properties = Factory::m_Reflection->at(component->GetName())->getProperties();
        for (auto& property : properties)
        {
            if (property.type == typeid(std::string).name())
            {
                std::string s = reinterpret_cast<std::string*>(currAddr + property.offset)->c_str();
                *reinterpret_cast<std::string*>(compAddr + property.offset) = s;
            }
            else if (property.type.find("ptr") != std::string::npos)
                continue;
            else if (property.type.find("std::") != std::string::npos)
                continue;
            else if (property.type.find("*") != std::string::npos)
                continue;
            else
            {
                if ((property.type.find("struct") != std::string::npos || property.type.find("class") != std::string::npos) && property.type.find("std::") == std::string::npos)
                {
                    try
                    {
                        CloneStructComponents(currAddr + property.offset, compAddr + property.offset, property.type);
                    }
                    catch (...)
                    {
                        // std::cout << property.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                        for (unsigned i = 0; i < property.size; ++i)
                            *(compAddr + property.offset + i) = *(currAddr + property.offset + i);
                    }
                }
                else
                {
                    for (unsigned i = 0; i < property.size; ++i)
                        *(compAddr + property.offset + i) = *(currAddr + property.offset + i);
                }
            }
        }
    }
    rhs->UpdateComponents();
}

void GameObject::Destroy()
{
    m_IsFlaggedForDestroy = true;
    DeleteAllChildren(this);
}

std::string const& GameObject::GetName() const
{
    return m_Name;
}

const unsigned& GameObject::GetID() const
{
    return m_Id;
}

void GameObject::SetActive(bool active)
{
    m_IsActiveToggled = m_IsActiveToggled ? true : (m_IsActive == active ? false : true);
}

void GameObject::AttachChild(unsigned childObject)
{
    for (unsigned& _childObject : m_ChildrenObjects)
    {
        if (_childObject == childObject) return;
    }
    m_ChildrenObjects.push_back(childObject);
}

void GameObject::RemoveChild(unsigned childObject)
{
    m_ChildrenObjects.remove(childObject);
}

Layer* GameObject::GetParentLayer() const
{
    return m_ParentLayer;
}

unsigned GameObject::GetParentObject() const
{
    return m_ParentObject;
}

unsigned GameObject::GetChildObjectByName(std::string const& objectName) const
{
    for (unsigned childObject : m_ChildrenObjects)
    {
        if (GameObject* child = m_ParentLayer->GetObjectById(childObject))
        {
            if (child->m_Name == objectName)
            {
                return childObject;
            }
        }
    }
    return 0;
}

unsigned GameObject::GetChildObjectByNameCascade(std::string const& objectName) const
{
    for (unsigned childObject : m_ChildrenObjects)
    {
        if (GameObject* child = m_ParentLayer->GetObjectById(childObject))
        {
            if (child->m_Name == objectName)
            {
                return childObject;
            }
        }
    }

    for (unsigned childObject : m_ChildrenObjects)
    {
        unsigned result = m_ParentLayer->GetObjectById(childObject)->GetChildObjectByNameCascade(objectName);
        if (result)
        {
            return result;
        }
    }
    return 0;
}

ChildrenList GameObject::GetChildObjectsByName(std::string const& objectName) const
{
    ChildrenList childObjects;
    for (unsigned childObject : m_ChildrenObjects)
    {
        if (GameObject* child = m_ParentLayer->GetObjectById(childObject))
        {
            if (child->m_Name == objectName) childObjects.push_back(childObject);
        }
    }
    return childObjects;
}

ChildrenList GameObject::GetChildrenObjects() const
{
    return m_ChildrenObjects;
}

IComponentList GameObject::GetComponentList()
{
    return m_ActiveComponents;
}

bool GameObject::GetActive() const
{
    return m_IsActive;
}

void GameObject::UpdateComponents()
{
    for (auto& activeComp : m_ActiveComponents)
    {
        if (LuaScript* comp = dynamic_cast<LuaScript*>(activeComp))
        {
            comp->InitScript();
        }
        if (MeshRenderer* mesh = dynamic_cast<MeshRenderer*>(activeComp))
        {
            mesh->RevalidateResources();
        }
        // if (CuboidColliderMesh * cube = dynamic_cast<CuboidColliderMesh *>(activeComp))
        //{
        //  cube->ComputeRadius();
        //}
        if (AudioEmitter* comp = dynamic_cast<AudioEmitter*>(activeComp))
        {
            comp->SetAudioClip(comp->GetSoundName());
        }
        if (MeshAnimator* meshAnimator = dynamic_cast<MeshAnimator*>(activeComp))
        {
            meshAnimator->RevalidateResources();
            // meshAnimator->InitMeshAnimator();
        }
        if (Camera* camera = dynamic_cast<Camera*>(activeComp))
        {
            camera->RevalidateResources();
        }
        if (AudioListener* comp = dynamic_cast<AudioListener*>(activeComp))
        {
            comp->MakeMain(comp->IsMain());
        }
        if (ParticleEmitter* pe = dynamic_cast<ParticleEmitter*>(activeComp))
        {
            pe->RevalidateResources();
        }
        if (RigidbodyComponent* rc = dynamic_cast<RigidbodyComponent*>(activeComp))
        {
            rc->Initialize();
        }
    }
}

void GameObject::DeleteAllChildren(GameObject* go)
{
    for (auto& id : go->GetChildrenObjects())
    {
        auto tmp = go->GetParentLayer()->GetObjectById(id);
        tmp->Destroy();
        DeleteAllChildren(tmp);
    }
}
