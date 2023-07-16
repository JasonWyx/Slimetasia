#pragma once
#include "CorePrerequisites.h"
#include "IComponent.h"
#include "Mesh.h"
#include "SmartEnums.h"
#include "Transform.h"

#define TilingAxis_List(m) m(TilingAxis, XY) m(TilingAxis, XZ) m(TilingAxis, YZ)

SMARTENUM_DEFINE_ENUM(TilingAxis, TilingAxis_List)
SMARTENUM_DEFINE_NAMES(TilingAxis, TilingAxis_List)
SMARTENUM_DEFINE_GET_VALUE_FROM_STRING(TilingAxis)
SMARTENUM_DEFINE_GET_ALL_VALUES_IN_STRING(TilingAxis)
REFLECT_ENUM(TilingAxis)

#define TilingMode_List(m) m(TilingMode, Mirror) m(TilingMode, Repeat)
SMARTENUM_DEFINE_ENUM(TilingMode, TilingMode_List)
SMARTENUM_DEFINE_NAMES(TilingMode, TilingMode_List)
SMARTENUM_DEFINE_GET_VALUE_FROM_STRING(TilingMode)
SMARTENUM_DEFINE_GET_ALL_VALUES_IN_STRING(TilingMode)
REFLECT_ENUM(TilingMode)

class MeshRenderer : public IComponent
{
public:

    MeshRenderer(GameObject* parentObject);
    ~MeshRenderer();

    bool m_EmissiveEnabled;

    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void RevalidateResources() override;

    Transform* GetTransform();
    HMesh GetMesh() const;
    void SetMesh(HMesh mesh);
    HTexture GetDiffuseTexture() const;
    void SetDiffuseTexture(HTexture diffuse);
    HTexture GetNormalTexture() const;
    void SetNormalTexture(HTexture normal);
    HTexture GetSpecularTexture() const;
    void SetSpecularTexture(HTexture specular);
    bool IsEmissiveEnabled() const;
    HTexture GetEmissiveTexture() const;
    void SetEmissiveTexture(HTexture emissive);
    Color3 GetEmissiveColor() const;
    void SetEmissiveColor(Color3 intensity);
    Color4 GetMeshColor() const;
    void SetMeshColor(Color4 const& color);
    bool IsCastShadow() const;
    void SetCastShadow(bool cast);
#ifdef USE_VULKAN

#else
    GLuint GetTextureSampler() const;
#endif  // USE_VULKAN
    bool IsTilingEnabled() const;
    void SetTilingEnabled(bool enabled);
    TilingAxis GetTilingAxis() const;
    void SetTilingAxis(TilingAxis axis);
    TilingMode GetTilingMode() const;
    void SetTilingMode(TilingMode mode);
    float GetTilingSize() const;
    void SetTilingSize(float size);

    REFLECT()

private:

    Transform* m_Transform;
    HMesh m_Mesh;
    Color4 m_MeshColor;
    HTexture m_DiffuseTexture;
    HTexture m_NormalTexture;
    HTexture m_SpecularTexture;
    HTexture m_EmissiveTexture;
    Color3 m_EmissiveColor;
    bool m_CastShadow;
#ifdef USE_VULKAN
#else
    GLuint m_TextureSampler;
#endif  // USE_VULKAN
    bool m_TilingEnabled;
    TilingAxis m_TilingAxis;
    TilingMode m_TilingMode;
    float m_TilingSize;
};
