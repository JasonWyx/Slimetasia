#pragma once
#include "CorePrerequisites.h"

#define discpoints 16u
#define raylinesegments 8u
#define discradincrement RAD360 / static_cast<float>(discpoints)
#define rayradincrement RAD360 / static_cast<float>(raylinesegments)

struct Ray
{
    Ray(const Vector3& start = Vector3 {}, const Vector3& dir = Vector3 {}, const float& max = 1.f)
        : m_start { start }
        , m_dir { dir }
        , m_MaxFraction { max }
    {
    }

    Ray(const Ray& rhs)
        : m_start { rhs.m_start }
        , m_dir { rhs.m_dir }
        , m_MaxFraction { rhs.m_MaxFraction }
    {
    }

    Ray& operator=(const Ray& rhs)
    {
        // check for self assignment
        if (this != &rhs)
        {
            m_start = rhs.m_start;
            m_dir = rhs.m_dir;
            m_MaxFraction = rhs.m_MaxFraction;
        }
        return *this;
    }

    ~Ray() = default;

    Vector3 m_start;
    Vector3 m_dir;
    float m_MaxFraction;
};

void DrawRay(const Ray& ray, float t, unsigned int layerID, Color4 color = Color4(0.f, 1.f, 0.f, 1.f));