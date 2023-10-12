#pragma once
#include "CorePrerequisites.h"
#include "IComponent.h"

class Transform : public IComponent
{
public:

    static constexpr Vector3 WorldRight = Vector3::Base(0);
    static constexpr Vector3 WorldUp = Vector3::Base(1);
    static constexpr Vector3 WorldForward = Vector3::Base(2);

    Transform(GameObject* parentObject);
    ~Transform() = default;

    Vector3 GetWorldPosition() const;
    Vector3 GetWorldRotation() const;
    Vector3 GetWorldScale() const;

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
    Matrix4 GetWorldTransformMatrix();

    // Get vectors with respect to the object
    Vector3 GetForwardVector() const;
    Vector3 GetRightVector() const;
    Vector3 GetUpwardVector() const;

    Vector3 m_WorldPosition;
    Vector3 m_WorldRotation;
    Vector3 m_WorldScale;

private:

    Vector3 m_DefaultPosition;
    Vector3 m_DeltaPosition;

    bool m_IsChanged;

    Vector3 m_ParentPosition;
    Vector3 m_ParentRotation;
    Vector3 m_ParentScale;
    Matrix4 m_ParentTransform;

    REFLECT()
};
