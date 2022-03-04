#include "Animation.h"

#include <assimp/anim.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <fstream>

#include "Logger.h"

// Helper lambdas to convert from assimp type to my engine types
Vector3 ConvertAssimpVec3(aiVector3D const& value)
{
    return Vector3(value.x, value.y, value.z);
}

Quaternion ConvertAssimpQuat(aiQuaternion const& value)
{
    return Quaternion(value.x, value.y, value.z, value.w);
}

AnimationSet::AnimationSet(const std::string& resourceName, const filesystem::path& filepath)
    : ResourceBase(resourceName, filepath)
    , m_Animations()
    , m_AnimationsMap()
    , m_AnimationsNames()
{
}

AnimationSet::~AnimationSet() {}

void AnimationSet::ImportFromAssimp(aiScene const* scene)
{
    p_assert(scene != nullptr);

    // For each anim data...
    for (unsigned i = 0; i < scene->mNumAnimations; ++i)
    {
        aiAnimation* currAnim = scene->mAnimations[i];

        // Register anim name with anim id map, asserting if duplicate anim found
        p_assert(m_AnimationsMap.try_emplace(currAnim->mName.C_Str(), static_cast<unsigned>(m_Animations.size())).second);

        m_AnimationsNames.push_back(currAnim->mName.C_Str());

        // Create new anim
        m_Animations.push_back(Animation());
        Animation& newAnim = m_Animations.back();
        newAnim.m_AnimationName = currAnim->mName.C_Str();
        newAnim.m_TicksPerSecond = static_cast<float>(currAnim->mTicksPerSecond);
        newAnim.m_Duration = static_cast<float>(currAnim->mDuration);

        // Get joint animations
        for (unsigned j = 0; j < currAnim->mNumChannels; ++j)
        {
            // Current bone/channel
            aiNodeAnim* currChannel = currAnim->mChannels[j];
            std::string animNodeName = currChannel->mNodeName.C_Str();
            std::replace_if(
                animNodeName.begin(), animNodeName.end(), [](const auto& c) { return c == ' '; }, '_');

            p_assert(newAnim.m_NodeAnimationsMap.try_emplace(animNodeName, static_cast<unsigned>(newAnim.m_NodeAnimations.size())).second);

            // Create new joint/node anim
            newAnim.m_NodeAnimations.push_back(NodeAnimation());
            NodeAnimation& newNodeAnim = newAnim.m_NodeAnimations.back();
            newNodeAnim.m_NodeName = animNodeName;

            // Position key frames
            for (unsigned k = 0; k < currChannel->mNumPositionKeys; ++k)
            {
                newNodeAnim.m_PositionKeys.push_back(PositionKey{static_cast<float>(currChannel->mPositionKeys[k].mTime), ConvertAssimpVec3(currChannel->mPositionKeys[k].mValue)});
            }

            // Scaling key frames
            for (unsigned k = 0; k < currChannel->mNumScalingKeys; ++k)
            {
                newNodeAnim.m_ScalingKeys.push_back(ScalingKey{static_cast<float>(currChannel->mScalingKeys[k].mTime), ConvertAssimpVec3(currChannel->mScalingKeys[k].mValue)});
            }

            // Rotation key frames
            for (unsigned k = 0; k < currChannel->mNumRotationKeys; ++k)
            {
                newNodeAnim.m_RotationKeys.push_back(RotationKey{static_cast<float>(currChannel->mRotationKeys[k].mTime), ConvertAssimpQuat(currChannel->mRotationKeys[k].mValue)});
            }
        }
    }

    m_FilePath.replace_extension(".anim");

    std::ofstream file = std::ofstream(m_FilePath.string(), std::ios::binary);

    std::size_t count = 0;

    count = m_Animations.size();
    file.write((char*)&count, sizeof(count));

    // For each animation...
    for (Animation const& anim : m_Animations)
    {
        count = anim.m_AnimationName.size();
        file.write((char*)&count, sizeof(count));
        file.write(anim.m_AnimationName.c_str(), count);

        count = anim.m_NodeAnimations.size();
        file.write((char*)&count, sizeof(count));

        // For each node animation...
        for (NodeAnimation const& nodeAnim : anim.m_NodeAnimations)
        {
            count = nodeAnim.m_NodeName.size();
            file.write((char*)&count, sizeof(count));
            file.write(nodeAnim.m_NodeName.c_str(), count);

            count = nodeAnim.m_PositionKeys.size();
            file.write((char*)&count, sizeof(count));
            file.write((char*)nodeAnim.m_PositionKeys.data(), count * sizeof(nodeAnim.m_PositionKeys[0]));

            count = nodeAnim.m_ScalingKeys.size();
            file.write((char*)&count, sizeof(count));
            file.write((char*)nodeAnim.m_ScalingKeys.data(), count * sizeof(nodeAnim.m_ScalingKeys[0]));

            count = nodeAnim.m_RotationKeys.size();
            file.write((char*)&count, sizeof(count));
            file.write((char*)nodeAnim.m_RotationKeys.data(), count * sizeof(nodeAnim.m_RotationKeys[0]));
        }

        // Save node animation map
        count = anim.m_NodeAnimationsMap.size();
        file.write((char*)&count, sizeof(count));
        for (auto const& pair : anim.m_NodeAnimationsMap)
        {
            count = pair.first.size();
            file.write((char*)&count, sizeof(count));
            file.write(pair.first.c_str(), count);
            file.write((char*)&pair.second, sizeof(pair.second));
        }

        file.write((char*)&anim.m_TicksPerSecond, sizeof(anim.m_TicksPerSecond));
        file.write((char*)&anim.m_Duration, sizeof(anim.m_Duration));
    }

    // Save animation name map
    count = m_AnimationsMap.size();
    file.write((char*)&count, sizeof(count));

    for (std::pair<std::string, unsigned> const& pair : m_AnimationsMap)
    {
        count = pair.first.size();
        file.write((char*)&count, sizeof(count));
        file.write(pair.first.c_str(), count);
        file.write((char*)&pair.second, sizeof(pair.second));
    }

    // Save animation names
    count = m_AnimationsNames.size();
    file.write((char*)&count, sizeof(count));

    for (std::string const& name : m_AnimationsNames)
    {
        count = name.size();
        file.write((char*)&count, sizeof(count));
        file.write(name.c_str(), count);
    }

    file.close();

    m_LoadStatus = ResourceStatus::Loaded;
}

void AnimationSet::Load()
{
    if (m_FilePath.extension() != ".anim")
    {
        Assimp::Importer importer;
        aiScene const* scene = importer.ReadFile(m_FilePath.string(), aiProcessPreset_TargetRealtime_Quality);
        ImportFromAssimp(scene);
    }
    else
    {
        std::ifstream file = std::ifstream(m_FilePath.string(), std::ios::binary);

        std::size_t count = 0;

        file.read((char*)&count, sizeof(count));
        m_Animations.resize(count);

        // For each animation...
        for (Animation& anim : m_Animations)
        {
            file.read((char*)&count, sizeof(count));
            anim.m_AnimationName.resize(count);
            file.read(anim.m_AnimationName.data(), count);

            file.read((char*)&count, sizeof(count));
            anim.m_NodeAnimations.resize(count);

            // For each node animation...
            for (NodeAnimation& nodeAnim : anim.m_NodeAnimations)
            {
                file.read((char*)&count, sizeof(count));
                nodeAnim.m_NodeName.resize(count);
                file.read(nodeAnim.m_NodeName.data(), count);

                file.read((char*)&count, sizeof(count));
                nodeAnim.m_PositionKeys.resize(count);
                file.read((char*)nodeAnim.m_PositionKeys.data(), count * sizeof(nodeAnim.m_PositionKeys[0]));

                file.read((char*)&count, sizeof(count));
                nodeAnim.m_ScalingKeys.resize(count);
                file.read((char*)nodeAnim.m_ScalingKeys.data(), count * sizeof(nodeAnim.m_ScalingKeys[0]));

                file.read((char*)&count, sizeof(count));
                nodeAnim.m_RotationKeys.resize(count);
                file.read((char*)nodeAnim.m_RotationKeys.data(), count * sizeof(nodeAnim.m_RotationKeys[0]));
            }

            // Read node animations map
            file.read((char*)&count, sizeof(count));

            for (std::size_t i = 0; i < count; ++i)
            {
                std::size_t countName = 0;
                file.read((char*)&countName, sizeof(countName));
                std::string name = std::string(countName, 0);
                file.read(name.data(), countName);
                unsigned id = 0;
                file.read((char*)&id, sizeof(id));
                anim.m_NodeAnimationsMap.try_emplace(name, id);
            }

            file.read((char*)&anim.m_TicksPerSecond, sizeof(anim.m_TicksPerSecond));
            file.read((char*)&anim.m_Duration, sizeof(anim.m_Duration));
        }

        // Read animation name map
        file.read((char*)&count, sizeof(count));
        for (std::size_t i = 0; i < count; ++i)
        {
            std::size_t countName = 0;
            file.read((char*)&countName, sizeof(countName));
            std::string name = std::string(countName, 0);
            file.read(name.data(), countName);
            unsigned id = 0;
            file.read((char*)&id, sizeof(id));
            m_AnimationsMap.try_emplace(name, id);
        }

        file.read((char*)&count, sizeof(count));
        m_AnimationsNames.resize(count);
        for (std::string& name : m_AnimationsNames)
        {
            file.read((char*)&count, sizeof(count));
            name.resize(count);
            file.read(name.data(), count);
        }

        file.close();
    }
    m_LoadStatus = ResourceStatus::Loaded;
}

void AnimationSet::Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem)
{
    tinyxml2::XMLElement* resourceElem = doc->NewElement("AnimationSet");
    tinyxml2::XMLElement* nameElem = doc->NewElement("Name");
    tinyxml2::XMLElement* filepathElem = doc->NewElement("Filepath");

    resourceElem->SetAttribute("GUID", static_cast<int64_t>(m_GUID));
    nameElem->SetText(m_Name.c_str());
    filepathElem->SetText(m_FilePath.string().c_str());

    parentElem->InsertEndChild(resourceElem);
    resourceElem->InsertEndChild(nameElem);
    resourceElem->InsertEndChild(filepathElem);
}

void AnimationSet::Unserialize(tinyxml2::XMLElement* currElem)
{
    m_GUID = (ResourceGUID)currElem->Int64Attribute("GUID");

    m_Name = currElem->FirstChildElement("Name")->GetText();
    m_FilePath = currElem->FirstChildElement("Filepath")->GetText();
}

Animation const* AnimationSet::GetAnimation(char const* name) const
{
    auto it = m_AnimationsMap.find(name);
    if (it == m_AnimationsMap.end())
    {
        std::cout << "All available animations : " << std::endl;
        for (const auto& c : m_AnimationsMap)
            std::cout << c.first << std::endl;

        std::cout << "Animation " << name << " could not be found." << std::endl;
        return nullptr;
    }

    return &(m_Animations[(*it).second]);
}

std::vector<std::string> const& AnimationSet::GetAnimationNames() const
{
    return m_AnimationsNames;
}

Vector3 NodeAnimation::GetInterpolatedPosition(float tick) const
{
    std::size_t size = m_PositionKeys.size();
    for (std::size_t i = 0; (i + 1) < size; ++i)
    {
        if (m_PositionKeys[i].m_Tick <= tick && tick < m_PositionKeys[i + 1].m_Tick)
        {
            // Calculate interpolant
            float t = (tick - m_PositionKeys[i].m_Tick) / (m_PositionKeys[(i + 1) % size].m_Tick - m_PositionKeys[i].m_Tick);

            return Math::Lerp(m_PositionKeys[i].m_Position, m_PositionKeys[(i + 1) % size].m_Position, t);
        }
    }
    // std::cout << "WARNING: Scale for node animation " << m_NodeName
    //          << " not found at tick " << tick
    //          << "." << std::endl;

    return m_PositionKeys[0].m_Position;
}

Quaternion NodeAnimation::GetInterpolatedRotation(float tick) const
{
    std::size_t size = m_RotationKeys.size();

    for (std::size_t i = 0; (i + 1) < m_RotationKeys.size(); ++i)
    {
        if (m_RotationKeys[i].m_Tick <= tick && tick < m_RotationKeys[i + 1].m_Tick)
        {
            // Calculate interpolant
            float t = (tick - m_RotationKeys[i].m_Tick) / (m_RotationKeys[(i + 1) % size].m_Tick - m_RotationKeys[i].m_Tick);

            return Quaternion::Slerp(m_RotationKeys[i].m_Rotation, m_RotationKeys[(i + 1) % size].m_Rotation, t);
        }
    }
    // std::cout << "WARNING: Rotation for node animation " << m_NodeName
    //          << " not found at tick " << tick
    //          << "." << std::endl;

    return m_RotationKeys[0].m_Rotation;
}

Vector3 NodeAnimation::GetInterpolatedScale(float tick) const
{
    std::size_t size = m_ScalingKeys.size();

    for (std::size_t i = 0; (i + 1) < m_ScalingKeys.size(); ++i)
    {
        if (m_ScalingKeys[i].m_Tick <= tick && tick < m_ScalingKeys[(i + 1) % size].m_Tick)
        {
            // Calculate interpolant
            float t = (tick - m_ScalingKeys[i].m_Tick) / (m_ScalingKeys[(i + 1) % size].m_Tick - m_ScalingKeys[i].m_Tick);

            return Math::Lerp(m_ScalingKeys[i].m_Scale, m_ScalingKeys[(i + 1) % size].m_Scale, t);
        }
    }
    // std::cout << "WARNING: Scale for node animation " << m_NodeName
    //          << " not found at tick " << tick
    //					<< "." << std::endl;

    return m_ScalingKeys[0].m_Scale;
}