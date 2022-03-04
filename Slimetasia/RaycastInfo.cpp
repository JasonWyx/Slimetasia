#pragma once
#include "RaycastInfo.h"

#include "CorePrerequisites.h"

RaycastInfo::RaycastInfo()
    : m_WorldHitPt(0.f, 0.f, 0.f)
    , m_WorldNormal(0.f, 0.f, 0.f)
    , m_HitFrac(0.f)
    , m_MeshSubpart(-1)
    , m_TriangleIndex(-1)
    , m_Body(nullptr)
    , m_BV(nullptr)
{
}

RaycastData_tmp::RaycastData_tmp()
    : m_WorldHitPt(0.f, 0.f, 0.f)
    , m_WorldNormal(0.f, 0.f, 0.f)
    , m_HitFrac(0.f)
    , m_HitObject(nullptr)
{
}
