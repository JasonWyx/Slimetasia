#ifndef USE_VULKAN

#include "ParticlePass.h"

#include <algorithm>
#include <map>
#include <vector>

#include "ParticleSystem.h"
#include "ResourceManager.h"

ParticlePass::ParticlePass()
    : m_VertexArray(0)
    , m_PositionBuffer(0)
    , m_ColorBuffer(0)
    , m_SizeBuffer(0)
    , m_FadeBuffer(0)
    , m_ParticleBillboardShader(ResourceManager::Instance().CreateResource<Shader>("ParticleBillboardShader"))
    , m_ParticleStencilShader(ResourceManager::Instance().CreateResource<Shader>("ParticleStencilShader"))
{
    glCreateVertexArrays(1, &m_VertexArray);
    glCreateBuffers(1, &m_PositionBuffer);
    glCreateBuffers(1, &m_ColorBuffer);
    glCreateBuffers(1, &m_SizeBuffer);
    glCreateBuffers(1, &m_FadeBuffer);

    glNamedBufferStorage(m_PositionBuffer, 1000000 * sizeof(Vector4), 0, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(m_ColorBuffer, 1000000 * sizeof(Vector4), 0, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(m_SizeBuffer, 1000000 * sizeof(GLfloat), 0, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(m_FadeBuffer, 1000000 * sizeof(GLfloat), 0, GL_DYNAMIC_STORAGE_BIT);

    glVertexArrayVertexBuffer(m_VertexArray, 0, m_PositionBuffer, 0, sizeof(Vector4));
    glVertexArrayVertexBuffer(m_VertexArray, 1, m_ColorBuffer, 0, sizeof(Vector4));
    glVertexArrayVertexBuffer(m_VertexArray, 2, m_SizeBuffer, 0, sizeof(GLfloat));
    glVertexArrayVertexBuffer(m_VertexArray, 3, m_FadeBuffer, 0, sizeof(GLfloat));

    glEnableVertexArrayAttrib(m_VertexArray, 0);
    glVertexArrayAttribBinding(m_VertexArray, 0, 0);
    glVertexArrayAttribFormat(m_VertexArray, 0, 4, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(m_VertexArray, 1);
    glVertexArrayAttribBinding(m_VertexArray, 1, 1);
    glVertexArrayAttribFormat(m_VertexArray, 1, 4, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(m_VertexArray, 2);
    glVertexArrayAttribBinding(m_VertexArray, 2, 2);
    glVertexArrayAttribFormat(m_VertexArray, 2, 1, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(m_VertexArray, 3);
    glVertexArrayAttribBinding(m_VertexArray, 3, 3);
    glVertexArrayAttribFormat(m_VertexArray, 3, 1, GL_FLOAT, GL_FALSE, 0);

    m_ParticleBillboardShader->SetVertShaderFilePath("ParticleBillboard.vert");
    m_ParticleBillboardShader->SetGeomShaderFilePath("ParticleBillboard.geom");
    m_ParticleBillboardShader->SetFragShaderFilePath("ParticleBillboard.frag");
    m_ParticleBillboardShader->Compile();

    m_ParticleStencilShader->SetVertShaderFilePath("ParticleStencil.vert");
    m_ParticleStencilShader->SetGeomShaderFilePath("ParticleStencil.geom");
    m_ParticleStencilShader->SetFragShaderFilePath("BasicEmpty.frag");
    m_ParticleStencilShader->Compile();
}

ParticlePass::~ParticlePass()
{
    glDeleteBuffers(1, &m_PositionBuffer);
    glDeleteBuffers(1, &m_ColorBuffer);
    glDeleteBuffers(1, &m_SizeBuffer);
    glDeleteBuffers(1, &m_FadeBuffer);
    glDeleteVertexArrays(1, &m_VertexArray);
}

void ParticlePass::Render(Camera& camera)
{
    const unsigned CameraLayerID = camera.GetOwner()->GetParentLayer()->GetId();
    ParticleData* data = ParticleSystem::Instance().finalData();

    std::map<std::pair<GLuint, GLuint>, std::vector<Vector4>> drawPositionData;
    std::map<std::pair<GLuint, GLuint>, std::vector<Vector4>> drawColorData;
    std::map<std::pair<GLuint, GLuint>, std::vector<float>> drawSizeData;
    std::map<std::pair<GLuint, GLuint>, std::vector<float>> drawFadeData;

    Vector3 const& camDirection = camera.GetLookAtDirection();
    Vector3 const& camPosition = camera.GetTransform()->GetWorldPosition();

    std::map<size_t, GLuint> textureMap;

    for (unsigned i = 0; i < data->m_CountAlive; ++i)
    {
        Vector3 const& particlePosition = Vector3(data->m_Position[i][0], data->m_Position[i][1], data->m_Position[i][2]);

        // Check particle infront of camera
        if (camDirection.Dot(particlePosition - camPosition) < 0) continue;

        // Value interpolant
        Vector4 color = data->m_Color[i];

        GLuint startTextureHandle = 0;
        GLuint endTextureHandle = 0;

        if (data->m_Texture[i].Validate())
        {
            auto it = textureMap.find(data->m_Texture[i]);

            if (it != textureMap.end())
            {
                startTextureHandle = it->second;
            }
            else
            {
                textureMap[data->m_Texture[i]] = data->m_Texture[i]->GetHandle();
                startTextureHandle = textureMap[data->m_Texture[i]];
            }
        }

        if (data->m_EndTexture[i].Validate())
        {
            auto it = textureMap.find(data->m_EndTexture[i]);

            if (it != textureMap.end())
            {
                endTextureHandle = it->second;
            }
            else
            {
                textureMap[data->m_EndTexture[i]] = data->m_EndTexture[i]->GetHandle();
                endTextureHandle = textureMap[data->m_EndTexture[i]];
            }
        }

        std::pair<GLuint, GLuint> texHandles(startTextureHandle, endTextureHandle);

        if (CameraLayerID == data->m_LayerID[i])
        {
            drawPositionData[texHandles].emplace_back(data->m_Position[i]);
            drawColorData[texHandles].emplace_back(color);
            drawSizeData[texHandles].emplace_back(data->m_Size[i]);
            drawFadeData[texHandles].emplace_back(data->m_TextureFade[i]);
        }
    }

    glBindVertexArray(m_VertexArray);

    if (m_ParticleBillboardShader->Enable())
    {
        static GLuint viewTransformLoc = glGetUniformLocation(m_ParticleBillboardShader->GetProgramHandle(), "gViewTransform");
        static GLuint projTransformLoc = glGetUniformLocation(m_ParticleBillboardShader->GetProgramHandle(), "gProjTransform");
        static GLuint textureLoc = glGetUniformLocation(m_ParticleBillboardShader->GetProgramHandle(), "gBillboardTexture");
        static GLuint endTextureLoc = glGetUniformLocation(m_ParticleBillboardShader->GetProgramHandle(), "gBillboardEndTexture");

        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glUniformMatrix4fv(viewTransformLoc, 1, GL_FALSE, ValuePtrFloat(camera.GetViewTransform()));
        glUniformMatrix4fv(projTransformLoc, 1, GL_FALSE, ValuePtrFloat(camera.GetProjTransform()));

        std::map<std::pair<GLuint, GLuint>, std::vector<Vector4>>::iterator drawPositionIt;
        std::map<std::pair<GLuint, GLuint>, std::vector<Vector4>>::iterator drawColorIt;
        std::map<std::pair<GLuint, GLuint>, std::vector<float>>::iterator drawSizeIt;
        std::map<std::pair<GLuint, GLuint>, std::vector<float>>::iterator drawFadeIt;

        if (m_DataSorted.valid())
        {
            m_DataSorted.wait();
            drawPositionIt = m_SortedPositionData.begin();
            drawColorIt = m_SortedColorData.begin();
            drawSizeIt = m_SortedSizeData.begin();
            drawFadeIt = m_SortedFadeData.begin();
        }
        else
        {
            drawPositionIt = drawPositionData.begin();
            drawColorIt = drawColorData.begin();
            drawSizeIt = drawSizeData.begin();
            drawFadeIt = drawFadeData.begin();
        }

        size_t dataSize = m_DataSorted.valid() ? m_SortedPositionData.size() : drawPositionData.size();

        for (std::size_t i = 0; i < dataSize; ++i)
        {
            std::pair<GLuint, GLuint> texturesHandle = drawPositionIt->first;
            GLuint startTexture = texturesHandle.first;
            GLuint endTexture = texturesHandle.second;

            glBindTextureUnit(0, startTexture);
            glBindTextureUnit(1, endTexture);

            glUniform1i(textureLoc, 0);
            glUniform1i(endTextureLoc, 1);

            glNamedBufferSubData(m_PositionBuffer, 0, drawPositionIt->second.size() * sizeof(drawPositionIt->second[0]), drawPositionIt->second.data());
            glNamedBufferSubData(m_ColorBuffer, 0, drawColorIt->second.size() * sizeof(drawColorIt->second[0]), drawColorIt->second.data());
            glNamedBufferSubData(m_SizeBuffer, 0, drawSizeIt->second.size() * sizeof(drawSizeIt->second[0]), drawSizeIt->second.data());
            glNamedBufferSubData(m_FadeBuffer, 0, drawFadeIt->second.size() * sizeof(drawFadeIt->second[0]), drawFadeIt->second.data());

            glDrawArrays(GL_POINTS, 0, (GLsizei)drawPositionIt->second.size());

            drawPositionIt++;
            drawColorIt++;
            drawSizeIt++;
            drawFadeIt++;
        }

        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }
}

void ParticlePass::StartSortingParticleData(Camera& camera)
{
    m_DataSorted = Application::Instance().GetThreadPool().enqueue(ParticlePass::SortParticleData, std::ref(*this), std::ref(camera));
}

void ParticlePass::SortParticleData(ParticlePass& particlePass, Camera& camera)
{
    const unsigned CameraLayerID = camera.GetOwner()->GetParentLayer()->GetId();
    ParticleData* data = ParticleSystem::Instance().finalData();

    std::map<std::pair<GLuint, GLuint>, std::vector<Vector4>> drawPositionData;
    std::map<std::pair<GLuint, GLuint>, std::vector<Vector4>> drawColorData;
    std::map<std::pair<GLuint, GLuint>, std::vector<float>> drawSizeData;
    std::map<std::pair<GLuint, GLuint>, std::vector<float>> drawFadeData;

    Vector3 const& camDirection = camera.GetLookAtDirection();
    Vector3 const& camPosition = camera.GetTransform()->GetWorldPosition();

    for (unsigned i = 0; i < data->m_CountAlive; ++i)
    {
        Vector3 const& particlePosition = Vector3(data->m_Position[i][0], data->m_Position[i][1], data->m_Position[i][2]);

        // Check particle infront of camera
        if (camDirection.Dot(particlePosition - camPosition) < 0) continue;

        // Value interpolant
        Vector4 color = data->m_Color[i];

        GLuint textureHandle = data->m_Texture[i].Validate() ? data->m_Texture[i]->GetHandle() : 0;

        GLuint endTextureHandle = data->m_EndTexture[i].Validate() ? data->m_EndTexture[i]->GetHandle() : 0;

        std::pair<GLuint, GLuint> texHandles(textureHandle, endTextureHandle);

        if (CameraLayerID == data->m_LayerID[i])
        {
            drawPositionData[texHandles].emplace_back(data->m_Position[i]);
            drawColorData[texHandles].emplace_back(color);
            drawSizeData[texHandles].emplace_back(data->m_Size[i]);
            drawFadeData[texHandles].emplace_back(data->m_TextureFade[i]);
        }
    }

    // Perform sort from back to front
    auto positionIt = drawPositionData.begin();
    auto colorIt = drawColorData.begin();
    auto sizeIt = drawSizeData.begin();
    auto fadeIt = drawFadeData.begin();

    for (std::size_t i = 0; i < drawPositionData.size(); ++i)
    {
        for (std::size_t j = 0; j + 1 < positionIt->second.size(); ++j)
        {
            for (std::size_t k = 0; k + 1 < positionIt->second.size(); ++k)
            {
                Vector3 const& particlePosition1 = Vector3(positionIt->second[k][0], positionIt->second[k][1], positionIt->second[k][2]);
                Vector3 const& particlePosition2 = Vector3(positionIt->second[k + 1][0], positionIt->second[k + 1][1], positionIt->second[k + 1][2]);

                float distance1 = camDirection.Dot(particlePosition1 - camPosition);
                float distance2 = camDirection.Dot(particlePosition2 - camPosition);

                if (distance1 > distance2)
                {
                    std::swap(positionIt->second[k], positionIt->second[k + 1]);
                    std::swap(colorIt->second[k], colorIt->second[k + 1]);
                    std::swap(sizeIt->second[k], sizeIt->second[k + 1]);
                    std::swap(fadeIt->second[k], fadeIt->second[k + 1]);
                }
            }
        }
        sizeIt++;
        positionIt++;
        colorIt++;
        fadeIt++;
    }

    particlePass.m_SortedPositionData = std::move(drawPositionData);
    particlePass.m_SortedColorData = std::move(drawColorData);
    particlePass.m_SortedSizeData = std::move(drawSizeData);
    particlePass.m_SortedFadeData = std::move(drawFadeData);
}

#endif // !USE_VULKAN