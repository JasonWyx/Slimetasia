#pragma once
#include <GL/glew.h>

#include "IComponent.h"
#include "MathDefs.h"
#include "Transform.h"

class LightBase : public IComponent
{
protected:
    Transform* m_Transform;

    float m_Intensity;
    Color3 m_LightColor;

    bool m_CastShadows;
    GLsizei m_ShadowResolution;
    GLsizei m_ShadowResolutionPrev;
    // float   m_ShadowDistance;
    float m_ShadowBias;
    GLuint m_ShadowMap;

public:
    LightBase(GameObject* owner, char const* componentName = "LightBase");
    virtual ~LightBase();

    Transform* GetTransform() const;

    void OnActive() override;
    void OnInactive() override;

    float GetIntensity() const;
    void SetIntensity(float intensity);

    Color3 GetLightColor() const;
    void SetLightColor(Color3 const& lightColor);

    bool IsCastShadows() const;
    void SetCastShadows(bool flag);

    GLsizei GetShadowResolution() const;
    virtual void BuildShadowMap() = 0;

    virtual float GetShadowDistance() const;
    // void          SetShadowDistance(float dist);

    float GetShadowBias() const;
    void SetShadowBias(float bias);

    GLuint GetShadowMapTexture();

    virtual std::vector<Matrix4> GetLightViewProjMatricies() { return std::vector<Matrix4>(); }

    REFLECT()
};