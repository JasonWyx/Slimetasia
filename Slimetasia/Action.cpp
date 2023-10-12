#include "Action.h"

#include <utility>

#include "Editor.h"
#include "MeshAnimator.h"
#include "MeshRenderer.h"
#include "ParticleSystem.h"
#include "ResourceManager.h"
#include "luascript.h"

void ActionCreateArchetype::Execute()
{
    del = false;
    Editor::Instance().AddArchetype(obj, uobj);
    for (auto& go : a_go)
        go->SetArchetype(obj->GetName());
    a_go.clear();
}

void ActionCreateArchetype::Revert()
{
    del = true;
    Editor::Instance().RemoveArchetype(obj, uobj);
    auto lys = Application::Instance().GetCurrentScene()->GetLayers();
    for (auto& ly : lys)
    {
        auto gos = ly->GetObjectsList();
        for (auto& go : gos)
        {
            if (go->GetArchetype() == obj->GetName())
            {
                a_go.push_back(go);
                go->SetArchetype(std::string {});
            }
        }
    }
}

ActionCreateArchetype::~ActionCreateArchetype()
{
    if (del) delete obj;
}

void ActionAddComponent::Execute()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    void* script = (*Factory::m_Factories).at(comp.c_str())->create(go);
    if (comp == "LuaScript") script_name = reinterpret_cast<LuaScript*>(script)->GetScript();
}

void ActionAddComponent::Revert()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    if (comp == "LuaScript")
        go->RemoveScript(script_name);
    else
        (*Factory::m_Factories).at(comp.c_str())->remove(go);
}

void ActionRemoveComponent::StructRecurrsion(unsigned char* curr, unsigned char* tmp, std::string prop)
{
    if (!Factory::m_Reflection->at(prop)->getParents().back().get_parents().empty()) ParentRecurrsion(curr, tmp, Factory::m_Reflection->at(prop)->getParents().back().key);
    auto properties = Factory::m_Reflection->at(prop)->getProperties();
    for (auto& property : properties)
    {
        if (property.type == typeid(std::string).name())
        {
            std::string s = reinterpret_cast<std::string*>(curr + property.offset)->c_str();
            *reinterpret_cast<std::string*>(tmp + property.offset) = s;
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
                    StructRecurrsion(curr + property.offset, tmp + property.offset, property.type);
                }
                catch (...)
                {
                    std::cout << property.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                    for (unsigned i = 0; i < property.size; ++i)
                        *(curr + property.offset + i) = *(tmp + property.offset + i);
                }
            }
            else
            {
                for (unsigned i = 0; i < property.size; ++i)
                    *(curr + property.offset + i) = *(tmp + property.offset + i);
            }
        }
    }
}

void ActionRemoveComponent::ParentRecurrsion(unsigned char* curr, unsigned char* tmp, std::string comp)
{
    if (!Factory::m_Reflection->at(comp)->getParents().back().get_parents().empty()) ParentRecurrsion(curr, tmp, Factory::m_Reflection->at(comp)->getParents().back().key);
    auto properties = Factory::m_Reflection->at(comp)->getParents().back().get_properties();
    for (auto& property : properties)
    {
        if (property.type == typeid(std::string).name())
        {
            std::string s = reinterpret_cast<std::string*>(curr + property.offset)->c_str();
            *reinterpret_cast<std::string*>(tmp + property.offset) = s;
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
                    StructRecurrsion(curr + property.offset, tmp + property.offset, property.type);
                }
                catch (...)
                {
                    std::cout << property.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                    for (unsigned i = 0; i < property.size; ++i)
                        *(curr + property.offset + i) = *(tmp + property.offset + i);
                }
            }
            else
            {
                for (unsigned i = 0; i < property.size; ++i)
                    *(curr + property.offset + i) = *(tmp + property.offset + i);
            }
        }
    }
}
// May have to do the whole recurrsion shit again
ActionRemoveComponent::ActionRemoveComponent(GameObject* g, std::string c, SceneLayer* l)
    : go(g)
    , comp(c)
    , tmp(nullptr)
    , ly(l)
{
    id = go->GetID();
    name = go->GetName();
    tmp = new GameObject(nullptr, 0);
    go->Clone(tmp);
    l_name = ly->GetName();
}

void ActionRemoveComponent::Execute()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    (*Factory::m_Factories).at(comp.c_str())->remove(go);
}

void ActionRemoveComponent::Revert()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    unsigned char* Char = reinterpret_cast<unsigned char*>((*Factory::m_Factories).at(comp.c_str())->create(go));
    unsigned char* tmpChar = nullptr;
    for (auto& c : tmp->GetComponentList())
        if (c->GetName() == comp) tmpChar = reinterpret_cast<unsigned char*>(c);
    auto properties = (*Factory::m_Reflection).at(comp.c_str())->getProperties();
    if (!Factory::m_Reflection->at(comp.c_str())->getParents().empty()) ParentRecurrsion(Char, tmpChar, comp);
    for (auto prop : properties)
    {
        if (prop.type == typeid(std::string).name())
        {
            std::string s = reinterpret_cast<std::string*>(tmpChar + prop.offset)->c_str();
            *reinterpret_cast<std::string*>(Char + prop.offset) = s;
        }
        else if (prop.type.find("ptr") != std::string::npos)
            continue;
        else if (prop.type.find("std::") != std::string::npos)
            continue;
        else if (prop.type.find("*") != std::string::npos)
            continue;
        else
        {
            if ((prop.type.find("struct") != std::string::npos || prop.type.find("class") != std::string::npos) && prop.type.find("std::") == std::string::npos)
            {
                try
                {
                    StructRecurrsion(Char + prop.offset, tmpChar + prop.offset, prop.type);
                }
                catch (...)
                {
                    std::cout << prop.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                    for (unsigned i = 0; i < prop.size; ++i)
                        *(Char + prop.offset + i) = *(tmpChar + prop.offset + i);
                }
            }
            else
            {
                for (unsigned i = 0; i < prop.size; ++i)
                    *(Char + prop.offset + i) = *(tmpChar + prop.offset + i);
            }
        }
    }
    go->UpdateComponents();
}

ActionRemoveComponent::~ActionRemoveComponent()
{
    delete tmp;
}

void ActionRevertArchetype::Execute()
{
    obj = Editor::Instance().RevertArchetype(obj);
}

void ActionRevertArchetype::Revert()
{
    obj = Editor::Instance().RevertArchetype(obj);
}

ActionRevertArchetype::~ActionRevertArchetype()
{
    delete obj;
}

void ActionMakeChanges::Execute()
{
    obj = Editor::Instance().MakeChangesArchetype(obj);
    GameObject* tmp = Editor::Instance().GetArchetype(obj->GetName());
    // Need to do for all layers
    auto lys = Application::Instance().GetCurrentScene()->GetLayers();
    for (auto& ly : lys)
    {
        auto objs = ly->GetObjectsList();
        for (auto& a : objs)
        {
            if (a->GetArchetype() == obj->GetName())
            {
                auto go = archetypeList.emplace_back(nullptr, 0);
                archetypeObj.push_back(a);
            }
        }
    }
    int i = 0;
    for (auto& a : archetypeObj)
    {
        a->Clone(&archetypeList[i]);
        tmp->ArchetypeClone(a);
        ++i;
        a->UpdateComponents();
    }
}

void ActionMakeChanges::Revert()
{
    obj = Editor::Instance().MakeChangesArchetype(obj);
    int i = 0;
    for (auto& a : archetypeObj)
    {
        archetypeList[i].ArchetypeClone(a);
        ++i;
        a->UpdateComponents();
    }
    archetypeObj.clear();
    archetypeList.clear();
}

ActionMakeChanges::~ActionMakeChanges()
{
    delete obj;
}

void ActionCreateObjectArchetype::Execute()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->CreateObject(name);
    Editor::Instance().GetArchetype(archetype)->Clone(go);
    go->SetArchetype(name);
    id = go->GetID();
}

void ActionCreateObjectArchetype::Revert()
{
    Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->Destroy();
}

void ActionRevertScript::Execute()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    go->RemoveScript(script);
}

void ActionRevertScript::Revert()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    go->AddComponent<LuaScript>()->SetScript(script);
}

void ActionParentStructInput::StructRecurrsion(unsigned char* curr, unsigned char* tmp, std::string prop)
{
    if (!Factory::m_Reflection->at(prop)->getParents().back().get_parents().empty()) ParentRecurrsion(curr, tmp, Factory::m_Reflection->at(prop)->getParents().back().key);
    auto properties = Factory::m_Reflection->at(prop)->getProperties();
    for (auto& property : properties)
    {
        if (property.type == typeid(std::string).name())
        {
            std::string s = reinterpret_cast<std::string*>(curr + property.offset)->c_str();
            *reinterpret_cast<std::string*>(tmp + property.offset) = s;
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
                    StructRecurrsion(curr + property.offset, tmp + property.offset, property.type);
                }
                catch (...)
                {
                    std::cout << property.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                    for (unsigned i = 0; i < property.size; ++i)
                        *(curr + property.offset + i) = *(tmp + property.offset + i);
                }
            }
            else
            {
                for (unsigned i = 0; i < property.size; ++i)
                    *(curr + property.offset + i) = *(tmp + property.offset + i);
            }
        }
    }
}

void ActionParentStructInput::ParentRecurrsion(unsigned char* curr, unsigned char* tmp, std::string comp)
{
    if (!Factory::m_Reflection->at(comp)->getParents().back().get_parents().empty()) ParentRecurrsion(curr, tmp, Factory::m_Reflection->at(comp)->getParents().back().key);
    auto properties = Factory::m_Reflection->at(comp)->getParents().back().get_properties();
    for (auto& property : properties)
    {
        if (property.type == typeid(std::string).name())
        {
            std::string s = reinterpret_cast<std::string*>(curr + property.offset)->c_str();
            *reinterpret_cast<std::string*>(tmp + property.offset) = s;
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
                    StructRecurrsion(curr + property.offset, tmp + property.offset, property.type);
                }
                catch (...)
                {
                    std::cout << property.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                    for (unsigned i = 0; i < property.size; ++i)
                        *(curr + property.offset + i) = *(tmp + property.offset + i);
                }
            }
            else
            {
                for (unsigned i = 0; i < property.size; ++i)
                    *(curr + property.offset + i) = *(tmp + property.offset + i);
            }
        }
    }
}

ActionParentStructInput::ActionParentStructInput(GameObject* g, std::string c, SceneLayer* l)
    : name()
    , comp(c)
    , oldObj(nullptr)
    , newObj(nullptr)
    , ly(l)
    , id()
{
    name = g->GetName();
    id = g->GetID();
    oldObj = new GameObject(nullptr, 0);
    g->Clone(oldObj);
    l_name = l->GetName();
}

void ActionParentStructInput::Execute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    unsigned char* Char = nullptr;
    for (auto& c : go->GetComponentList())
        if (c->GetName() == comp) Char = reinterpret_cast<unsigned char*>(c);
    unsigned char* tmpChar = nullptr;
    for (auto& c : newObj->GetComponentList())
        if (c->GetName() == comp) tmpChar = reinterpret_cast<unsigned char*>(c);
    auto properties = (*Factory::m_Reflection).at(comp.c_str())->getProperties();
    if (!Factory::m_Reflection->at(comp.c_str())->getParents().empty()) ParentRecurrsion(Char, tmpChar, comp);
    for (auto prop : properties)
    {
        if (prop.type == typeid(std::string).name() && tmpChar)
        {
            std::string s = reinterpret_cast<std::string*>(tmpChar + prop.offset)->c_str();
            *reinterpret_cast<std::string*>(Char + prop.offset) = s;
        }
        else if (prop.type.find("ptr") != std::string::npos)
            continue;
        else if (prop.type.find("std::") != std::string::npos)
            continue;
        else if (prop.type.find("*") != std::string::npos)
            continue;
        else
        {
            if ((prop.type.find("struct") != std::string::npos || prop.type.find("class") != std::string::npos) && prop.type.find("std::") == std::string::npos)
            {
                try
                {
                    StructRecurrsion(Char + prop.offset, tmpChar + prop.offset, prop.type);
                }
                catch (...)
                {
                    std::cout << prop.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                    for (unsigned i = 0; i < prop.size; ++i)
                        *(Char + prop.offset + i) = *(tmpChar + prop.offset + i);
                }
            }
            else
            {
                for (unsigned i = 0; i < prop.size; ++i)
                    *(Char + prop.offset + i) = *(tmpChar + prop.offset + i);
            }
        }
    }
    go->UpdateComponents();
}

void ActionParentStructInput::Revert()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    unsigned char* Char = nullptr;
    for (auto& c : go->GetComponentList())
        if (c->GetName() == comp) Char = reinterpret_cast<unsigned char*>(c);
    unsigned char* tmpChar = nullptr;
    for (auto& c : oldObj->GetComponentList())
        if (c->GetName() == comp) tmpChar = reinterpret_cast<unsigned char*>(c);
    auto properties = (*Factory::m_Reflection).at(comp.c_str())->getProperties();
    if (!Factory::m_Reflection->at(comp.c_str())->getParents().empty()) ParentRecurrsion(Char, tmpChar, comp);
    for (auto prop : properties)
    {
        if (prop.type == typeid(std::string).name() && tmpChar)
        {
            std::string s = reinterpret_cast<std::string*>(tmpChar + prop.offset)->c_str();
            *reinterpret_cast<std::string*>(Char + prop.offset) = s;
        }
        else if (prop.type.find("ptr") != std::string::npos)
            continue;
        else if (prop.type.find("std::") != std::string::npos)
            continue;
        else if (prop.type.find("*") != std::string::npos)
            continue;
        else
        {
            if ((prop.type.find("struct") != std::string::npos || prop.type.find("class") != std::string::npos) && prop.type.find("std::") == std::string::npos)
            {
                try
                {
                    StructRecurrsion(Char + prop.offset, tmpChar + prop.offset, prop.type);
                }
                catch (...)
                {
                    std::cout << prop.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                    for (unsigned i = 0; i < prop.size; ++i)
                        *(Char + prop.offset + i) = *(tmpChar + prop.offset + i);
                }
            }
            else
            {
                for (unsigned i = 0; i < prop.size; ++i)
                    *(Char + prop.offset + i) = *(tmpChar + prop.offset + i);
            }
        }
    }
    go->UpdateComponents();
}

void ActionParentStructInput::SetNew(GameObject*& go)
{
    newObj = new GameObject(nullptr, 0);
    go->Clone(newObj);
}

ActionParentStructInput::~ActionParentStructInput()
{
    delete oldObj;
    delete newObj;
}

void ActionMeshInput::Execute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    auto renderer = go->GetComponent<MeshRenderer>();
    if (renderer)
    {
        renderer->SetMesh(ResourceManager::Instance().GetResource<Mesh>(newIndex));
    }
}

void ActionMeshInput::Revert()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    auto renderer = go->GetComponent<MeshRenderer>();
    if (renderer)
    {
        renderer->SetMesh(ResourceManager::Instance().GetResource<Mesh>(oldIndex));
    }
}

void ActionCreateLayer::Execute()
{
    Application::Instance().GetCurrentScene()->CreateLayer(name);
}

void ActionCreateLayer::Revert()
{
    Application::Instance().GetCurrentScene()->DeleteLayer(name);
    Editor::Instance().SetLayer(Application::Instance().GetCurrentScene()->GetLayers().back());
    Editor::Instance().SetCurrentObject(nullptr);
}

void ActionDeleteObject::DuplicateChildren(GameObject* root)
{
    SceneLayer* m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayerByName(l_name);
    if (root->GetChildrenObjects().empty()) return;
    for (auto& id : root->GetChildrenObjects())
    {
        childrens.push_back(GameObject(nullptr, id));
        auto& go = childrens.back();
        go.SetName(m_CurrentLayer->GetObjectById(id)->GetName());
        m_CurrentLayer->GetObjectById(id)->Clone(&go);
        go.SetChildrenObjects(m_CurrentLayer->GetObjectById(id)->GetChildrenObjects());
        DuplicateChildren(m_CurrentLayer->GetObjectById(id));
    }
}

void ActionDeleteObject::ReattachChildren(GameObject* root)
{
    SceneLayer* m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayerByName(l_name);
    for (unsigned i = 0; i < childrens.size(); ++i)
    {
        auto tmp = m_CurrentLayer->CreateObject(std::string {});
        unsigned id = tmp->GetID();
        for (auto& go : childrens)
        {
            if (id == go.GetID())
            {
                tmp->SetName(go.GetName());
                tmp->SetArchetype(go.GetArchetype());
                go.Clone(tmp);
                break;
            }
        }
    }
}

void ActionDeleteObject::Execute()
{
    GameObject* g = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    g->Destroy();
}

void ActionDeleteObject::Revert()
{
    GameObject* g = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->CreateObject(go->GetName());
    go->Clone(g);
    g->SetArchetype(go->GetArchetype());
    ReattachChildren(go);
}

ActionDeleteObject::~ActionDeleteObject()
{
    delete go;
}

void ActionDeleteArchetype::Execute()
{
    del = true;
    std::vector<unsigned> ids_;
    Editor::Instance().RemoveArchetype(go, ugo);
    auto lys = Application::Instance().GetCurrentScene()->GetLayers();
    for (auto& ly : lys)
    {
        auto gos = ly->GetObjectsList();
        for (auto& go : gos)
        {
            if (go->GetArchetype() == go->GetName())
            {
                ids_.push_back(go->GetID());
                go->SetArchetype(std::string {});
            }
        }
        a_go.insert(std::pair<std::string, std::vector<unsigned>>(ly->GetName(), ids_));
    }
}

void ActionDeleteArchetype::Revert()
{
    del = false;
    Editor::Instance().AddArchetype(go, ugo);
    auto lys = Application::Instance().GetCurrentScene()->GetLayers();
    for (auto& ly : lys)
    {
        try
        {
            auto& vectorOfIds = a_go.at(ly->GetName());
            for (auto& id : vectorOfIds)
            {
                ly->GetObjectById(id)->SetArchetype(go->GetName());
            }
            vectorOfIds.clear();
        }
        catch (...)
        {
            // kagami no mori no naka de
        }
    }
    a_go.clear();
}

ActionDeleteArchetype::~ActionDeleteArchetype()
{
    if (del)
    {
        delete go;
        delete ugo;
    }
}

void ActionDeleteLayer::Execute()
{
    Application::Instance().GetCurrentScene()->DeleteLayer(ly->GetName());
    Editor::Instance().SetLayer(Application::Instance().GetCurrentScene()->GetLayers().back());
    Editor::Instance().SetCurrentObject(nullptr);
}

void ActionDeleteLayer::Revert()
{
    auto layer = Application::Instance().GetCurrentScene()->CreateLayerWithoutCamera(ly->GetName());
    ly->Clone(layer);
    auto tmp = layer;
}

ActionDeleteLayer::~ActionDeleteLayer()
{
    delete ly;
}

void ActionInputTexture::Execute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    auto renderer = go->GetComponent<MeshRenderer>();
    if (renderer)
    {
        if (type == "m_DiffuseTexture")
            renderer->SetDiffuseTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
        else if (type == "m_NormalTexture")
            renderer->SetNormalTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
        else if (type == "m_SpecularTexture")
            renderer->SetSpecularTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
        else if (type == "m_EmissiveTexture")
            renderer->SetEmissiveTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
    }
    else if (go->GetComponent<BoxParticleEmitter>())
    {
        ParticleEmitter* particleEmitter = go->GetComponent<BoxParticleEmitter>();
        particleEmitter->SetTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
    }
    else if (go->GetComponent<CircleParticleEmitter>())
    {
        ParticleEmitter* particleEmitter = go->GetComponent<CircleParticleEmitter>();
        particleEmitter->SetTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
    }
    else if (go->GetComponent<Camera>())
    {
        Camera* cam = go->GetComponent<Camera>();
        cam->SetSkyboxTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
    }
}

void ActionInputTexture::Revert()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    auto renderer = go->GetComponent<MeshRenderer>();
    if (renderer)
    {
        if (type == "m_DiffuseTexture")
            renderer->SetDiffuseTexture(ResourceManager::Instance().GetResource<Texture>(oldIndex));
        else if (type == "m_NormalTexture")
            renderer->SetNormalTexture(ResourceManager::Instance().GetResource<Texture>(oldIndex));
        else if (type == "m_SpecularTexture")
            renderer->SetSpecularTexture(ResourceManager::Instance().GetResource<Texture>(oldIndex));
        else if (type == "m_EmissiveTexture")
            renderer->SetEmissiveTexture(ResourceManager::Instance().GetResource<Texture>(oldIndex));
    }
    else if (go->GetComponent<BoxParticleEmitter>())
    {
        ParticleEmitter* particleEmitter = go->GetComponent<BoxParticleEmitter>();
        particleEmitter->SetTexture(ResourceManager::Instance().GetResource<Texture>(oldIndex));
    }
    else if (go->GetComponent<CircleParticleEmitter>())
    {
        ParticleEmitter* particleEmitter = go->GetComponent<CircleParticleEmitter>();
        particleEmitter->SetTexture(ResourceManager::Instance().GetResource<Texture>(oldIndex));
    }
    else if (go->GetComponent<Camera>())
    {
        Camera* cam = go->GetComponent<Camera>();
        cam->SetSkyboxTexture(ResourceManager::Instance().GetResource<Texture>(oldIndex));
    }
}

void ActionInputAnimation::Execute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    auto animator = go->GetComponent<MeshAnimator>();
    if (animator)
    {
        animator->SetAnimationSet(ResourceManager::Instance().GetResource<AnimationSet>(newIndex));
    }
}

void ActionInputAnimation::Revert()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    auto animator = go->GetComponent<MeshAnimator>();
    if (animator)
    {
        animator->SetAnimationSet(ResourceManager::Instance().GetResource<AnimationSet>(oldIndex));
    }
}

void ActionAddTag::Execute()
{
    Editor::Instance().AddTag(name);
}

void ActionAddTag::Revert()
{
    Editor::Instance().RemoveTag(name);
}

void ActionDeleteTag::Execute()
{
    auto s = Application::Instance().GetCurrentScene();
    auto lys = s->GetLayers();
    for (auto& ly : lys)
    {
        auto gos = ly->GetObjectsList();
        for (auto& go : gos)
        {
            if (go->GetTag() == name)
            {
                go->SetTag(std::string {});
                list.push_back(std::pair<std::string, ImGuiID>(ly->GetName(), go->GetID()));
            }
        }
    }
    Editor::Instance().RemoveTag(name);
}

void ActionDeleteTag::Revert()
{
    auto s = Application::Instance().GetCurrentScene();
    Editor::Instance().AddTag(name);
    for (auto l : list)
    {
        auto ly = s->GetLayerByName(l.first);
        ly->GetObjectById(l.second)->SetTag(name);
    }
}

void ActionChangeTag::Execute()
{
    Editor::Instance().RemoveTag(oldName);
    Editor::Instance().AddTag(newName);
    auto s = Application::Instance().GetCurrentScene();
    auto lys = s->GetLayers();
    for (auto& ly : lys)
    {
        auto gos = ly->GetObjectsList();
        for (auto& go : gos)
        {
            if (go->GetTag() == oldName)
            {
                go->SetTag(newName);
                list.push_back(std::pair<std::string, ImGuiID>(ly->GetName(), go->GetID()));
            }
        }
    }
}

void ActionChangeTag::Revert()
{
    Editor::Instance().AddTag(oldName);
    Editor::Instance().RemoveTag(newName);
    auto s = Application::Instance().GetCurrentScene();
    for (auto l : list)
    {
        auto ly = s->GetLayerByName(l.first);
        ly->GetObjectById(l.second)->SetTag(oldName);
    }
}

void ActionTagObject::Execute()
{
    Application::Instance().GetCurrentScene()->GetLayerByName(layerName)->GetObjectById(id)->SetTag(newTag);
}

void ActionTagObject::Revert()
{
    Application::Instance().GetCurrentScene()->GetLayerByName(layerName)->GetObjectById(id)->SetTag(oldTag);
}

void ActionMultiTransform::Execute()
{
    auto curr = Application::Instance().GetCurrentScene()->GetLayerByName(layer);
    for (auto& id : ids)
    {
        GameObject* obj = curr->GetObjectById(id);
        auto t = obj->GetComponent<Transform>();
        if (t)
        {
            if (variable == "m_WorldPosition")
            {
                Vector3 tmp = t->GetWorldPosition();
                tmp += displacement;
                t->SetWorldPosition(tmp);
            }
            else if (variable == "m_WorldRotation")
            {
                Vector3 tmp = t->GetWorldRotation();
                tmp += displacement;
                t->SetWorldRotation(tmp);
            }
            else if (variable == "m_WorldScale")
            {
                Vector3 tmp = t->GetWorldScale();
                tmp += displacement;
                t->SetWorldScale(tmp);
            }
        }
    }
}

void ActionMultiTransform::Revert()
{
    auto curr = Application::Instance().GetCurrentScene()->GetLayerByName(layer);
    for (auto& id : ids)
    {
        GameObject* obj = curr->GetObjectById(id);
        auto t = obj->GetComponent<Transform>();
        if (t)
        {
            if (variable == "m_WorldPosition")
            {
                Vector3 tmp = t->GetWorldPosition();
                tmp -= displacement;
                t->SetWorldPosition(tmp);
            }
            else if (variable == "m_WorldRotation")
            {
                Vector3 tmp = t->GetWorldRotation();
                tmp -= displacement;
                t->SetWorldRotation(tmp);
            }
            else if (variable == "m_WorldScale")
            {
                Vector3 tmp = t->GetWorldScale();
                tmp -= displacement;
                t->SetWorldScale(tmp);
            }
        }
    }
}

void ActionDuplicate::Duplicate()
{
    SceneLayer* m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayerByName(layerName);
    if (!m_CurrentLayer) return;
    for (auto& id : selectedObjects)
    {
        GameObject* go = m_CurrentLayer->GetObjectById(id);
        GameObject* tmp = m_CurrentLayer->CreateObject(go->GetName());
        duplicatedObjects.push_back(tmp->GetID());
        go->Clone(tmp);
        ChildrenDuplicate(m_CurrentLayer, go, tmp);
    }
}

void ActionDuplicate::ChildrenDuplicate(SceneLayer* m_CurrentLayer, GameObject* original, GameObject* Clone)
{
    if (original->GetChildrenObjects().empty()) return;
    ChildrenList m_Childrens;
    ChildrenList correctChildren = original->GetChildrenObjects();
    for (auto& id : original->GetChildrenObjects())
    {
        GameObject* go = m_CurrentLayer->GetObjectById(id);
        GameObject* tmp = m_CurrentLayer->CreateObject(go->GetName());
        duplicatedObjects.push_back(tmp->GetID());
        go->Clone(tmp);
        m_Childrens.push_back(tmp->GetID());
        tmp->SetParentObject(Clone->GetID());
        ChildrenDuplicate(m_CurrentLayer, go, tmp);
    }
    Clone->SetChildrenObjects(m_Childrens);
    original->SetChildrenObjects(correctChildren);
}

void ActionDuplicate::Execute()
{
    Duplicate();
}

void ActionDuplicate::Revert()
{
    SceneLayer* m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayerByName(layerName);
    for (auto& id : duplicatedObjects)
    {
        m_CurrentLayer->GetObjectById(id)->Destroy();
    }
}

void ActionInputFont::Execute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    TextRenderer* font = go->GetComponent<TextRenderer>();
    if (font) font->m_Font = ResourceManager::Instance().GetResource<Font>(newIndex);
}

void ActionInputFont::Revert()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    TextRenderer* font = go->GetComponent<TextRenderer>();
    if (font) font->m_Font = ResourceManager::Instance().GetResource<Font>(oldIndex);
}

void ActionAddAttractor::Execute()
{
    auto go = Application::Instance().GetCurrentScene()->GetLayerByName(ly_name)->GetObjectById(id);
    auto boxParticle = go->GetComponent<BoxParticleEmitter>();
    if (!boxParticle)
    {
        auto circleParticle = go->GetComponent<CircleParticleEmitter>();
        circleParticle->m_Attractors.push_back(attractor);
    }
    else
    {
        boxParticle->m_Attractors.push_back(attractor);
    }
}

void ActionAddAttractor::Revert()
{
    auto go = Application::Instance().GetCurrentScene()->GetLayerByName(ly_name)->GetObjectById(id);
    auto boxParticle = go->GetComponent<BoxParticleEmitter>();
    if (!boxParticle)
    {
        auto circleParticle = go->GetComponent<CircleParticleEmitter>();
        circleParticle->m_Attractors = o_attractorList;
    }
    else
    {
        boxParticle->m_Attractors = o_attractorList;
    }
}

void ActionDeleteAttractor::Execute()
{
    auto go = Application::Instance().GetCurrentScene()->GetLayerByName(ly_name)->GetObjectById(id);
    auto boxParticle = go->GetComponent<BoxParticleEmitter>();
    if (!boxParticle)
    {
        auto circleParticle = go->GetComponent<CircleParticleEmitter>();
        auto tmp = std::find(circleParticle->m_Attractors.begin(), circleParticle->m_Attractors.end(), attractor);
        if (tmp != circleParticle->m_Attractors.end()) circleParticle->m_Attractors.erase(tmp);
    }
    else
    {
        auto tmp = std::find(boxParticle->m_Attractors.begin(), boxParticle->m_Attractors.end(), attractor);
        if (tmp != boxParticle->m_Attractors.end()) boxParticle->m_Attractors.erase(tmp);
    }
}

void ActionDeleteAttractor::Revert()
{
    auto go = Application::Instance().GetCurrentScene()->GetLayerByName(ly_name)->GetObjectById(id);
    auto boxParticle = go->GetComponent<BoxParticleEmitter>();
    if (!boxParticle)
    {
        auto circleParticle = go->GetComponent<CircleParticleEmitter>();
        circleParticle->m_Attractors = o_attractorList;
    }
    else
    {
        boxParticle->m_Attractors = o_attractorList;
    }
}
