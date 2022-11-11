#pragma once
#include <map>
#include <vector>

#include "MathDefs.h"
#include "ResourceBase.h"

struct aiScene;

struct PositionKey
{
    float m_Tick;
    Vector3 m_Position;
};

struct ScalingKey
{
    float m_Tick;
    Vector3 m_Scale;
};

struct RotationKey
{
    float m_Tick;
    Quaternion m_Rotation;
};

struct NodeAnimation
{
    std::string m_NodeName;
    std::vector<PositionKey> m_PositionKeys;
    std::vector<ScalingKey> m_ScalingKeys;
    std::vector<RotationKey> m_RotationKeys;

    Vector3 GetInterpolatedPosition(float tick) const;
    Quaternion GetInterpolatedRotation(float tick) const;
    Vector3 GetInterpolatedScale(float tick) const;
};

struct Animation
{
    std::string m_AnimationName;
    std::vector<NodeAnimation> m_NodeAnimations;
    std::map<std::string, unsigned> m_NodeAnimationsMap;
    float m_TicksPerSecond;
    float m_Duration;
};

class AnimationSet : public ResourceBase
{
public:
    AnimationSet(const std::string& resourceName = "AnimationSet", const std::filesystem::path& filepath = "");
    ~AnimationSet();

    void ImportFromAssimp(aiScene const* scene);
    virtual void Load() override;
    virtual void Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem) override;
    virtual void Unserialize(tinyxml2::XMLElement* currElem) override;

    Animation const* GetAnimation(char const* name) const;
    std::vector<std::string> const& GetAnimationNames() const;

private:
    std::vector<Animation> m_Animations;
    std::map<std::string, unsigned> m_AnimationsMap;
    std::vector<std::string> m_AnimationsNames;
};