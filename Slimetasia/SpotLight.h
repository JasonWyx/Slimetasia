#pragma once
#include "LightBase.h"

class SpotLight : public LightBase
{
private:
    Vector3 m_Direction;
    float m_InnerAngle;
    float m_OuterAngle;
    float m_FalloffExponent;

public:
    SpotLight(GameObject* parentObject);
    ~SpotLight();

    Vector3 GetDirection() const;
    void SetDirection(Vector3 const& direction);

    float GetInnerAngle() const;
    void SetInnerAngle(float angle);

    float GetOuterAngle() const;
    void SetOuterAngle(float angle);

    float GetFalloffExponent() const;
    void SetFalloffExponent(float exponent);

    // Inherited via LightBase
    virtual void BuildShadowMap() override;
    virtual std::vector<Matrix4> GetLightViewProjMatricies() override;

    REFLECT()
};