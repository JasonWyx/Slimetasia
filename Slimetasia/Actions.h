#pragma once
#include <deque>

#include "Application.h"
#include "GameObject.h"
#include "Layer.h"
#include "Reflection.h"
#include "Scene.h"
#include "Transform.h"

// may need all the Actions to take in gameobject and postoptionsupdate function

// To replace all geobjectbyname to getobjectbyid

// all commands/actions will inherit from this class
class Actions
{
public:
    virtual void Execute() {}
    virtual void UnExecute() {}
    virtual ~Actions() {}
};

class Name_Action : public Actions
{
    std::string oldValue;
    std::string newValue;
    Layer* ly;
    std::string l_name;
    unsigned id;

public:
    Name_Action(std::string pv, std::string nv, Layer* l, GameObject* g)
        : oldValue(pv)
        , newValue(nv)
        , ly(l)
    {
        l_name = l->GetName();
        id = g->GetID();
    }
    void Execute() override { Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->SetName(newValue); }
    void UnExecute() override { Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->SetName(oldValue); }
};

template <typename T> class Input_Action : public Actions
{
    std::string name;
    std::string comp;
    std::string prop;
    T oldValue;
    T newValue;
    Layer* ly;
    std::string l_name;
    unsigned id;

public:
    Input_Action(T pv, T nv, std::string o, std::string c, std::string p, Layer* l, GameObject* g)
        : name(o)
        , oldValue(pv)
        , newValue(nv)
        , comp(c)
        , prop(p)
        , ly(l)
    {
        l_name = ly->GetName();
        id = g->GetID();
    }
    void Execute() override;
    void UnExecute() override;
};

template <typename T> inline void Input_Action<T>::Execute()
{
    GameObject* obj = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    for (auto& component : obj->GetComponentList())
    {
        if (component->GetName() == comp)
        {
            auto properties = (*Factory::m_Reflection).at(comp)->getProperties();
            for (auto property : properties)
            {
                if (property.name == prop)
                {
                    unsigned char* compAddr = reinterpret_cast<unsigned char*>(component) + property.offset;
                    unsigned char* valueAddr = reinterpret_cast<unsigned char*>(&newValue);
                    for (unsigned i = 0; i < property.size; ++i)
                    {
                        *(compAddr + i) = *(valueAddr + i);
                    }
                    // obj->UpdateComponents();
                    return;
                }
            }
        }
    }
}

template <typename T> inline void Input_Action<T>::UnExecute()
{
    GameObject* obj = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
    for (auto& component : obj->GetComponentList())
    {
        if (component->GetName() == comp)
        {
            auto properties = (*Factory::m_Reflection).at(comp)->getProperties();
            for (auto property : properties)
            {
                if (property.name == prop)
                {
                    unsigned char* compAddr = reinterpret_cast<unsigned char*>(component) + property.offset;
                    unsigned char* valueAddr = reinterpret_cast<unsigned char*>(&oldValue);
                    for (unsigned i = 0; i < property.size; ++i)
                    {
                        *(compAddr + i) = *(valueAddr + i);
                    }
                    // obj->UpdateComponents();
                    return;
                }
            }
        }
    }
}

template <> class Input_Action<std::string> : public Actions
{
    std::string name;
    std::string comp;
    std::string prop;
    std::string oldValue;
    std::string newValue;
    Layer* ly;
    std::string l_name;
    unsigned id;

public:
    Input_Action(std::string pv, std::string nv, std::string o, std::string c, std::string p, Layer* l, GameObject* g)
        : name(o)
        , oldValue(pv)
        , newValue(nv)
        , comp(c)
        , prop(p)
        , ly(l)
    {
        l_name = ly->GetName();
        id = g->GetID();
    }
    void Execute() override
    {
        GameObject* obj = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
        for (auto& component : obj->GetComponentList())
        {
            if (component->GetName() == comp)
            {
                if (component->GetName() == "LuaScript" && dynamic_cast<LuaScript*>(component)->GetScript() != oldValue) continue;
                auto properties = (*Factory::m_Reflection).at(comp)->getProperties();
                for (auto property : properties)
                {
                    if (property.name == prop)
                    {
                        std::string* compAddr = reinterpret_cast<std::string*>(reinterpret_cast<unsigned char*>(component) + property.offset);
                        *compAddr = newValue;
                        obj->UpdateComponents();
                        return;
                    }
                }
            }
        }
    }
    void UnExecute() override
    {
        GameObject* obj = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id);
        for (auto& component : obj->GetComponentList())
        {
            if (component->GetName() == comp)
            {
                if (component->GetName() == "LuaScript" && dynamic_cast<LuaScript*>(component)->GetScript() != newValue) continue;
                auto properties = (*Factory::m_Reflection).at(comp)->getProperties();
                for (auto property : properties)
                {
                    if (property.name == prop)
                    {
                        std::string* compAddr = reinterpret_cast<std::string*>(reinterpret_cast<unsigned char*>(component) + property.offset);
                        *compAddr = oldValue;
                        obj->UpdateComponents();
                        return;
                    }
                }
            }
        }
    }
};

class Creation_Action : public Actions
{
    Layer* layer;
    GameObject* obj;
    std::string name;
    unsigned id;
    std::string l_name;
    bool childFlag;
    unsigned parentID;

public:
    Creation_Action(Layer* ly, std::string n)
        : layer(ly)
        , obj(nullptr)
        , name(n)
        , id(0)
        , childFlag(false)
        , parentID(0)
    {
        l_name = ly->GetName();
    }
    void Execute() override
    {
        obj = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->CreateObject(name);
        id = obj->GetID();
        obj->SetParentObject(parentID);
        obj->SetIsChildren(childFlag);
        if (childFlag) Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(parentID)->AttachChild(id);
    }
    void UnExecute() override
    {
        childFlag = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->GetIsChildren();
        parentID = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->GetParentObject();
        Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->Destroy();
    }
};

class Add_Component_Action : public Actions
{
    GameObject* go;
    std::string comp;
    std::string script_name;
    Layer* l;
    std::string l_name;
    std::string name;
    unsigned id;

public:
    Add_Component_Action(GameObject* g, std::string c, Layer* ly)
        : go(g)
        , comp(c)
        , script_name{}
        , l(ly)
    {
        name = go->GetName();
        id = go->GetID();
        l_name = l->GetName();
    }
    void Execute() override;
    void UnExecute() override;
};

class Remove_Component_Action : public Actions
{
    GameObject* go;
    std::string comp;
    GameObject* tmp;
    Layer* ly;
    std::string l_name;
    std::string name;
    unsigned id;
    void StructRecurrsion(unsigned char* curr, unsigned char* tmp, std::string prop);
    void ParentRecurrsion(unsigned char* curr, unsigned char* tmp, std::string comp);

public:
    Remove_Component_Action(GameObject* g, std::string c, Layer* l);
    void Execute() override;
    void UnExecute() override;
    ~Remove_Component_Action();
};

class CreateArchetype_Action : public Actions
{
    GameObject* obj;
    GameObject* uobj;
    std::vector<GameObject*> a_go;
    bool del;

public:
    CreateArchetype_Action(GameObject* go, GameObject* ugo)
        : obj(go)
        , uobj(ugo)
        , del(false)
    {
    }
    void Execute() override;
    void UnExecute() override;
    ~CreateArchetype_Action() override;
};

class RevertArchetype_Action : public Actions
{
    GameObject* obj;

public:
    RevertArchetype_Action(GameObject* go)
        : obj(go)
    {
    }
    void Execute() override;
    void UnExecute() override;
    ~RevertArchetype_Action() override;
};

class MakeChanges_Action : public Actions
{
    std::vector<GameObject> archetypeList;
    std::vector<GameObject*> archetypeObj;
    GameObject* obj;

public:
    MakeChanges_Action(GameObject* go)
        : obj(go)
    {
    }
    void Execute() override;
    void UnExecute() override;
    ~MakeChanges_Action() override;
};

class CreateObjectArchetype_Action : public Actions
{
    Layer* ly;
    std::string l_name;
    GameObject* go;
    std::string archetype;
    std::string name;
    unsigned id;

public:
    CreateObjectArchetype_Action(Layer* l, std::string a, std::string n)
        : ly(l)
        , archetype(a)
        , name(n)
        , go(nullptr)
        , id(0)
    {
        l_name = ly->GetName();
    }
    void Execute() override;
    void UnExecute() override;
};

class RemoveScript_Action : public Actions
{
    std::string script;
    GameObject* go;
    Layer* l;
    std::string l_name;
    std::string name;
    unsigned id;

public:
    RemoveScript_Action(GameObject* g, std::string s, Layer* ly)
        : go(g)
        , script(s)
        , l(ly)
    {
        id = go->GetID();
        l_name = l->GetName();
    }
    void Execute() override;
    void UnExecute() override;
};

class ParentStruct_InputAction : public Actions
{
    std::string name;
    std::string comp;
    GameObject* oldObj;
    GameObject* newObj;
    Layer* ly;
    std::string l_name;
    unsigned id;
    void StructRecurrsion(unsigned char* curr, unsigned char* tmp, std::string prop);
    void ParentRecurrsion(unsigned char* curr, unsigned char* tmp, std::string comp);

public:
    ParentStruct_InputAction(GameObject* g, std::string c, Layer* l);
    void Execute() override;
    void UnExecute() override;
    void SetNew(GameObject*& go);
    ~ParentStruct_InputAction();
};

class HMesh_InputAction : public Actions
{
    ResourceGUID oldIndex;
    ResourceGUID newIndex;
    std::string name;
    std::string comp;
    Layer* ly;
    std::string l_name;
    unsigned id;

public:
    HMesh_InputAction(GameObject* g, std::string c, Layer* l, ResourceGUID o, ResourceGUID n)
        : oldIndex(o)
        , newIndex(n)
        , ly(l)
        , comp(c)
    {
        l_name = ly->GetName();
        id = g->GetID();
        name = g->GetName();
    }
    void Execute() override;
    void UnExecute() override;
};

class HFont_InputAction : public Actions
{
    ResourceGUID oldIndex;
    ResourceGUID newIndex;
    std::string name;
    std::string comp;
    std::string l_name;
    unsigned id;

public:
    HFont_InputAction(GameObject* g, std::string c, Layer* l, ResourceGUID o, ResourceGUID n)
        : oldIndex(o)
        , newIndex(n)
        , comp(c)
    {
        l_name = l->GetName();
        id = g->GetID();
        name = g->GetName();
    }
    void Execute() override;
    void UnExecute() override;
};

class CreateLayer_Action : public Actions
{
    std::string name;
    Layer* p;

public:
    CreateLayer_Action(std::string n, Layer* l)
        : name(n)
        , p(l)
    {
    }
    void Execute() override;
    void UnExecute() override;
};

class DeleteObject_Action : public Actions
{
    std::list<GameObject> childrens;
    GameObject* go;
    unsigned id;
    Layer* ly;
    std::string l_name;
    void DuplicateChildren(GameObject* root);
    void ReattachChildren(GameObject* root);

public:
    DeleteObject_Action(GameObject* g, Layer* l)
        : ly(l)
    {
        go = new GameObject(nullptr, 0);
        g->Clone(go);
        id = g->GetID();
        go->SetName(g->GetName());
        go->SetArchetype(g->GetArchetype());
        go->SetChildrenObjects(g->GetChildrenObjects());
        l_name = l->GetName();
        DuplicateChildren(g);
    }
    void Execute() override;
    void UnExecute() override;
    ~DeleteObject_Action();
};

class DeleteArchetype_Action : public Actions
{
    bool del;
    GameObject* go;
    GameObject* ugo;
    std::map<std::string, std::vector<unsigned>> a_go;

public:
    DeleteArchetype_Action(GameObject* g, GameObject* ug)
        : del(true)
        , go(g)
        , ugo(ug)
        , a_go()
    {
    }
    void Execute() override;
    void UnExecute() override;
    ~DeleteArchetype_Action();
};

class DeleteLayer_Action : public Actions
{
    Layer* ly;

public:
    DeleteLayer_Action(Layer* l)
    {
        ly = new Layer(nullptr, 0, l->GetName(), false);
        l->Clone(ly);
    }
    void Execute() override;
    void UnExecute() override;
    ~DeleteLayer_Action();
};

class TextureInput_Action : public Actions
{
    ResourceGUID oldIndex;
    ResourceGUID newIndex;
    std::string name;
    std::string comp;
    Layer* ly;
    unsigned id;
    std::string type;
    std::string l_name;

public:
    TextureInput_Action(GameObject* g, std::string c, Layer* l, ResourceGUID o, ResourceGUID n, std::string t)
        : oldIndex(o)
        , newIndex(n)
        , ly(l)
        , comp(c)
        , type(t)
    {
        l_name = ly->GetName();
        id = g->GetID();
        name = g->GetName();
    }
    void Execute() override;
    void UnExecute() override;
};

class AnimationInput_Action : public Actions
{
    ResourceGUID oldIndex;
    ResourceGUID newIndex;
    std::string name;
    std::string comp;
    Layer* ly;
    unsigned id;
    std::string type;
    std::string l_name;

public:
    AnimationInput_Action(GameObject* g, std::string c, Layer* l, ResourceGUID o, ResourceGUID n, std::string t)
        : oldIndex(o)
        , newIndex(n)
        , ly(l)
        , comp(c)
        , type(t)
    {
        l_name = ly->GetName();
        id = g->GetID();
        name = g->GetName();
    }
    void Execute() override;
    void UnExecute() override;
};

template <typename T> class ScriptInput_Action : public Actions
{
    unsigned id;
    std::string script;
    T oldValue;
    T newValue;
    std::string l_name;
    std::string var;

public:
    ScriptInput_Action(GameObject* g, std::string s, T o, T n, std::string v)
        : id(0)
        , script(s)
        , oldValue(o)
        , newValue(n)
        , var(v)
    {
        l_name = g->GetParentLayer()->GetName();
        id = g->GetID();
    }
    void Execute() override;
    void UnExecute() override;
};

template <typename T> inline void ScriptInput_Action<T>::Execute()
{
    Layer* ly = Application::Instance().GetCurrentScene()->GetLayerByName(l_name);
    GameObject* g = ly->GetObjectById(id);
    g->GetScript(script)->set(var, newValue);
}

template <typename T> inline void ScriptInput_Action<T>::UnExecute()
{
    Layer* ly = Application::Instance().GetCurrentScene()->GetLayerByName(l_name);
    GameObject* g = ly->GetObjectById(id);
    g->GetScript(script)->set(var, oldValue);
}

class AddTag_Action : public Actions
{
    std::string name;

public:
    AddTag_Action(std::string n)
        : name(n)
    {
    }
    void Execute() override;
    void UnExecute() override;
};

class DeleteTag_Action : public Actions
{
    std::vector<std::pair<std::string, unsigned>> list;
    std::string name;

public:
    DeleteTag_Action(std::string n)
        : name(n)
    {
    }
    void Execute() override;
    void UnExecute() override;
};

class ChangeTag_Action : public Actions
{
    std::vector<std::pair<std::string, unsigned>> list;
    std::string oldName;
    std::string newName;

public:
    ChangeTag_Action(std::string o, std::string n)
        : oldName(o)
        , newName(n)
    {
    }
    void Execute() override;
    void UnExecute() override;
};

class ObjectTag_Action : public Actions
{
    unsigned id;
    std::string oldTag;
    std::string newTag;
    std::string layerName;

public:
    ObjectTag_Action(std::string o, std::string n, GameObject* go)
        : oldTag(o)
        , newTag(n)
    {
        layerName = go->GetParentLayer()->GetName();
        id = go->GetID();
    }
    void Execute() override;
    void UnExecute() override;
};

template <class T> class PhysicsInput_Action : public Actions
{
    T oldValue;
    T newValue;
    T* value;

public:
    PhysicsInput_Action(T o, T n, T* v)
        : oldValue(o)
        , newValue(n)
        , value(v)
    {
    }
    void Execute() override;
    void UnExecute() override;
};

template <class T> inline void PhysicsInput_Action<T>::Execute()
{
    *value = newValue;
}

template <class T> inline void PhysicsInput_Action<T>::UnExecute()
{
    *value = oldValue;
}

class MultipleTransformation_Action : public Actions
{
    Vector3 displacement;
    std::vector<unsigned> ids;
    std::string component;
    std::string variable;
    std::string layer;

public:
    MultipleTransformation_Action(Vector3 d, std::vector<unsigned> id, std::string c, std::string v, Layer* ly)
        : component(c)
        , variable(v)
        , displacement(d)
    {
        layer = ly->GetName();
        ids = id;
    }
    void Execute() override;
    void UnExecute() override;
};

class Duplicate_Action : public Actions
{
    std::string layerName;
    std::vector<unsigned> selectedObjects;
    std::vector<unsigned> duplicatedObjects;
    void Duplicate();
    void ChildrenDuplicate(Layer* m_CurrentLayer, GameObject* original, GameObject* Clone);

public:
    Duplicate_Action(Layer* ly, std::vector<GameObject*> selected)
    {
        layerName = ly->GetName();
        for (auto& go : selected)
            selectedObjects.push_back(go->GetID());
    }
    void Execute() override;
    void UnExecute() override;
};

class AddAttractor_Action : public Actions
{
    std::vector<unsigned> o_attractorList;
    unsigned id;
    std::string ly_name;
    unsigned attractor;

public:
    AddAttractor_Action(GameObject* go, std::vector<unsigned> list, unsigned attract)
    {
        id = go->GetID();
        ly_name = go->GetParentLayer()->GetName();
        attractor = attract;
        o_attractorList = list;
    }
    void Execute() override;
    void UnExecute() override;
};

class DeleteAttractor_Action : public Actions
{
    std::vector<unsigned> o_attractorList;
    unsigned id;
    std::string ly_name;
    unsigned attractor;

public:
    DeleteAttractor_Action(GameObject* go, std::vector<unsigned> list, unsigned attract)
    {
        id = go->GetID();
        ly_name = go->GetParentLayer()->GetName();
        attractor = attract;
        o_attractorList = list;
    }
    void Execute() override;
    void UnExecute() override;
};

// class DeleteResource_Action : public Actions
// {
//   std::string filePath;
//   std::string type;
//   std::
// };
