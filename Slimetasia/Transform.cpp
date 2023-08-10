#include "Transform.h"

#include "GameObject.h"
#include "MeshRenderer.h"

Transform::Transform(GameObject* parentObject)
    : IComponent(parentObject, "Transform")
    ,
    // m_LocalPosition(0.0f, 0.0f, 0.0f),
    // m_LocalRotation(0.0f, 0.0f, 0.0f),
    // m_LocalScale(1.0f, 1.0f, 1.0f),
    m_WorldPosition(0.0f, 0.0f, 0.0f)
    , m_WorldRotation(0.0f, 0.0f, 0.0f)
    , m_WorldScale(1.0f, 1.0f, 1.0f)
    , parentPos(0.f, 0.f, 0.f)
    , parentRot(0.f, 0.f, 0.f)
    , parentScale(0.f, 0.f, 0.f)
    , parentMat(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f)
    , change(false)
{
    m_deltaPos = m_defaultPos = m_WorldPosition;
}

void Transform::CalculateWorldToLocal()
{
    // if (m_OwnerObject == nullptr)
    //{
    //  m_LocalPosition = m_WorldPosition;
    //  m_LocalRotation = m_WorldRotation;
    //  m_LocalScale = m_WorldScale;
    //}
    //
    // GameObject *parentObject = m_OwnerObject;
    // while (parentObject != nullptr)
    //{
    //
    //}
    //
    //
}

void Transform::CalculateLocalToWorld() {}

// Vector3 Transform::GetLocalPosition() const
//{
//  return m_LocalPosition;
//}
//
// Vector3 Transform::GetLocalRotation() const
//{
//  return m_LocalRotation;
//}
//
// Vector3 Transform::GetLocalScale() const
//{
//  return m_LocalScale;
//}

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

// void Transform::SetLocalPosition(Vector3 const & position)
//{
//  m_LocalPosition = position;
//}
//
// void Transform::SetLocalRotation(Vector3 const & rotation)
//{
//  m_LocalRotation = rotation;
//}
//
// void Transform::SetLocalScale(Vector3 const & Scale)
//{
//  m_LocalScale = Scale;
//}

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
    if (!change)
    {
        m_deltaPos = m_WorldPosition;
        parentMat = Matrix4::Identity();
        if (m_OwnerObject->GetParentObject())
        {
            auto pTrans = m_OwnerObject->GetParentLayer()->GetObjectById(m_OwnerObject->GetParentObject())->GetComponent<Transform>();
            if (pTrans)
            {
                parentPos = pTrans->GetWorldPosition();
                parentRot = pTrans->GetWorldRotation();
                parentScale = pTrans->GetWorldScale();
                m_defaultPos = m_WorldPosition - parentPos;
            }
        }
        change = true;
    }
    static auto lambda = [](Vector3 a, Vector3 b) -> bool { return a[0] == b[0] && a[1] == b[1] && a[2] == b[2]; };
    if (m_OwnerObject->GetParentObject())
    {
        auto pTrans = m_OwnerObject->GetParentLayer()->GetObjectById(m_OwnerObject->GetParentObject())->GetComponent<Transform>();
        if (pTrans)
        {
            auto mat = pTrans->GetWorldTransformMatrix();
            // std::cout << m_defaultPos[0] << " " << m_defaultPos[1] << " " << m_defaultPos[2] << std::endl;
            Vector4 pos { m_defaultPos[0], m_defaultPos[1], m_defaultPos[2], 1 };
            if (!lambda(pTrans->GetWorldPosition(), parentPos) || !lambda(pTrans->GetWorldRotation(), parentRot) || !lambda(pTrans->GetWorldScale(), parentScale))
            {
                parentMat = mat;
                Vector4 result = mat * pos;
                parentPos = pTrans->GetWorldPosition();
                m_WorldPosition[0] = result[0];
                m_WorldPosition[1] = result[1];
                m_WorldPosition[2] = result[2];

                parentRot = pTrans->GetWorldRotation();
                parentScale = pTrans->GetWorldScale();
                // auto tmp = mat.Inverted() * result;
                // std::cout << tmp[0] << " " << tmp[1] << " " << tmp[2] << " " << tmp[3]  << std::endl;

                // Vector4 tmp = mat.Inverted() * result;
                // m_defaultPos = Vector3{ pos[0], pos[1], pos[2] };
            }
            else if (!lambda(m_WorldPosition, m_deltaPos))
            {
                // Vector3 m_diff = m_WorldPosition - m_deltaPos;
                Vector4 tmp { m_WorldPosition[0], m_WorldPosition[1], m_WorldPosition[2], 1 };
                Vector4 r = mat.Inverted() * tmp;
                m_defaultPos[0] = r[0];
                m_defaultPos[1] = r[1];
                m_defaultPos[2] = r[2];
            }
            // Vector4 tmp = mat.Inverted() * pos;
            // defaultPos = Vector3{ tmp[0], tmp[1], tmp[2] };
        }
    }
    else
    {
        change = false;
    }
    m_deltaPos = m_WorldPosition;
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

// Matrix4 Transform::GetLocalTransformMatrix()
//{
//  return  Matrix4::Translate(m_LocalPosition) *
//          Matrix4::RotateZ(m_LocalRotation[2]) *
//          Matrix4::RotateY(m_LocalRotation[1]) *
//          Matrix4::RotateX(m_LocalRotation[0]) *
//          Matrix4::Scale(m_LocalScale);
//}

Matrix4 Transform::GetWorldTransformMatrix()
{
    return  // GetParentTransformMatrix() * GetLocalTransformMatrix();
        Matrix4::Translate(m_WorldPosition) * Matrix4::RotateZ(m_WorldRotation[2]) * Matrix4::RotateY(m_WorldRotation[1]) * Matrix4::RotateX(m_WorldRotation[0]) * Matrix4::Scale(m_WorldScale);
}

Vector3 Transform::GetForwardVector() const
{
    Vector4 result { worldForward, 0 };

    result = Matrix4::RotateX(m_WorldRotation[0]) * result;
    result = Matrix4::RotateY(m_WorldRotation[1]) * result;
    result = Matrix4::RotateZ(m_WorldRotation[2]) * result;

    return Vector3 { result[0], result[1], result[2] }.Normalized();
}

Vector3 Transform::GetRightVector() const
{
    Vector4 result { worldRight, 0 };

    result = Matrix4::RotateX(m_WorldRotation[0]) * result;
    result = Matrix4::RotateY(m_WorldRotation[1]) * result;
    result = Matrix4::RotateZ(m_WorldRotation[2]) * result;

    return Vector3 { result[0], result[1], result[2] }.Normalized();
}

Vector3 Transform::GetUpwardVector() const
{
    Vector4 result { worldUpward, 0 };

    result = Matrix4::RotateX(m_WorldRotation[0]) * result;
    result = Matrix4::RotateY(m_WorldRotation[1]) * result;
    result = Matrix4::RotateZ(m_WorldRotation[2]) * result;

    return Vector3 { result[0], result[1], result[2] }.Normalized();
}

// Quat Transform::GetOrientation() const
//{
//	return m_Orientation;
//}
//
// void Transform::GetInverse(Vector3& pos, Quat& orientation) const
//{
//	orientation = m_Orientation.GetInverse();
//	pos = orientation.RotateVector(-m_WorldPosition);
//}
//
// Transform Transform::GetTransformFromInverse(const Vector3 & pos, const Quat & orientation) const
//{
//	Transform tmp(nullptr);
//	const float productX = m_Orientation[3] * pos[0] + m_Orientation[1] * pos[2] - m_Orientation[2] * pos[1];
//	const float productY = m_Orientation[3] * pos[1] + m_Orientation[2] * pos[0] - m_Orientation[0] * pos[2];
//	const float productZ = m_Orientation[3] * pos[2] + m_Orientation[0] * pos[1] - m_Orientation[1] * pos[0];
//	const float productW = -m_Orientation[0] * pos[0] - m_Orientation[1] * pos[1] - m_Orientation[2] * pos[2];
//
//	tmp.m_WorldPosition[0] = m_WorldPosition[0] + productX * m_Orientation[3] -
//												productY * m_Orientation[2] +
//												productZ * m_Orientation[1] -
//												productW * m_Orientation[0];
//
//	tmp.m_WorldPosition[1] = m_WorldPosition[1] + productY * m_Orientation[3] -
//												productZ * m_Orientation[0] +
//												productX * m_Orientation[2] -
//												productW * m_Orientation[1];
//
//	tmp.m_WorldPosition[2] = m_WorldPosition[2] + productZ * m_Orientation[3] -
//												productX * m_Orientation[1] +
//												productY * m_Orientation[0] -
//												productW * m_Orientation[0];
//
//	tmp.m_Orientation[0] = m_Orientation[3] * orientation[0] +
//						  m_Orientation[0] * orientation[3] +
//						  m_Orientation[1] * orientation[2] +
//						  m_Orientation[2] * orientation[1];
//
//	tmp.m_Orientation[1] = m_Orientation[3] * orientation[1] +
//						  m_Orientation[0] * orientation[3] +
//						  m_Orientation[1] * orientation[0] +
//						  m_Orientation[2] * orientation[2];
//
//	tmp.m_Orientation[2] = m_Orientation[3] * orientation[2] +
//						  m_Orientation[0] * orientation[3] +
//						  m_Orientation[1] * orientation[1] +
//						  m_Orientation[2] * orientation[0];
//
//	tmp.m_Orientation[3] = m_Orientation[3] * orientation[3] +
//						  m_Orientation[0] * orientation[0] +
//						  m_Orientation[1] * orientation[1] +
//						  m_Orientation[2] * orientation[2];
//
//	return tmp;
//}
//
// Vector3 Transform::operator*(const Vector3 & vec) const
//{
//	return Vector3(m_Orientation * vec + m_WorldPosition);
//}
//
// Transform Transform::GetInverseTransform() const
//{
//	Transform tmp(nullptr);
//
//	const Quat invQuat = m_Orientation.GetInverse();
//
//	tmp.m_WorldPosition = invQuat * -m_WorldPosition;
//	tmp.m_Orientation = invQuat;
//
//	return tmp;
//}
//
// Transform Transform::operator*(const Transform & rhs) const
//{
//	Transform tmp(nullptr);
//
//	tmp.m_WorldPosition = m_WorldPosition + m_Orientation * rhs.m_WorldPosition;
//	tmp.m_Orientation = m_Orientation * rhs.m_Orientation;
//
//	return tmp;
//}
//
// void Transform::SetOrientation(const Quat& orientation)
//{
//	m_Orientation = orientation;
//}
//
// void Transform::SetToIdentity()
//{
//	m_WorldPosition = Vector3(0.f, 0.f, 0.f);
//	m_Orientation.Identity();
//}

REFLECT_INIT(Transform)
REFLECT_PARENT(IComponent)
// REFLECT_PROPERTY(m_LocalPosition)
// REFLECT_PROPERTY(m_LocalRotation)
// REFLECT_PROPERTY(m_LocalScale)
REFLECT_PROPERTY(m_WorldPosition)
REFLECT_PROPERTY(m_WorldRotation)
REFLECT_PROPERTY(m_WorldScale)
REFLECT_END()