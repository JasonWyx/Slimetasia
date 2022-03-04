#pragma once
// functional include unordered_map, thus crbegin and rbegin are also included.
#include <functional>

#include "MathDefs.h"
#include "MemoryAllocator.h"

#define DUFF_DEVICE_4(aCount, aAction)  \
    do                                  \
    {                                   \
        int count_ = (aCount);          \
        int times_ = (count_ + 3) >> 2; \
        switch (count_ & 3)             \
        {                               \
            case 0:                     \
                do                      \
                {                       \
                    aAction;            \
                    case 3: aAction;    \
                    case 2: aAction;    \
                    case 1: aAction;    \
                } while (--times_ > 0); \
        }                               \
    } while (false)

#define DUFF_DEVICE_8(aCount, aAction)  \
    do                                  \
    {                                   \
        int count_ = (aCount);          \
        int times_ = (count_ + 7) >> 3; \
        switch (count_ & 7)             \
        {                               \
            case 0:                     \
                do                      \
                {                       \
                    aAction;            \
                    case 7: aAction;    \
                    case 6: aAction;    \
                    case 5: aAction;    \
                    case 4: aAction;    \
                    case 3: aAction;    \
                    case 2: aAction;    \
                    case 1: aAction;    \
                } while (--times_ > 0); \
        }                               \
    } while (false)

#define DUFF_DEVICE_16(aCount, aAction)  \
    do                                   \
    {                                    \
        int count_ = (aCount);           \
        int times_ = (count_ + 15) >> 4; \
        switch (count_ & 15)             \
        {                                \
            case 0:                      \
                do                       \
                {                        \
                    aAction;             \
                    case 15: aAction;    \
                    case 14: aAction;    \
                    case 13: aAction;    \
                    case 12: aAction;    \
                    case 11: aAction;    \
                    case 10: aAction;    \
                    case 9: aAction;     \
                    case 8: aAction;     \
                    case 7: aAction;     \
                    case 6: aAction;     \
                    case 5: aAction;     \
                    case 4: aAction;     \
                    case 3: aAction;     \
                    case 2: aAction;     \
                    case 1: aAction;     \
                } while (--times_ > 0);  \
        }                                \
    } while (false)

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using uint = unsigned int;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using ulong = unsigned long int;
using ullong = unsigned long long int;
using ushort = unsigned short;

namespace PE
{
    template <typename T> void hash2(size_t& seed, const T& val)
    {
        std::hash<T> hasher;
        seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <typename Container> struct reverse_wrapper
    {
        reverse_wrapper(Container& inp)
            : m_Container(inp)
        {
        }

        Container& m_Container;
    };

    template <typename Container> auto begin(const reverse_wrapper<Container>& inp) -> decltype(std::rbegin(inp.mContainer)) { return std::rbegin(inp.mContainer); }

    template <typename Container> auto cbegin(const reverse_wrapper<Container>& inp) -> decltype(std::crbegin(inp.mContainer)) { return std::crbegin(inp.mContainer); }

    template <typename Container> auto end(const reverse_wrapper<Container>& inp) -> decltype(std::rend(inp.mContainer)) { return std::rend(inp.mContainer); }

    template <typename Container> auto cend(const reverse_wrapper<Container>& inp) -> decltype(std::crend(inp.mContainer)) { return std::crend(inp.mContainer); }

    template <typename Container> static reverse_wrapper<Container> Reverse(Container&& cont) { return reverse_wrapper<Container>(std::forward<Container>(cont)); }

    inline bool approxEqual(const float& a, const float& b, const float& epsilon = EPSILON) { return (std::fabs(a - b) < epsilon); }

    // Function to test if two vectors are (almost) equal
    inline bool approxEqual(const Vector3& vec1, const Vector3& vec2, const float& epsilon)
    {
        return approxEqual(vec1.x, vec2.x, epsilon) && approxEqual(vec1.y, vec2.y, epsilon) && approxEqual(vec1.z, vec2.z, epsilon);
    }

    // Function to test if two vectors are (almost) equal
    inline bool approxEqual(const Vector2& vec1, const Vector2& vec2, const float& epsilon) { return approxEqual(vec1.x, vec2.x, epsilon) && approxEqual(vec1.y, vec2.y, epsilon); }

    template <typename T> bool AreVectorsParallel(const TVector3<T>& lhs, const TVector3<T>& rhs) { return lhs.Cross(rhs).SquareLength() < static_cast<T>(EPSILON); }

    template <typename T> bool AreVectorsOrthogonal(const TVector3<T>& lhs, const TVector3<T>& rhs) { return std::abs(lhs.Dot(rhs)) < static_cast<T>(EPSILON); }

    template <typename T> T Clamp(const T& inp, const T& min, const T& max) { return inp < min ? min : inp > max ? max : inp; }

    template <typename T> TVector3<T> Clamp(const TVector3<T>& vec, const T& maxLength)
    {
        if (vec.SquareLength() > maxLength * maxLength)
        {
            return vec.Normalized() * maxLength;
        }
        return vec;
    }

    template <typename T> Vector3 GetClosestPointOnSegment(const Vector3& segFirstPt, const Vector3& segSecondPt, const Vector3& point)
    {
        const Vector3 ab = segSecondPt - segFirstPt;

        float abLengthSquare = ab.SquareLength();

        // If the segment has almost zero length
        if (abLengthSquare < EPSILON)
        {
            // Return one end-point of the segment as the closest point
            return segFirstPt;
        }

        // Project point C onto "AB" line
        float t = (point - segFirstPt).Dot(ab) / abLengthSquare;

        // If projected point onto the line is outside the segment, clamp it to the segment
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;

        // Return the closest point on the segment
        return segFirstPt + t * ab;
    }

    inline void GetClosestPointsOnTwoSegments(const Vector3& firstSegFirstPt, const Vector3& firstSegSecondPt, const Vector3& secondSegFirstPt, const Vector3& secondSegSecondPt, Vector3& firstSegPt,
                                              Vector3& secondSegPt)
    {
        const Vector3 d1 = firstSegSecondPt - firstSegFirstPt;
        const Vector3 d2 = secondSegSecondPt - secondSegFirstPt;
        const Vector3 r = firstSegFirstPt - secondSegFirstPt;
        float a = d1.SquareLength();
        float e = d2.SquareLength();
        float f = d2.Dot(r);
        float s, t;

        // If both segments degenerate into points
        if (a <= EPSILON && e <= EPSILON)
        {
            firstSegPt = firstSegFirstPt;
            secondSegPt = secondSegFirstPt;
            return;
        }

        if (a <= EPSILON)
        {  // If first segment degenerates into a point

            s = 0.f;

            // Compute the closest point on second segment
            t = Clamp(f / e, 0.f, 1.f);
        }
        else
        {
            float c = d1.Dot(r);

            // If the second segment degenerates into a point
            if (e <= EPSILON)
            {
                t = 0.f;
                s = Clamp(-c / a, 0.f, 1.f);
            }
            else
            {
                float b = d1.Dot(d2);
                float denom = a * e - b * b;

                // If the segments are not parallel
                if (denom != 0.f)
                {
                    // Compute the closest point on line 1 to line 2 and
                    // clamp to first segment.
                    s = Clamp((b * f - c * e) / denom, 0.f, 1.f);
                }
                else
                {
                    // Pick an arbitrary point on first segment
                    s = 0.f;
                }

                // Compute the point on line 2 closest to the closest point
                // we have just found
                t = (b * s + f) / e;

                // If this closest point is inside second segment (t in [0, 1]), we are done.
                // Otherwise, we clamp the point to the second segment and compute again the
                // closest point on segment 1
                if (t < 0.f)
                {
                    t = 0.f;
                    s = Clamp(-c / a, 0.f, 1.f);
                }
                else if (t > 1.f)
                {
                    t = 1.f;
                    s = Clamp((b - c) / a, 0.f, 1.f);
                }
            }
        }

        // Compute the closest points on both segments
        firstSegPt = firstSegFirstPt + d1 * s;
        secondSegPt = secondSegFirstPt + d2 * t;
    }

    // Compute the barycentric coordinates of a point w.r.t an input triangle.
    inline void computeBarycentricCoordinatesInTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w)
    {
        const Vector3 v0 = b - a;
        const Vector3 v1 = c - a;
        const Vector3 v2 = p - a;

        float d00 = v0.Dot(v0);
        float d01 = v0.Dot(v1);
        float d11 = v1.Dot(v1);
        float d20 = v2.Dot(v0);
        float d21 = v2.Dot(v1);

        float denom = d00 * d11 - d01 * d01;
        v = (d11 * d20 - d01 * d21) / denom;
        w = (d00 * d21 - d01 * d20) / denom;
        u = 1.f - v - w;
    }

    // Compute the intersection between a plane and a segment
    inline float computePlaneSegmentIntersection(const Vector3& segA, const Vector3& segB, const float& planeD, const Vector3& planeNormal)
    {
        float t = -1.f;

        float nDotAB = planeNormal.Dot(segB - segA);

        // If the segment is not parallel to the plane
        if (std::abs(nDotAB) > EPSILON)
        {
            t = (planeD - planeNormal.Dot(segA)) / nDotAB;
        }

        return t;
    }

    // Compute the distance between a point and a line
    inline float computePointToLineDistance(const Vector3& linePointA, const Vector3& linePointB, const Vector3& point)
    {
        float distAB = (linePointB - linePointA).Length();

        if (distAB < EPSILON)
        {
            return (point - linePointA).Length();
        }

        return ((point - linePointA).Cross(point - linePointB)).Length() / distAB;
    }

    // Project a point onto a plane that is given by a point and its unit length normal
    inline Vector3 projectPointOntoPlane(const Vector3& point, const Vector3& planeNormal, const Vector3& planePoint) { return point - planeNormal.Dot(point - planePoint) * planeNormal; }

    // Return the distance between a point and a plane (the plane normal must be normalized)
    inline float computePointToPlaneDistance(const Vector3& point, const Vector3& planeNormal, const Vector3& planePoint) { return planeNormal.Dot(point - planePoint); }

}  // namespace PE
