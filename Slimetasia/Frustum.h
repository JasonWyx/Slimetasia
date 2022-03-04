#pragma once
#include "CorePrerequisites.h"
#include "Plane.h"

class Frustum
{
public:
    Frustum() = default;

    ~Frustum() = default;

    Vector4* GetPlanes() const;

    void Set(const Vector3& lbn, const Vector3& rbn, const Vector3& rtn, const Vector3& ltn, const Vector3& lbf, const Vector3& rbf, const Vector3& rtf, const Vector3& ltf);

    void DebugDraw(const unsigned int& parentID);

    static IntersectionType::Type FrustumAabb(const Vector4 planes[6], const Vector3& aabbMin, const Vector3& aabbMax, size_t& lastAxis);

private:
    Vector3 m_Vertices[8];

    Plane m_Planes[6];
};