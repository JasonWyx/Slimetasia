/* Start Header ----------------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.

File Name: Mesh.h
Purpose: This is the header file for the Mesh class.
Language: C++ MSVC Compiler
Platform: Windows SDK 10.0.15063.0, x64 Machine, Windows 10 x64
Project: Assignment 4 - Refraction Mapping
Author: WANG YUXUAN JASON, yuxuanjason.wang, 230000617
Creation date: 7/18/2018

End Header ------------------------------------------------------------------ */

#pragma once
#include <GL\glew.h>

#include <map>
#include <vector>

#include "AABB.h"
#include "Material.h"
#include "MathDefs.h"
#include "ResourceBase.h"
#include "ResourceHandle.h"

using HMaterial = ResourceHandle<Material>;

struct aiScene;
struct aiNode;

struct MeshBone
{
    std::string m_BoneName;
    Matrix4 m_BoneOffset;  // Moves vertex from local mesh space to bone space; INVERSE BIND POSE
};

struct MeshNode
{
    std::string m_NodeName;
    std::vector<unsigned> m_ChildrenNodes;

    Vector3 m_Scaling;
    Quaternion m_Rotation;
    Vector3 m_Translation;
};

struct MeshEntry
{
    std::string m_MeshEntryName;
    GLuint m_BaseVertex;
    GLuint m_BaseIndex;
    GLuint m_Size;
    Matrix4 m_NodeTransform;
};

enum class MeshBufferID
{
    Position = 0,
    Normal,
    Tangent,
    Bitangent,
    Color,
    TexCoord,
    JointId,
    JointWeight,
    Index,
    Count,
};

class Mesh : public ResourceBase
{
public:
    Mesh(const std::string& resourceName = "Mesh", const filesystem::path& filePath = "");
    ~Mesh();

    // Inherited via FileResourceBase
    virtual void Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem) override;
    virtual void Unserialize(tinyxml2::XMLElement* currElem) override;
    virtual void Load() override;

    void ImportFromAssimp(aiScene const* scene);

    std::vector<GLuint> const& GetIndices() const;
    std::vector<Vector3> const& GetVertices() const;
    std::vector<Vector3> const& GetNormals() const;
    std::vector<Vector3> const& GetColors() const;
    std::vector<Vector2> const& GetTexCoords() const;
    std::vector<HMaterial> const& GetMaterials() const;
    std::vector<MeshEntry> const& GetMeshEntries() const;
    std::vector<MeshBone> const& GetBones() const;
    std::map<std::string, unsigned> const& GetBonesMap() const;
    std::vector<MeshNode> const& GetNodes() const;
    std::map<std::string, unsigned> const& GetNodesMap() const;

    Matrix4 GetGlobalInverseTransform() const;
    AABB GetAABB() const;

    GLuint GetVAO() const;

private:
    std::vector<GLuint> m_Indices;
    std::vector<Vector3> m_Vertices;
    std::vector<Vector3> m_Normals;
    std::vector<Vector3> m_Tangents;
    std::vector<Vector3> m_Bitangents;
    std::vector<Vector3> m_Colors;
    std::vector<Vector2> m_TexCoords;
    std::vector<iVector4> m_JointIds;
    std::vector<Vector4> m_JointWeights;

    std::vector<MeshEntry> m_MeshEntries;
    std::vector<HMaterial> m_Materials;

    GLuint m_VertexArray;
    GLuint m_VertexBuffers[(int)MeshBufferID::Count];

    Matrix4 m_GlobalInverseTransform;
    std::vector<MeshBone> m_Bones;
    std::map<std::string, unsigned> m_BonesIDMap;
    std::vector<MeshNode> m_Nodes;
    std::map<std::string, unsigned> m_NodesIDMap;

    AABB m_AABB;

    void ParseNodes(aiNode* currNode);
    void ParseNodesChildren(aiNode* currNode);
};

using HMesh = ResourceHandle<Mesh>;