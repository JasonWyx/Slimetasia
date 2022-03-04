#pragma once
#include "LightBase.h"

class PointLight : public LightBase
{
public:
    PointLight(GameObject* parentObject);
    ~PointLight();

    // Inherited via LightBase
    virtual float GetShadowDistance() const override;
    virtual void BuildShadowMap() override;
    virtual std::vector<Matrix4> GetLightViewProjMatricies() override;

    REFLECT()
};