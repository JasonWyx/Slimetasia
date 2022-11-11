#pragma once
#include <string>
#include <vector>

#include "CorePrerequisites.h"
#include "EditorCamera.h"
#include "RenderLayer.h"

class Scene;
class Camera;
class GameObject;
class MeshRenderer;
class LightBase;
class PointLight;
class SpotLight;
class DirectionalLight;

class Layer
{
    friend class GameObject;

    unsigned m_Id;
    std::string m_Name;
    Scene* m_ParentScene;
    bool m_IsFlaggedForDestroy;

    EditorCamera* m_EditorCamera;

    std::vector<GameObject> m_GameObjects;
    GameObjectList m_ActiveObjects;
    GameObjectList m_CreatedObjects;

    RenderLayer m_RenderLayer;

public:

    Layer(Scene* parentScene, unsigned id, std::string const& layerName, bool createCamera, unsigned objectPoolSize = 1 << 12);
    ~Layer();

    void Update(float dt);
    void PostFrameUpdate();
    void Destroy();

    void Clone(Layer* ly);

    // Creates an empty object
    GameObject* CreateObject(std::string const& objectName);

    // Creates an object with transform at specific position
    GameObject* CreateObjectAt(std::string const& objectName, Vector3 position);

    // Create Objects with id instead
    GameObject* CreateObjectWithId(std::string const& objectName, unsigned id);

    GameObjectList GetObjectsList();
    GameObjectList GetObjectsByName(std::string const& objectName);
    GameObjectList GetObjectsById(std::vector<unsigned> const& objectIndices);
    GameObjectList GetObjectByStrMatch(std::string const& objectName, unsigned startpos = 0);
    std::vector<unsigned> GetObjectsByTag(std::string const& tag);
    GameObjectList GetObjectListByTag(std::string const& tag);

    GameObject* GetObjectByName(std::string const& objectName);
    GameObject* GetObjectById(unsigned id);

    Scene* GetParentScene();

    EditorCamera* GetEditorCamera();

    RenderLayer& GetRenderLayer();

    unsigned GetId() const;
    const std::string& GetName() const;
    void SetName(std::string const& name);
};
