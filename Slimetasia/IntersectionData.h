#pragma once

#include "CorePrerequisites.h"
#include "IComponent.h"
#include "RigidbodyComponent.h"

struct IntersectionData
{
public:
    IntersectionData(const bool& intersect = false, const float& dist = 0.f, const Vector3& dir = Vector3{}, const Vector3& push = Vector3{}, const bool& sp = false)
        : m_is_intersect(intersect)
        , m_penetration(dist)
        , m_direction{dir}
        , m_push(push)
        , m_sphere(sp)
        , m_FirstRigidbody(nullptr)
        , m_SecondRigidbody(nullptr)
    {
    }
    IntersectionData& operator=(const IntersectionData& a)
    {
        m_is_intersect = a.m_is_intersect;
        m_penetration = a.m_penetration;
        m_direction = a.m_direction;
        m_push = a.m_push;
        m_sphere = a.m_sphere;
        return *this;
    }
    bool operator==(const IntersectionData& a)
    {
        return  // m_is_intersect == a.m_is_intersect &&
                // m_penetration == a.m_penetration &&
                // m_direction == a.m_direction &&
                // m_push == a.m_push &&
                // m_sphere == a.m_sphere &&
            m_FirstRigidbody == a.m_FirstRigidbody && m_SecondRigidbody == a.m_SecondRigidbody;
    }
    bool operator==(RigidbodyComponent* a)
    {
        return  // m_is_intersect == a.m_is_intersect &&
                // m_penetration == a.m_penetration &&
                // m_direction == a.m_direction &&
                // m_push == a.m_push &&
                // m_sphere == a.m_sphere &&
            m_FirstRigidbody == a || m_SecondRigidbody == a;
    }
    /*IntersectionData operator-()const
    {
        auto tmp{ *this };
        tmp.m_direction;
    }*/

    IntersectionData operator-() const
    {
        IntersectionData tmp(*this);
        tmp.m_direction *= -1.f;
        tmp.m_push *= -1.f;
        tmp.m_FirstRigidbody = m_SecondRigidbody;
        tmp.m_SecondRigidbody = m_FirstRigidbody;
        return tmp;
    }

    bool m_is_intersect;
    float m_penetration;
    Vector3 m_direction;
    Vector3 m_push;
    bool m_sphere;
    RigidbodyComponent* m_FirstRigidbody;
    RigidbodyComponent* m_SecondRigidbody;
};

using IntersectionList = std::vector<IntersectionData>;