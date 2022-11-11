#include "ResourceImporter.h"

#include "CorePrerequisites.h"
// #include <filesystem>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>

void ResourceImporter::ImportModel(const std::filesystem::path& filePath)
{
    Assimp::Importer importer;
    importer.SetPropertyBool("AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS", false);
    aiScene const* scene = importer.ReadFile(filePath.string().c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

    // Import new mesh
    if (scene->HasMeshes())
    {
        HMesh newMesh = ResourceManager::Instance().CreateResource<Mesh>(filePath.filename().replace_extension().string(), filePath);
        newMesh->ImportFromAssimp(scene);
    }

    // Import animation data
    if (scene->HasAnimations())
    {
        HAnimationSet newAnimationSet = ResourceManager::Instance().CreateResource<AnimationSet>(filePath.filename().replace_extension().string(), filePath);
        newAnimationSet->ImportFromAssimp(scene);
    }
}

void ResourceImporter::ImportTexture(const std::vector<std::filesystem::path>& filePaths)
{
    HTexture newTexture = ResourceManager::Instance().CreateResource<Texture>(filePaths[0].filename().replace_extension().string(), filePaths[0]);

    if (filePaths.size() == 1)
    {
        if (filePaths[0].extension() == ".dds")
        {
            newTexture->Load();
        }
        else
        {
            newTexture->ImportTexture(filePaths[0]);
        }
    }
    else
    {
        std::vector<char const*> faceTags = { "_ft", "_bk", "_up", "_dn", "_rt", "_lf" };
        std::vector<int> faceIndex(6);

        for (std::size_t i = 0; i < filePaths.size(); ++i)
        {
            if (filePaths[i].filename().string().rfind(faceTags[0]) != std::string::npos)
            {
                faceIndex[0] = (int)i;
                continue;
            }
            if (filePaths[i].filename().string().rfind(faceTags[1]) != std::string::npos)
            {
                faceIndex[1] = (int)i;
                continue;
            }
            if (filePaths[i].filename().string().rfind(faceTags[2]) != std::string::npos)
            {
                faceIndex[2] = (int)i;
                continue;
            }
            if (filePaths[i].filename().string().rfind(faceTags[3]) != std::string::npos)
            {
                faceIndex[3] = (int)i;
                continue;
            }
            if (filePaths[i].filename().string().rfind(faceTags[4]) != std::string::npos)
            {
                faceIndex[4] = (int)i;
                continue;
            }
            if (filePaths[i].filename().string().rfind(faceTags[5]) != std::string::npos)
            {
                faceIndex[5] = (int)i;
                continue;
            }
        }

        std::vector<std::filesystem::path> sortedFaces { filePaths[faceIndex[0]], filePaths[faceIndex[1]], filePaths[faceIndex[2]],
                                                         filePaths[faceIndex[3]], filePaths[faceIndex[4]], filePaths[faceIndex[5]] };

        newTexture->ImportSkybox(sortedFaces);
    }
}
