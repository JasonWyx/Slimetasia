#include "AnimationSystem.h"

#include "Application.h"

AnimationSystem::AnimationSystem()
    : m_ActivateAnimators()
{
}

AnimationSystem::~AnimationSystem() {}

void AnimationSystem::UpdateAnimatorsAsync(const float dt, const std::list<MeshAnimator*>& animators)
{
    for (MeshAnimator* animator : animators)
    {
        animator->Update(dt);
    }
}

void AnimationSystem::Update(float dt)
{
    ThreadPool& threadPool = Application::Instance().GetThreadPool();
    unsigned threadCount = std::thread::hardware_concurrency() - 1;

    std::vector<std::list<MeshAnimator*>> animatorGroups(threadCount);

    unsigned index = 0;
    for (MeshAnimator* animator : m_ActivateAnimators)
    {
        animatorGroups[index].push_back(animator);
        index = (index + 1) % threadCount;
    }

    std::vector<std::future<void>> results;

    for (unsigned i = 0; i < threadCount; ++i)
    {
        results.push_back(threadPool.enqueue(AnimationSystem::UpdateAnimatorsAsync, dt, animatorGroups[i]));
    }

    for (unsigned i = 0; i < threadCount; ++i)
    {
        results[i].wait();
    }

    // for (MeshAnimator* animator : m_ActivateAnimators)
    //{
    //  animator->Update(dt);
    //}
}

void AnimationSystem::InsertAnimator(MeshAnimator* animator)
{
    m_ActivateAnimators.push_back(animator);
}

void AnimationSystem::RemoveAnimator(MeshAnimator* animator)
{
    m_ActivateAnimators.remove(animator);
}
