#include "SphereVsConvexPolygon.h"

#include <cassert>

#include "BoxCollider.h"
#include "Manifold.h"
#include "RigidbodyComponent.h"
#include "SphereCollider.h"

void SphereVsConvexPolygon::ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second)
{
    // ensuring all parameters are valid.
    assert(manifold != nullptr && first != nullptr && second != nullptr);

    // finding out which object is which.
    SphereCollider* sphere = nullptr;
    BoxCollider* box = nullptr;

    sphere = first->GetOwner()->GetComponent<SphereCollider>();
    box = second->GetOwner()->GetComponent<BoxCollider>();

    // if we guessed correctly and the first object is the sphere.
    if (sphere && box)
    {
        // ensuring that both colliders exist
        assert(box != nullptr && sphere != nullptr);

        manifold->m_ContactCount = 0;

        Vector3 spherePos = sphere->GetPosition() + sphere->m_offset;
        float sphereRadius = sphere->m_radius;

        Vector3 boxPos = box->GetPosition() + box->m_offset;
        Vector3 boxHE = box->m_HalfExtent;
        Vector3 negativeboxHE = -boxHE;

        Vector3 dist = boxPos - spherePos;

        Vector3 closestPt = dist;
        closestPt.x = closestPt.x > boxHE.x ? boxHE.x : closestPt.x < negativeboxHE.x ? negativeboxHE.x : closestPt.x;
        closestPt.y = closestPt.y > boxHE.y ? boxHE.y : closestPt.y < negativeboxHE.y ? negativeboxHE.y : closestPt.y;
        closestPt.z = closestPt.z > boxHE.z ? boxHE.z : closestPt.z < negativeboxHE.z ? negativeboxHE.z : closestPt.z;

        bool isSphereInBox = false;

        if (dist == closestPt)
        {
            isSphereInBox = true;
            float absX = fabsf(dist.x);
            float absY = fabsf(dist.y);
            float absZ = fabsf(dist.z);

            // finding the closest axis
            if (absX < absY && absX < absZ)
            {
                if (closestPt.x > 0.f)
                    closestPt.x = boxHE.x;
                else
                    closestPt.x = negativeboxHE.x;
            }
            else if (absY < absX && absY < absZ)
            {
                if (closestPt.y > 0.f)
                    closestPt.y = boxHE.y;
                else
                    closestPt.y = negativeboxHE.y;
            }
            else
            {
                if (closestPt.z > 0.f)
                    closestPt.z = boxHE.z;
                else
                    closestPt.z = negativeboxHE.z;
            }
        }

        Vector3 normal = dist - closestPt;
        float normalSq = normal.SquareLength();

        if (normalSq > sphereRadius * sphereRadius && !isSphereInBox) return;

        normalSq = sqrtf(normalSq);
        manifold->m_ContactCount = 1;
        float absX = fabs(normal.x), absY = fabs(normal.y), absZ = fabs(normal.z);

        manifold->m_PenetrationDepth = sphereRadius - normalSq;
        if (isSphereInBox)
        {
            manifold->m_Normal = -normal.Normalized();
            manifold->m_Contacts[0] = boxPos;
        }
        else
        {
            /*if (absX > absY && absX > absZ)
            {
                if (normal.x > 0.f)
                    manifold->m_Normal = Vector3{ 1.f, 0.f, 0.f };
                else
                    manifold->m_Normal = Vector3{ -1.f, 0.f, 0.f };
            }
            else if (absY > absX && absY > absZ)
            {
                if (normal.y > 0.f)
                    manifold->m_Normal = Vector3{ 0.f, 1.f, 0.f };
                else
                    manifold->m_Normal = Vector3{ 0.f, -1.f, 0.f };
            }
            else
            {
                if (normal.z > 0.f)
                    manifold->m_Normal = Vector3{ 0.f, 0.f, 1.f };
                else
                    manifold->m_Normal = Vector3{ 0.f, 0.f, -1.f };
            }*/
            manifold->m_Normal = normal.Normalized();
            manifold->m_Contacts[0] = closestPt;
        }
    }
    // we guessed wrongly and the order is reversed.
    else
    {
        box = first->GetOwner()->GetComponent<BoxCollider>();
        sphere = second->GetOwner()->GetComponent<SphereCollider>();

        // ensuring that both colliders exist
        assert(box != nullptr && sphere != nullptr);

        manifold->m_ContactCount = 0;

        Vector3 spherePos = sphere->GetPosition() + sphere->m_offset;
        float sphereRadius = sphere->m_radius;

        Vector3 boxPos = box->GetPosition() + box->m_offset;
        Vector3 boxHE = box->m_HalfExtent;
        Vector3 negativeboxHE = -boxHE;

        Vector3 dist = spherePos - boxPos;

        Vector3 closestPt = dist;
        closestPt.x = closestPt.x > boxHE.x ? boxHE.x : closestPt.x < negativeboxHE.x ? negativeboxHE.x : closestPt.x;
        closestPt.y = closestPt.y > boxHE.y ? boxHE.y : closestPt.y < negativeboxHE.y ? negativeboxHE.y : closestPt.y;
        closestPt.z = closestPt.z > boxHE.z ? boxHE.z : closestPt.z < negativeboxHE.z ? negativeboxHE.z : closestPt.z;

        bool isSphereInBox = false;

        if (dist == closestPt)
        {
            isSphereInBox = true;
            float absX = fabsf(dist.x);
            float absY = fabsf(dist.y);
            float absZ = fabsf(dist.z);

            // finding the closest axis
            if (absX < absY && absX < absZ)
            {
                if (closestPt.x > 0.f)
                    closestPt.x = boxHE.x;
                else
                    closestPt.x = negativeboxHE.x;
            }
            else if (absY < absX && absY < absZ)
            {
                if (closestPt.y > 0.f)
                    closestPt.y = boxHE.y;
                else
                    closestPt.y = negativeboxHE.y;
            }
            else
            {
                if (closestPt.z > 0.f)
                    closestPt.z = boxHE.z;
                else
                    closestPt.z = negativeboxHE.z;
            }
        }

        Vector3 normal = dist - closestPt;
        float normalSq = normal.SquareLength();

        if (normalSq > sphereRadius * sphereRadius && !isSphereInBox) return;

        normalSq = sqrtf(normalSq);
        manifold->m_ContactCount = 1;
        float absX = fabs(normal.x), absY = fabs(normal.y), absZ = fabs(normal.z);

        manifold->m_PenetrationDepth = sphereRadius - normalSq;

        if (isSphereInBox)
        {
            manifold->m_Normal = -normal.Normalized();
            manifold->m_Contacts[0] = boxPos;
        }
        else
        {
            /*if (absX > absY && absX > absZ)
            {
                if (normal.x > 0.f)
                    manifold->m_Normal = Vector3{ 1.f, 0.f, 0.f };
                else
                    manifold->m_Normal = Vector3{ -1.f, 0.f, 0.f };
            }
            else if (absY > absX && absY > absZ)
            {
                if (normal.y > 0.f)
                    manifold->m_Normal = Vector3{ 0.f, 1.f, 0.f };
                else
                    manifold->m_Normal = Vector3{ 0.f, -1.f, 0.f };
            }
            else
            {
                if (normal.z > 0.f)
                    manifold->m_Normal = Vector3{ 0.f, 0.f, 1.f };
                else
                    manifold->m_Normal = Vector3{ 0.f, 0.f, -1.f };
            }*/
            manifold->m_Normal = normal.Normalized();
            manifold->m_Contacts[0] = closestPt;
        }
    }
}
