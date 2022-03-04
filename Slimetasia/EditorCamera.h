#pragma once
#include "Camera.h"
#include "CorePrerequisites.h"

class EditorCamera : public Camera
{
private:
    float m_Latitude;
    float m_Azimuth;

    bool m_IsPanning;
    bool m_IsRotating;
    bool m_IsZooming;
    float m_FocalDistance;
    float m_ZoomDampenFactor;
    bool m_Update;

public:
    EditorCamera(GameObject* parentObject);
    ~EditorCamera() = default;

    void LookAt(GameObject* go);
    void OnUpdate(float dt) override;
    void Reset();
    void Cancel();
    void SetUpdate(bool b) { m_Update = b; }

    REFLECT()
};