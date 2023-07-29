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
        closestPt[0] = closestPt[0] > boxHE[0] ? boxHE[0] : closestPt[0] < negativeboxHE[0] ? negativeboxHE[0] : closestPt[0];
        closestPt[1] = closestPt[1] > boxHE[1] ? boxHE[1] : closestPt[1] < negativeboxHE[1] ? negativeboxHE[1] : closestPt[1];
        closestPt[2] = closestPt[2] > boxHE[2] ? boxHE[2] : closestPt[2] < negativeboxHE[2] ? negativeboxHE[2] : closestPt[2];

        bool isSphereInBox = false;

        if (dist == closestPt)
        {
            isSphereInBox = true;
            float absX = fabsf(dist[0]);
            float absY = fabsf(dist[1]);
            float absZ = fabsf(dist[2]);

            // finding the closest axis
            if (absX < absY && absX < absZ)
            {
                if (closestPt[0] > 0.f)
                    closestPt[0] = boxHE[0];
                else
                    closestPt[0] = negativeboxHE[0];
            }
            else if (absY < absX && absY < absZ)
            {
                if (closestPt[1] > 0.f)
                    closestPt[1] = boxHE[1];
                else
                    closestPt[1] = negativeboxHE[1];
            }
            else
            {
                if (closestPt[2] > 0.f)
                    closestPt[2] = boxHE[2];
                else
                    closestPt[2] = negativeboxHE[2];
            }
        }

        Vector3 normal = dist - closestPt;
        float normalSq = normal.SquareLength();

        if (normalSq > sphereRadius * sphereRadius && !isSphereInBox) return;

        normalSq = sqrtf(normalSq);
        manifold->m_ContactCount = 1;
        float absX = fabs(normal[0]), absY = fabs(normal[1]), absZ = fabs(normal[2]);

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
                if (normal[0] > 0.f)
                    manifold->m_Normal = Vector3{ 1.f, 0.f, 0.f };
                else
                    manifold->m_Normal = Vector3{ -1.f, 0.f, 0.f };
            }
            else if (absY > absX && absY > absZ)
            {
                if (normal[1] > 0.f)
                    manifold->m_Normal = Vector3{ 0.f, 1.f, 0.f };
                else
                    manifold->m_Normal = Vector3{ 0.f, -1.f, 0.f };
            }
            else
            {
                if (normal[2] > 0.f)
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
        closestPt[0] = closestPt[0] > boxHE[0] ? boxHE[0] : closestPt[0] < negativeboxHE[0] ? negativeboxHE[0] : closestPt[0];
        closestPt[1] = closestPt[1] > boxHE[1] ? boxHE[1] : closestPt[1] < negativeboxHE[1] ? negativeboxHE[1] : closestPt[1];
        closestPt[2] = closestPt[2] > boxHE[2] ? boxHE[2] : closestPt[2] < negativeboxHE[2] ? negativeboxHE[2] : closestPt[2];

        bool isSphereInBox = false;

        if (dist == closestPt)
        {
            isSphereInBox = true;
            float absX = fabsf(dist[0]);
            float absY = fabsf(dist[1]);
            float absZ = fabsf(dist[2]);

            // finding the closest axis
            if (absX < absY && absX < absZ)
            {
                if (closestPt[0] > 0.f)
                    closestPt[0] = boxHE[0];
                else
                    closestPt[0] = negativeboxHE[0];
            }
            else if (absY < absX && absY < absZ)
            {
                if (closestPt[1] > 0.f)
                    closestPt[1] = boxHE[1];
                else
                    closestPt[1] = negativeboxHE[1];
            }
            else
            {
                if (closestPt[2] > 0.f)
                    closestPt[2] = boxHE[2];
                else
                    closestPt[2] = negativeboxHE[2];
            }
        }

        Vector3 normal = dist - closestPt;
        float normalSq = normal.SquareLength();

        if (normalSq > sphereRadius * sphereRadius && !isSphereInBox) return;

        normalSq = sqrtf(normalSq);
        manifold->m_ContactCount = 1;
        float absX = fabs(normal[0]), absY = fabs(normal[1]), absZ = fabs(normal[2]);

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
                if (normal[0] > 0.f)
                    manifold->m_Normal = Vector3{ 1.f, 0.f, 0.f };
                else
                    manifold->m_Normal = Vector3{ -1.f, 0.f, 0.f };
            }
            else if (absY > absX && absY > absZ)
            {
                if (normal[1] > 0.f)
                    manifold->m_Normal = Vector3{ 0.f, 1.f, 0.f };
                else
                    manifold->m_Normal = Vector3{ 0.f, -1.f, 0.f };
            }
            else
            {
                if (normal[2] > 0.f)
                    manifold->m_Normal = Vector3{ 0.f, 0.f, 1.f };
                else
                    manifold->m_Normal = Vector3{ 0.f, 0.f, -1.f };
            }*/
            manifold->m_Normal = normal.Normalized();
            manifold->m_Contacts[0] = closestPt;
        }
    }
}
