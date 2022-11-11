#pragma once
#include "CorePrerequisites.h"

class Scene
{
    friend class Application;

    std::string m_Name;

    LayerList m_Layers;
    int m_LayerCount;

public:

    Scene(char const* sceneName);
    // Scene(Scene const &otherScene);
    ~Scene();

    // Remove copy assignments
    Scene(Scene&&) = delete;
    Scene operator=(Scene&&) = delete;
    Scene operator=(Scene const&) = delete;

    void Update(float dt);
    void PostFrameUpdate();

    Layer* CreateLayer(std::string const& layerName);
    Layer* CreateLayerWithoutCamera(std::string const& layerName);
    Layer* GetLayerByName(std::string const& layerName) const;
    LayerList GetLayers() const;
    std::string GetName() const;
    void DeleteLayer(std::string const& layerName);
    void ClearLayer();
};
