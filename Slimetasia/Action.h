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
class Action
{
public:

    virtual ~Action() {}

    virtual void Execute() = 0;
    virtual void Revert() = 0;
};

class ActionChangeName : public Action
{
    std::string oldValue;
    std::string newValue;
    SceneLayer* ly;
    std::string l_name;
    unsigned id;

public:

    ActionChangeName(std::string pv, std::string nv, SceneLayer* l, GameObject* g)
        : oldValue(pv)
        , newValue(nv)
        , ly(l)
    {
        l_name = l->GetName();
        id = g->GetID();
    }
    void Execute() override { Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->SetName(newValue); }
    void Revert() override { Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->SetName(oldValue); }
};

template <typename T>
class ActionInput : public Action
{
    std::string name;
    std::string comp;
    std::string prop;
    T oldValue;
    T newValue;
    SceneLayer* ly;
    std::string l_name;
    unsigned id;

public:

    ActionInput(T pv, T nv, std::string o, std::string c, std::string p, SceneLayer* l, GameObject* g)
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
    void Revert() override;
};

template <typename T>
inline void ActionInput<T>::Execute()
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

template <typename T>
inline void ActionInput<T>::Revert()
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

template <>
class ActionInput<std::string> : public Action
{
    std::string name;
    std::string comp;
    std::string prop;
    std::string oldValue;
    std::string newValue;
    SceneLayer* ly;
    std::string l_name;
    unsigned id;

public:

    ActionInput(std::string pv, std::string nv, std::string o, std::string c, std::string p, SceneLayer* l, GameObject* g)
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
    void Revert() override
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

class ActionCreate : public Action
{
    SceneLayer* layer;
    GameObject* obj;
    std::string name;
    unsigned id;
    std::string l_name;
    bool childFlag;
    unsigned parentID;

public:

    ActionCreate(SceneLayer* ly, std::string n)
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
    void Revert() override
    {
        childFlag = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->GetIsChildren();
        parentID = Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->GetParentObject();
        Application::Instance().GetCurrentScene()->GetLayerByName(l_name)->GetObjectById(id)->Destroy();
    }
};

class ActionAddComponent : public Action
{
    GameObject* go;
    std::string comp;
    std::string script_name;
    SceneLayer* l;
    std::string l_name;
    std::string name;
    unsigned id;

public:

    ActionAddComponent(GameObject* g, std::string c, SceneLayer* ly)
        : go(g)
        , comp(c)
        , script_name {}
        , l(ly)
    {
        name = go->GetName();
        id = go->GetID();
        l_name = l->GetName();
    }
    void Execute() override;
    void Revert() override;
};

class ActionRemoveComponent : public Action
{
    GameObject* go;
    std::string comp;
    GameObject* tmp;
    SceneLayer* ly;
    std::string l_name;
    std::string name;
    unsigned id;
    void StructRecurrsion(unsigned char* curr, unsigned char* tmp, std::string prop);
    void ParentRecurrsion(unsigned char* curr, unsigned char* tmp, std::string comp);

public:

    ActionRemoveComponent(GameObject* g, std::string c, SceneLayer* l);
    void Execute() override;
    void Revert() override;
    ~ActionRemoveComponent();
};

class ActionCreateArchetype : public Action
{
    GameObject* obj;
    GameObject* uobj;
    std::vector<GameObject*> a_go;
    bool del;

public:

    ActionCreateArchetype(GameObject* go, GameObject* ugo)
        : obj(go)
        , uobj(ugo)
        , del(false)
    {
    }
    void Execute() override;
    void Revert() override;
    ~ActionCreateArchetype() override;
};

class ActionRevertArchetype : public Action
{
    GameObject* obj;

public:

    ActionRevertArchetype(GameObject* go)
        : obj(go)
    {
    }
    void Execute() override;
    void Revert() override;
    ~ActionRevertArchetype() override;
};

class ActionMakeChanges : public Action
{
    std::vector<GameObject> archetypeList;
    std::vector<GameObject*> archetypeObj;
    GameObject* obj;

public:

    ActionMakeChanges(GameObject* go)
        : obj(go)
    {
    }
    void Execute() override;
    void Revert() override;
    ~ActionMakeChanges() override;
};

class ActionCreateObjectArchetype : public Action
{
    SceneLayer* ly;
    std::string l_name;
    GameObject* go;
    std::string archetype;
    std::string name;
    unsigned id;

public:

    ActionCreateObjectArchetype(SceneLayer* l, std::string a, std::string n)
        : ly(l)
        , archetype(a)
        , name(n)
        , go(nullptr)
        , id(0)
    {
        l_name = ly->GetName();
    }
    void Execute() override;
    void Revert() override;
};

class ActionRevertScript : public Action
{
    std::string script;
    GameObject* go;
    SceneLayer* l;
    std::string l_name;
    std::string name;
    unsigned id;

public:

    ActionRevertScript(GameObject* g, std::string s, SceneLayer* ly)
        : go(g)
        , script(s)
        , l(ly)
    {
        id = go->GetID();
        l_name = l->GetName();
    }
    void Execute() override;
    void Revert() override;
};

class ActionParentStructInput : public Action
{
    std::string name;
    std::string comp;
    GameObject* oldObj;
    GameObject* newObj;
    SceneLayer* ly;
    std::string l_name;
    unsigned id;
    void StructRecurrsion(unsigned char* curr, unsigned char* tmp, std::string prop);
    void ParentRecurrsion(unsigned char* curr, unsigned char* tmp, std::string comp);

public:

    ActionParentStructInput(GameObject* g, std::string c, SceneLayer* l);
    void Execute() override;
    void Revert() override;
    void SetNew(GameObject*& go);
    ~ActionParentStructInput();
};

class ActionMeshInput : public Action
{
    ResourceGUID oldIndex;
    ResourceGUID newIndex;
    std::string name;
    std::string comp;
    SceneLayer* ly;
    std::string l_name;
    unsigned id;

public:

    ActionMeshInput(GameObject* g, std::string c, SceneLayer* l, ResourceGUID o, ResourceGUID n)
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
    void Revert() override;
};

class ActionInputFont : public Action
{
    ResourceGUID oldIndex;
    ResourceGUID newIndex;
    std::string name;
    std::string comp;
    std::string l_name;
    unsigned id;

public:

    ActionInputFont(GameObject* g, std::string c, SceneLayer* l, ResourceGUID o, ResourceGUID n)
        : oldIndex(o)
        , newIndex(n)
        , comp(c)
    {
        l_name = l->GetName();
        id = g->GetID();
        name = g->GetName();
    }
    void Execute() override;
    void Revert() override;
};

class ActionCreateLayer : public Action
{
    std::string name;
    SceneLayer* p;

public:

    ActionCreateLayer(std::string n, SceneLayer* l)
        : name(n)
        , p(l)
    {
    }
    void Execute() override;
    void Revert() override;
};

class ActionDeleteObject : public Action
{
    std::list<GameObject> childrens;
    GameObject* go;
    unsigned id;
    SceneLayer* ly;
    std::string l_name;
    void DuplicateChildren(GameObject* root);
    void ReattachChildren(GameObject* root);

public:

    ActionDeleteObject(GameObject* g, SceneLayer* l)
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
    void Revert() override;
    ~ActionDeleteObject();
};

class ActionDeleteArchetype : public Action
{
    bool del;
    GameObject* go;
    GameObject* ugo;
    std::map<std::string, std::vector<unsigned>> a_go;

public:

    ActionDeleteArchetype(GameObject* g, GameObject* ug)
        : del(true)
        , go(g)
        , ugo(ug)
        , a_go()
    {
    }
    void Execute() override;
    void Revert() override;
    ~ActionDeleteArchetype();
};

class ActionDeleteLayer : public Action
{
    SceneLayer* ly;

public:

    ActionDeleteLayer(SceneLayer* l)
    {
        ly = new SceneLayer(nullptr, 0, l->GetName(), false);
        l->Clone(ly);
    }
    void Execute() override;
    void Revert() override;
    ~ActionDeleteLayer();
};

class ActionInputTexture : public Action
{
    ResourceGUID oldIndex;
    ResourceGUID newIndex;
    std::string name;
    std::string comp;
    SceneLayer* ly;
    unsigned id;
    std::string type;
    std::string l_name;

public:

    ActionInputTexture(GameObject* g, std::string c, SceneLayer* l, ResourceGUID o, ResourceGUID n, std::string t)
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
    void Revert() override;
};

class ActionInputAnimation : public Action
{
    ResourceGUID oldIndex;
    ResourceGUID newIndex;
    std::string name;
    std::string comp;
    SceneLayer* ly;
    unsigned id;
    std::string type;
    std::string l_name;

public:

    ActionInputAnimation(GameObject* g, std::string c, SceneLayer* l, ResourceGUID o, ResourceGUID n, std::string t)
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
    void Revert() override;
};

template <typename T>
class ActionInputScript : public Action
{
    unsigned id;
    std::string script;
    T oldValue;
    T newValue;
    std::string l_name;
    std::string var;

public:

    ActionInputScript(GameObject* g, std::string s, T o, T n, std::string v)
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
    void Revert() override;
};

template <typename T>
inline void ActionInputScript<T>::Execute()
{
    SceneLayer* ly = Application::Instance().GetCurrentScene()->GetLayerByName(l_name);
    GameObject* g = ly->GetObjectById(id);
    g->GetScript(script)->set(var, newValue);
}

template <typename T>
inline void ActionInputScript<T>::Revert()
{
    SceneLayer* ly = Application::Instance().GetCurrentScene()->GetLayerByName(l_name);
    GameObject* g = ly->GetObjectById(id);
    g->GetScript(script)->set(var, oldValue);
}

class ActionAddTag : public Action
{
    std::string name;

public:

    ActionAddTag(std::string n)
        : name(n)
    {
    }
    void Execute() override;
    void Revert() override;
};

class ActionDeleteTag : public Action
{
    std::vector<std::pair<std::string, unsigned>> list;
    std::string name;

public:

    ActionDeleteTag(std::string n)
        : name(n)
    {
    }
    void Execute() override;
    void Revert() override;
};

class ActionChangeTag : public Action
{
    std::vector<std::pair<std::string, unsigned>> list;
    std::string oldName;
    std::string newName;

public:

    ActionChangeTag(std::string o, std::string n)
        : oldName(o)
        , newName(n)
    {
    }
    void Execute() override;
    void Revert() override;
};

class ActionTagObject : public Action
{
    unsigned id;
    std::string oldTag;
    std::string newTag;
    std::string layerName;

public:

    ActionTagObject(std::string o, std::string n, GameObject* go)
        : oldTag(o)
        , newTag(n)
    {
        layerName = go->GetParentLayer()->GetName();
        id = go->GetID();
    }
    void Execute() override;
    void Revert() override;
};

template <class T>
class ActionInputPhysics : public Action
{
    T oldValue;
    T newValue;
    T* value;

public:

    ActionInputPhysics(T o, T n, T* v)
        : oldValue(o)
        , newValue(n)
        , value(v)
    {
    }
    void Execute() override;
    void Revert() override;
};

template <class T>
inline void ActionInputPhysics<T>::Execute()
{
    *value = newValue;
}

template <class T>
inline void ActionInputPhysics<T>::Revert()
{
    *value = oldValue;
}

class ActionMultiTransform : public Action
{
    Vector3 displacement;
    std::vector<unsigned> ids;
    std::string component;
    std::string variable;
    std::string layer;

public:

    ActionMultiTransform(Vector3 d, std::vector<unsigned> id, std::string c, std::string v, SceneLayer* ly)
        : component(c)
        , variable(v)
        , displacement(d)
    {
        layer = ly->GetName();
        ids = id;
    }
    void Execute() override;
    void Revert() override;
};

class ActionDuplicate : public Action
{
    std::string layerName;
    std::vector<unsigned> selectedObjects;
    std::vector<unsigned> duplicatedObjects;
    void Duplicate();
    void ChildrenDuplicate(SceneLayer* m_CurrentLayer, GameObject* original, GameObject* Clone);

public:

    ActionDuplicate(SceneLayer* ly, std::vector<GameObject*> selected)
    {
        layerName = ly->GetName();
        for (auto& go : selected)
            selectedObjects.push_back(go->GetID());
    }
    void Execute() override;
    void Revert() override;
};

class ActionAddAttractor : public Action
{
    std::vector<unsigned> o_attractorList;
    unsigned id;
    std::string ly_name;
    unsigned attractor;

public:

    ActionAddAttractor(GameObject* go, std::vector<unsigned> list, unsigned attract)
    {
        id = go->GetID();
        ly_name = go->GetParentLayer()->GetName();
        attractor = attract;
        o_attractorList = list;
    }
    void Execute() override;
    void Revert() override;
};

class ActionDeleteAttractor : public Action
{
    std::vector<unsigned> o_attractorList;
    unsigned id;
    std::string ly_name;
    unsigned attractor;

public:

    ActionDeleteAttractor(GameObject* go, std::vector<unsigned> list, unsigned attract)
    {
        id = go->GetID();
        ly_name = go->GetParentLayer()->GetName();
        attractor = attract;
        o_attractorList = list;
    }
    void Execute() override;
    void Revert() override;
};

// class DeleteResource_Action : public Actions
// {
//   std::string filePath;
//   std::string type;
//   std::
// };
