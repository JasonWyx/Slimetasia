/* Start Header ----------------------------------------------------------------
  Copyright (C) 2018 DigiPen Institute of Technology.
  Reproduction or disclosure of this file or its contents without the
  prior written consent of DigiPen Institute of Technology is prohibited.

  File Name: Mesh.cpp
  Purpose: This is the implementation file for the Mesh class.
  Language: C++ MSVC Compiler
  Platform: Windows SDK 10.0.15063.0, x64 Machine, Windows 10 x64
  Project: Assignment 4 - Refraction Mapping
  Author: WANG YUXUAN JASON, yuxuanjason.wang, 230000617
  Creation date: 7/18/2018

End Header ------------------------------------------------------------------ */

#include "Mesh.h"

#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <fstream>
#include <sstream>
#include <string>

#include "CorePrerequisites.h"
#include "Material.h"
#include "ResourceManager.h"

auto ConvertAssimpMat4 = [](aiMatrix4x4 const& matrix)
{ return Matrix4(matrix.a1, matrix.b1, matrix.c1, matrix.d1, matrix.a2, matrix.b2, matrix.c2, matrix.d2, matrix.a3, matrix.b3, matrix.c3, matrix.d3, matrix.a4, matrix.b4, matrix.c4, matrix.d4); };

Mesh::Mesh(const std::string& resourceName, const std::filesystem::path& filePath)
    : ResourceBase(resourceName, filePath)
    , m_Indices()
    , m_Vertices()
    , m_Normals()
    , m_Tangents()
    , m_Bitangents()
    , m_Colors()
    , m_TexCoords()
    , m_JointIds()
    , m_JointWeights()
    , m_MeshEntries()
    , m_Materials()
    , m_VertexArray(0)
    , m_VertexBuffers { 0 }
    , m_GlobalInverseTransform()
    , m_Bones()
    , m_Nodes()
    , m_NodesIDMap()
{
    glCreateVertexArrays(1, &m_VertexArray);
    glCreateBuffers((int)MeshBufferID::Count, m_VertexBuffers);
}

Mesh::~Mesh()
{
    glDeleteBuffers((int)MeshBufferID::Count, m_VertexBuffers);
    glDeleteVertexArrays(1, &m_VertexArray);
}

void Mesh::Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem)
{
    tinyxml2::XMLElement* resourceElem = doc->NewElement("Mesh");
    tinyxml2::XMLElement* nameElem = doc->NewElement("Name");
    tinyxml2::XMLElement* filepathElem = doc->NewElement("Filepath");

    resourceElem->SetAttribute("GUID", static_cast<int64_t>(m_GUID));
    nameElem->SetText(m_Name.c_str());
    filepathElem->SetText(m_FilePath.string().c_str());

    parentElem->InsertEndChild(resourceElem);
    resourceElem->InsertEndChild(nameElem);
    resourceElem->InsertEndChild(filepathElem);
}

void Mesh::Unserialize(tinyxml2::XMLElement* currElem)
{
    m_GUID = (ResourceGUID)currElem->Int64Attribute("GUID");
    m_Name = currElem->FirstChildElement("Name")->GetText();
    m_FilePath = currElem->FirstChildElement("Filepath")->GetText();
}

void Mesh::Load()
{
    // Mesh not loaded
    if (m_FilePath.extension() != ".mesh")
    {
        Assimp::Importer importer;
        aiScene const* scene = importer.ReadFile(m_FilePath.string().c_str(), aiProcessPreset_TargetRealtime_Quality ^ aiProcess_GenUVCoords);

        ImportFromAssimp(scene);
        return;
    }

    std::ifstream inFile = std::ifstream(m_FilePath, std::ios::binary);

    if (!inFile.is_open()) return;

    ASSERT(inFile.is_open());

    while (!inFile.eof())
    {
        int type = -1;
        std::size_t size = 0;
        inFile.read((char*)&type, sizeof(type));
        inFile.read((char*)&size, sizeof(size));

        switch (type)
        {
            case 0:  // Vertex
            {
                m_Vertices.resize(size);
                inFile.read((char*)(m_Vertices.data()), m_Vertices.size() * sizeof(m_Vertices[0]));
            }
            break;

            case 1:  // Normal
            {
                m_Normals.resize(size);
                inFile.read((char*)(m_Normals.data()), m_Normals.size() * sizeof(m_Normals[0]));
            }
            break;

            case 2:  // Tangent
            {
                m_Tangents.resize(size);
                inFile.read((char*)(m_Tangents.data()), m_Tangents.size() * sizeof(m_Tangents[0]));
            }
            break;

            case 3:  // Bitangent
            {
                m_Bitangents.resize(size);
                inFile.read((char*)(m_Bitangents.data()), m_Bitangents.size() * sizeof(m_Bitangents[0]));
            }
            break;

            case 4:  // Color
            {
                m_Colors.resize(size);
                inFile.read((char*)(m_Colors.data()), m_Colors.size() * sizeof(m_Colors[0]));
            }
            break;

            case 5:  // TexCoord
            {
                m_TexCoords.resize(size);
                inFile.read((char*)(m_TexCoords.data()), m_TexCoords.size() * sizeof(m_TexCoords[0]));
            }
            break;

            case 6:  // JointId
            {
                m_JointIds.resize(size);
                inFile.read((char*)(m_JointIds.data()), m_JointIds.size() * sizeof(m_JointIds[0]));
            }
            break;

            case 7:  // JointWeights
            {
                m_JointWeights.resize(size);
                inFile.read((char*)(m_JointWeights.data()), m_JointWeights.size() * sizeof(m_JointWeights[0]));
            }
            break;

            case 8:  // Normal
            {
                m_Indices.resize(size);
                inFile.read((char*)(m_Indices.data()), m_Indices.size() * sizeof(m_Indices[0]));
            }
            break;

            case 10:  // Mesh Entry
            {
                m_MeshEntries.resize(size);

                std::size_t nameLen = 0;

                for (std::size_t i = 0; i < m_MeshEntries.size(); ++i)
                {
                    inFile.read((char*)&nameLen, sizeof(nameLen));
                    m_MeshEntries[i].m_MeshEntryName.resize(nameLen);
                    inFile.read(m_MeshEntries[i].m_MeshEntryName.data(), nameLen);
                    inFile.read((char*)&(m_MeshEntries[i].m_BaseVertex), sizeof(m_MeshEntries[i].m_BaseVertex));
                    inFile.read((char*)&(m_MeshEntries[i].m_BaseIndex), sizeof(m_MeshEntries[i].m_BaseIndex));
                    inFile.read((char*)&(m_MeshEntries[i].m_Size), sizeof(m_MeshEntries[i].m_Size));
                    // GLuint dummy;
                    // inFile.read((char*)&(dummy),          sizeof(m_MeshEntries[i].m_Size));
                    inFile.read((char*)&(m_MeshEntries[i].m_NodeTransform), sizeof(Matrix4));
                }
            }
            break;

            case 11:  // Bone Data
            {
                m_Bones.resize(size);

                std::size_t nameLen = 0;

                for (std::size_t i = 0; i < m_Bones.size(); ++i)
                {
                    inFile.read((char*)&nameLen, sizeof(nameLen));
                    m_Bones[i].m_BoneName.resize(nameLen);
                    inFile.read(m_Bones[i].m_BoneName.data(), nameLen);
                    inFile.read((char*)(&m_Bones[i].m_BoneOffset), sizeof(Matrix4));
                }
            }
            break;

            case 12:  // Bone ID Map Data
            {
                std::size_t keySize = 0;
                unsigned keyData = 0;
                std::string keyName;
                for (std::size_t i = 0; i < size; ++i)
                {
                    inFile.read((char*)&keySize, sizeof(keySize));
                    keyName.resize(keySize);
                    inFile.read(keyName.data(), keySize);
                    inFile.read((char*)&keyData, sizeof(keyData));
                    m_BonesIDMap.try_emplace(keyName, keyData);
                }
            }
            break;

            case 13:  // Node Data
            {
                m_Nodes.resize(size);

                std::size_t nameLen = 0;
                std::size_t childCount = 0;

                for (std::size_t i = 0; i < m_Nodes.size(); ++i)
                {
                    inFile.read((char*)&nameLen, sizeof(nameLen));
                    m_Nodes[i].m_NodeName.resize(nameLen);
                    inFile.read(m_Nodes[i].m_NodeName.data(), nameLen);
                    inFile.read((char*)&childCount, sizeof(childCount));
                    m_Nodes[i].m_ChildrenNodes.resize(childCount);
                    inFile.read((char*)(m_Nodes[i].m_ChildrenNodes.data()), m_Nodes[i].m_ChildrenNodes.size() * sizeof(m_Nodes[i].m_ChildrenNodes[0]));
                    // Matrix4 tmp;
                    // inFile.read((char*)&(tmp), sizeof(Matrix4));
                    inFile.read((char*)(&m_Nodes[i].m_Scaling), sizeof(Vector3));
                    inFile.read((char*)(&m_Nodes[i].m_Rotation), sizeof(Quaternion));
                    inFile.read((char*)(&m_Nodes[i].m_Translation), sizeof(Vector3));
                }
            }
            break;

            case 14:  // Node ID Map Data
            {
                std::size_t keySize = 0;
                unsigned keyData = 0;
                std::string keyName;
                for (std::size_t i = 0; i < size; ++i)
                {
                    inFile.read((char*)&keySize, sizeof(keySize));
                    keyName.resize(keySize);
                    inFile.read(keyName.data(), keySize);
                    inFile.read((char*)&keyData, sizeof(keyData));
                    m_NodesIDMap.try_emplace(keyName, keyData);
                }
            }
            break;

            case 15:  // Global Inverse Transform
            {
                inFile.read((char*)(&m_GlobalInverseTransform), sizeof(Matrix4));
            }
            break;

            default: break;
        }
    }

    inFile.close();

#ifndef USE_VULKAN
    glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Index], m_Indices.size() * sizeof(m_Indices[0]), m_Indices.data(), 0);
    glVertexArrayElementBuffer(m_VertexArray, m_VertexBuffers[(int)MeshBufferID::Index]);

    glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Position], m_Vertices.size() * sizeof(m_Vertices[0]), m_Vertices.data(), 0);
    glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::Position, (int)MeshBufferID::Position);
    glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::Position, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::Position);
    glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::Position, m_VertexBuffers[(int)MeshBufferID::Position], 0, sizeof(m_Vertices[0]));

    if (!m_Normals.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Normal], m_Normals.size() * sizeof(m_Normals[0]), m_Normals.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::Normal, (int)MeshBufferID::Normal);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::Normal, 3, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::Normal);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::Normal, m_VertexBuffers[(int)MeshBufferID::Normal], 0, sizeof(m_Normals[0]));
    }

    if (!m_Tangents.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Tangent], m_Tangents.size() * sizeof(m_Tangents[0]), m_Tangents.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::Tangent, (int)MeshBufferID::Tangent);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::Tangent, 3, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::Tangent);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::Tangent, m_VertexBuffers[(int)MeshBufferID::Tangent], 0, sizeof(m_Tangents[0]));
    }

    if (!m_Bitangents.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Bitangent], m_Bitangents.size() * sizeof(m_Bitangents[0]), m_Bitangents.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::Bitangent, (int)MeshBufferID::Bitangent);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::Bitangent, 3, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::Bitangent);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::Bitangent, m_VertexBuffers[(int)MeshBufferID::Bitangent], 0, sizeof(m_Bitangents[0]));
    }

    if (!m_Colors.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Color], m_Colors.size() * sizeof(m_Colors[0]), m_Colors.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::Color, (int)MeshBufferID::Color);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::Color, 3, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::Color);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::Color, m_VertexBuffers[(int)MeshBufferID::Color], 0, sizeof(m_Colors[0]));
    }

    if (!m_TexCoords.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::TexCoord], m_TexCoords.size() * sizeof(m_TexCoords[0]), m_TexCoords.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::TexCoord, (int)MeshBufferID::TexCoord);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::TexCoord, 2, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::TexCoord);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::TexCoord, m_VertexBuffers[(int)MeshBufferID::TexCoord], 0, sizeof(m_TexCoords[0]));
    }

    if (!m_JointIds.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::JointId], m_JointIds.size() * sizeof(m_JointIds[0]), m_JointIds.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::JointId, (int)MeshBufferID::JointId);
        glVertexArrayAttribIFormat(m_VertexArray, (int)MeshBufferID::JointId, 4, GL_INT, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::JointId);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::JointId, m_VertexBuffers[(int)MeshBufferID::JointId], 0, sizeof(m_JointIds[0]));
    }

    if (!m_JointWeights.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::JointWeight], m_JointWeights.size() * sizeof(m_JointWeights[0]), m_JointWeights.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::JointWeight, (int)MeshBufferID::JointWeight);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::JointWeight, 4, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::JointWeight);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::JointWeight, m_VertexBuffers[(int)MeshBufferID::JointWeight], 0, sizeof(m_JointWeights[0]));
    }
#endif  // !USE_VULKAN

#ifndef EDITOR

    m_Indices.clear();
    m_Vertices.clear();
    m_Normals.clear();
    m_Tangents.clear();
    m_Bitangents.clear();
    m_Colors.clear();
    m_TexCoords.clear();
    m_JointIds.clear();
    m_JointWeights.clear();

#endif

    // Generate AABB
    Vector3 min = Vector3(MAX_FLOAT);
    Vector3 max = Vector3(-MAX_FLOAT);

    for (const Vector3& position : m_Vertices)
    {
        if (position.x < min.x) min.x = position.x;
        if (position.x > max.x) max.x = position.x;

        if (position.y < min.y) min.y = position.y;
        if (position.y > max.y) max.y = position.y;

        if (position.z < min.z) min.z = position.z;
        if (position.z > max.z) max.z = position.z;
    }

    m_AABB = AABB(min, max);
    m_LoadStatus = ResourceStatus::Loaded;
}

void Mesh::ImportFromAssimp(aiScene const* scene)
{
    unsigned totalNumIndices = 0;
    unsigned totalNumVertices = 0;

    // Count total number of vertices
    for (unsigned i = 0; i < scene->mNumMeshes; ++i)
    {
        totalNumIndices += scene->mMeshes[i]->mNumFaces * 3;
        totalNumVertices += scene->mMeshes[i]->mNumVertices;
    }

    m_Indices.reserve(totalNumIndices);
    m_Vertices.resize(totalNumVertices);
    m_Normals.resize(totalNumVertices);
    m_Tangents.resize(totalNumVertices);
    m_Bitangents.resize(totalNumVertices);
    m_Colors.resize(totalNumVertices);
    m_TexCoords.resize(totalNumVertices);
    m_JointIds.resize(totalNumVertices);
    m_JointWeights.resize(totalNumVertices);

    unsigned currVertexOffset = 0;
    unsigned currIndexOffset = 0;

    // Load materials
    for (unsigned i = 0; i < scene->mNumMaterials; ++i)
    {
        aiString path;
        scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
        // TODO: Import material files and register as resource type
    }

    // Load per mesh details
    for (unsigned i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* currMesh = scene->mMeshes[i];
        unsigned currNumIndices = 0;

        // Load face indicies
        for (unsigned j = 0; j < currMesh->mNumFaces; ++j)
        {
            m_Indices.emplace_back(currMesh->mFaces[j].mIndices[0]);
            m_Indices.emplace_back(currMesh->mFaces[j].mIndices[1]);
            m_Indices.emplace_back(currMesh->mFaces[j].mIndices[2]);
            currNumIndices += 3;
        }

        // Load vertex position and normals
        std::memcpy(m_Vertices.data() + currVertexOffset, currMesh->mVertices, currMesh->mNumVertices * sizeof(aiVector3D));
        std::memcpy(m_Normals.data() + currVertexOffset, currMesh->mNormals, currMesh->mNumVertices * sizeof(aiVector3D));
        if (currMesh->HasTangentsAndBitangents())
        {
            std::memcpy(m_Tangents.data() + currVertexOffset, currMesh->mTangents, currMesh->mNumVertices * sizeof(aiVector3D));
            std::memcpy(m_Bitangents.data() + currVertexOffset, currMesh->mBitangents, currMesh->mNumVertices * sizeof(aiVector3D));
        }

        // Load vertex colors if available
        if (currMesh->HasVertexColors(0))
        {
            for (unsigned j = 0; j < currMesh->mNumVertices; ++j)
            {
                m_Colors[currVertexOffset + j] = Vector3(currMesh->mColors[0][j].r, currMesh->mColors[0][j].g, currMesh->mColors[0][j].b);
            }
        }

        // Load texture coords if available
        if (currMesh->HasTextureCoords(0))
        {
            for (unsigned j = 0; j < currMesh->mNumVertices; ++j)
            {
                m_TexCoords[currVertexOffset + j] = Vector2(currMesh->mTextureCoords[0][j].x, currMesh->mTextureCoords[0][j].y);
            }
        }

        // Load bones if available
        if (currMesh->HasBones())
        {
            // For each bone...
            for (unsigned k = 0; k < currMesh->mNumBones; ++k)
            {
                unsigned boneId = static_cast<unsigned>(m_Bones.size());
                std::string boneName = currMesh->mBones[k]->mName.C_Str();
                std::replace_if(
                    boneName.begin(), boneName.end(), [](const auto& c) { return c == ' '; }, '_');

                // Register bone id if doesn't exist, get bone id if exist
                auto result = m_BonesIDMap.try_emplace(boneName, boneId);
                if (!result.second)
                {
                    boneId = result.first->second;
                }
                else
                {
                    // Create new bone
                    m_Bones.push_back(MeshBone());
                    MeshBone& newBoneInfo = m_Bones.back();
                    // Set bone name and bone offset matrix
                    newBoneInfo.m_BoneName = boneName;
                    newBoneInfo.m_BoneOffset = ConvertAssimpMat4(currMesh->mBones[k]->mOffsetMatrix);
                }

                // For each weight...
                for (unsigned m = 0; m < currMesh->mBones[k]->mNumWeights; ++m)
                {
                    unsigned vertId = currMesh->mBones[k]->mWeights[m].mVertexId + currVertexOffset;
                    float vertWeight = currMesh->mBones[k]->mWeights[m].mWeight;
                    bool vertAssigned = false;

                    // Assign bone ID and weights
                    for (unsigned n = 0; n < 4; ++n)
                    {
                        // Unassigned
                        if (m_JointWeights[vertId][n] == 0)
                        {
                            m_JointIds[vertId][n] = boneId;
                            m_JointWeights[vertId][n] = vertWeight;
                            vertAssigned = true;
                            break;
                        }
                    }

                    // No free space; replace smallest weight bone
                    if (!vertAssigned)
                    {
                        int smallestIndex = -1;
                        float smallestValue = 1.0f;

                        // Find smallest component index
                        for (unsigned n = 0; n < 4; ++n)
                        {
                            if (m_JointWeights[vertId][n] < smallestValue)
                            {
                                smallestIndex = n;
                                smallestValue = m_JointWeights[vertId][n];
                            }
                        }

                        // Valid component found, replace weight and id values
                        if (smallestIndex > -1)
                        {
                            m_JointIds[vertId][smallestIndex] = boneId;
                            m_JointWeights[vertId][smallestIndex] = vertWeight;
                        }
                    }
                }
            }
        }

        m_MeshEntries.push_back(MeshEntry { currMesh->mName.data, currVertexOffset, currIndexOffset, currNumIndices, Matrix4() });
        currVertexOffset += currMesh->mNumVertices;
        currIndexOffset += currNumIndices;
    }

    m_GlobalInverseTransform = ConvertAssimpMat4(scene->mRootNode->mTransformation);
    m_GlobalInverseTransform.Invert();

    // Read nodes and their hierachy
    aiNode* rootNode = scene->mRootNode;

    ParseNodes(rootNode);
    ParseNodesChildren(rootNode);

#ifndef USE_VULKAN
    glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Index], m_Indices.size() * sizeof(m_Indices[0]), m_Indices.data(), 0);
    glVertexArrayElementBuffer(m_VertexArray, m_VertexBuffers[(int)MeshBufferID::Index]);

    glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Position], m_Vertices.size() * sizeof(m_Vertices[0]), m_Vertices.data(), 0);
    glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::Position, (int)MeshBufferID::Position);
    glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::Position, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::Position);
    glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::Position, m_VertexBuffers[(int)MeshBufferID::Position], 0, sizeof(m_Vertices[0]));

    if (!m_Normals.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Normal], m_Normals.size() * sizeof(m_Normals[0]), m_Normals.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::Normal, (int)MeshBufferID::Normal);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::Normal, 3, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::Normal);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::Normal, m_VertexBuffers[(int)MeshBufferID::Normal], 0, sizeof(m_Normals[0]));
    }

    if (!m_Tangents.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Tangent], m_Tangents.size() * sizeof(m_Tangents[0]), m_Tangents.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::Tangent, (int)MeshBufferID::Tangent);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::Tangent, 3, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::Tangent);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::Tangent, m_VertexBuffers[(int)MeshBufferID::Tangent], 0, sizeof(m_Tangents[0]));
    }

    if (!m_Bitangents.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Bitangent], m_Bitangents.size() * sizeof(m_Bitangents[0]), m_Bitangents.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::Bitangent, (int)MeshBufferID::Bitangent);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::Bitangent, 3, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::Bitangent);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::Bitangent, m_VertexBuffers[(int)MeshBufferID::Bitangent], 0, sizeof(m_Bitangents[0]));
    }

    if (!m_Colors.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::Color], m_Colors.size() * sizeof(m_Colors[0]), m_Colors.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::Color, (int)MeshBufferID::Color);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::Color, 3, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::Color);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::Color, m_VertexBuffers[(int)MeshBufferID::Color], 0, sizeof(m_Colors[0]));
    }

    if (!m_TexCoords.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::TexCoord], m_TexCoords.size() * sizeof(m_TexCoords[0]), m_TexCoords.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::TexCoord, (int)MeshBufferID::TexCoord);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::TexCoord, 2, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::TexCoord);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::TexCoord, m_VertexBuffers[(int)MeshBufferID::TexCoord], 0, sizeof(m_TexCoords[0]));
    }

    if (!m_JointIds.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::JointId], m_JointIds.size() * sizeof(m_JointIds[0]), m_JointIds.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::JointId, (int)MeshBufferID::JointId);
        glVertexArrayAttribIFormat(m_VertexArray, (int)MeshBufferID::JointId, 4, GL_INT, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::JointId);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::JointId, m_VertexBuffers[(int)MeshBufferID::JointId], 0, sizeof(m_JointIds[0]));
    }

    if (!m_JointWeights.empty())
    {
        glNamedBufferStorage(m_VertexBuffers[(int)MeshBufferID::JointWeight], m_JointWeights.size() * sizeof(m_JointWeights[0]), m_JointWeights.data(), 0);
        glVertexArrayAttribBinding(m_VertexArray, (int)MeshBufferID::JointWeight, (int)MeshBufferID::JointWeight);
        glVertexArrayAttribFormat(m_VertexArray, (int)MeshBufferID::JointWeight, 4, GL_FLOAT, GL_FALSE, 0);
        glEnableVertexArrayAttrib(m_VertexArray, (int)MeshBufferID::JointWeight);
        glVertexArrayVertexBuffer(m_VertexArray, (int)MeshBufferID::JointWeight, m_VertexBuffers[(int)MeshBufferID::JointWeight], 0, sizeof(m_JointWeights[0]));
    }
#endif  // !USE_VULKAN

    m_LoadStatus = ResourceStatus::Loaded;

    m_FilePath.replace_extension(".mesh");

    // Write data to output
    std::ofstream outFile = std::ofstream(m_FilePath, std::ios::binary);

    {
        int id = (int)MeshBufferID::Position;
        std::size_t size = m_Vertices.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        outFile.write((char*)(m_Vertices.data()), m_Vertices.size() * sizeof(m_Vertices[0]));
    }

    {
        int id = (int)MeshBufferID::Index;
        std::size_t size = m_Indices.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        outFile.write((char*)(m_Indices.data()), m_Indices.size() * sizeof(m_Indices[0]));
    }

    if (!m_Normals.empty())
    {
        int id = (int)MeshBufferID::Normal;
        std::size_t size = m_Normals.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        outFile.write((char*)(m_Normals.data()), m_Normals.size() * sizeof(m_Normals[0]));
    }

    if (!m_Tangents.empty())
    {
        int id = (int)MeshBufferID::Tangent;
        std::size_t size = m_Tangents.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        outFile.write((char*)(m_Tangents.data()), m_Tangents.size() * sizeof(m_Tangents[0]));
    }

    if (!m_Bitangents.empty())
    {
        int id = (int)MeshBufferID::Bitangent;
        std::size_t size = m_Bitangents.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        outFile.write((char*)(m_Bitangents.data()), m_Bitangents.size() * sizeof(m_Bitangents[0]));
    }

    if (!m_Colors.empty())
    {
        int id = (int)MeshBufferID::Color;
        std::size_t size = m_Colors.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        outFile.write((char*)(m_Colors.data()), m_Colors.size() * sizeof(m_Colors[0]));
    }

    if (!m_TexCoords.empty())
    {
        int id = (int)MeshBufferID::TexCoord;
        std::size_t size = m_TexCoords.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        outFile.write((char*)(m_TexCoords.data()), m_TexCoords.size() * sizeof(m_TexCoords[0]));
    }

    if (!m_JointIds.empty())
    {
        int id = (int)MeshBufferID::JointId;
        std::size_t size = m_JointIds.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        outFile.write((char*)(m_JointIds.data()), m_JointIds.size() * sizeof(m_JointIds[0]));
    }

    if (!m_JointWeights.empty())
    {
        int id = (int)MeshBufferID::JointWeight;
        std::size_t size = m_JointWeights.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        outFile.write((char*)(m_JointWeights.data()), m_JointWeights.size() * sizeof(m_JointWeights[0]));
    }

    // Mesh entries

    {
        int id = 10;
        std::size_t size = m_MeshEntries.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        for (std::size_t i = 0; i < m_MeshEntries.size(); ++i)
        {
            std::size_t nameLen = m_MeshEntries[i].m_MeshEntryName.size();
            outFile.write((char*)&nameLen, sizeof(nameLen));
            outFile.write(m_MeshEntries[i].m_MeshEntryName.data(), m_MeshEntries[i].m_MeshEntryName.size());
            outFile.write((char*)&(m_MeshEntries[i].m_BaseVertex), sizeof(m_MeshEntries[i].m_BaseVertex));
            outFile.write((char*)&(m_MeshEntries[i].m_BaseIndex), sizeof(m_MeshEntries[i].m_BaseIndex));
            outFile.write((char*)&(m_MeshEntries[i].m_Size), sizeof(m_MeshEntries[i].m_Size));
            // outFile.write((char*)&(m_MeshEntries[i].m_Size),          sizeof(m_MeshEntries[i].m_Size));
            outFile.write((char*)&(m_MeshEntries[i].m_NodeTransform), sizeof(Matrix4));
        }
    }

    {
        // Bone data
        int id = 11;
        std::size_t size = m_Bones.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));

        for (std::size_t i = 0; i < m_Bones.size(); ++i)
        {
            std::size_t nameLen = m_Bones[i].m_BoneName.size();
            outFile.write((char*)&nameLen, sizeof(nameLen));
            outFile.write(m_Bones[i].m_BoneName.data(), m_Bones[i].m_BoneName.size());
            outFile.write((char*)&(m_Bones[i].m_BoneOffset), sizeof(m_Bones[i].m_BoneOffset));
        }
    }

    {
        // Bone map data
        int id = 12;
        std::size_t size = m_BonesIDMap.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));

        for (auto& pair : m_BonesIDMap)
        {
            std::size_t nameLen = pair.first.size();
            outFile.write((char*)&nameLen, sizeof(nameLen));
            outFile.write(pair.first.data(), pair.first.size());
            outFile.write((char*)&(pair.second), sizeof(pair.second));
        }
    }

    {
        // Node data
        int id = 13;
        std::size_t size = m_Nodes.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));

        for (std::size_t i = 0; i < m_Nodes.size(); ++i)
        {
            std::size_t nameLen = m_Nodes[i].m_NodeName.size();
            outFile.write((char*)&nameLen, sizeof(nameLen));
            outFile.write(m_Nodes[i].m_NodeName.data(), m_Nodes[i].m_NodeName.size());

            std::size_t childCount = m_Nodes[i].m_ChildrenNodes.size();
            outFile.write((char*)(&childCount), sizeof(childCount));
            outFile.write((char*)(m_Nodes[i].m_ChildrenNodes.data()), m_Nodes[i].m_ChildrenNodes.size() * sizeof(m_Nodes[i].m_ChildrenNodes[0]));
            outFile.write((char*)(&m_Nodes[i].m_Scaling), sizeof(Vector3));
            outFile.write((char*)(&m_Nodes[i].m_Rotation), sizeof(Quaternion));
            outFile.write((char*)(&m_Nodes[i].m_Translation), sizeof(Vector3));
        }
    }

    {
        // Node map data
        int id = 14;
        std::size_t size = m_NodesIDMap.size();
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));

        for (auto& pair : m_NodesIDMap)
        {
            std::size_t nameLen = pair.first.size();
            outFile.write((char*)&nameLen, sizeof(nameLen));
            outFile.write(pair.first.data(), pair.first.size());
            outFile.write((char*)(&pair.second), sizeof(pair.second));
        }
    }

    // Global inverse matrix
    {
        int id = 15;
        std::size_t size = 1;
        outFile.write((char*)&id, sizeof(id));
        outFile.write((char*)&size, sizeof(size));
        outFile.write((char*)(&m_GlobalInverseTransform), sizeof(Matrix4));
    }

    outFile.close();

    // Generate AABB
    Vector3 min = Vector3(std::numeric_limits<float>::max());
    Vector3 max = Vector3(std::numeric_limits<float>::min());

    for (const Vector3& position : m_Vertices)
    {
        if (position.x < min.x) min.x = position.x;
        if (position.x > max.x) max.x = position.x;

        if (position.y < min.y) min.y = position.y;
        if (position.y > max.y) max.y = position.y;

        if (position.z < min.z) min.z = position.z;
        if (position.z > max.z) max.z = position.z;
    }

    m_AABB = AABB(min, max);
    m_LoadStatus = ResourceStatus::Loaded;
}

std::vector<GLuint> const& Mesh::GetIndices() const
{
    return m_Indices;
}

std::vector<Vector3> const& Mesh::GetVertices() const
{
    return m_Vertices;
}

std::vector<Vector3> const& Mesh::GetNormals() const
{
    return m_Normals;
}

std::vector<Vector3> const& Mesh::GetColors() const
{
    return m_Colors;
}

std::vector<Vector2> const& Mesh::GetTexCoords() const
{
    return m_TexCoords;
}

std::vector<HMaterial> const& Mesh::GetMaterials() const
{
    return m_Materials;
}

std::vector<MeshEntry> const& Mesh::GetMeshEntries() const
{
    return m_MeshEntries;
}

std::vector<MeshBone> const& Mesh::GetBones() const
{
    return m_Bones;
}

std::map<std::string, unsigned> const& Mesh::GetBonesMap() const
{
    return m_BonesIDMap;
}

std::vector<MeshNode> const& Mesh::GetNodes() const
{
    return m_Nodes;
}

std::map<std::string, unsigned> const& Mesh::GetNodesMap() const
{
    return m_NodesIDMap;
}

Matrix4 Mesh::GetGlobalInverseTransform() const
{
    return m_GlobalInverseTransform;
}

AABB Mesh::GetAABB() const
{
    return m_AABB;
}

#ifdef USE_VULKAN
constexpr vk::VertexInputBindingDescription Mesh::GetVertexBindingDescription()
{
    return vk::VertexInputBindingDescription();
}

constexpr std::vector<vk::VertexInputAttributeDescription> Mesh::GetVertexAttributeDescriptions()
{
    return std::vector<vk::VertexInputAttributeDescription>();
}

#else

GLuint Mesh::GetVAO() const
{
    return m_VertexArray;
}
#endif  // USE_VULKAN

void Mesh::ParseNodes(aiNode* currNode)
{
    if (currNode)
    {
        // if (currNode->mNumMeshes)
        //{
        //  Matrix4 transform = ConvertAssimpMat4(currNode->mTransformation);
        //
        //  for (unsigned i = 0; i < currNode->mNumMeshes; ++i)
        //  {
        //    m_MeshEntries[currNode->mMeshes[i]].m_NodeTransform *= transform;
        //  }
        //}

        // if (currNode->mNumMeshes)
        //{
        //  Matrix4 transform = ConvertAssimpMat4(currNode->mTransformation);
        //
        //  for (unsigned i = 0; i < currNode->mNumMeshes; ++i)
        //  {
        //    const MeshEntry& entry = m_MeshEntries[currNode->mMeshes[i]];
        //    const unsigned meshEntryIndex = currNode->mMeshes[i];
        //
        //    unsigned numVertices = entry.m_BaseVertex + ((meshEntryIndex + 1) < m_MeshEntries.size() ?
        //      m_MeshEntries[meshEntryIndex + 1].m_BaseVertex - m_MeshEntries[meshEntryIndex].m_BaseVertex :
        //      m_Vertices.size() - m_MeshEntries[meshEntryIndex].m_BaseVertex);
        //
        //    for (unsigned j = entry.m_BaseVertex; j < numVertices; ++j)
        //    {
        //      m_Vertices[j] = (transform * Vector4(m_Vertices[j], 1.0f)).V3();
        //    }
        //  }
        //}

        // Register node with node id map, assert if node already exist
        std::string nodeName = currNode->mName.C_Str();
        std::replace_if(
            nodeName.begin(), nodeName.end(), [](const auto& c) { return c == ' '; }, '_');
        ASSERT(m_NodesIDMap.try_emplace(nodeName, static_cast<unsigned>(m_NodesIDMap.size())).second);

        m_Nodes.push_back(MeshNode());
        MeshNode& newNode = m_Nodes.back();
        newNode.m_NodeName = nodeName;

        aiVector3D scale;
        aiQuaternion rotate;
        aiVector3D translate;
        currNode->mTransformation.Decompose(scale, rotate, translate);

        newNode.m_Scaling = Vector3(scale.x, scale.y, scale.z);
        newNode.m_Rotation = Quaternion(rotate.x, rotate.y, rotate.z, rotate.w);
        newNode.m_Translation = Vector3(translate.x, translate.y, translate.z);

        for (unsigned i = 0; i < currNode->mNumChildren; ++i)
        {
            ParseNodes(currNode->mChildren[i]);
        }
    }
}

void Mesh::ParseNodesChildren(aiNode* currNode)
{
    if (currNode)
    {
        std::string nodeName = currNode->mName.C_Str();
        std::replace_if(
            nodeName.begin(), nodeName.end(), [](const auto& c) { return c == ' '; }, '_');

        MeshNode& node = m_Nodes[m_NodesIDMap[nodeName]];

        for (unsigned i = 0; i < currNode->mNumChildren; ++i)
        {
            // Add children id
            std::string childNodeName = currNode->mChildren[i]->mName.C_Str();
            std::replace_if(
                childNodeName.begin(), childNodeName.end(), [](const auto& c) { return c == ' '; }, '_');

            // if (m_NodesIDMap.find(childNodeName) != m_NodesIDMap.end())
            //{
            node.m_ChildrenNodes.push_back(m_NodesIDMap[childNodeName]);
            ParseNodesChildren(currNode->mChildren[i]);
            //}
        }
    }
}
