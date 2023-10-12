#include "Scene.h"

#include "Layer.h"

Scene::Scene(char const* sceneName)
    : m_Name(sceneName)
    , m_Layers()
    , m_LayerCount(0)
{
}

/*Scene::Scene(Scene const & otherScene)
{
  for (SceneLayer *layer : otherScene.m_Layers)
  {
    // TODO: Layer copy
    //SceneLayer *copyLayer = CreateLayer(layer->GetName());
  }
}*/

Scene::~Scene()
{
    for (SceneLayer* layer : m_Layers)
        delete layer;
}

void Scene::Update(float dt)
{
    for (SceneLayer* layer : m_Layers)
        layer->Update(dt);
}

void Scene::PostFrameUpdate()
{
    // for (SceneLayer *layer : m_Layers)
    //  layer->PostFrameUpdate();
}

SceneLayer* Scene::CreateLayer(std::string const& layerName)
{
    for (SceneLayer* layer : m_Layers)
    {
        if (layer->GetName() == layerName)
        {
            std::cout << "ERROR: Layer with name '" << layerName << "' already exists!" << std::endl;
            return nullptr;
        }
    }

    SceneLayer* newLayer = new SceneLayer(this, ++m_LayerCount, layerName, true);
    m_Layers.push_back(newLayer);
    return newLayer;
}

SceneLayer* Scene::CreateLayerWithoutCamera(std::string const& layerName)
{
    for (SceneLayer* layer : m_Layers)
    {
        if (layer->GetName() == layerName)
        {
            std::cout << "ERROR: Layer with name '" << layerName << "' already exists!" << std::endl;
            return nullptr;
        }
    }

    SceneLayer* newLayer = new SceneLayer(this, ++m_LayerCount, layerName, false);
    m_Layers.push_back(newLayer);
    return newLayer;
}

SceneLayer* Scene::GetLayerByName(std::string const& layerName) const
{
    for (SceneLayer* layer : m_Layers)
    {
        if (layer->GetName() == layerName)
        {
            return layer;
        }
    }
    return nullptr;
}

LayerList Scene::GetLayers() const
{
    return m_Layers;
}

std::string Scene::GetName() const
{
    return m_Name;
}

void Scene::DeleteLayer(std::string const& layerName)
{
    auto start = m_Layers.begin();
    while (start != m_Layers.end())
    {
        if ((*start)->GetName() == layerName)
        {
            delete *start;
            m_Layers.erase(start);
            return;
        }
        ++start;
    }
}

void Scene::ClearLayer()
{
    auto start = m_Layers.begin();
    while (start != m_Layers.end())
    {
        delete *start;
        ++start;
    }
    m_Layers.clear();
}
