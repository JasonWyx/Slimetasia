#include "EditorCamera.h"

#include <algorithm>

#include "Application.h"
#include "GameObject.h"
#include "Input.h"
#include "Renderer.h"
#include "ResourceManager.h"

EditorCamera::EditorCamera(GameObject* parentObject)
    : Camera(parentObject, "EditorCamera")
    , m_Latitude(0.0f)
    , m_Azimuth(0.0f)
    , m_IsPanning(false)
    , m_IsRotating(false)
    , m_IsZooming(false)
    , m_FocalDistance(1.0f)
    , m_ZoomDampenFactor(1.0f)
    , m_Update(false)
{
    m_LookAtDirection = Vector3(0.0f, 0.0f, -1.0f).RotateX(m_Latitude).RotateY(m_Azimuth);
    m_SkyboxTexture = ResourceManager::Instance().GetResource<Texture>("Daylight Box");
    m_AmbientColor = Color3(0.5f);
}

void EditorCamera::LookAt(GameObject* go)
{
    Transform* other = go->GetComponent<Transform>();
    if (!other) return;
    float scale = std::max(std::max(other->GetWorldScale().x, other->GetWorldScale().y), other->GetWorldScale().z);
    Vector3 tmp = other->GetWorldPosition();
    tmp.z += scale + 5.f;
    m_Transform->SetWorldPosition(tmp);
    m_Azimuth = m_Latitude = 0.f;
    m_LookAtDirection = Vector3(0.0f, 0.0f, -1.0f).RotateX(m_Latitude).RotateY(m_Azimuth);
}

void EditorCamera::OnUpdate(float dt)
{
    if (!m_Update) return;

    Vector3 right = m_LookAtDirection.Cross(m_CameraUp).Normalize();
    Vector3 up = right.Cross(m_LookAtDirection).Normalize();

    if (Input::Instance().GetKeyDown(KEY_LALT))
    {
        if (Input::Instance().GetKeyDown(MOUSE_LEFT)) m_IsRotating = true;
        if (Input::Instance().GetKeyDown(MOUSE_MID)) m_IsPanning = true;
        if (Input::Instance().GetKeyDown(MOUSE_RIGHT)) m_IsZooming = true;
    }

    if (Input::Instance().GetKeyUp(MOUSE_LEFT)) m_IsRotating = false;
    if (Input::Instance().GetKeyUp(MOUSE_MID)) m_IsPanning = false;
    if (Input::Instance().GetKeyUp(MOUSE_RIGHT)) m_IsZooming = false;

    if (Input::Instance().GetKeyDown(KEY_UP)) m_Transform->SetWorldPosition(m_Transform->GetWorldPosition() + m_LookAtDirection.Normalized() * dt);
    if (Input::Instance().GetKeyDown(KEY_LEFT)) m_Transform->SetWorldPosition(m_Transform->GetWorldPosition() - right * dt);
    if (Input::Instance().GetKeyDown(KEY_DOWN)) m_Transform->SetWorldPosition(m_Transform->GetWorldPosition() - m_LookAtDirection.Normalized() * dt);
    if (Input::Instance().GetKeyDown(KEY_RIGHT)) m_Transform->SetWorldPosition(m_Transform->GetWorldPosition() + right * dt);

    Vector2 mouseDelta = Input::Instance().GetMouseDelta();

    if (m_IsRotating)
    {
        m_Azimuth -= mouseDelta.x * 0.5f;
        m_Latitude += mouseDelta.y * 0.5f;

        m_Azimuth = std::fmodf(m_Azimuth, 360.0f);
        m_Latitude = std::clamp(m_Latitude, -89.0f, 89.0f);

        m_LookAtDirection = Vector3(0.0f, 0.0f, -1.0f).RotateX(m_Latitude).RotateY(m_Azimuth);
    }

    if (m_IsPanning)
    {
        m_Transform->SetWorldPosition(m_Transform->GetWorldPosition() - right * mouseDelta.x * dt);
        m_Transform->SetWorldPosition(m_Transform->GetWorldPosition() - up * mouseDelta.y * dt);
    }

    if (m_IsZooming)
    {
        /*m_ZoomDampenFactor += mouseDelta.y / 100;
        m_ZoomDampenFactor = m_ZoomDampenFactor < 1.0f ? 1.0f : m_ZoomDampenFactor;

        if (mouseDelta.y > 0)
          m_Position += (m_LookAtDirection * (m_ZoomDampenFactor / m_FocalDistance));
        if (mouseDelta.y < 0)
          m_Position -= (m_LookAtDirection * (m_ZoomDampenFactor / m_FocalDistance));*/

        m_Transform->SetWorldPosition(m_Transform->GetWorldPosition() + m_LookAtDirection * mouseDelta.y / 100);
        m_Transform->SetWorldPosition(m_Transform->GetWorldPosition() + m_LookAtDirection * mouseDelta.x / 100);
    }
}

void EditorCamera::Reset()
{
    m_Transform->SetWorldPosition(Vector3(0.0f, 0.0f, 0.0f));
    m_CameraUp = Vector3(0.0f, 1.0f, 0.0f);
    m_LookAtDirection = Vector3(0.0f, 0.0f, -1.0f);
}

void EditorCamera::Cancel()
{
    m_IsRotating = false;
    m_IsPanning = false;
    m_IsZooming = false;
}

REFLECT_INIT(EditorCamera)
REFLECT_PARENT(Camera)
REFLECT_END()