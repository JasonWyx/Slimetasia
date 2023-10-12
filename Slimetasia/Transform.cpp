#include "Transform.h"

#include "GameObject.h"
#include "MeshRenderer.h"

Transform::Transform(GameObject* parentObject)
    : IComponent(parentObject, "Transform")
    , m_WorldPosition(0.0f, 0.0f, 0.0f)
    , m_WorldRotation(0.0f, 0.0f, 0.0f)
    , m_WorldScale(1.0f, 1.0f, 1.0f)
    , m_ParentPosition(0.f, 0.f, 0.f)
    , m_ParentRotation(0.f, 0.f, 0.f)
    , m_ParentScale(0.f, 0.f, 0.f)
    , m_ParentTransform(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f)
    , m_IsChanged(false)
{
    m_DeltaPosition = m_DefaultPosition = m_WorldPosition;
}

Vector3 Transform::GetWorldPosition() const
{
    return m_WorldPosition;
}

Vector3 Transform::GetWorldRotation() const
{
    return m_WorldRotation;
}

Vector3 Transform::GetWorldScale() const
{
    return m_WorldScale;
}

void Transform::SetWorldPosition(Vector3 const& position)
{
    m_WorldPosition = position;
}

void Transform::SetWorldRotation(Vector3 const& rotation)
{
    m_WorldRotation = rotation;
}

void Transform::SetWorldScale(Vector3 const& scale)
{
    m_WorldScale = scale;
}

void Transform::Translate(Vector3 const& displacement) {}

void Transform::Rotate(Vector3 const& rotation) {}

void Transform::Scale(Vector3 const& scale) {}

void Transform::OnUpdate(float dt)
{
    if (!m_IsChanged)
    {
        m_DeltaPosition = m_WorldPosition;
        m_ParentTransform = Matrix4::Identity();
        if (m_OwnerObject->GetParentObject())
        {
            auto pTrans = m_OwnerObject->GetParentLayer()->GetObjectById(m_OwnerObject->GetParentObject())->GetComponent<Transform>();
            if (pTrans)
            {
                m_ParentPosition = pTrans->GetWorldPosition();
                m_ParentRotation = pTrans->GetWorldRotation();
                m_ParentScale = pTrans->GetWorldScale();
                m_DefaultPosition = m_WorldPosition - m_ParentPosition;
            }
        }
        m_IsChanged = true;
    }
    static auto lambda = [](Vector3 a, Vector3 b) -> bool { return a[0] == b[0] && a[1] == b[1] && a[2] == b[2]; };
    if (m_OwnerObject->GetParentObject())
    {
        auto pTrans = m_OwnerObject->GetParentLayer()->GetObjectById(m_OwnerObject->GetParentObject())->GetComponent<Transform>();
        if (pTrans)
        {
            auto mat = pTrans->GetWorldTransformMatrix();
            // std::cout << m_defaultPos[0] << " " << m_defaultPos[1] << " " << m_defaultPos[2] << std::endl;
            Vector4 pos { m_DefaultPosition[0], m_DefaultPosition[1], m_DefaultPosition[2], 1 };
            if (!lambda(pTrans->GetWorldPosition(), m_ParentPosition) || !lambda(pTrans->GetWorldRotation(), m_ParentRotation) || !lambda(pTrans->GetWorldScale(), m_ParentScale))
            {
                m_ParentTransform = mat;
                Vector4 result = mat * pos;
                m_ParentPosition = pTrans->GetWorldPosition();
                m_WorldPosition[0] = result[0];
                m_WorldPosition[1] = result[1];
                m_WorldPosition[2] = result[2];

                m_ParentRotation = pTrans->GetWorldRotation();
                m_ParentScale = pTrans->GetWorldScale();
                // auto tmp = mat.Inverted() * result;
                // std::cout << tmp[0] << " " << tmp[1] << " " << tmp[2] << " " << tmp[3]  << std::endl;

                // Vector4 tmp = mat.Inverted() * result;
                // m_defaultPos = Vector3{ pos[0], pos[1], pos[2] };
            }
            else if (!lambda(m_WorldPosition, m_DeltaPosition))
            {
                // Vector3 m_diff = m_WorldPosition - m_deltaPos;
                Vector4 tmp { m_WorldPosition[0], m_WorldPosition[1], m_WorldPosition[2], 1 };
                Vector4 r = mat.Inverted() * tmp;
                m_DefaultPosition[0] = r[0];
                m_DefaultPosition[1] = r[1];
                m_DefaultPosition[2] = r[2];
            }
            // Vector4 tmp = mat.Inverted() * pos;
            // defaultPos = Vector3{ tmp[0], tmp[1], tmp[2] };
        }
    }
    else
    {
        m_IsChanged = false;
    }
    m_DeltaPosition = m_WorldPosition;
}

void Transform::GetBounds(Vector3& min, Vector3& max)
{
    min = -m_WorldScale * 0.5f;

    max = m_WorldScale * 0.5f;
}

Matrix4 Transform::GetParentTransformMatrix()
{
    if (m_OwnerObject->GetParentObject())
    {
        if (GameObject* parentObject = m_OwnerObject->GetParentLayer()->GetObjectById(m_OwnerObject->GetParentObject()))
        {
            if (Transform* parentTransform = parentObject->GetComponent<Transform>())
            {
                return parentTransform->GetWorldTransformMatrix();
            }
        }
    }
    // Parent does not have transform
    return Matrix4(1.0f);
}

Matrix4 Transform::GetWorldTransformMatrix()
{
    return Matrix4::Translate(m_WorldPosition) * Matrix4::RotateZ(m_WorldRotation[2]) * Matrix4::RotateY(m_WorldRotation[1]) * Matrix4::RotateX(m_WorldRotation[0]) * Matrix4::Scale(m_WorldScale);
}

Vector3 Transform::GetForwardVector() const
{
    Vector4 result { WorldForward, 0 };

    result = Matrix4::RotateX(m_WorldRotation[0]) * result;
    result = Matrix4::RotateY(m_WorldRotation[1]) * result;
    result = Matrix4::RotateZ(m_WorldRotation[2]) * result;

    return Vector3 { result[0], result[1], result[2] }.Normalized();
}

Vector3 Transform::GetRightVector() const
{
    Vector4 result { WorldRight, 0 };

    result = Matrix4::RotateX(m_WorldRotation[0]) * result;
    result = Matrix4::RotateY(m_WorldRotation[1]) * result;
    result = Matrix4::RotateZ(m_WorldRotation[2]) * result;

    return Vector3 { result[0], result[1], result[2] }.Normalized();
}

Vector3 Transform::GetUpwardVector() const
{
    Vector4 result { WorldUp, 0 };

    result = Matrix4::RotateX(m_WorldRotation[0]) * result;
    result = Matrix4::RotateY(m_WorldRotation[1]) * result;
    result = Matrix4::RotateZ(m_WorldRotation[2]) * result;

    return Vector3 { result[0], result[1], result[2] }.Normalized();
}

REFLECT_INIT(Transform)
REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(m_WorldPosition)
REFLECT_PROPERTY(m_WorldRotation)
REFLECT_PROPERTY(m_WorldScale)
REFLECT_END()