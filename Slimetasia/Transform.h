#pragma once
#include "CorePrerequisites.h"
#include "IComponent.h"
#include "Quaternion.h"

class Transform : public IComponent
{
public:
    // Quaternion m_Orientation;

    Vector3 m_LocalPosition;
    // Vector3 m_LocalRotation;
    // Vector3 m_LocalScale;

    Vector3 m_WorldPosition;
    Vector3 m_WorldRotation;
    Vector3 m_WorldScale;

private:
    Vector3 m_defaultPos;
    Vector3 m_deltaPos;

    void CalculateWorldToLocal();
    void CalculateLocalToWorld();

    bool change;

    Vector3 parentPos;
    Vector3 parentRot;
    Vector3 parentScale;
    Matrix4 parentMat;

public:
    static const Vector3 worldForward;
    static const Vector3 worldUpward;
    static const Vector3 worldRight;

    Transform(GameObject* parentObject);
    ~Transform() = default;

    // Vector3 GetLocalPosition() const;
    // Vector3 GetLocalRotation() const;
    // Vector3 GetLocalScale() const;
    // Quaternion GetOrientation() const;

    Vector3 GetWorldPosition() const;
    Vector3 GetWorldRotation() const;
    Vector3 GetWorldScale() const;

    // void    SetLocalPosition(Vector3 const &position);
    // void    SetLocalRotation(Vector3 const &rotation);
    // void    SetLocalScale(Vector3 const &Scale);
    // void	  SetOrientation(const Quaternion& orientation);
    // void	  SetToIdentity();
    void SetWorldPosition(Vector3 const& position);
    void SetWorldRotation(Vector3 const& rotation);
    void SetWorldScale(Vector3 const& scale);

    // Applies only to local transforms
    void Translate(Vector3 const& displacement);
    void Rotate(Vector3 const& rotation);
    void Scale(Vector3 const& scale);
    void OnUpdate(float dt) override;

    void GetBounds(Vector3& min, Vector3& max);

    // For rendering purposes
    Matrix4 GetParentTransformMatrix();
    // Matrix4 GetLocalTransformMatrix();
    Matrix4 GetWorldTransformMatrix();

    // Get vectors with respect to the object
    Vector3 GetForwardVector() const;
    Vector3 GetRightVector() const;
    Vector3 GetUpwardVector() const;

    // solely used by physics.
    // void	GetInverse(Vector3& pos, Quaternion& orientation) const;
    // Transform	GetTransformFromInverse(const Vector3& pos, const Quaternion& orientation) const;
    // Vector3 operator*(const Vector3& vec)const;
    // Transform GetInverseTransform() const;
    // Transform operator*(const Transform& rhs) const;

    REFLECT()
};
