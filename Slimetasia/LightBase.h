#pragma once
#include <GL/glew.h>

#include "IComponent.h"
#include "MathDefs.h"
#include "Transform.h"

class LightBase : public IComponent
{
public:

    LightBase(GameObject* owner, char const* componentName = "LightBase");
    virtual ~LightBase();

    Transform* GetTransform() const { return m_Transform; }

    void OnActive() override;
    void OnInactive() override;
    float GetIntensity() const { return m_Intensity; }
    void SetIntensity(float intensity) { m_Intensity = intensity; }
    Color3 GetLightColor() const { return m_LightColor; }
    void SetLightColor(Color3 const& lightColor) { m_LightColor = lightColor; }
    bool IsCastShadows() const { return m_CastShadows; }
    void SetCastShadows(bool flag) { m_CastShadows = flag; }
    GLsizei GetShadowResolution() const { return m_ShadowResolution; }
    virtual void BuildShadowMap() = 0;
    virtual float GetShadowDistance() const;

    float GetShadowBias() const { return m_ShadowBias; }
    void SetShadowBias(float bias) { m_ShadowBias = bias; }

    GLuint GetShadowMapTexture();

    virtual std::vector<Matrix4> GetLightViewProjMatricies() { return std::vector<Matrix4>(); }

    REFLECT()

protected:

    Transform* m_Transform;
    float m_Intensity;
    Color3 m_LightColor;
    bool m_CastShadows;
    GLsizei m_ShadowResolution;
    GLsizei m_ShadowResolutionPrev;
    float m_ShadowBias;
    GLuint m_ShadowMap;
};