#include "SpotLight.h"

#include "GameObject.h"

SpotLight::SpotLight(GameObject* parentObject)
    : LightBase(parentObject, "SpotLight")
    , m_Direction(0.0f, -1.0f, 0.0f)
    , m_InnerAngle(10.0f)
    , m_OuterAngle(20.0f)
    , m_FalloffExponent(1.0f)
{
    ASSERT(m_Transform);
    BuildShadowMap();
}

SpotLight::~SpotLight()
{
#ifndef USE_VULKAN
    glDeleteTextures(1, &m_ShadowMap);
#endif // USE_VULKAN
}

void SpotLight::BuildShadowMap()
{
#ifndef USE_VULKAN
    if (m_ShadowMap == 0)
    {
        glDeleteTextures(1, &m_ShadowMap);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &m_ShadowMap);
    glTextureStorage2D(m_ShadowMap, 1, GL_DEPTH_COMPONENT32F, m_ShadowResolution, m_ShadowResolution);
    glTextureParameteri(m_ShadowMap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_ShadowMap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_ShadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(m_ShadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTextureParameterfv(m_ShadowMap, GL_TEXTURE_BORDER_COLOR, borderColor);
#endif // USE_VULKAN
}

std::vector<Matrix4> SpotLight::GetLightViewProjMatricies()
{
    Vector3 dir = m_Direction.Normalized();
    Vector3 up = dir[1] == 1 ? Vector3(0.0f, 0.0f, -1.0f) : dir[1] == -1 ? Vector3(0.0f, 0.0f, 1.0f) : Vector3(0.0f, 1.0f, 0.0f);

    return std::vector<Matrix4> { Matrix4::Perspective(m_OuterAngle * 2, 1, 0.5, GetShadowDistance()) * Matrix4::LookAt(m_Transform->GetWorldPosition(), m_Transform->GetWorldPosition() + dir, up) };
}

REFLECT_INIT(SpotLight)
REFLECT_PARENT(LightBase)
REFLECT_PROPERTY(m_Direction)
REFLECT_PROPERTY(m_InnerAngle)
REFLECT_PROPERTY(m_OuterAngle)
REFLECT_PROPERTY(m_FalloffExponent)
REFLECT_END()