#include "Transform.h"

#include "GameObject.h"
#include "MeshRenderer.h"

const Vector3 Transform::worldForward{0, 0, 1};
const Vector3 Transform::worldUpward{0, 1, 0};
const Vector3 Transform::worldRight{1, 0, 0};

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
        parentMat = Matrix4{}.Identity();
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
    static auto lambda = [](Vector3 a, Vector3 b) -> bool { return a.x == b.x && a.y == b.y && a.z == b.z; };
    if (m_OwnerObject->GetParentObject())
    {
        auto pTrans = m_OwnerObject->GetParentLayer()->GetObjectById(m_OwnerObject->GetParentObject())->GetComponent<Transform>();
        if (pTrans)
        {
            auto mat = pTrans->GetWorldTransformMatrix();
            // std::cout << m_defaultPos[0] << " " << m_defaultPos[1] << " " << m_defaultPos[2] << std::endl;
            Vector4 pos{m_defaultPos.x, m_defaultPos.y, m_defaultPos.z, 1};
            if (!lambda(pTrans->GetWorldPosition(), parentPos) || !lambda(pTrans->GetWorldRotation(), parentRot) || !lambda(pTrans->GetWorldScale(), parentScale))
            {
                parentMat = mat;
                Vector4 result = mat * pos;
                parentPos = pTrans->GetWorldPosition();
                m_WorldPosition.x = result.x;
                m_WorldPosition.y = result.y;
                m_WorldPosition.z = result.z;

                parentRot = pTrans->GetWorldRotation();
                parentScale = pTrans->GetWorldScale();
                // auto tmp = mat.Inverted() * result;
                // std::cout << tmp[0] << " " << tmp[1] << " " << tmp[2] << " " << tmp[3]  << std::endl;

                // Vector4 tmp = mat.Inverted() * result;
                // m_defaultPos = Vector3{ pos.x, pos.y, pos.z };
            }
            else if (!lambda(m_WorldPosition, m_deltaPos))
            {
                // Vector3 m_diff = m_WorldPosition - m_deltaPos;
                Vector4 tmp{m_WorldPosition.x, m_WorldPosition.y, m_WorldPosition.z, 1};
                auto r = mat.Invert() * tmp;
                m_defaultPos.x = r.x;
                m_defaultPos.y = r.y;
                m_defaultPos.z = r.z;
            }
            // Vector4 tmp = mat.Inverted() * pos;
            // defaultPos = Vector3{ tmp.x, tmp.y, tmp.z };
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
//          Matrix4::RotateZ(m_LocalRotation.z) *
//          Matrix4::RotateY(m_LocalRotation.y) *
//          Matrix4::RotateX(m_LocalRotation.x) *
//          Matrix4::Scale(m_LocalScale);
//}

Matrix4 Transform::GetWorldTransformMatrix()
{
    return  // GetParentTransformMatrix() * GetLocalTransformMatrix();
        Matrix4::Translate(m_WorldPosition) * Matrix4::RotateZ(m_WorldRotation.z) * Matrix4::RotateY(m_WorldRotation.y) * Matrix4::RotateX(m_WorldRotation.x) * Matrix4::Scale(m_WorldScale);
}

Vector3 Transform::GetForwardVector() const
{
    Vector4 result{worldForward, 0};

    result = Matrix4::RotateX(m_WorldRotation.x) * result;
    result = Matrix4::RotateY(m_WorldRotation.y) * result;
    result = Matrix4::RotateZ(m_WorldRotation.z) * result;

    return Vector3{result.x, result.y, result.z}.Normalize();
}

Vector3 Transform::GetRightVector() const
{
    Vector4 result{worldRight, 0};

    result = Matrix4::RotateX(m_WorldRotation.x) * result;
    result = Matrix4::RotateY(m_WorldRotation.y) * result;
    result = Matrix4::RotateZ(m_WorldRotation.z) * result;

    return Vector3{result.x, result.y, result.z}.Normalize();
}

Vector3 Transform::GetUpwardVector() const
{
    Vector4 result{worldUpward, 0};

    result = Matrix4::RotateX(m_WorldRotation.x) * result;
    result = Matrix4::RotateY(m_WorldRotation.y) * result;
    result = Matrix4::RotateZ(m_WorldRotation.z) * result;

    return Vector3{result.x, result.y, result.z}.Normalize();
}

// Quaternion Transform::GetOrientation() const
//{
//	return m_Orientation;
//}
//
// void Transform::GetInverse(Vector3& pos, Quaternion& orientation) const
//{
//	orientation = m_Orientation.GetInverse();
//	pos = orientation.RotateVector(-m_WorldPosition);
//}
//
// Transform Transform::GetTransformFromInverse(const Vector3 & pos, const Quaternion & orientation) const
//{
//	Transform tmp(nullptr);
//	const float productX = m_Orientation.w * pos.x + m_Orientation.y * pos.z - m_Orientation.z * pos.y;
//	const float productY = m_Orientation.w * pos.y + m_Orientation.z * pos.x - m_Orientation.x * pos.z;
//	const float productZ = m_Orientation.w * pos.z + m_Orientation.x * pos.y - m_Orientation.y * pos.x;
//	const float productW = -m_Orientation.x * pos.x - m_Orientation.y * pos.y - m_Orientation.z * pos.z;
//
//	tmp.m_WorldPosition.x = m_WorldPosition.x + productX * m_Orientation.w -
//												productY * m_Orientation.z +
//												productZ * m_Orientation.y -
//												productW * m_Orientation.x;
//
//	tmp.m_WorldPosition.y = m_WorldPosition.y + productY * m_Orientation.w -
//												productZ * m_Orientation.x +
//												productX * m_Orientation.z -
//												productW * m_Orientation.y;
//
//	tmp.m_WorldPosition.z = m_WorldPosition.z + productZ * m_Orientation.w -
//												productX * m_Orientation.y +
//												productY * m_Orientation.x -
//												productW * m_Orientation.x;
//
//	tmp.m_Orientation.x = m_Orientation.w * orientation.x +
//						  m_Orientation.x * orientation.w +
//						  m_Orientation.y * orientation.z +
//						  m_Orientation.z * orientation.y;
//
//	tmp.m_Orientation.y = m_Orientation.w * orientation.y +
//						  m_Orientation.x * orientation.w +
//						  m_Orientation.y * orientation.x +
//						  m_Orientation.z * orientation.z;
//
//	tmp.m_Orientation.z = m_Orientation.w * orientation.z +
//						  m_Orientation.x * orientation.w +
//						  m_Orientation.y * orientation.y +
//						  m_Orientation.z * orientation.x;
//
//	tmp.m_Orientation.w = m_Orientation.w * orientation.w +
//						  m_Orientation.x * orientation.x +
//						  m_Orientation.y * orientation.y +
//						  m_Orientation.z * orientation.z;
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
//	const Quaternion invQuat = m_Orientation.GetInverse();
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
// void Transform::SetOrientation(const Quaternion& orientation)
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