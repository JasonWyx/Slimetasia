#include "MeshAnimator.h"

#include <thread>

#include "AnimationSystem.h"
#include "GameObject.h"
#include "ResourceManager.h"

MeshAnimator::MeshAnimator(GameObject* ownerObject)
    : IComponent(ownerObject, "MeshAnimator")
    , m_IsPlaying(true)
    , m_IsPlayOnce(false)
    , m_MeshRenderer(ownerObject->GetComponent<MeshRenderer>())
    , m_AnimationSet()
    , m_AnimationTimeScale(1.0f)
    , m_CurrentAnimationTick(0.0f)
    , m_CurrentAnimation()
    , m_PreviousPose()
    , m_IsCrossFading(false)
    , m_CrossFadeAnimations(true)
    , m_CrossFadeDuration(0.5f)
    , m_CrossFadeTimer(0.0f)
    , m_GlobalInverseTransform()
    , m_BoneTransforms()
{
    // Check mesh renderer component present
    ASSERT(m_MeshRenderer != nullptr);
}

MeshAnimator::~MeshAnimator() {}

void MeshAnimator::InitMeshAnimator()
{
    m_BoneTransforms.clear();
    m_PreviousPose.clear();

    static std::mutex mtx;

    std::lock_guard<std::mutex> guard(mtx);

    HMesh mesh = m_MeshRenderer->GetMesh();
    if (mesh.Validate())
    {
        m_GlobalInverseTransform = mesh->GetGlobalInverseTransform();
        m_BoneTransforms.resize(mesh->GetBones().size());
        m_PreviousPose.resize(mesh->GetNodes().size());
        m_TransitionPose = m_PreviousPose;

        if (m_OwnerObject->GetParentLayer())
        {
            const std::vector<MeshNode>& nodes = mesh->GetNodes();
            const std::vector<MeshBone>& bones = mesh->GetBones();

            m_NodeObjects.resize(bones.size());
            m_NodeObjectTransforms.resize(bones.size());

            for (size_t i = 0; i < bones.size(); ++i)
            {
                unsigned childId = m_OwnerObject->GetChildObjectByName(bones[i].m_BoneName);

                m_NodeObjects[i] = childId == 0 ? m_OwnerObject->GetParentLayer()->CreateObject(bones[i].m_BoneName) : m_OwnerObject->GetParentLayer()->GetObjectById(childId);
                m_NodeObjects[i]->SetParentObject(m_OwnerObject->GetID());
                m_NodeObjects[i]->SetIsChildren(true);

                m_NodeObjectTransforms[i] = m_NodeObjects[i]->AddIfDoesntExist<Transform>();

                m_OwnerObject->AttachChild(m_NodeObjects[i]->GetID());
            }
        }
    }
    else
    {
        std::cout << "ERROR: Failed to load MeshAnimator for object " << m_OwnerObject->GetID() << std::endl;
    }
}

void MeshAnimator::Update(float dt)
{
    if (m_AnimationSet != m_PrevAnimationSet)
    {
        InitMeshAnimator();
        m_PrevAnimationSet = m_AnimationSet;
    }
    if (m_CurrentAnimation != nullptr && m_IsPlaying && m_BoneTransforms.size())
    {
        const std::vector<MeshNode>& nodes = m_MeshRenderer->GetMesh()->GetNodes();
        const std::vector<MeshBone>& bones = m_MeshRenderer->GetMesh()->GetBones();
        const std::map<std::string, unsigned>& bonesMap = m_MeshRenderer->GetMesh()->GetBonesMap();
        const std::map<std::string, unsigned>& nodesMap = m_MeshRenderer->GetMesh()->GetNodesMap();

        UpdateBoneTransforms(nodes, bones, nodesMap, bonesMap, nodes[0], Matrix4(1.0f));

        m_CurrentAnimationTick += m_CurrentAnimation->m_TicksPerSecond * m_AnimationTimeScale * dt;

        if (m_IsPlayOnce && m_CurrentAnimationTick > m_CurrentAnimation->m_Duration)
        {
            m_IsPlaying = false;
        }

        m_CurrentAnimationTick = fmodf(m_CurrentAnimationTick, m_CurrentAnimation->m_Duration);

        if (m_CrossFadeAnimations && m_IsCrossFading)
        {
            m_CrossFadeTimer += dt;
            m_IsCrossFading = m_CrossFadeTimer >= m_CrossFadeDuration ? false : true;
        }
    }
}

void MeshAnimator::OnActive()
{
    AnimationSystem::Instance().InsertAnimator(this);
}

void MeshAnimator::OnInactive()
{
    AnimationSystem::Instance().RemoveAnimator(this);
}

void MeshAnimator::RevalidateResources()
{
    if (m_AnimationSet.Validate() && !m_CurrentAnimation)
    {
        m_CurrentAnimation = m_AnimationSet->GetAnimation(m_AnimationSet->GetAnimationNames().front().c_str());
    }
}

void MeshAnimator::Play(char const* animName)
{
    m_CurrentAnimation = m_AnimationSet->GetAnimation(animName);

    if (m_CurrentAnimation == nullptr)
    {
        m_IsPlaying = false;
        std::cout << "WARNING: Animation " << animName << " could not be found in " << m_AnimationSet->m_Name << "." << std::endl;
        return;
    }

    if (m_CrossFadeAnimations)
    {
        m_IsCrossFading = true;
        m_CrossFadeTimer = 0.0f;
        // SaveCurrentPose();
        m_TransitionPose = m_PreviousPose;
    }

    m_IsPlaying = true;
    m_IsPlayOnce = false;
    m_CurrentAnimationTick = 0.0f;
}

void MeshAnimator::PlayOnce(char const* animName)
{
    m_CurrentAnimation = m_AnimationSet->GetAnimation(animName);

    if (m_CurrentAnimation == nullptr)
    {
        m_IsPlaying = false;
        std::cout << "WARNING: Animation " << animName << " could not be found in " << m_AnimationSet->m_Name << "." << std::endl;
        return;
    }

    if (m_CrossFadeAnimations)
    {
        m_IsCrossFading = true;
        m_CrossFadeTimer = 0.0f;
        // SaveCurrentPose();
        m_TransitionPose = m_PreviousPose;
    }

    m_IsPlaying = true;
    m_IsPlayOnce = true;
    m_CurrentAnimationTick = 0.0f;
}

void MeshAnimator::Stop()
{
    m_CurrentAnimationTick = 0.0f;
    m_IsPlaying = false;
}

void MeshAnimator::Pause()
{
    m_IsPlaying = false;
}

void MeshAnimator::Resume()
{
    m_IsPlaying = true;
}

bool MeshAnimator::IsPlaying() const
{
    return m_IsPlaying;
}

HAnimationSet MeshAnimator::GetAnimationSet() const
{
    return m_AnimationSet;
}

void MeshAnimator::SetAnimationSet(HAnimationSet animationSet)
{
    m_AnimationSet = animationSet;
    if (animationSet.Validate())
    {
        m_CurrentAnimation = m_AnimationSet->GetAnimation(m_AnimationSet->GetAnimationNames().front().c_str());
    }
}

std::vector<Matrix4> const& MeshAnimator::GetBoneTransforms() const
{
    return m_BoneTransforms;
}

void MeshAnimator::UpdateBoneTransforms(std::vector<MeshNode> const& nodes, std::vector<MeshBone> const& bones, std::map<std::string, unsigned> const& nodesMap,
                                        std::map<std::string, unsigned> const& bonesMap, MeshNode const& currNode, Matrix4 const& parentTransform)
{
    // Find corresponding bone and node
    auto currBoneIt = bonesMap.find(currNode.m_NodeName);
    auto currNodeIt = nodesMap.find(currNode.m_NodeName);

    // Find corresponding node animation
    auto currNodeAnimIt = m_CurrentAnimation->m_NodeAnimationsMap.find(currNode.m_NodeName);

    // Matrix4 localTransform = currNode.m_LocalTransform;

    Matrix4 localTransform = Matrix4::Translate(currNode.m_Translation) * currNode.m_Rotation.EulerTransform() * Matrix4::Scale(currNode.m_Scaling);

    // Update node transform if animation exists for it
    if (currNodeAnimIt != m_CurrentAnimation->m_NodeAnimationsMap.end())
    {
        const NodeAnimation& currNodeAnim = m_CurrentAnimation->m_NodeAnimations[currNodeAnimIt->second];
        Pose& prevPose = m_PreviousPose[currNodeAnimIt->second];
        Pose nextPose;
        if (m_IsCrossFading)
        {
            float crossFadeFactor = m_CrossFadeTimer / m_CrossFadeDuration;
            ASSERT(crossFadeFactor >= 0.0f && crossFadeFactor <= 1.0f);

            const Pose& transitionPose = m_TransitionPose[currNodeAnimIt->second];
            nextPose = { Math::Lerp(transitionPose.scaling, currNodeAnim.GetInterpolatedScale(m_CurrentAnimationTick), crossFadeFactor),
                         Quaternion::Slerp(transitionPose.rotation, currNodeAnim.GetInterpolatedRotation(m_CurrentAnimationTick), crossFadeFactor),
                         Math::Lerp(transitionPose.translation, currNodeAnim.GetInterpolatedPosition(m_CurrentAnimationTick), crossFadeFactor) };
        }
        else
        {
            nextPose = { currNodeAnim.GetInterpolatedScale(m_CurrentAnimationTick), currNodeAnim.GetInterpolatedRotation(m_CurrentAnimationTick),
                         currNodeAnim.GetInterpolatedPosition(m_CurrentAnimationTick) };
        }

        localTransform = Matrix4::Translate(nextPose.translation) * nextPose.rotation.EulerTransform() * Matrix4::Scale(nextPose.scaling);

        prevPose = nextPose;
    }

    Matrix4 globalTransform = parentTransform * localTransform;

    // Update bone transform if exists
    if (currBoneIt != bonesMap.end())
    {
        m_BoneTransforms[currBoneIt->second] = m_GlobalInverseTransform * globalTransform * bones[currBoneIt->second].m_BoneOffset;

        Matrix4 finalTransform = m_OwnerObject->GetComponent<Transform>()->GetWorldTransformMatrix() * m_GlobalInverseTransform * globalTransform;

        Vector3 scale;
        Vector3 rotate;
        Vector3 translate;

        finalTransform.Decompose(translate, rotate, scale);

        Transform* currNodeTransform = m_NodeObjectTransforms[currBoneIt->second];

        currNodeTransform->SetWorldScale(scale);
        currNodeTransform->SetWorldRotation(rotate);
        currNodeTransform->SetWorldPosition(translate);

        // Apply update to child object of bone also
        GameObject* nodeObject = m_NodeObjects[currBoneIt->second];

        for (unsigned childId : nodeObject->GetChildrenObjects())
        {
            GameObject* childObject = m_OwnerObject->GetParentLayer()->GetObjectById(childId);
            Transform* childTransform = childObject->GetComponent<Transform>();

            if (childTransform)
            {
                childTransform->SetWorldScale(scale);
                childTransform->SetWorldRotation(rotate);
                childTransform->SetWorldPosition(translate);
            }
        }
    }

    // Update children nodes
    for (unsigned childNodeId : currNode.m_ChildrenNodes)
    {
        UpdateBoneTransforms(nodes, bones, nodesMap, bonesMap, nodes[childNodeId], globalTransform);
    }
}

void MeshAnimator::SaveCurrentPose()
{
    if (m_CurrentAnimation != nullptr)
    {
        std::vector<Pose> newPrevPose(m_CurrentAnimation->m_NodeAnimations.size());

        const std::vector<NodeAnimation>& nodeAnims = m_CurrentAnimation->m_NodeAnimations;
        size_t size = m_CurrentAnimation->m_NodeAnimations.size();

        for (size_t i = 0; i < size; ++i)
        {
            newPrevPose[i].translation = nodeAnims[i].GetInterpolatedPosition(m_CurrentAnimationTick);
            newPrevPose[i].rotation = nodeAnims[i].GetInterpolatedRotation(m_CurrentAnimationTick);
            newPrevPose[i].scaling = nodeAnims[i].GetInterpolatedScale(m_CurrentAnimationTick);
        }

        m_PreviousPose = std::move(newPrevPose);
    }
}

REFLECT_INIT(MeshAnimator)

REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(m_IsPlaying)
REFLECT_PROPERTY(m_AnimationSet)
REFLECT_PROPERTY(m_AnimationTimeScale)
REFLECT_PROPERTY(m_CurrentAnimationTick)
REFLECT_PROPERTY(m_CrossFadeAnimations)
REFLECT_PROPERTY(m_CrossFadeDuration)
REFLECT_END()
