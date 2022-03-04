#include "Actions.h"

#include <utility>

#include "Editor.h"
#include "MeshAnimator.h"
#include "MeshRenderer.h"
#include "ParticleSystem.h"
#include "ResourceManager.h"
#include "luascript.h"

void CreateArchetype_Action::Execute()
{
    del = false;
    Editor::Instance().AddArchetype(obj, uobj);
    for (auto& go : a_go)
        go->SetArchetype(obj->GetName());
    a_go.clear();
}

void CreateArchetype_Action::UnExecute()
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
                go->SetArchetype(std::string{});
            }
        }
    }
}

CreateArchetype_Action::~CreateArchetype_Action()
{
    if (del) delete obj;
}

void Add_Component_Action::Execute()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    void* script = (*Factory::m_Factories).at(comp.c_str())->create(go);
    if (comp == "LuaScript") script_name = reinterpret_cast<LuaScript*>(script)->GetScript();
}

void Add_Component_Action::UnExecute()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    if (comp == "LuaScript")
        go->RemoveScript(script_name);
    else
        (*Factory::m_Factories).at(comp.c_str())->remove(go);
}

void Remove_Component_Action::StructRecurrsion(unsigned char* curr, unsigned char* tmp, std::string prop)
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

void Remove_Component_Action::ParentRecurrsion(unsigned char* curr, unsigned char* tmp, std::string comp)
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
Remove_Component_Action::Remove_Component_Action(GameObject* g, std::string c, Layer* l)
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

void Remove_Component_Action::Execute()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    (*Factory::m_Factories).at(comp.c_str())->remove(go);
}

void Remove_Component_Action::UnExecute()
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

Remove_Component_Action::~Remove_Component_Action()
{
    delete tmp;
}

void RevertArchetype_Action::Execute()
{
    obj = Editor::Instance().RevertArchetype(obj);
}

void RevertArchetype_Action::UnExecute()
{
    obj = Editor::Instance().RevertArchetype(obj);
}

RevertArchetype_Action::~RevertArchetype_Action()
{
    delete obj;
}

void MakeChanges_Action::Execute()
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

void MakeChanges_Action::UnExecute()
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

MakeChanges_Action::~MakeChanges_Action()
{
    delete obj;
}

void CreateObjectArchetype_Action::Execute()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->CreateObject(name);
    Editor::Instance().GetArchetype(archetype)->Clone(go);
    go->SetArchetype(name);
    id = go->GetID();
}

void CreateObjectArchetype_Action::UnExecute()
{
    Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->Destroy();
}

void RemoveScript_Action::Execute()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    go->RemoveScript(script);
}

void RemoveScript_Action::UnExecute()
{
    go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    go->AddComponent<LuaScript>()->SetScript(script);
}

void ParentStruct_InputAction::StructRecurrsion(unsigned char* curr, unsigned char* tmp, std::string prop)
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

void ParentStruct_InputAction::ParentRecurrsion(unsigned char* curr, unsigned char* tmp, std::string comp)
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

ParentStruct_InputAction::ParentStruct_InputAction(GameObject* g, std::string c, Layer* l)
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

void ParentStruct_InputAction::Execute()
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

void ParentStruct_InputAction::UnExecute()
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

void ParentStruct_InputAction::SetNew(GameObject*& go)
{
    newObj = new GameObject(nullptr, 0);
    go->Clone(newObj);
}

ParentStruct_InputAction::~ParentStruct_InputAction()
{
    delete oldObj;
    delete newObj;
}

void HMesh_InputAction::Execute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    auto renderer = go->GetComponent<MeshRenderer>();
    if (renderer)
    {
        renderer->SetMesh(ResourceManager::Instance().GetResource<Mesh>(newIndex));
    }
}

void HMesh_InputAction::UnExecute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    auto renderer = go->GetComponent<MeshRenderer>();
    if (renderer)
    {
        renderer->SetMesh(ResourceManager::Instance().GetResource<Mesh>(oldIndex));
    }
}

void CreateLayer_Action::Execute()
{
    Application::Instance().GetCurrentScene()->CreateLayer(name);
}

void CreateLayer_Action::UnExecute()
{
    Application::Instance().GetCurrentScene()->DeleteLayer(name);
    Editor::Instance().SetLayer(Application::Instance().GetCurrentScene()->GetLayers().back());
    Editor::Instance().SetCurrentObject(nullptr);
}

void DeleteObject_Action::DuplicateChildren(GameObject* root)
{
    Layer* m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayerByName(l_name);
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

void DeleteObject_Action::ReattachChildren(GameObject* root)
{
    Layer* m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayerByName(l_name);
    for (unsigned i = 0; i < childrens.size(); ++i)
    {
        auto tmp = m_CurrentLayer->CreateObject(std::string{});
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

void DeleteObject_Action::Execute()
{
    GameObject* g = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    g->Destroy();
}

void DeleteObject_Action::UnExecute()
{
    GameObject* g = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->CreateObject(go->GetName());
    go->Clone(g);
    g->SetArchetype(go->GetArchetype());
    ReattachChildren(go);
}

DeleteObject_Action::~DeleteObject_Action()
{
    delete go;
}

void DeleteArchetype_Action::Execute()
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
                go->SetArchetype(std::string{});
            }
        }
        a_go.insert(std::pair<std::string, std::vector<unsigned>>(ly->GetName(), ids_));
    }
}

void DeleteArchetype_Action::UnExecute()
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

DeleteArchetype_Action::~DeleteArchetype_Action()
{
    if (del)
    {
        delete go;
        delete ugo;
    }
}

void DeleteLayer_Action::Execute()
{
    Application::Instance().GetCurrentScene()->DeleteLayer(ly->GetName());
    Editor::Instance().SetLayer(Application::Instance().GetCurrentScene()->GetLayers().back());
    Editor::Instance().SetCurrentObject(nullptr);
}

void DeleteLayer_Action::UnExecute()
{
    auto layer = Application::Instance().GetCurrentScene()->CreateLayerWithoutCamera(ly->GetName());
    ly->Clone(layer);
    auto tmp = layer;
}

DeleteLayer_Action::~DeleteLayer_Action()
{
    delete ly;
}

void TextureInput_Action::Execute()
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

void TextureInput_Action::UnExecute()
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

void AnimationInput_Action::Execute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    auto animator = go->GetComponent<MeshAnimator>();
    if (animator)
    {
        animator->SetAnimationSet(ResourceManager::Instance().GetResource<AnimationSet>(newIndex));
    }
}

void AnimationInput_Action::UnExecute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    auto animator = go->GetComponent<MeshAnimator>();
    if (animator)
    {
        animator->SetAnimationSet(ResourceManager::Instance().GetResource<AnimationSet>(oldIndex));
    }
}

void AddTag_Action::Execute()
{
    Editor::Instance().AddTag(name);
}

void AddTag_Action::UnExecute()
{
    Editor::Instance().RemoveTag(name);
}

void DeleteTag_Action::Execute()
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
                go->SetTag(std::string{});
                list.push_back(std::pair<std::string, ImGuiID>(ly->GetName(), go->GetID()));
            }
        }
    }
    Editor::Instance().RemoveTag(name);
}

void DeleteTag_Action::UnExecute()
{
    auto s = Application::Instance().GetCurrentScene();
    Editor::Instance().AddTag(name);
    for (auto l : list)
    {
        auto ly = s->GetLayerByName(l.first);
        ly->GetObjectById(l.second)->SetTag(name);
    }
}

void ChangeTag_Action::Execute()
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

void ChangeTag_Action::UnExecute()
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

void ObjectTag_Action::Execute()
{
    Application::Instance().GetCurrentScene()->GetLayerByName(layerName)->GetObjectById(id)->SetTag(newTag);
}

void ObjectTag_Action::UnExecute()
{
    Application::Instance().GetCurrentScene()->GetLayerByName(layerName)->GetObjectById(id)->SetTag(oldTag);
}

void MultipleTransformation_Action::Execute()
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

void MultipleTransformation_Action::UnExecute()
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

void Duplicate_Action::Duplicate()
{
    Layer* m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayerByName(layerName);
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

void Duplicate_Action::ChildrenDuplicate(Layer* m_CurrentLayer, GameObject* original, GameObject* Clone)
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

void Duplicate_Action::Execute()
{
    Duplicate();
}

void Duplicate_Action::UnExecute()
{
    Layer* m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayerByName(layerName);
    for (auto& id : duplicatedObjects)
    {
        m_CurrentLayer->GetObjectById(id)->Destroy();
    }
}

void HFont_InputAction::Execute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    TextRenderer* font = go->GetComponent<TextRenderer>();
    if (font) font->m_Font = ResourceManager::Instance().GetResource<Font>(newIndex);
}

void HFont_InputAction::UnExecute()
{
    GameObject* go = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    TextRenderer* font = go->GetComponent<TextRenderer>();
    if (font) font->m_Font = ResourceManager::Instance().GetResource<Font>(oldIndex);
}

void AddAttractor_Action::Execute()
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

void AddAttractor_Action::UnExecute()
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

void DeleteAttractor_Action::Execute()
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

void DeleteAttractor_Action::UnExecute()
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
