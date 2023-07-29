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
    , m_ShadowBias(0.0f)
#ifdef USE_VULKAN
#else
    , m_ShadowMap(0)
#endif  // USE_VULKAN
{
}

LightBase::~LightBase() {}

void LightBase::OnActive()
{
    m_OwnerObject->GetParentLayer()->GetRenderLayer().AddLight(this);
}

void LightBase::OnInactive()
{
    m_OwnerObject->GetParentLayer()->GetRenderLayer().RemoveLight(this);
}

float LightBase::GetShadowDistance() const
{
    return std::sqrtf(4 * (256 * std::max({ m_LightColor[0], m_LightColor[1], m_LightColor[2] }) * m_Intensity)) * 0.5f;
}

#ifdef USE_VULKAN
#else
GLuint LightBase::GetShadowMapTexture()
{
    // #ifdef EDITOR
    //  Detect change in shadow resolution size. Should only change in editor mode.
    if (m_ShadowResolution != m_ShadowResolutionPrev)
    {
        this->BuildShadowMap();
        m_ShadowResolutionPrev = m_ShadowResolution;
    }
    // #endif
    return m_ShadowMap;
}
#endif  // USE_VULKAN

// REFLECT_VIRTUAL(LightBase)
// REFLECT_PARENT(IComponent)
// REFLECT_PROPERTY(m_Intensity)
// REFLECT_PROPERTY(m_LightColor)
// REFLECT_PROPERTY(m_CastShadows)
// REFLECT_PROPERTY(m_ShadowResolution)
// REFLECT_PROPERTY(m_ShadowDistance)
// REFLECT_PROPERTY(m_ShadowBias)
// REFLECT_END()