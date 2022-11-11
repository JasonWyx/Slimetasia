#pragma once
#include "LightBase.h"

class DirectionalLight : public LightBase
{
private:

    Vector3 m_Direction;
    float m_ShadowDistance;

public:

    DirectionalLight(GameObject* parentObject);
    ~DirectionalLight();

    Vector3 GetDirection() const;
    void SetDirection(Vector3 const& direction);

    // Inherited via LightBase
    void BuildShadowMap() override;
    float GetShadowDistance() const override;
    std::vector<Matrix4> GetLightViewProjMatricies() override;

    REFLECT()
};