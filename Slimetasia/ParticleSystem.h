#pragma once
#include <algorithm>

#include "GameObject.h"
#include "IComponent.h"
#include "ISystem.h"
#include "ResourceHandle.h"
#include "ResourceManager.h"

class Attractor;

class ParticleEmitter;

struct ParticleData
{
    std::vector<Vector4> m_Position;
    std::vector<Vector4> m_Color;
    std::vector<Vector4> m_StartColor;
    std::vector<Vector4> m_EndColor;
    std::vector<Vector4> m_Velocity;
    std::vector<Vector4> m_Acceleration;
    std::vector<Vector4> m_Time;
    std::vector<HTexture> m_Texture;
    std::vector<HTexture> m_EndTexture;
    std::vector<float> m_TextureFade;
    std::vector<bool> m_IsAlive;
    std::vector<float> m_Size;
    std::vector<unsigned> m_LayerID;
    std::vector<float> m_Gravity;
    std::vector<bool> m_IsAffectedByAttractor;
    std::vector<bool> m_IsAffectedByFloor;
    std::vector<float> m_FloorHeight;
    std::vector<bool> m_IsAffectedByAllAttractor;
    std::vector<std::vector<unsigned>> m_AttractorList;

    unsigned m_Count;
    unsigned m_CountAlive;
    ParticleData()
        : m_Count(0)
        , m_CountAlive(0)
    {
    }
    explicit ParticleData(unsigned maxCount)
        : m_Count(0)
        , m_CountAlive(0)
    {
        generate(maxCount);
    }
    ~ParticleData() {};
    ParticleData(const ParticleData&) = delete;
    ParticleData& operator=(const ParticleData&) = delete;
    void generate(unsigned maxSize);
    void kill(unsigned id);
    void wake(unsigned id, HTexture t, HTexture e);
    void swapData(unsigned a, unsigned b);
};

struct ParticleUpdater
{
    ParticleUpdater() {}
    virtual ~ParticleUpdater() {}

    virtual void update(float dt, ParticleData* p) = 0;
};

class TimeUpdater : public ParticleUpdater
{
public:

    virtual void update(float dt, ParticleData* p) override;
};

class EulerUpdater : public ParticleUpdater
{
public:

    Vector3 m_GlobalAcceleration {};

public:

    virtual void update(float dt, ParticleData* p) override;
};

class FloorUpdater : public ParticleUpdater
{
public:

    float m_FloorY { 0.0f };
    float m_BounceFactor { 0.5f };

public:

    virtual void update(float dt, ParticleData* p) override;
};

class AttractorUpdater : public ParticleUpdater
{
protected:

    std::vector<Attractor*> m_Attractors;  // .w is force
public:

    virtual void update(float dt, ParticleData* p) override;

    size_t collectionSize() const { return m_Attractors.size(); }
    void add(Attractor* attr) { m_Attractors.push_back(attr); }
    void remove(Attractor* attr) { m_Attractors.erase(std::remove(m_Attractors.begin(), m_Attractors.end(), attr), m_Attractors.end()); }
    std::vector<Attractor*> get() { return m_Attractors; }
};

class ColorUpdater : public ParticleUpdater
{
    Vector4 mix(const Vector4& x, const Vector4& y, const float& a);

public:

    virtual void update(float dt, ParticleData* p) override;
};

class ParticleSystem : public ISystem<ParticleSystem>
{
    ParticleData m_Particles;
    unsigned m_Count;
    std::vector<ParticleEmitter*> m_Emitters;
    std::vector<ParticleUpdater*> m_Updaters;
    AttractorUpdater* m_AttractorUpdator;

public:

    explicit ParticleSystem(unsigned maxCount);
    ~ParticleSystem();

    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    void Update(float dt);
    void Reset();
    size_t GetParticleCount() const { return m_Particles.m_Count; }
    size_t GetAliveParticleCount() const { return m_Particles.m_CountAlive; }

    void addEmitter(ParticleEmitter* em) { m_Emitters.push_back(em); }
    void removeEmitter(ParticleEmitter* em)
    {
        auto tmp = std::find(m_Emitters.begin(), m_Emitters.end(), em);
        if (tmp != m_Emitters.end()) m_Emitters.erase(tmp);
    }
    void addUpdater(ParticleUpdater* up) { m_Updaters.push_back(up); }

    ParticleData* finalData() { return &m_Particles; }
    AttractorUpdater* GetAttractorUpdater() { return m_AttractorUpdator; }
};

class ParticleEmitter : public IComponent
{
public:

    Vector4 m_StartMinColor;
    Vector4 m_StartMaxColor;
    Vector4 m_EndMinColor;
    Vector4 m_EndMaxColor;
    float m_MinSize;
    float m_MaxSize;
    // Vector4  m_color;
    HTexture m_Texture;
    HTexture m_EndTexture;
    float m_TextureFade { 1.0f };
    float m_ElapsedTime;
    float m_Gravity;
    bool m_IsAffectedByAttractor;
    bool m_IsAffectedByFloor;
    bool m_IsAffectedByAllAttractor;
    float m_FloorHeight;
    std::vector<unsigned> m_Attractors;
    float m_EmissionRate { 0.0 };

public:

    void SetTexture(HTexture t) { m_Texture = t; }

    HTexture GetTexture() const { return m_Texture; }

    ParticleEmitter(GameObject* parentObject, std::string n = "ParticleEmitter")
        : IComponent(parentObject, n)
    {
        HTexture t = ResourceManager::Instance().GetResource<Texture>("Default_Particle");
        m_Texture = t;
        m_EndTexture = t;
        m_StartMinColor = m_StartMaxColor = m_EndMinColor = m_EndMaxColor = Vector4(0, 0, 0, 1.f);
        m_IsAffectedByAttractor = false;
        m_IsAffectedByFloor = false;
        m_IsAffectedByAllAttractor = true;
        m_MinSize = m_MaxSize = 1.f;
        if (m_OwnerObject && m_OwnerObject->GetParentLayer() && m_OwnerObject->GetParentLayer()->GetParentScene()) ParticleSystem::Instance().addEmitter(this);
    }
    virtual ~ParticleEmitter()
    {
        if (m_OwnerObject && m_OwnerObject->GetParentLayer() && m_OwnerObject->GetParentLayer()->GetParentScene()) ParticleSystem::Instance().removeEmitter(this);
    }

    // calls all the generators and at the end it activates (wakes) particle
    virtual void emit(float dt, ParticleData* p) {};
    virtual void RevalidateResources() override
    {
        // m_texture.Validate();
    }

    REFLECT()
};

class BoxParticleEmitter : public ParticleEmitter
{
public:

    Vector3 m_Position;
    Vector3 m_StartPositionOffset;
    Vector3 m_MinVelocity;
    Vector3 m_MaxVelocity;
    float m_MinTime;
    float m_MaxTime;

private:

    Vector4 linearRand(const Vector4& Min, const Vector4& Max) const;
    float linearRand(const float& Min, const float& Max) const;

public:

    BoxParticleEmitter(GameObject* parentObject)
        : ParticleEmitter(parentObject, "BoxParticleEmitter")
        , m_Position()
        , m_StartPositionOffset()
        , m_MinVelocity()
        , m_MaxVelocity()
        , m_MinTime()
        , m_MaxTime()
    {
    }
    void emit(float dt, ParticleData* p) override;
    REFLECT()
};

class CircleParticleEmitter : public ParticleEmitter
{
    Vector3 m_Center;
    float m_RadiusX;
    float m_RadiusY;
    float m_RadiusZ;
    Vector3 m_StartPositionOffset;
    Vector3 m_MinVelocity;
    Vector3 m_MaxVelocity;
    float m_MinTime;
    float m_MaxTime;

    Vector4 linearRand(const Vector4& Min, const Vector4& Max) const;
    float linearRand(const float& Min, const float& Max) const;

public:

    CircleParticleEmitter(GameObject* parentObject)
        : ParticleEmitter(parentObject, "CircleParticleEmitter")
        , m_Center()
        , m_RadiusX()
        , m_RadiusY()
        , m_RadiusZ()
        , m_StartPositionOffset()
        , m_MinVelocity()
        , m_MaxVelocity()
        , m_MinTime()
        , m_MaxTime()
    {
    }
    void emit(float dt, ParticleData* p) override;
    REFLECT()
};

class Attractor : public IComponent
{
    Transform* m_Transform;

public:

    float m_Force;

    Attractor(GameObject* parentObject, std::string n = "Attractor")
        : IComponent(parentObject, n)
        , m_Transform(nullptr)
        , m_Force(0)
    {
        m_OwnerObject->AddIfDoesntExist<Transform>();
        m_Transform = m_OwnerObject->GetComponent<Transform>();
        if (m_OwnerObject && m_OwnerObject->GetParentLayer() && m_OwnerObject->GetParentLayer()->GetParentScene())
        {
            ParticleSystem::Instance().GetAttractorUpdater()->add(this);
        }
    }

    void Update(float dt) {}

    Transform* GetTransform() { return m_Transform; }

    float GetForce() { return m_Force; }

    virtual ~Attractor()
    {
        if (m_OwnerObject && m_OwnerObject->GetParentLayer() && m_OwnerObject->GetParentLayer()->GetParentScene())
        {
            ParticleSystem::Instance().GetAttractorUpdater()->remove(this);
        }
    }
    REFLECT()
};
