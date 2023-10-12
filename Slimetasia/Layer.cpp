#include "Layer.h"

#include "Camera.h"
#include "CorePrerequisites.h"
#include "DirectionalLight.h"
#include "EditorCamera.h"
#include "GameObject.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Transform.h"

SceneLayer::SceneLayer(Scene* parentScene, unsigned id, std::string const& layerName, bool createCamera, unsigned objectPoolSize)
    : m_Id(id)
    , m_Name(layerName)
    , m_ParentScene(parentScene)
    , m_IsFlaggedForDestroy(false)
    , m_GameObjects()
    , m_ActiveObjects()
    , m_CreatedObjects()
    , m_EditorCamera(nullptr)
{
    // Initialize object ids
    for (unsigned i = 0; i < objectPoolSize; ++i)
        m_GameObjects.emplace_back(this, i);

    m_EditorCamera = CreateObjectAt("EditorCamera", Vector3(0.0f, 1.0f, -1.0f))->AddComponent<EditorCamera>();

    if (createCamera)
    {
        GameObject* cameraObject = CreateObjectAt("MainCamera", Vector3(0.0f, 1.0f, -1.0f));
        Camera* camera = cameraObject->AddComponent<Camera>();
        camera->SetLookAtDirection(Vector3(0.0f, -1.0f, -1.0f));
    }
}

SceneLayer::~SceneLayer() {}

void SceneLayer::Update(float dt)
{
    for (auto& go : m_ActiveObjects)
        go->Update(dt);

    PostFrameUpdate();
}

void SceneLayer::PostFrameUpdate()
{
    // Reset game objects tagged for deletion
    for (GameObject& gameObject : m_GameObjects)
    {
        gameObject.PostFrameUpdate();
        if (gameObject.m_IsFlaggedForDestroy)
        {
            if (gameObject.GetIsChildren())
            {
                GetObjectById(gameObject.GetParentObject())->RemoveChild(gameObject.GetID());
            }
            gameObject.m_Name = "";
            gameObject.m_IsActive = false;
            gameObject.m_IsActiveToggled = false;
            gameObject.m_IsInitialized = false;
            gameObject.m_IsFlaggedForDestroy = false;
            gameObject.m_IsChildren = false;
            gameObject.m_ParentObject = 0;
            gameObject.m_ChildrenObjects.clear();
            gameObject.ClearComponents();
            m_ActiveObjects.remove(&gameObject);
        }
    }
}

void SceneLayer::Destroy()
{
    m_IsFlaggedForDestroy = true;
}

void SceneLayer::Clone(SceneLayer* ly)
{
    auto& gos = ly->m_GameObjects;
    auto& mygos = m_GameObjects;
    std::size_t sz = gos.size();
    for (std::size_t i = 1; i < sz; ++i)
    {
        gos[i].m_Name = mygos[i].m_Name;
        gos[i].m_Archetype = mygos[i].m_Archetype;
        gos[i].m_IsActive = mygos[i].m_IsActive;
        gos[i].m_IsInitialized = mygos[i].m_IsInitialized;
        mygos[i].Clone(&gos[i]);
        if (gos[i].m_IsActive) ly->m_ActiveObjects.push_back(&gos[i]);
    }
}

GameObject* SceneLayer::CreateObject(std::string const& objectName)
{
    for (GameObject& gameObject : m_GameObjects)
    {
        if (!gameObject.m_IsInitialized)
        {
            gameObject.m_Name = objectName;
            gameObject.m_IsActive = true;
            gameObject.m_IsInitialized = true;
            gameObject.SetArchetype(std::string {});
            gameObject.SetChildrenObjects(ChildrenList {});
            gameObject.SetParentObject(0);
            gameObject.SetTag(std::string {});

            m_ActiveObjects.push_back(&gameObject);
            return &gameObject;
        }
    }
    return nullptr;
}

GameObject* SceneLayer::CreateObjectAt(std::string const& objectName, Vector3 position)
{
    for (GameObject& gameObject : m_GameObjects)
    {
        if (!gameObject.m_IsInitialized)
        {
            gameObject.m_Name = objectName;
            gameObject.m_IsActive = true;
            gameObject.m_IsInitialized = true;

            Transform* objectTransform = gameObject.AddComponent<Transform>();
            // objectTransform->SetLocalPosition(position);

            m_ActiveObjects.push_back(&gameObject);
            return &gameObject;
        }
    }
    return nullptr;
}

GameObject* SceneLayer::CreateObjectWithId(std::string const& objectName, unsigned id)
{
    GameObject& gameObject = m_GameObjects[id];

    if (!gameObject.m_IsInitialized)
    {
        gameObject.m_Name = objectName;
        gameObject.m_IsActive = true;
        gameObject.m_IsInitialized = true;

        m_ActiveObjects.push_back(&gameObject);
        return &gameObject;
    }

    return nullptr;
}

GameObjectList SceneLayer::GetObjectsList()
{
    return m_ActiveObjects;
}

GameObjectList SceneLayer::GetObjectsByName(std::string const& objectName)
{
    GameObjectList gameObjects;

    for (GameObject* gameObject : m_ActiveObjects)
    {
        if (gameObject->m_Name == objectName) gameObjects.push_back(gameObject);
    }

    return gameObjects;
}

GameObjectList SceneLayer::GetObjectsById(std::vector<unsigned> const& objectIndices)
{
    GameObjectList gameObjects;

    for (unsigned index : objectIndices)
    {
        if (index < 500 && m_GameObjects[index].GetActive()) gameObjects.push_back(&m_GameObjects[index]);
    }

    return gameObjects;
}

GameObject* SceneLayer::GetObjectByName(std::string const& objectName)
{
    for (GameObject* gameObject : m_ActiveObjects)
    {
        if (gameObject->m_Name == objectName) return gameObject;
    }
    return nullptr;
}

GameObjectList SceneLayer::GetObjectByStrMatch(std::string const& objectName, unsigned startpos)
{
    GameObjectList gameObjects;

    for (GameObject* gameObject : m_ActiveObjects)
    {
        if (gameObject->m_Name.compare(startpos, objectName.size(), objectName) == 0) gameObjects.push_back(gameObject);
    }

    return gameObjects;
}

std::vector<unsigned> SceneLayer::GetObjectsByTag(std::string const& tag)
{
    std::vector<unsigned> gameObjects;
    for (GameObject* gameObject : m_ActiveObjects)
    {
        if (gameObject->m_Tag == tag) gameObjects.push_back(gameObject->m_Id);
    }
    return gameObjects;
}

GameObjectList SceneLayer::GetObjectListByTag(std::string const& tag)
{
    GameObjectList gameObjects;
    for (GameObject* gameObject : m_ActiveObjects)
    {
        if (gameObject->m_Tag == tag) gameObjects.push_back(gameObject);
    }
    return gameObjects;
}

GameObject* SceneLayer::GetObjectById(unsigned id)
{
    if (id >= m_GameObjects.size()) return nullptr;
    return &m_GameObjects[id];
}

Scene* SceneLayer::GetParentScene()
{
    return m_ParentScene;
}

RenderLayer& SceneLayer::GetRenderLayer()
{
    return m_RenderLayer;
}

unsigned SceneLayer::GetId() const
{
    return m_Id;
}

std::string const& SceneLayer::GetName() const
{
    return m_Name;
}

void SceneLayer::SetName(std::string const& name)
{
    m_Name = name;
}

EditorCamera* SceneLayer::GetEditorCamera()
{
    return m_EditorCamera;
}
