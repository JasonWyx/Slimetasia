#pragma once
#include "CorePrerequisites.h"

struct Ray;
class BoundingVolume;
class CollisionBody;

struct RaycastInfo
{
    RaycastInfo();

    ~RaycastInfo() = default;

    RaycastInfo(const RaycastInfo& rhs) = delete;

    RaycastInfo& operator=(const RaycastInfo& rhs) = delete;

    Vector3 m_WorldHitPt;

    Vector3 m_WorldNormal;

    float m_HitFrac;

    int m_MeshSubpart;

    int m_TriangleIndex;

    CollisionBody* m_Body;

    BoundingVolume* m_BV;
};

struct RaycastData_tmp
{
    RaycastData_tmp();

    ~RaycastData_tmp() = default;

    Vector3 m_WorldHitPt;

    Vector3 m_WorldNormal;

    float m_HitFrac;

    GameObject* m_HitObject;
};