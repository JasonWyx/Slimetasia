#pragma once
#include "Animation.h"
#include "CorePrerequisites.h"
#include "IComponent.h"
#include "MeshRenderer.h"

class MeshAnimator : public IComponent
{
public:

    REFLECT()
    MeshAnimator(GameObject* ownerObject);
    ~MeshAnimator();

    void InitMeshAnimator();
    void Update(float dt);

    void OnActive() override;
    void OnInactive() override;
    void RevalidateResources() override;
    void Play(char const* animName);
    void PlayOnce(char const* animName);
    void Stop();
    void Pause();
    void Resume();
    bool IsPlaying() const;

    HAnimationSet GetAnimationSet() const;
    void SetAnimationSet(HAnimationSet animationSet);

    std::vector<Matrix4> const& GetBoneTransforms() const;
    // void                        SetMeshData(HMesh mesh);

private:

    bool m_IsPlaying;
    bool m_IsPlayOnce;
    MeshRenderer* m_MeshRenderer;
    HAnimationSet m_AnimationSet;
    HAnimationSet m_PrevAnimationSet;

public:

    float m_AnimationTimeScale;

private:

    float m_CurrentAnimationTick;
    Animation const* m_CurrentAnimation;

    struct Pose
    {
        Vector3 scaling;
        Quat rotation;
        Vector3 translation;
    };

    std::vector<Pose> m_PreviousPose;
    std::vector<Pose> m_TransitionPose;

    // float             m_PreviousAnimationTick;
    // Animation const*  m_PreviousAnimation;
    bool m_IsCrossFading;

public:

    bool m_CrossFadeAnimations;

private:

    float m_CrossFadeDuration;
    float m_CrossFadeTimer;

    Matrix4 m_GlobalInverseTransform;
    std::vector<Matrix4> m_BoneTransforms;
    std::vector<GameObject*> m_NodeObjects;
    std::vector<Transform*> m_NodeObjectTransforms;

private:  // Functions

    void UpdateBoneTransforms(std::vector<MeshNode> const& nodes, std::vector<MeshBone> const& bones, std::map<std::string, unsigned> const& nodesMap, std::map<std::string, unsigned> const& bonesMap,
                              MeshNode const& currNode, Matrix4 const& parentTransform);

    void SaveCurrentPose();
};
