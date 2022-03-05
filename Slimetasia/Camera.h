#pragma once
#include <GL/glew.h>

#include "CorePrerequisites.h"
#include "Frustum.h"
#include "IComponent.h"
#include "Transform.h"
#include "SmartEnums.h"

#define CameraProjectionMode_List(m) m(CameraProjectionMode, Orthographic) m(CameraProjectionMode, Perspective)

SMARTENUM_DEFINE_ENUM(CameraProjectionMode, CameraProjectionMode_List)
SMARTENUM_DEFINE_NAMES(CameraProjectionMode, CameraProjectionMode_List)
SMARTENUM_DEFINE_GET_VALUE_FROM_STRING(CameraProjectionMode)
SMARTENUM_DEFINE_GET_ALL_VALUES_IN_STRING(CameraProjectionMode)
REFLECT_ENUM(CameraProjectionMode)

class Camera : public IComponent
{
protected:
    Transform* m_Transform;  // Only position is used
    CameraProjectionMode m_ProjectionMode;
    float m_OrthoVerticalSize;

    bool m_IsMainCamera;
    iVector2 m_PrevViewportSize;
    iVector2 m_ViewportSize;
    iVector2 m_ViewportOffset;
    float m_FieldOfView;  // Vertical fov
    float m_NearPlane;
    float m_FarPlane;

    Color3 m_FogColor;
    Vector2 m_FogAttenuation;
    Color3 m_AmbientColor;
    Vector3 m_LightAttenuation;
    float m_Gamma;
    float m_Exposure;
    HTexture m_SkyboxTexture;
    Color4 m_SkyboxColor;
    bool m_EnablePostProcessing;
    Vector3 m_LookAtDirection;
    Vector3 m_CameraUp;

    bool m_EnableBloom;
    bool m_EnableSSAO;

    GLuint m_RenderTarget;
    bool m_IsReflectionView;
    float m_ReflectionHeight;

public:
    Camera(GameObject* parentObject, char const* componentName = "Camera");
    ~Camera();

    Transform* GetTransform();

    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void RevalidateResources() override;

    bool IsMainCamera() const;
    bool IsUICamera() const;

    void SetViewportOffset(iVector2 const& viewportOffset);
    void SetViewportSize(iVector2 const& viewportSize, bool rebuildTextures = true);
    void SetFieldOfView(float const& fieldOfView);
    void SetNearPlane(float const& nearPlane);
    void SetFarPlane(float const& farPlane);
    void SetFogColor(Color3 const& clearColor);
    void SetAmbientColor(Color3 const& ambientColor);
    void SetLightAttenuation(Vector3 const& attenuation);
    void SetFogAttenuation(Vector2 const& attenuation);
    void SetSkyboxTexture(HTexture const& skyboxTexture);
    void SetLookAtDirection(Vector3 const& direction);
    void SetUpDirection(Vector3 const& direction);
    void SetIsReflectionView(const bool& isReflectionView);
    void SetReflectionHeight(const float& height);

    iVector2 GetViewportOffset() const;
    iVector2 GetViewportSize() const;
    float GetFieldOfView() const;
    float GetNearPlane() const;
    float GetFarPlane() const;
    Color3 GetFogColor() const;
    Color3 GetAmbientColor() const;
    Vector3 GetLightAttenuation() const;
    Vector2 GetFogAttenuation() const;
    HTexture GetSkyboxTexture() const;
    Vector3 GetLookAtDirection() const;
    Color4 GetSkyboxColor() const;
    float GetGamma() const;
    float GetExposure() const;
    bool GetIsReflectionView() const;
    float GetReflectionHeight() const;

    bool IsPostProcessingEnabled() const;
    bool IsBloomEnabled() const;
    bool IsSSAOEnabled() const;
    GLuint GetRenderTarget();
    Matrix4 GetViewProjTransform() const;
    Matrix4 GetViewTransform() const;
    Matrix4 GetProjTransform() const;

    Vector2 WorldToScreen(Vector3 const& worldPosition);
    Vector3 ScreenToWorld(Vector2 const& screenPosition);
    std::vector<Vector3> GetFrustumPoints() const;

    bool m_IsUICamera;

    REFLECT()
};