#pragma once
#ifndef USE_VULKAN

#include <future>

#include "Camera.h"

class ParticlePass
{
public:

    ParticlePass();
    ~ParticlePass();

    void Render(Camera& camera);
    void StartSortingParticleData(Camera& camera);

private:

    GLuint m_VertexArray;
    GLuint m_PositionBuffer;
    GLuint m_ColorBuffer;
    GLuint m_SizeBuffer;
    GLuint m_FadeBuffer;

    HShader m_ParticleBillboardShader;
    HShader m_ParticleStencilShader;

    std::future<void> m_DataSorted;
    std::map<std::pair<GLuint, GLuint>, std::vector<Vector4>> m_SortedPositionData;
    std::map<std::pair<GLuint, GLuint>, std::vector<Vector4>> m_SortedColorData;
    std::map<std::pair<GLuint, GLuint>, std::vector<float>> m_SortedSizeData;
    std::map<std::pair<GLuint, GLuint>, std::vector<float>> m_SortedFadeData;

    static void SortParticleData(ParticlePass& particlePass, Camera& camera);
};

#endif // !USE_VULKAN