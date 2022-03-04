#include "PECollisionDispatch.h"

#include <cassert>

#include "PhysicsDefs.h"

BaseDetectionAlgorithm* PECollisionDispatch::SelectAlgorithm(const CollisionShapeType& shape1, const CollisionShapeType& shape2)
{
    switch (shape1)
    {
        case eCollisionShapeType_CAPSULE:

            switch (shape2)
            {
                case eCollisionShapeType_CAPSULE: return &m_CapsuleCapsule;

                case eCollisionShapeType_CONVEX_POLY: return &m_CapsuleConvex;

                case eCollisionShapeType_SPHERE: return &m_SphereCapsule;

                default:
                    std::cout << "An invalid type is passed in as shape2 parameter!\n";
                    assert(0);
                    break;
            }

            break;

        case eCollisionShapeType_CONVEX_POLY:

            switch (shape2)
            {
                case eCollisionShapeType_CAPSULE: return &m_CapsuleConvex;

                case eCollisionShapeType_CONVEX_POLY: return &m_ConvexConvex;

                case eCollisionShapeType_SPHERE: return &m_SphereConvex;

                default:
                    std::cout << "An invalid type is passed in as shape2 parameter!\n";
                    assert(0);
                    break;
            }

            break;

        case eCollisionShapeType_SPHERE:

            switch (shape2)
            {
                case eCollisionShapeType_CAPSULE: return &m_SphereCapsule;

                case eCollisionShapeType_CONVEX_POLY: return &m_SphereConvex;

                case eCollisionShapeType_SPHERE: return &m_SphereSphere;

                default:
                    std::cout << "An invalid type is passed in as shape2 parameter!\n";
                    assert(0);
                    break;
            }

            break;

        default:
            std::cout << "An invalid type is passed in as shape1 parameter!\n";
            assert(0);
            break;
    }

    return nullptr;
}
