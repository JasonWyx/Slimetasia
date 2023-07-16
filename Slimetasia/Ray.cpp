#include "Ray.h"

#include "Layer.h"
#include "Renderer.h"

void DrawRay(const Ray& ray, float t, unsigned int layerID, Color4 color)
{
#ifdef USE_VULKAN
    auto currentLayerID = 0U;  // todo
#else
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
#endif  // USE_VULKAN

    if (layerID != currentLayerID) return;

    std::vector<Vector3> pts;

    pts.emplace_back(ray.m_start);
    pts.emplace_back(ray.m_start + t * ray.m_dir);

    // shape.mSegments.emplace_back(ray.m_start, ray.m_start + t * ray.m_dir);
    auto z = Vector3 { 0.f, 0.f, 1.f };
    auto v = Vector3 { 0.f, 0.f, 1.f }.Cross(ray.m_dir);

    if (v.SquareLength() == 0.f) v = ray.m_dir.Cross(Vector3 { 1.f, 0.f, 0.f });

    v = v.Normalized();

    v = v.Normalized();
    auto w = ray.m_dir.Cross(v);
    w = w.Normalized();
    auto discpt = ray.m_start + ray.m_dir * t * 0.85f;
    // drawing the disc.
    for (auto i = 0u, j = 0u; i < discpoints; ++i)
    {
        j = i + 1u >= discpoints ? 0u : i + 1u;
        pts.emplace_back(discpt);
        pts.emplace_back(discpt + 0.5f * (v * cosf(i * discradincrement) + w * sinf(i * discradincrement)));
        pts.emplace_back(discpt + 0.5f * (v * cosf(j * discradincrement) + w * sinf(j * discradincrement)));
        pts.emplace_back(discpt + 0.5f * (v * cosf(i * discradincrement) + w * sinf(i * discradincrement)));
    }

    auto arrowhead = ray.m_start + ray.m_dir * t;
    // drawing the arrowhead.
    for (auto i = 0u; i < raylinesegments; ++i)
    {
        pts.emplace_back(discpt + 0.5f * (v * cosf(i * rayradincrement) + w * sinf(i * rayradincrement)));
        pts.emplace_back(arrowhead);
    }

#ifdef USE_VULKAN
#else
    Renderer::Instance().DrawDebug(currentLayerID, pts, color, DebugPrimitiveType::Lines);
#endif  // USE_VULKAN
}
