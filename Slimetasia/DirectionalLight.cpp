#include "DirectionalLight.h"

#include "GameObject.h"

DirectionalLight::DirectionalLight(GameObject* parentObject)
    : LightBase(parentObject, "DirectionalLight")
    , m_Direction(0.0f, -1.0f, 0.0f)
    , m_ShadowDistance(100.0f)
{
    BuildShadowMap();
}

DirectionalLight::~DirectionalLight()
{
    glDeleteTextures(1, &m_ShadowMap);
}

Vector3 DirectionalLight::GetDirection() const
{
    return m_Direction;
}

void DirectionalLight::SetDirection(Vector3 const& direction)
{
    m_Direction = direction;
}

void DirectionalLight::BuildShadowMap()
{
#ifndef USE_VULKAN_RENDERER
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
#endif // USE_VULKAN_RENDERER
}

float DirectionalLight::GetShadowDistance() const
{
    return 0.0f;
}

std::vector<Matrix4> DirectionalLight::GetLightViewProjMatricies()
{
    Vector3 pos = m_Transform->GetWorldPosition();
    Vector3 dirNorm = m_Direction.Normalized();
    Vector3 up = dirNorm.y == 1 ? Vector3(0.0f, 0.0f, -1.0f) : dirNorm.y == -1 ? Vector3(0.0f, 0.0f, 1.0f) : Vector3(0.0f, 1.0f, 0.0f);

    return std::vector<Matrix4> { Matrix4::SetFrustumOrtho(-m_ShadowDistance, m_ShadowDistance, -m_ShadowDistance, m_ShadowDistance, -m_ShadowDistance, m_ShadowDistance) *
                                  Matrix4::LookAt(pos, pos + dirNorm, up) };
}

REFLECT_VIRTUAL(LightBase)
REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(m_Intensity)
REFLECT_PROPERTY(m_LightColor)
REFLECT_PROPERTY(m_CastShadows)
REFLECT_PROPERTY(m_ShadowResolution)
REFLECT_PROPERTY(m_ShadowBias)
REFLECT_END()

REFLECT_INIT(DirectionalLight)
REFLECT_PARENT(LightBase)
REFLECT_PROPERTY(m_Direction)
REFLECT_PROPERTY(m_ShadowDistance)
REFLECT_END()