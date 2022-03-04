#include "LightBase.h"

#include "GameObject.h"
#include "Layer.h"

LightBase::LightBase(GameObject* owner, char const* componentName)
    : IComponent(owner, componentName)
    , m_Transform(owner->GetComponent<Transform>())
    , m_Intensity(1.0f)
    , m_LightColor(1.0f, 1.0f, 1.0f)
    , m_CastShadows(true)
    , m_ShadowResolution(1024)
    , m_ShadowResolutionPrev(m_ShadowResolution)
    ,
    // m_ShadowDistance(100.0f),
    m_ShadowBias(0.0f)
    , m_ShadowMap(0)
{
}

LightBase::~LightBase() {}

Transform* LightBase::GetTransform() const
{
    return m_Transform;
}

void LightBase::OnActive()
{
    m_OwnerObject->GetParentLayer()->GetRenderLayer().AddLight(this);
}

void LightBase::OnInactive()
{
    m_OwnerObject->GetParentLayer()->GetRenderLayer().RemoveLight(this);
}

float LightBase::GetIntensity() const
{
    return m_Intensity;
}

void LightBase::SetIntensity(float intensity)
{
    m_Intensity = intensity;
}

Color3 LightBase::GetLightColor() const
{
    return m_LightColor;
}

void LightBase::SetLightColor(Color3 const& lightColor)
{
    m_LightColor = lightColor;
}

bool LightBase::IsCastShadows() const
{
    return m_CastShadows;
}

void LightBase::SetCastShadows(bool flag)
{
    if (!m_CastShadows && flag)
    {
    }

    m_CastShadows = flag;
}

GLsizei LightBase::GetShadowResolution() const
{
    return m_ShadowResolution;
}

float LightBase::GetShadowDistance() const
{
    return std::sqrtf(4 * (256 * std::max({m_LightColor.r, m_LightColor.g, m_LightColor.b}) * m_Intensity)) * 0.5f;
}

// void LightBase::SetShadowDistance(float dist)
//{
//  m_ShadowDistance = dist;
//}

float LightBase::GetShadowBias() const
{
    return m_ShadowBias;
}

void LightBase::SetShadowBias(float bias)
{
    m_ShadowBias = bias;
}

GLuint LightBase::GetShadowMapTexture()
{
    //#ifdef EDITOR_ENABLED
    // Detect change in shadow resolution size. Should only change in editor mode.
    if (m_ShadowResolution != m_ShadowResolutionPrev)
    {
        this->BuildShadowMap();
        m_ShadowResolutionPrev = m_ShadowResolution;
    }
    //#endif
    return m_ShadowMap;
}

// REFLECT_VIRTUAL(LightBase)
// REFLECT_PARENT(IComponent)
// REFLECT_PROPERTY(m_Intensity)
// REFLECT_PROPERTY(m_LightColor)
// REFLECT_PROPERTY(m_CastShadows)
// REFLECT_PROPERTY(m_ShadowResolution)
// REFLECT_PROPERTY(m_ShadowDistance)
// REFLECT_PROPERTY(m_ShadowBias)
// REFLECT_END()