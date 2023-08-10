#include "Camera.h"

#include "EditorCamera.h"
#include "GameObject.h"
#include "Renderer.h"
#include "ResourceManager.h"

Camera::Camera(GameObject* parentObject, char const* componentName)
    : IComponent(parentObject, componentName)
    , m_IsMainCamera(true)
    , m_IsUICamera(false)
    , m_ProjectionMode(eCameraProjectionMode_Perspective)
    , m_ViewportSize(0)
    , m_ViewportOffset(0, 0)
    , m_OrthoVerticalSize(10.0f)
    , m_FieldOfView(45.0f)
    , m_NearPlane(1)
    , m_FarPlane(1000)
    , m_FogColor(0.1f, 0.1f, 0.1f)
    , m_FogAttenuation(20.0f, 100.0f)
    , m_AmbientColor(0.0f, 0.0f, 0.0f)
    , m_LightAttenuation(1.0f, 0.0f, 0.0f)
    , m_Gamma(1.0f)
    , m_Exposure(1.0f)
    , m_SkyboxTexture()
    , m_SkyboxColor(1.0f)
    , m_EnablePostProcessing(true)
    , m_Transform(parentObject->GetComponent<Transform>())
    , m_LookAtDirection(0.0f, -1.0f, -1.0f)
    , m_CameraUp(0.0f, 1.0f, 0.0f)
    , m_EnableBloom(false)
    , m_EnableSSAO(false)
    , m_RenderTarget(0)
    , m_IsReflectionView(false)
    , m_ReflectionHeight(0.0f)
{
    ASSERT(m_Transform);
    SetViewportSize(iVector2(1, 1), true);
}

Camera::~Camera()
{
    glDeleteTextures(1, &m_RenderTarget);
}

Transform* Camera::GetTransform()
{
    return m_Transform;
}

void Camera::OnActive()
{
    if (!dynamic_cast<EditorCamera*>(this)) m_OwnerObject->GetParentLayer()->GetRenderLayer().AddCamera(this);
}

void Camera::OnInactive()
{
    if (!dynamic_cast<EditorCamera*>(this))
    {
        auto parent_layer = m_OwnerObject->GetParentLayer();
        if (parent_layer != nullptr)
        {
            parent_layer->GetRenderLayer().RemoveCamera(this);
        }
        // m_OwnerObject->GetParentLayer()->GetRenderLayer().RemoveCamera(this);
    }
}

void Camera::RevalidateResources()
{
    // m_SkyboxTexture.Validate();
}

bool Camera::IsMainCamera() const
{
    return m_IsMainCamera;
}

bool Camera::IsUICamera() const
{
    return m_IsUICamera;
}

iVector2 Camera::GetViewportOffset() const
{
    return m_ViewportOffset;
}

void Camera::SetViewportOffset(iVector2 const& viewportOffset)
{
    m_ViewportOffset = viewportOffset;
}

iVector2 Camera::GetViewportSize() const
{
    return m_ViewportSize;
}

void Camera::SetViewportSize(iVector2 const& viewportSize, bool rebuildTextures)
{
    if (m_ViewportSize != viewportSize)
    {
        m_ViewportSize = viewportSize;

#ifndef USE_VULKAN
        if (rebuildTextures)
        {
            glDeleteTextures(1, &m_RenderTarget);
            glCreateTextures(GL_TEXTURE_2D, 1, &m_RenderTarget);

            glTextureStorage2D(m_RenderTarget, 1, GL_RGBA16F, m_ViewportSize[0], m_ViewportSize[1]);
            glTextureParameteri(m_RenderTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(m_RenderTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
#endif  // USE_VULKAN
    }
}

float Camera::GetFieldOfView() const
{
    return m_FieldOfView;
}

void Camera::SetFieldOfView(float const& fieldOfView)
{
    m_FieldOfView = fieldOfView;
}

float Camera::GetNearPlane() const
{
    return m_NearPlane;
}

void Camera::SetNearPlane(float const& nearPlane)
{
    m_NearPlane = nearPlane;
}

float Camera::GetFarPlane() const
{
    return m_FarPlane;
}

void Camera::SetFarPlane(float const& farPlane)
{
    m_FarPlane = farPlane;
}

Color3 Camera::GetFogColor() const
{
    return m_FogColor;
}

void Camera::SetFogColor(Color3 const& clearColor)
{
    m_FogColor = clearColor;
}

Color3 Camera::GetAmbientColor() const
{
    return m_AmbientColor;
}

void Camera::SetAmbientColor(Color3 const& ambientColor)
{
    m_AmbientColor = ambientColor;
}

Vector3 Camera::GetLightAttenuation() const
{
    return m_LightAttenuation;
}

void Camera::SetLightAttenuation(Vector3 const& attenuation)
{
    m_LightAttenuation = attenuation;
}

Vector2 Camera::GetFogAttenuation() const
{
    return m_FogAttenuation;
}

void Camera::SetFogAttenuation(Vector2 const& attenuation)
{
    m_FogAttenuation = attenuation;
}

HTexture Camera::GetSkyboxTexture() const
{
    return m_SkyboxTexture;
}

void Camera::SetSkyboxTexture(HTexture const& skyboxTexture)
{
    m_SkyboxTexture = skyboxTexture;
}

Color4 Camera::GetSkyboxColor() const
{
    return m_SkyboxColor;
}

float Camera::GetGamma() const
{
    return m_Gamma;
}

float Camera::GetExposure() const
{
    return m_Exposure;
}

bool Camera::IsPostProcessingEnabled() const
{
    return m_EnablePostProcessing;
}

bool Camera::IsBloomEnabled() const
{
    return m_EnableBloom;
}

bool Camera::IsSSAOEnabled() const
{
    return m_EnableSSAO;
}

GLuint Camera::GetRenderTarget()
{
    if (m_ViewportSize != m_PrevViewportSize)
    {
        m_PrevViewportSize = m_ViewportSize;

#ifndef USE_VULKAN
        glDeleteTextures(1, &m_RenderTarget);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RenderTarget);

        glTextureStorage2D(m_RenderTarget, 1, GL_RGBA16F, m_ViewportSize[0], m_ViewportSize[1]);
        glTextureParameteri(m_RenderTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(m_RenderTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif  // USE_VULKAN
    }
    return m_RenderTarget;
}

Vector3 Camera::GetLookAtDirection() const
{
    return m_IsReflectionView ? Vector3(m_LookAtDirection[0], -m_LookAtDirection[1], m_LookAtDirection[2]) : m_LookAtDirection;
}

void Camera::SetLookAtDirection(Vector3 const& direction)
{
    m_LookAtDirection = direction;
}

void Camera::SetIsReflectionView(const bool& isReflectionView)
{
    m_IsReflectionView = isReflectionView;
}

void Camera::SetReflectionHeight(const float& height)
{
    m_ReflectionHeight = height;
}

Vector2 Camera::WorldToScreen(Vector3 const& worldPosition)
{
    Matrix4 viewProjectionMatrix = GetViewProjTransform();
    // transform world to clipping coordinates

    Vector4 clipPosition = viewProjectionMatrix * Vector4(worldPosition, 1.0f);
    clipPosition /= clipPosition[3];

    float winX = ((clipPosition[0] + 1) / 2.0f) * m_ViewportSize[0];
    // we calculate -point3D.getY() because the screen Y axis is oriented top->down
    float winY = ((clipPosition[1] + 1) / 2.0f) * m_ViewportSize[1];

    return Vector2(winX, winY);
}

Vector3 Camera::ScreenToWorld(Vector2 const& screenPosition)
{
    Vector3 pointPlane = m_Transform->m_WorldPosition + m_LookAtDirection;
    Vector3 right = m_LookAtDirection.Cross(m_CameraUp).Normalized();
    Vector3 up = right.Cross(m_LookAtDirection).Normalized();

    float t = tanf(Math::ToRadians(m_FieldOfView) / 2);
    float h = m_NearPlane * t;
    float w = h * ((float)m_ViewportSize[1] / m_ViewportSize[0]);

    Vector2 screenOffset = screenPosition / Vector2((float)m_ViewportSize[0], (float)m_ViewportSize[1]);
    screenOffset *= 2.0f;
    screenOffset -= Vector2(1.0f);
    screenOffset *= Vector2(w, h);

    pointPlane += right * screenOffset[0] + up * screenOffset[1];

    return pointPlane;
}

std::vector<Vector3> Camera::GetFrustumPoints() const
{
    std::vector<Vector3> results(8);

    Vector3 zAxis = m_IsReflectionView ? m_LookAtDirection * Vector3(1, -1, 1) : m_LookAtDirection;
    zAxis.Normalize();

    Vector3 cameraUp = zAxis[1] >= 0.99999f ? Vector3(0.0f, 0.0f, -1.0f) : zAxis[1] <= -0.99999f ? Vector3(0.0f, 0.0f, 1.0f) : m_CameraUp;
    Vector3 xAxis = zAxis.Cross(cameraUp).Normalized();
    Vector3 yAxis = xAxis.Cross(zAxis).Normalized();

    // Near/far plane center points
    Vector3 camPosition = m_Transform->m_WorldPosition;
    if (m_IsReflectionView)
    {
        camPosition[1] = camPosition[1] - 2 * (camPosition[1] - m_ReflectionHeight);
    }

    Vector3 nearCenter = camPosition + zAxis * m_NearPlane;
    Vector3 farCenter = camPosition + zAxis * m_FarPlane;

    // Get projected viewport extents on near/far planes
    float e = tanf(m_FieldOfView * 0.5f);
    float nearExtY = e * m_NearPlane;
    float nearExtX = nearExtY * ((float)m_ViewportSize[0] / m_ViewportSize[1]);
    float farExtY = m_ProjectionMode == CameraProjectionMode::eCameraProjectionMode_Orthographic ? nearExtY : e * m_FarPlane;
    float farExtX = m_ProjectionMode == CameraProjectionMode::eCameraProjectionMode_Orthographic ? nearExtX : farExtY * ((float)m_ViewportSize[0] / m_ViewportSize[1]);

    // Points are just offset from the center points along camera basis
    // lbn, rbn, rtn, ltn, lbf, rbf, rtf, ltf
    results[0] = nearCenter - xAxis * nearExtX - yAxis * nearExtY;  // lbn
    results[1] = nearCenter + xAxis * nearExtX - yAxis * nearExtY;  // rbn
    results[2] = nearCenter + xAxis * nearExtX + yAxis * nearExtY;  // rtn
    results[3] = nearCenter - xAxis * nearExtX + yAxis * nearExtY;  // ltn
    results[4] = farCenter - xAxis * farExtX - yAxis * farExtY;     // lbf
    results[5] = farCenter + xAxis * farExtX - yAxis * farExtY;     // rbf
    results[6] = farCenter + xAxis * farExtX + yAxis * farExtY;     // rtf
    results[7] = farCenter - xAxis * farExtX + yAxis * farExtY;     // ltf

    return results;
}

Matrix4 Camera::GetViewProjTransform() const
{
    return GetProjTransform() * GetViewTransform();
}

Matrix4 Camera::GetViewTransform() const
{
    if (m_IsReflectionView)
    {
        Vector3 invertView = m_LookAtDirection;
        invertView[1] = -invertView[1];
        Vector3 invertPosition = m_Transform->GetWorldPosition();
        invertPosition[1] += (m_ReflectionHeight - invertPosition[1]) * 2;

        return Matrix4::LookAt(invertPosition, invertPosition + invertView, m_CameraUp);
    }
    else
    {
        return Matrix4::LookAt(m_Transform->GetWorldPosition(), m_Transform->GetWorldPosition() + m_LookAtDirection, m_CameraUp);
    }
}

Matrix4 Camera::GetProjTransform() const
{
    float aspectRatio = static_cast<float>(m_ViewportSize[0]) / m_ViewportSize[1];
    switch (m_ProjectionMode)
    {
        case CameraProjectionMode::eCameraProjectionMode_Perspective:
        {
            return Matrix4::Perspective(m_FieldOfView, aspectRatio, m_NearPlane, m_FarPlane);
        }

        case CameraProjectionMode::eCameraProjectionMode_Orthographic:
        {
            float halfHeight = m_OrthoVerticalSize / 2;
            float halfWidth = halfHeight * aspectRatio;
            return Matrix4::FrustumPerspective(-halfWidth, halfWidth, -halfHeight, halfHeight, m_NearPlane, m_FarPlane);
        }

            // Should not reach here
        default:
        {
            return Matrix4();
        }
    }
}

void Camera::SetUpDirection(Vector3 const& direction)
{
    m_CameraUp = direction;
}

REFLECT_INIT(Camera)
REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(m_ProjectionMode)
REFLECT_PROPERTY(m_IsMainCamera)
REFLECT_PROPERTY(m_IsUICamera)
REFLECT_PROPERTY(m_ViewportSize)
REFLECT_PROPERTY(m_ViewportOffset)
REFLECT_PROPERTY(m_OrthoVerticalSize)
REFLECT_PROPERTY(m_FieldOfView)
REFLECT_PROPERTY(m_NearPlane)
REFLECT_PROPERTY(m_FarPlane)
REFLECT_PROPERTY(m_FogColor)
REFLECT_PROPERTY(m_FogAttenuation)
REFLECT_PROPERTY(m_Gamma)
REFLECT_PROPERTY(m_Exposure)
REFLECT_PROPERTY(m_EnablePostProcessing)
REFLECT_PROPERTY(m_SkyboxTexture)
REFLECT_PROPERTY(m_SkyboxColor)
REFLECT_PROPERTY(m_AmbientColor)
REFLECT_PROPERTY(m_LightAttenuation)
REFLECT_PROPERTY(m_LookAtDirection)
REFLECT_PROPERTY(m_CameraUp)
REFLECT_PROPERTY(m_EnableBloom)
REFLECT_PROPERTY(m_EnableSSAO)
REFLECT_END()
