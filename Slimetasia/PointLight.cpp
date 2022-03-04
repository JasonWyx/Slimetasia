#include "PointLight.h"

#include "GameObject.h"

PointLight::PointLight(GameObject* parentObject)
    : LightBase(parentObject, "PointLight")
{
    ASSERT(m_Transform);
    BuildShadowMap();
}

PointLight::~PointLight()
{
    glDeleteTextures(1, &m_ShadowMap);
}

float PointLight::GetShadowDistance() const
{
    return 2.0f * std::sqrtf(4 * (256 * std::max({m_LightColor.r, m_LightColor.g, m_LightColor.b}) * m_Intensity)) / 2.0f;
}

void PointLight::BuildShadowMap()
{
    if (m_ShadowMap == 0)
    {
        glDeleteTextures(1, &m_ShadowMap);
    }

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ShadowMap);
    glTextureStorage2D(m_ShadowMap, 1, GL_DEPTH_COMPONENT32F, m_ShadowResolution, m_ShadowResolution);
    glTextureParameteri(m_ShadowMap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_ShadowMap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_ShadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(m_ShadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTextureParameterfv(m_ShadowMap, GL_TEXTURE_BORDER_COLOR, borderColor);
}

std::vector<Matrix4> PointLight::GetLightViewProjMatricies()
{
    std::vector<Matrix4> matrices;

    static Vector3 const directions[] = {
        Vector3(1.0f, 0.0f, 0.0f),   // X+
        Vector3(-1.0f, 0.0f, 0.0f),  // X-
        Vector3(0.0f, 1.0f, 0.0f),   // Y+
        Vector3(0.0f, -1.0f, 0.0f),  // Y-
        Vector3(0.0f, 0.0f, 1.0f),   // Z+
        Vector3(0.0f, 0.0f, -1.0f)   // Z-
    };

    for (int i = 0; i < 6; ++i)
    {
        // Setup directions
        Vector3 dir = directions[i];
        Vector3 up = dir.y == 1 ? Vector3(0.0f, 0.0f, 1.0f) : dir.y == -1 ? Vector3(0.0f, 0.0f, -1.0f) : Vector3(0.0f, -1.0f, 0.0f);

        matrices.emplace_back(Matrix4::Perspective(90.0f, 1.0f, 0.1f, GetShadowDistance()) * Matrix4::LookAt(m_Transform->GetWorldPosition(), m_Transform->GetWorldPosition() + dir, up));
    }

    return matrices;
}

REFLECT_INIT(PointLight)
REFLECT_PARENT(LightBase)
REFLECT_END()