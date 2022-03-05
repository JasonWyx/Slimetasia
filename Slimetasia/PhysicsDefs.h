#pragma once
#include "Factory.h"
#include "Utility.h"
#include "SmartEnums.h"

constexpr uint SPHERE_DEBUGDRAW_POINTS = 16;
constexpr uint HALF_SPHERE_DEBUGDRAW_POINTS = SPHERE_DEBUGDRAW_POINTS / 2;
constexpr float SPHERE_DEBUGDRAW_POINTS_GAP = TWO_PI / static_cast<float>(SPHERE_DEBUGDRAW_POINTS);

#define CollisionShapeType_List(m) m(CollisionShapeType, SPHERE) m(CollisionShapeType, CAPSULE) m(CollisionShapeType, CONVEX_POLY)

SMARTENUM_DEFINE_ENUM(CollisionShapeType, CollisionShapeType_List)
SMARTENUM_DEFINE_NAMES(CollisionShapeType, CollisionShapeType_List)
SMARTENUM_DEFINE_GET_VALUE_FROM_STRING(CollisionShapeType)
SMARTENUM_DEFINE_GET_ALL_VALUES_IN_STRING(CollisionShapeType)
REFLECT_ENUM(CollisionShapeType)

#define CollisionShape_List(m)                                                                                                             \
    m(CollisionShape, TRIANGLE) m(CollisionShape, CAPSULE) m(CollisionShape, SPHERE) m(CollisionShape, BOX) m(CollisionShape, CONVEX_MESH) \
        m(CollisionShape, PLANE)  //m(CollisionShape, TRIANGLE_MESH)		\
//m(CollisionShape, HEIGHT_FIELD)

SMARTENUM_DEFINE_ENUM(CollisionShape, CollisionShape_List)
SMARTENUM_DEFINE_NAMES(CollisionShape, CollisionShape_List)
SMARTENUM_DEFINE_GET_VALUE_FROM_STRING(CollisionShape)
SMARTENUM_DEFINE_GET_ALL_VALUES_IN_STRING(CollisionShape)
REFLECT_ENUM(CollisionShape)

#define Rigidbody_Bodytype_List(m)              \
    m(Bodytype, STATIC) m(Bodytype, DYNAMIC) /* \
             m(Bodytype, KINEMATIC)*/

SMARTENUM_DEFINE_ENUM(Bodytype, Rigidbody_Bodytype_List)
SMARTENUM_DEFINE_NAMES(Bodytype, Rigidbody_Bodytype_List)
SMARTENUM_DEFINE_GET_VALUE_FROM_STRING(Bodytype)
REFLECT_ENUM(Bodytype)

enum class ContactsPositionCorrectionTechnique
{
    BAUMGARTE_CONTACTS = 0,
    SPLIT_IMPULSES
};

using IndexPair = std::pair<uint, uint>;
using ColShapeID = ullong;
using BodyID = unsigned long long int;
using OverlapPairID = std::pair<uint, uint>;
using ShapePairID = std::pair<uint, uint>;

constexpr bool operator<(const std::pair<uint, uint>& first, const std::pair<uint, uint>& second)
{
    return first.first < second.first ? true : first.first == second.first ? first.second < second.second : false;
}

constexpr bool operator<=(const std::pair<uint, uint>& first, const std::pair<uint, uint>& second)
{
    return first.first <= second.first;
}

constexpr bool operator>(const std::pair<uint, uint>& first, const std::pair<uint, uint>& second)
{
    return first.first > second.first ? true : first.first == second.first ? first.second > second.second : false;
}

constexpr bool operator>=(const std::pair<uint, uint>& first, const std::pair<uint, uint>& second)
{
    return first.first >= second.first;
}

constexpr bool operator==(const std::pair<uint, uint>& first, const std::pair<uint, uint>& second)
{
    return first.first == second.first && first.second == second.second;
}

constexpr bool operator!=(const std::pair<uint, uint>& first, const std::pair<uint, uint>& second)
{
    return !(first == second);
}