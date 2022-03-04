#include "ConvexPolygonVsConvexPolygon.h"

#include <cassert>

#include "BoxCollider.h"
#include "Manifold.h"
#include "RigidbodyComponent.h"

bool BiasGreaterThan(float a, float b)
{
    const float k_biasRelative = 0.95f;
    const float k_biasAbsolute = 0.01f;
    return a >= b * k_biasRelative + a * k_biasAbsolute;
}

// just handling box for now. Will expand to handle proper convex polygons using SAT / GJK
void ConvexPolygonVsConvexPolygon::ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second)
{
    // ensuring all parameters are valid.
    assert(manifold != nullptr && first != nullptr && second != nullptr);

    BoxCollider* firstCollider = first->GetOwner()->GetComponent<BoxCollider>();
    BoxCollider* secondCollider = second->GetOwner()->GetComponent<BoxCollider>();

    assert(firstCollider && secondCollider);

    manifold->m_ContactCount = 0;

    Vector3 firstPos = firstCollider->GetPosition() + firstCollider->m_offset;
    Vector3 firstHE = firstCollider->m_HalfExtent;

    Vector3 secondPos = secondCollider->GetPosition() + secondCollider->m_offset;
    Vector3 secondHE = secondCollider->m_HalfExtent;

    Vector3 dist = secondPos - firstPos;

    // calculating the overlap on ALL axes.
    float x_overlap = firstHE.x + secondHE.x - fabsf(dist.x);
    float y_overlap = firstHE.y + secondHE.y - fabsf(dist.y);
    float z_overlap = firstHE.z + secondHE.z - fabsf(dist.z);

    // we have a collision IF all 3 axes overlap
    if (x_overlap > 0.f && y_overlap > 0.f && z_overlap > 0.f)
    {
        manifold->m_ContactCount = 1;
        // finding the axis of smallest overlap
        // if its x
        if (x_overlap < y_overlap && x_overlap < z_overlap)
        {
            if (dist.x > 0.f)
            {
                manifold->m_Normal = Vector3{1.f, 0.f, 0.f};
                manifold->m_Contacts[0] = Vector3{firstPos.x + x_overlap, firstPos.y, firstPos.z};
            }
            else
            {
                manifold->m_Normal = Vector3{-1.f, 0.f, 0.f};
                manifold->m_Contacts[0] = Vector3{firstPos.x - x_overlap, firstPos.y, firstPos.z};
            }

            manifold->m_PenetrationDepth = x_overlap;

            return;
        }

        // if its y
        if (y_overlap < x_overlap && y_overlap < z_overlap)
        {
            if (dist.y > 0.f)
            {
                manifold->m_Normal = Vector3{0.f, 1.f, 0.f};
                manifold->m_Contacts[0] = Vector3{firstPos.x, firstPos.y + y_overlap, firstPos.z};
            }
            else
            {
                manifold->m_Normal = Vector3{0.f, -1.f, 0.f};
                manifold->m_Contacts[0] = Vector3{firstPos.x, firstPos.y - y_overlap, firstPos.z};
            }

            manifold->m_PenetrationDepth = y_overlap;
            return;
        }

        // the only choice is z
        if (dist.z > 0.f)
        {
            manifold->m_Normal = Vector3{0.f, 0.f, 1.f};
            manifold->m_Contacts[0] = Vector3{firstPos.x, firstPos.y, firstPos.z + z_overlap};
        }
        else
        {
            manifold->m_Normal = Vector3{0.f, 0.f, -1.f};
            manifold->m_Contacts[0] = Vector3{firstPos.x, firstPos.y, firstPos.z - z_overlap};
        }

        manifold->m_PenetrationDepth = z_overlap;
    }
}

bool ConvexPolygonVsConvexPolygon::TestAxisStatic(const Vector3& axis, float minA, float maxA, float minB, float maxB, Vector3& mtvAxis, float& mtvDistance)
{
    // [Separating Axis Theorem]
    // Two convex shapes only overlap if they overlap on all axes of separation
    // In order to create accurate responses we need to find the collision vector (Minimum Translation Vector)
    // Find if the two boxes intersect along a single axis
    // Compute the intersection interval for that axis
    // Keep the smallest intersection/penetration value

    float axisLengthSquared = axis.Dot(axis);

    // If the axis is degenerate then ignore
    if (axisLengthSquared < EPSILON)
    {
        return true;
    }

    // Calculate the two possible overlap ranges
    // Either we overlap on the left or the right sides
    float d0 = (maxB - minA);  // 'Left' side
    float d1 = (maxA - minB);  // 'Right' side

    // Intervals do not overlap, so no intersection
    if (d0 <= 0.0f || d1 <= 0.0f)
    {
        return false;
    }

    // Find out if we overlap on the 'right' or 'left' of the object.
    float overlap = (d0 < d1) ? d0 : -d1;

    // The mtd vector for that axis
    Vector3 sep = axis * (overlap / axisLengthSquared);

    // The mtd vector length squared
    float sepLengthSquared = sep.Dot(sep);

    // If that vector is smaller than our computed Minimum Translation Distance use that vector as our current MTV distance
    if (sepLengthSquared < mtvDistance)
    {
        mtvDistance = sepLengthSquared;
        mtvAxis = sep;
    }

    return true;
}

float ConvexPolygonVsConvexPolygon::FindSmallestPenetration(uint& chosenFaceIndex, BoxCollider* first, BoxCollider* second)
{
    float largestPene = FLT_MIN;

    /*Vector3 firstPosition = first->GetPosition();

    for (uint i = 0u; i < 6; i++)
    {
        Vector3 currNormal = first->m_FaceNormals[i];

        //TODO : APPLY ROTATION ON THE FACE NORMAL!


        Vector3 supportPoint = second->GetSupportPoint(-currNormal);

        //total of 8 verts to test against
        for (uint j = 0; j < 8; j++)
        {
            float dist = currNormal.Dot(supportPoint - first->GetVertexPosition(j) + firstPosition);

            if (dist > largestPene)
            {
                largestPene = dist;
                chosenFaceIndex = i;
            }
        }
    }*/

    return largestPene;
}

void ConvexPolygonVsConvexPolygon::FindIncidentFace(Vector3 (&verts)[4], BoxCollider* referenceBox, BoxCollider* incidentBox, const uint& referenceIndex)
{
    /*Vector3 refNormal = referenceBox->m_FaceNormals[referenceIndex];

    uint face = 0;
    float minDot = FLT_MAX;

    for (uint i = 0u; i < 6; i++)
    {
        float dot = refNormal.Dot(incidentBox->m_FaceNormals[i]);
        if (dot < minDot)
        {
            minDot = dot;
            face = i;
        }
    }

    auto selectedFace = incidentBox->GetFace(face);
    verts[0] = incidentBox->GetVertexPosition(selectedFace.m_FaceVertices[0].m_VertexPointIndex);
    verts[1] = incidentBox->GetVertexPosition(selectedFace.m_FaceVertices[1].m_VertexPointIndex);
    verts[2] = incidentBox->GetVertexPosition(selectedFace.m_FaceVertices[2].m_VertexPointIndex);
    verts[3] = incidentBox->GetVertexPosition(selectedFace.m_FaceVertices[3].m_VertexPointIndex);*/
}

int ConvexPolygonVsConvexPolygon::Clip(const Vector3& normal, const float& c, Vector3 (&verts)[4])
{
    return 0;
}
