#include "WaterSimulator.h"

#include "GameObject.h"
#include "Layer.h"
#include "RenderLayer.h"

REFLECT_INIT(WaterSimulator)
REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(m_TilingFactor)
REFLECT_PROPERTY(m_FresnelPower)
REFLECT_PROPERTY(m_WaveStrength)
REFLECT_PROPERTY(m_WaveSpeed)
REFLECT_PROPERTY(m_WaterDepth)
REFLECT_PROPERTY(m_PlaneSize)
REFLECT_PROPERTY(m_WaterColor)
REFLECT_PROPERTY(m_TextureResolutionScale)
REFLECT_END()

WaterSimulator::WaterSimulator(GameObject* parentObject, const char* componentName)
    : IComponent(parentObject, componentName)
    , m_TilingFactor(10.0f)
    , m_FresnelPower(1.0f)
    , m_PlaneSize(10.0f)
    , m_WaterColor(0.0f)
    , m_WaveSpeed(1.0f)
    , m_WaveStrength(1.0f)
    , m_WaterDepth(5.0f)
    , m_TextureResolutionScale(0.25f)
    , m_Transform(parentObject->GetComponent<Transform>())
    , m_ViewportSize(0)
    , m_ReflectionTexture(GL_NONE)
    , m_RefractionTexture(GL_NONE)
    , m_WaveFactor(0.0f)
{
    ASSERT(m_Transform);
}

WaterSimulator::~WaterSimulator()
{
#ifndef USE_VULKAN_RENDERER
    glDeleteTextures(1, &m_ReflectionTexture);
    glDeleteTextures(1, &m_RefractionTexture);
#endif // USE_VULKAN_RENDERER
}

void WaterSimulator::OnActive()
{
    m_OwnerObject->GetParentLayer()->GetRenderLayer().AddWaterSimulator(this);
}

void WaterSimulator::OnInactive()
{
    m_OwnerObject->GetParentLayer()->GetRenderLayer().RemoveWaterSimulator(this);
}

void WaterSimulator::OnUpdate(float dt)
{
    m_WaveFactor = std::fmodf(m_WaveFactor + dt * m_WaveSpeed, 1.0f);
}

void WaterSimulator::GenerateTextures(const iVector2& viewportSize)
{
    if (viewportSize != m_ViewportSize)
    {
        m_ViewportSize = viewportSize;

#ifndef USE_VULKAN_RENDERER
        glDeleteTextures(1, &m_ReflectionTexture);
        glDeleteTextures(1, &m_RefractionTexture);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_ReflectionTexture);
        glTextureStorage2D(m_ReflectionTexture, 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
        glTextureParameteri(m_ReflectionTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_ReflectionTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_ReflectionTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_ReflectionTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RefractionTexture);
        glTextureStorage2D(m_RefractionTexture, 1, GL_RGBA16F, m_ViewportSize.x, m_ViewportSize.y);
        glTextureParameteri(m_RefractionTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RefractionTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_RefractionTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RefractionTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif USE_VULKAN_RENDERER
    }
}
