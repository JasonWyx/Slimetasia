#pragma once
#include "BaseDetectionAlgorithm.h"
#include "Utility.h"

class EdgeData;
class BoxCollider;

struct ConvexPolygonVsConvexPolygon : public BaseDetectionAlgorithm
{
    ConvexPolygonVsConvexPolygon() = default;

    ~ConvexPolygonVsConvexPolygon() = default;

    void ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second) override;

private:

    float FindSmallestPenetration(uint& chosenFaceIndex, BoxCollider* first, BoxCollider* second);

    void FindIncidentFace(Vector3 (&verts)[4], BoxCollider* referenceBox, BoxCollider* incidentBox, const uint& referenceIndex);

    int Clip(const Vector3& normal, const float& c, Vector3 (&verts)[4]);

    bool TestAxisStatic(const Vector3& axis, float minA, float maxA, float minB, float maxB, Vector3& mtvAxis, float& mtvDistance);
};
