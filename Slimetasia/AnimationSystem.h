#pragma once
#include "CorePrerequisites.h"
#include "ISystem.h"
#include "MeshAnimator.h"

class AnimationSystem : public ISystem<AnimationSystem>
{
public:

    void Update(float dt);
    void InsertAnimator(MeshAnimator* animator);
    void RemoveAnimator(MeshAnimator* animator);

private:

    friend class ISystem<AnimationSystem>;
    AnimationSystem();
    ~AnimationSystem();

    using AnimatorIterator = std::list<MeshAnimator*>::iterator;

    static void UpdateAnimatorsAsync(const float dt, const std::list<MeshAnimator*>& animators);

private:

    std::list<MeshAnimator*> m_ActivateAnimators;
};
