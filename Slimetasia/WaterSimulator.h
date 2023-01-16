#pragma once
#include <gl\glew.h>

#include "CorePrerequisites.h"
#include "IComponent.h"
#include "Transform.h"

class WaterSimulator : public IComponent
{
public:

    float m_TilingFactor;
    float m_FresnelPower;
    float m_PlaneSize;
    Color3 m_WaterColor;
    float m_WaveSpeed;
    float m_WaveStrength;
    float m_WaterDepth;
    float m_TextureResolutionScale;

    WaterSimulator(GameObject* parentObject, const char* componentName = "WaterSimulator");
    ~WaterSimulator();

    // Inherited members
    void OnActive() override;
    void OnInactive() override;
    void OnUpdate(float dt) override;

    Transform* GetTransform() { return m_Transform; }
    GLuint GetReflectionTexture() const { return m_ReflectionTexture; }
    GLuint GetRefractionTexture() const { return m_RefractionTexture; }
    float GetWaveFactor() const { return m_WaveFactor; }
    void GenerateTextures(const iVector2& viewportSize);

private:

    Transform* m_Transform;
    iVector2 m_ViewportSize;
    GLuint m_ReflectionTexture;
    GLuint m_RefractionTexture;
    float m_WaveFactor;

    REFLECT();
};
