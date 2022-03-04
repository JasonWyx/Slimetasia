#include "RenderLayer.h"

#include "GameObject.h"

RenderLayer::RenderLayer()
    : m_MeshRenderers()
    , m_PointLights()
    , m_SpotLights()
    , m_DirectionalLights()
{
}

RenderLayer::~RenderLayer() {}

void RenderLayer::AddMeshRenderer(MeshRenderer* meshRenderer)
{
    m_MeshRenderers.push_back(meshRenderer);

    Transform* meshTransform = meshRenderer->GetTransform();
    AABB aabb = meshRenderer->GetMesh().Validate() ? meshRenderer->GetMesh()->GetAABB() : AABB(Vector3(0.0f), Vector3(0.0f));

    if (meshRenderer->GetMesh().Validate())
    {
        aabb.Transform(meshTransform->m_WorldScale, Matrix3::Rotate(meshTransform->m_WorldRotation), meshTransform->m_WorldPosition);
    }

    m_MeshTree.InsertData(meshRenderer, aabb);
}

void RenderLayer::RemoveMeshRenderer(MeshRenderer* meshRenderer)
{
    m_MeshRenderers.erase(std::remove(m_MeshRenderers.begin(), m_MeshRenderers.end(), meshRenderer), m_MeshRenderers.end());

    m_MeshTree.RemoveData(meshRenderer);
}

void RenderLayer::AddTextRenderer(TextRenderer* textRenderer)
{
    m_TextRenderers.push_back(textRenderer);
}

void RenderLayer::RemoveTextRenderer(TextRenderer* textRenderer)
{
    m_TextRenderers.erase(std::remove(m_TextRenderers.begin(), m_TextRenderers.end(), textRenderer), m_TextRenderers.end());
}

void RenderLayer::AddCamera(Camera* camera)
{
    m_Cameras.push_back(camera);
}

void RenderLayer::RemoveCamera(Camera* camera)
{
    auto it = std::find(m_Cameras.begin(), m_Cameras.end(), camera);
    if (it != m_Cameras.end()) m_Cameras.erase(it);
}

void RenderLayer::AddWaterSimulator(WaterSimulator* waterSimulator)
{
    m_WaterSimulators.push_back(waterSimulator);
}

void RenderLayer::RemoveWaterSimulator(WaterSimulator* waterSimulator)
{
    m_WaterSimulators.erase(std::remove(m_WaterSimulators.begin(), m_WaterSimulators.end(), waterSimulator), m_WaterSimulators.end());
}

void RenderLayer::AddVideoPlayer(VideoPlayer* videoPlayer)
{
    m_VideoPlayers.push_back(videoPlayer);
}

void RenderLayer::RemoveVideoPlayer(VideoPlayer* videoPlayer)
{
    m_VideoPlayers.erase(std::remove(m_VideoPlayers.begin(), m_VideoPlayers.end(), videoPlayer), m_VideoPlayers.end());
}

void RenderLayer::AddLight(LightBase* light)
{
    if (!light->GetOwner()->GetParentLayer()) return;

    if (PointLight* pointLight = dynamic_cast<PointLight*>(light))
    {
        m_PointLights.push_back(pointLight);

        Transform* lightTransform = pointLight->GetTransform();
        AABB aabb(Vector3(-pointLight->GetShadowDistance()), Vector3(pointLight->GetShadowDistance()));
        aabb.Transform(lightTransform->m_WorldScale, Matrix3::Rotate(lightTransform->m_WorldRotation), lightTransform->m_WorldPosition);
        m_PointLightTree.InsertData(pointLight, aabb);
    }
    else if (SpotLight* spotLight = dynamic_cast<SpotLight*>(light))
    {
        m_SpotLights.push_back(spotLight);

        Transform* lightTransform = spotLight->GetTransform();
        AABB aabb(Vector3(-spotLight->GetShadowDistance()), Vector3(spotLight->GetShadowDistance()));
        aabb.Transform(lightTransform->m_WorldScale, Matrix3::Rotate(lightTransform->m_WorldRotation), lightTransform->m_WorldPosition);
        m_SpotLightTree.InsertData(spotLight, aabb);
    }
    else if (DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(light))
    {
        m_DirectionalLights.push_back(directionalLight);
    }
    else
    {
        p_assert(false);  // Invalid light type
    }
}

void RenderLayer::RemoveLight(LightBase* light)
{
    if (!light->GetOwner()->GetParentLayer()) return;

    if (PointLight* pointLight = dynamic_cast<PointLight*>(light))
    {
        m_PointLights.erase(std::remove(m_PointLights.begin(), m_PointLights.end(), pointLight), m_PointLights.end());
        m_PointLightTree.RemoveData(pointLight);
    }
    else if (SpotLight* spotLight = dynamic_cast<SpotLight*>(light))
    {
        m_SpotLights.erase(std::remove(m_SpotLights.begin(), m_SpotLights.end(), spotLight), m_SpotLights.end());
        m_SpotLightTree.RemoveData(spotLight);
    }
    else if (DirectionalLight* dirLight = dynamic_cast<DirectionalLight*>(light))
    {
        m_DirectionalLights.erase(std::remove(m_DirectionalLights.begin(), m_DirectionalLights.end(), dirLight), m_DirectionalLights.end());
    }
    else
    {
        p_assert(false);  // Invalid light type
    }
}

const std::vector<Camera*>& RenderLayer::GetCameras() const
{
    return m_Cameras;
}

const std::vector<MeshRenderer*>& RenderLayer::GetMeshRenderers() const
{
    return m_MeshRenderers;
}

const std::vector<TextRenderer*>& RenderLayer::GetTextRenderers() const
{
    return m_TextRenderers;
}

const std::vector<PointLight*>& RenderLayer::GetPointLights() const
{
    return m_PointLights;
}

const std::vector<SpotLight*>& RenderLayer::GetSpotLights() const
{
    return m_SpotLights;
}

const std::vector<DirectionalLight*>& RenderLayer::GetDirectionalLights() const
{
    return m_DirectionalLights;
}

const std::vector<WaterSimulator*>& RenderLayer::GetWaterSimulators() const
{
    return m_WaterSimulators;
}

const std::vector<VideoPlayer*>& RenderLayer::GetVideoPlayers() const
{
    return m_VideoPlayers;
}

std::vector<MeshRenderer*> RenderLayer::GetCulledMeshRenderers(const RenderLayer& renderLayer, const Camera& camera)
{
    Frustum frustum;
    std::vector<Vector3> fp = camera.GetFrustumPoints();
    frustum.Set(fp[0], fp[1], fp[2], fp[3], fp[4], fp[5], fp[6], fp[7]);

    // frustum.DebugDraw(const_cast<Camera&>(camera).GetParentObject()->GetParentLayer()->GetId());

    std::vector<MeshRenderer*> results;
    results.reserve(renderLayer.m_MeshRenderers.size());
    renderLayer.m_MeshTree.CastFrustum(frustum, results);

    return results;
}

std::vector<SpotLight*> RenderLayer::GetCulledSpotLights(const RenderLayer& renderLayer, const Camera& camera)
{
    Frustum frustum;
    std::vector<Vector3> fp = camera.GetFrustumPoints();
    frustum.Set(fp[0], fp[1], fp[2], fp[3], fp[4], fp[5], fp[6], fp[7]);

    std::vector<SpotLight*> results;
    results.reserve(renderLayer.m_SpotLights.size());
    renderLayer.m_SpotLightTree.CastFrustum(frustum, results);
    return results;
}

std::vector<PointLight*> RenderLayer::GetCulledPointLights(const RenderLayer& renderLayer, const Camera& camera)
{
    Frustum frustum;
    std::vector<Vector3> fp = camera.GetFrustumPoints();
    frustum.Set(fp[0], fp[1], fp[2], fp[3], fp[4], fp[5], fp[6], fp[7]);

    std::vector<PointLight*> results;
    results.reserve(renderLayer.m_PointLights.size());
    renderLayer.m_PointLightTree.CastFrustum(frustum, results);

    // frustum.DebugDraw(const_cast<Camera&>(camera).GetParentObject()->GetParentLayer()->GetId());

    return results;
}

void RenderLayer::UpdateAabbTree()
{
    for (MeshRenderer* meshRenderer : m_MeshRenderers)
    {
        Transform* meshTransform = meshRenderer->GetTransform();
        AABB aabb = meshRenderer->GetMesh().Validate() ? meshRenderer->GetMesh()->GetAABB() : AABB(Vector3(0.0f), Vector3(0.0f));

        if (meshRenderer->GetMesh().Validate())
        {
            aabb.Transform(meshTransform->m_WorldScale, Matrix3::Rotate(meshTransform->m_WorldRotation), meshTransform->m_WorldPosition);
        }

        m_MeshTree.UpdateData(meshRenderer, aabb);
    }

    for (PointLight* pointLight : m_PointLights)
    {
        Transform* lightTransform = pointLight->GetTransform();
        AABB aabb(Vector3(-pointLight->GetShadowDistance()), Vector3(pointLight->GetShadowDistance()));
        aabb.Transform(lightTransform->m_WorldScale, Matrix3::Rotate(lightTransform->m_WorldRotation), lightTransform->m_WorldPosition);
        m_PointLightTree.UpdateData(pointLight, aabb);
    }

    for (SpotLight* spotLight : m_SpotLights)
    {
        Transform* lightTransform = spotLight->GetTransform();
        AABB aabb(Vector3(-spotLight->GetShadowDistance()), Vector3(spotLight->GetShadowDistance()));
        aabb.Transform(lightTransform->m_WorldScale, Matrix3::Rotate(lightTransform->m_WorldRotation), lightTransform->m_WorldPosition);
        m_SpotLightTree.UpdateData(spotLight, aabb);
    }

    // m_MeshTree.DebugDraw(0);
    // m_PointLightTree.DebugDraw(0);
    // m_SpotLightTree.DebugDraw(0);
}
