#include "MeshRenderer.h"

#include "GameObject.h"
#include "Layer.h"
#include "MeshAnimator.h"

REFLECT_INIT(MeshRenderer)
REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(m_Mesh)
REFLECT_PROPERTY(m_MeshColor)
REFLECT_PROPERTY(m_DiffuseTexture)
REFLECT_PROPERTY(m_NormalTexture)
REFLECT_PROPERTY(m_SpecularTexture)
REFLECT_PROPERTY(m_EmissiveEnabled)
REFLECT_PROPERTY(m_EmissiveTexture)
REFLECT_PROPERTY(m_EmissiveColor)
REFLECT_PROPERTY(m_CastShadow)
REFLECT_PROPERTY(m_TilingEnabled)
REFLECT_PROPERTY(m_TilingAxis)
REFLECT_PROPERTY(m_TilingMode)
REFLECT_PROPERTY(m_TilingSize)
REFLECT_END()

MeshRenderer::MeshRenderer(GameObject* parentObject)
    : IComponent(parentObject, "MeshRenderer")
    , m_Transform(nullptr)
    , m_Mesh()
    ,
    // m_Materials(),
    m_MeshColor(1.0f, 1.0f, 1.0f, 1.0f)
    , m_DiffuseTexture()
    , m_NormalTexture()
    , m_SpecularTexture()
    , m_CastShadow(true)
    , m_TilingEnabled(false)
    , m_TilingAxis(TilingAxis::eTilingAxis_XZ)
    , m_TilingMode(TilingMode::eTilingMode_Repeat)
    , m_TilingSize(1.0f)
{
    if (parentObject)
    {
        m_OwnerObject->AddIfDoesntExist<Transform>();
        m_Transform = GetOwner()->GetComponent<Transform>();
    }

    glCreateSamplers(1, &m_TextureSampler);
    glSamplerParameteri(m_TextureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_TextureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_TextureSampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(m_TextureSampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

MeshRenderer::~MeshRenderer()
{
    OnInactive();
    glDeleteSamplers(1, &m_TextureSampler);
}

void MeshRenderer::OnActive()
{
    m_OwnerObject->GetParentLayer()->GetRenderLayer().AddMeshRenderer(this);
}

void MeshRenderer::OnInactive()
{
    if (m_OwnerObject && m_OwnerObject->GetParentLayer()) m_OwnerObject->GetParentLayer()->GetRenderLayer().RemoveMeshRenderer(this);
}

void MeshRenderer::RevalidateResources()
{
    // m_Mesh.Validate();
    // m_DiffuseTexture.Validate();
    // m_NormalTexture.Validate();
    // m_SpecularTexture.Validate();
}

Transform* MeshRenderer::GetTransform()
{
    return m_Transform;
}

HMesh MeshRenderer::GetMesh() const
{
    return m_Mesh;
}

void MeshRenderer::SetMesh(HMesh mesh)
{
    m_Mesh = mesh;
    if (MeshAnimator* meshAnimator = m_OwnerObject->GetComponent<MeshAnimator>())
    {
        meshAnimator->InitMeshAnimator();
    }
}

HTexture MeshRenderer::GetDiffuseTexture() const
{
    return m_DiffuseTexture;
}

void MeshRenderer::SetDiffuseTexture(HTexture diffuse)
{
    m_DiffuseTexture = diffuse;
}

HTexture MeshRenderer::GetNormalTexture() const
{
    return m_NormalTexture;
}

void MeshRenderer::SetNormalTexture(HTexture normal)
{
    m_NormalTexture = normal;
}

HTexture MeshRenderer::GetSpecularTexture() const
{
    return m_SpecularTexture;
}

void MeshRenderer::SetSpecularTexture(HTexture specular)
{
    m_SpecularTexture = specular;
}

bool MeshRenderer::IsEmissiveEnabled() const
{
    return m_EmissiveEnabled;
}

HTexture MeshRenderer::GetEmissiveTexture() const
{
    return m_EmissiveTexture;
}

void MeshRenderer::SetEmissiveTexture(HTexture emissive)
{
    m_EmissiveTexture = emissive;
}

Color3 MeshRenderer::GetEmissiveColor() const
{
    return m_EmissiveColor;
}

void MeshRenderer::SetEmissiveColor(Color3 color)
{
    m_EmissiveColor = color;
}

Color4 MeshRenderer::GetMeshColor() const
{
    return m_MeshColor;
}

void MeshRenderer::SetMeshColor(Color4 const& color)
{
    m_MeshColor = color;
}

bool MeshRenderer::IsCastShadow() const
{
    return m_CastShadow;
}

void MeshRenderer::SetCastShadow(bool cast)
{
    m_CastShadow = cast;
}

GLuint MeshRenderer::GetTextureSampler() const
{
    switch (m_TilingMode)
    {
        case eTilingMode_Repeat:
        {
            glSamplerParameteri(m_TextureSampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glSamplerParameteri(m_TextureSampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        break;

        case eTilingMode_Mirror:
        {
            glSamplerParameteri(m_TextureSampler, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glSamplerParameteri(m_TextureSampler, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        }
        break;
    }

    return m_TextureSampler;
}

bool MeshRenderer::IsTilingEnabled() const
{
    return m_TilingEnabled;
}

void MeshRenderer::SetTilingEnabled(bool enabled)
{
    m_TilingEnabled = enabled;
}

TilingAxis MeshRenderer::GetTilingAxis() const
{
    return m_TilingAxis;
}

void MeshRenderer::SetTilingAxis(TilingAxis axis)
{
    m_TilingAxis = axis;
}

TilingMode MeshRenderer::GetTilingMode() const
{
    return m_TilingMode;
}

void MeshRenderer::SetTilingMode(TilingMode mode)
{
    m_TilingMode = mode;
}

float MeshRenderer::GetTilingSize() const
{
    return m_TilingSize;
}

void MeshRenderer::SetTilingSize(float size)
{
    m_TilingSize = size;
}
