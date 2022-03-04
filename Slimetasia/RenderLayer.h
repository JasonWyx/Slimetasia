#pragma once
#include <GL\glew.h>

#include "Camera.h"
#include "DirectionalLight.h"
#include "DynamicAABBTree.h"
#include "MeshRenderer.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "TextRenderer.h"
#include "VideoPlayer.h"
#include "WaterSimulator.h"

class RenderLayer
{
public:
    RenderLayer();
    ~RenderLayer();

    void AddMeshRenderer(MeshRenderer* meshRenderer);
    void RemoveMeshRenderer(MeshRenderer* meshRenderer);

    void AddTextRenderer(TextRenderer* textRenderer);
    void RemoveTextRenderer(TextRenderer* textRenderer);

    void AddLight(LightBase* light);
    void RemoveLight(LightBase* light);

    void AddCamera(Camera* camera);
    void RemoveCamera(Camera* camera);

    void AddWaterSimulator(WaterSimulator* waterSimulator);
    void RemoveWaterSimulator(WaterSimulator* waterSimulator);

    void AddVideoPlayer(VideoPlayer* videoPlayer);
    void RemoveVideoPlayer(VideoPlayer* videoPlayer);

    const std::vector<Camera*>& GetCameras() const;
    const std::vector<MeshRenderer*>& GetMeshRenderers() const;
    const std::vector<TextRenderer*>& GetTextRenderers() const;
    const std::vector<PointLight*>& GetPointLights() const;
    const std::vector<SpotLight*>& GetSpotLights() const;
    const std::vector<DirectionalLight*>& GetDirectionalLights() const;
    const std::vector<WaterSimulator*>& GetWaterSimulators() const;
    const std::vector<VideoPlayer*>& GetVideoPlayers() const;

    static std::vector<MeshRenderer*> GetCulledMeshRenderers(const RenderLayer& renderLayer, const Camera& camera);
    static std::vector<SpotLight*> GetCulledSpotLights(const RenderLayer& renderLayer, const Camera& camera);
    static std::vector<PointLight*> GetCulledPointLights(const RenderLayer& renderLayer, const Camera& camera);

    void UpdateAabbTree();

private:
    // Cameras
    std::vector<Camera*> m_Cameras;
    // Renderables
    std::vector<MeshRenderer*> m_MeshRenderers;
    std::vector<TextRenderer*> m_TextRenderers;
    // Lights
    std::vector<PointLight*> m_PointLights;
    std::vector<SpotLight*> m_SpotLights;
    std::vector<DirectionalLight*> m_DirectionalLights;
    // Misc
    std::vector<WaterSimulator*> m_WaterSimulators;
    std::vector<VideoPlayer*> m_VideoPlayers;

    DynamicAabbTree<MeshRenderer> m_MeshTree;
    DynamicAabbTree<SpotLight> m_SpotLightTree;
    DynamicAabbTree<PointLight> m_PointLightTree;
};
