#pragma once
#include "LightBase.h"

class SpotLight : public LightBase
{
public:

    SpotLight(GameObject* parentObject);
    ~SpotLight();

    Vector3 GetDirection() const { return m_Direction; }
    void SetDirection(Vector3 const& direction) { m_Direction = direction; }
    float GetInnerAngle() const { return m_InnerAngle; }
    void SetInnerAngle(float angle) { m_InnerAngle = angle; }
    float GetOuterAngle() const { return m_OuterAngle; }
    void SetOuterAngle(float angle) { m_OuterAngle = angle; }
    float GetFalloffExponent() const { return m_FalloffExponent; }
    void SetFalloffExponent(float exponent) { m_FalloffExponent = exponent; }

    // Inherited via LightBase
    void BuildShadowMap() override;
    std::vector<Matrix4> GetLightViewProjMatricies() override;

    REFLECT()

private:

    Vector3 m_Direction;
    float m_InnerAngle;
    float m_OuterAngle;
    float m_FalloffExponent;
};