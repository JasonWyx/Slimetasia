#include "ParticleSystem.h"

#include <algorithm>

void ParticleData::generate(unsigned maxSize)
{
    m_Count = maxSize;
    m_CountAlive = 0;

    m_Position.resize(maxSize);
    m_Color.resize(maxSize);
    m_StartColor.resize(maxSize);
    m_EndColor.resize(maxSize);
    m_Velocity.resize(maxSize);
    m_Acceleration.resize(maxSize);
    m_Time.resize(maxSize);
    m_IsAlive.resize(maxSize);
    m_Texture.resize(maxSize);
    m_EndTexture.resize(maxSize);
    m_TextureFade.resize(maxSize);
    m_Size.resize(maxSize);
    m_LayerID.resize(maxSize);
    m_Gravity.resize(maxSize);
    m_IsAffectedByAttractor.resize(maxSize);
    m_IsAffectedByFloor.resize(maxSize);
    m_FloorHeight.resize(maxSize);
    m_IsAffectedByAllAttractor.resize(maxSize);
    m_AttractorList.resize(maxSize);
}

void ParticleData::kill(unsigned id)
{
    if (m_CountAlive > 0)
    {
        m_IsAlive[id] = false;
        swapData(id, m_CountAlive - 1);
        --m_CountAlive;
    }
}

void ParticleData::wake(unsigned id, HTexture t, HTexture e)
{
    if (m_CountAlive < m_Count)
    {
        m_IsAlive[id] = true;
        m_Texture[id] = t;
        m_EndTexture[id] = e;
        swapData(id, m_CountAlive);
        ++m_CountAlive;
    }
}

void ParticleData::swapData(unsigned a, unsigned b)
{
    std::swap(m_Position[a], m_Position[b]);
    std::swap(m_Color[a], m_Color[b]);
    std::swap(m_StartColor[a], m_StartColor[b]);
    std::swap(m_EndColor[a], m_EndColor[b]);
    std::swap(m_Velocity[a], m_Velocity[b]);
    std::swap(m_Acceleration[a], m_Acceleration[b]);
    std::swap(m_Time[a], m_Time[b]);
    m_IsAlive.swap(m_IsAlive[a], m_IsAlive[b]);  // std::swap(m_alive[a], m_alive[b]);
    std::swap(m_Texture[a], m_Texture[b]);
    std::swap(m_EndTexture[a], m_EndTexture[b]);
    std::swap(m_TextureFade[a], m_TextureFade[b]);
    std::swap(m_Size[a], m_Size[b]);
    std::swap(m_LayerID[a], m_LayerID[b]);
    std::swap(m_Gravity[a], m_Gravity[b]);
    m_IsAffectedByAttractor.swap(m_IsAffectedByAttractor[a], m_IsAffectedByAttractor[b]);  // std::swap(m_AffectedByAttractor[a], m_AffectedByAttractor[b]);
    m_IsAffectedByFloor.swap(m_IsAffectedByFloor[a], m_IsAffectedByFloor[b]);              // std::swap(m_AffectedByFloor[a], m_AffectedByFloor[b]);
    std::swap(m_FloorHeight[a], m_FloorHeight[b]);
    m_IsAffectedByAllAttractor.swap(m_IsAffectedByAllAttractor[a], m_IsAffectedByAllAttractor[b]);  // std::swap(m_affectedbyallAttractor[a], m_affectedbyallAttractor[b]);
    std::swap(m_AttractorList[a], m_AttractorList[b]);
}

ParticleSystem::ParticleSystem(unsigned maxCount)
{
    m_Count = maxCount;
    m_Particles.generate(maxCount);
    for (unsigned i = 0; i < maxCount; ++i)
        m_Particles.m_IsAlive[i] = false;

    for (unsigned i = 0; i < maxCount; ++i)
        m_Particles.m_Color[i] = Vector4(1.f, 1.f, 1.f, 1.f);

    // Time Updater
    auto time_updater = new TimeUpdater {};
    addUpdater(time_updater);

    // Attractor Updater
    m_AttractorUpdator = new AttractorUpdater {};
    // m_attractorUpdater->add(Vector4{ 2,0,0,1000.f });
    addUpdater(m_AttractorUpdator);

    // Euler Updater
    auto euler_updater = new EulerUpdater {};
    euler_updater->m_GlobalAcceleration = Vector3 { 0.f, 0.f, 0.f };
    addUpdater(euler_updater);

    // color updater
    auto color_updater = new ColorUpdater {};
    addUpdater(color_updater);

    // Floor Updater
    auto floor_updater = new FloorUpdater {};
    addUpdater(floor_updater);
}

ParticleSystem::~ParticleSystem()
{
    for (auto& up : m_Updaters)
        delete up;
    m_Updaters.clear();
}

void ParticleSystem::Update(float dt)
{
    dt = std::min(dt, 1 / 30.0f);

    for (auto& em : m_Emitters)
    {
        em->emit(dt, &m_Particles);
    }

    for (size_t i = 0; i < m_Count; ++i)
    {
        m_Particles.m_Acceleration[i] = Vector4(0.0f);
    }

    for (auto& up : m_Updaters)
    {
        up->update(dt, &m_Particles);
    }
}

void ParticleSystem::Reset()
{
    m_Particles.m_CountAlive = 0;
    for (unsigned i = 0; i < m_Count; ++i)
        m_Particles.m_IsAlive[i] = false;
}

// void ParticleEmitter::emit(double dt, ParticleData *p)
// {
//   const unsigned maxNewParticles = static_cast<unsigned>(dt*m_emitRate);
//   const unsigned startId = p->m_countAlive;
//   const unsigned endId = std::min(startId + maxNewParticles, p->m_count - 1);
//
//   for (auto &gen : m_generators)
//     gen->generate(dt, p, startId, endId);
//
//   for (unsigned i = startId; i < endId; ++i)
//   {
//     p->wake(i, m_texture);
//   }
// }

Vector4 BoxParticleEmitter::linearRand(const Vector4& Min, const Vector4& Max) const
{
    Vector4 tmp;
    tmp.x = linearRand(Min.x, Max.x);
    tmp.y = linearRand(Min.y, Max.y);
    tmp.z = linearRand(Min.z, Max.z);
    tmp.w = linearRand(Min.w, Max.w);
    return tmp;
}

float BoxParticleEmitter::linearRand(const float& Min, const float& Max) const
{
    return float(std::rand()) / float(RAND_MAX) * (Max - Min) + Min;
}

void BoxParticleEmitter::emit(float dt, ParticleData* p)
{
    Transform* trans = m_OwnerObject->GetComponent<Transform>();

    m_ElapsedTime += dt;

    const unsigned maxNewParticles = static_cast<unsigned>(m_ElapsedTime * m_EmissionRate);
    const unsigned startId = p->m_CountAlive;
    const unsigned endId = std::min(startId + maxNewParticles, p->m_Count - 1);

    m_ElapsedTime = fmodf(m_ElapsedTime, (1.0f / m_EmissionRate));

    Vector4 posMin { m_Position.x - m_StartPositionOffset.x, m_Position.y - m_StartPositionOffset.y, m_Position.z - m_StartPositionOffset.z, 1.0 };
    Vector4 posMax { m_Position.x + m_StartPositionOffset.x, m_Position.y + m_StartPositionOffset.y, m_Position.z + m_StartPositionOffset.z, 1.0 };
    Vector4 velMin { m_MinVelocity, 0.f };
    Vector4 velMax { m_MaxVelocity, 0.f };

    if (!trans)
        for (unsigned i = startId; i < endId; ++i)
            p->m_Position[i] = linearRand(posMin, posMax);
    else
    {
        Vector4 pos = Vector4(trans->GetWorldPosition(), 0.f);
        for (unsigned i = startId; i < endId; ++i)
            p->m_Position[i] = pos + linearRand(posMin, posMax);
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_Velocity[i] = linearRand(velMin, velMax);
    }

    for (size_t i = startId; i < endId; ++i)
    {
        p->m_Time[i].x = linearRand(m_MinTime, m_MaxTime);
        p->m_Time[i].z = (float)0.0;
        p->m_Time[i].w = (float)1.0 / p->m_Time[i].x;
        p->m_Time[i].y = p->m_Time[i].x * m_TextureFade;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_StartColor[i] = linearRand(m_StartMinColor, m_StartMaxColor);
        p->m_EndColor[i] = linearRand(m_EndMinColor, m_EndMaxColor);
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_Size[i] = linearRand(m_MinSize, m_MaxSize);
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_LayerID[i] = GetOwner()->GetParentLayer()->GetId();
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_Gravity[i] = m_Gravity;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_IsAffectedByFloor[i] = m_IsAffectedByFloor;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_IsAffectedByAllAttractor[i] = m_IsAffectedByAllAttractor;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_FloorHeight[i] = m_FloorHeight;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_AttractorList[i] = m_Attractors;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_IsAffectedByAttractor[i] = m_IsAffectedByAttractor;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_TextureFade[i] = m_TextureFade <= 0.f ? 0.f : 1.f;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->wake(i, m_Texture, m_EndTexture);
    }
}

Vector4 ColorUpdater::mix(const Vector4& x, const Vector4& y, const float& a)
{
    return x * (1.0f - a) + y * a;
}

void ColorUpdater::update(float dt, ParticleData* p)
{
    const size_t endId = p->m_CountAlive;
    for (size_t i = 0; i < endId; ++i)
        p->m_Color[i] = mix(p->m_StartColor[i], p->m_EndColor[i], p->m_Time[i].z);
}

void TimeUpdater::update(float dt, ParticleData* p)
{
    unsigned int endId = p->m_CountAlive;
    const float localDT = dt;

    for (unsigned i = 0; i < endId; ++i)
    {
        p->m_Time[i].x -= localDT;
        p->m_Time[i].y -= localDT;
        if (p->m_Time[i].y <= 0.f) p->m_TextureFade[i] -= localDT;
        // interpolation: from 0 (start of life) till 1 (end of life)
        p->m_Time[i].z = (float)1.0 - (p->m_Time[i].x * p->m_Time[i].w);  // .w is 1.0/max life time

        if (p->m_Time[i].x < (float)0.0)
        {
            p->kill(i);
            endId = p->m_CountAlive < p->m_Count ? p->m_CountAlive : p->m_Count;
        }
    }
}

void EulerUpdater::update(float dt, ParticleData* p)
{
    const Vector4 globalA { dt * m_GlobalAcceleration.x, dt * m_GlobalAcceleration.y, dt * m_GlobalAcceleration.z, 0.0 };
    const float localDT = (float)dt;

    const unsigned int endId = p->m_CountAlive;
    for (size_t i = 0; i < endId; ++i)
    {
        p->m_Acceleration[i] += globalA;
        p->m_Acceleration[i].y += (p->m_Gravity[i] * dt);
    }

    for (size_t i = 0; i < endId; ++i)
        p->m_Velocity[i] += localDT * p->m_Acceleration[i];

    for (size_t i = 0; i < endId; ++i)
        p->m_Position[i] += localDT * p->m_Velocity[i];
}

void FloorUpdater::update(float dt, ParticleData* p)
{
    const float localDT = (float)dt;
    const Vector4 upVector = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
    const size_t endId = p->m_CountAlive;
    for (size_t i = 0; i < endId; ++i)
    {
        if (!p->m_IsAffectedByFloor[i]) continue;
        if (p->m_Position[i].y < p->m_FloorHeight[i])
        {
            Vector4 force = p->m_Acceleration[i];
            float normalFactor = force.Dot(upVector);
            if (normalFactor < 0.0f) force -= upVector * normalFactor;

            float velFactor = p->m_Velocity[i].Dot(upVector);

            p->m_Velocity[i] -= upVector * (1.0f + 0.5f) * velFactor;

            p->m_Acceleration[i] += force;
        }
    }
}

void AttractorUpdater::update(float dt, ParticleData* p)
{
    const float localDT = (float)dt;

    const size_t endId = p->m_CountAlive;
    const size_t countAttractors = m_Attractors.size();
    Vector4 off;
    float dist;
    size_t a;
    for (size_t i = 0; i < endId; ++i)
    {
        if (!p->m_IsAffectedByAttractor[i]) continue;
        unsigned particleID = p->m_LayerID[i];
        auto attractors = p->m_AttractorList[i];
        if (p->m_IsAffectedByAllAttractor[i])
        {
            for (a = 0; a < countAttractors; ++a)
            {
                unsigned attractorID = m_Attractors[a]->GetOwner()->GetParentLayer()->GetId();
                if (attractorID != particleID) continue;
                off.x = m_Attractors[a]->GetTransform()->GetWorldPosition().x - p->m_Position[i].x;
                off.y = m_Attractors[a]->GetTransform()->GetWorldPosition().y - p->m_Position[i].y;
                off.z = m_Attractors[a]->GetTransform()->GetWorldPosition().z - p->m_Position[i].z;
                dist = off.Dot(off);

                if (fabs(dist) > 0.00001) dist = dist * m_Attractors[a]->GetForce();

                p->m_Acceleration[i] += off * dist;
            }
        }
        else
        {
            for (a = 0; a < countAttractors; ++a)
            {
                unsigned attractorID = m_Attractors[a]->GetOwner()->GetParentLayer()->GetId();
                if (attractorID != particleID) continue;
                unsigned attractorRealID = m_Attractors[a]->GetOwner()->GetID();
                auto iterator = std::find(attractors.begin(), attractors.end(), attractorRealID);
                if (iterator == attractors.end()) continue;
                off.x = m_Attractors[a]->GetTransform()->GetWorldPosition().x - p->m_Position[i].x;
                off.y = m_Attractors[a]->GetTransform()->GetWorldPosition().y - p->m_Position[i].y;
                off.z = m_Attractors[a]->GetTransform()->GetWorldPosition().z - p->m_Position[i].z;
                dist = off.Dot(off);

                if (fabs(dist) > 0.00001) dist = dist * m_Attractors[a]->GetForce();

                p->m_Acceleration[i] += off * dist;
            }
        }
    }
}

Vector4 CircleParticleEmitter::linearRand(const Vector4& Min, const Vector4& Max) const
{
    Vector4 tmp;
    tmp.x = linearRand(Min.x, Max.x);
    tmp.y = linearRand(Min.y, Max.y);
    tmp.z = linearRand(Min.z, Max.z);
    tmp.w = linearRand(Min.w, Max.w);
    return tmp;
}

float CircleParticleEmitter::linearRand(const float& Min, const float& Max) const
{
    return float(std::rand()) / float(RAND_MAX) * (Max - Min) + Min;
}

void CircleParticleEmitter::emit(float dt, ParticleData* p)
{
    Transform* trans = m_OwnerObject->GetComponent<Transform>();

    m_ElapsedTime += dt;

    const unsigned maxNewParticles = static_cast<unsigned>(m_ElapsedTime * m_EmissionRate);
    const unsigned startId = p->m_CountAlive;
    const unsigned endId = std::min(startId + maxNewParticles, p->m_Count - 1);

    m_ElapsedTime = fmodf(m_ElapsedTime, (1.0f / m_EmissionRate));

    Vector4 velMin { m_MinVelocity, 0.f };
    Vector4 velMax { m_MaxVelocity, 0.f };

    if (!trans)
        for (size_t i = startId; i < endId; ++i)
        {
            double ang = linearRand(0.0, PI * 2.0);
            float multiplier = linearRand(-1.0, 1.0);
            p->m_Position[i] = Vector4(m_Center, 0.f) + Vector4(m_RadiusX * sinf((float)ang), m_RadiusY * cosf((float)ang), m_RadiusZ * multiplier, 1.f);
        }
    else
    {
        Vector4 pos = Vector4(trans->GetWorldPosition(), 0.f);
        for (size_t i = startId; i < endId; ++i)
        {
            double ang = linearRand(0.0, PI * 2.0);
            float multiplier = linearRand(-1.0, 1.0);
            p->m_Position[i] = pos + Vector4(m_Center, 0.f) + Vector4(m_RadiusX * sinf((float)ang), m_RadiusY * cosf((float)ang), m_RadiusZ * multiplier, 1.f);
        }
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_LayerID[i] = GetOwner()->GetParentLayer()->GetId();
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_Velocity[i] = linearRand(velMin, velMax);
    }

    for (size_t i = startId; i < endId; ++i)
    {
        p->m_Time[i].x = linearRand(m_MinTime, m_MaxTime);
        p->m_Time[i].z = (float)0.0;
        p->m_Time[i].w = (float)1.0 / p->m_Time[i].x;
        p->m_Time[i].y = p->m_Time[i].x * m_TextureFade;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_StartColor[i] = linearRand(m_StartMinColor, m_StartMaxColor);
        p->m_EndColor[i] = linearRand(m_EndMinColor, m_EndMaxColor);
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_Size[i] = linearRand(m_MinSize, m_MaxSize);
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_IsAffectedByAttractor[i] = m_IsAffectedByAttractor;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_Gravity[i] = m_Gravity;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_IsAffectedByAllAttractor[i] = m_IsAffectedByAllAttractor;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_IsAffectedByFloor[i] = m_IsAffectedByFloor;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_FloorHeight[i] = m_FloorHeight;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_TextureFade[i] = m_TextureFade <= 0.f ? 0.f : 1.f;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->m_AttractorList[i] = m_Attractors;
    }

    for (unsigned i = startId; i < endId; ++i)
    {
        p->wake(i, m_Texture, m_EndTexture);
    }
}

REFLECT_VIRTUAL(ParticleEmitter)
REFLECT_PROPERTY(m_EmissionRate)
REFLECT_PROPERTY(m_Texture)
REFLECT_PROPERTY(m_EndTexture)
REFLECT_PROPERTY(m_TextureFade)
REFLECT_PROPERTY(m_StartMinColor)
REFLECT_PROPERTY(m_StartMaxColor)
REFLECT_PROPERTY(m_EndMinColor)
REFLECT_PROPERTY(m_EndMaxColor)
REFLECT_PROPERTY(m_MinSize)
REFLECT_PROPERTY(m_MaxSize)
REFLECT_PROPERTY(m_Gravity)
REFLECT_PROPERTY(m_IsAffectedByAttractor)
REFLECT_PROPERTY(m_IsAffectedByFloor)
REFLECT_PROPERTY(m_IsAffectedByAllAttractor)
REFLECT_PROPERTY(m_FloorHeight)
REFLECT_END()

REFLECT_INIT(BoxParticleEmitter)
REFLECT_PARENT(ParticleEmitter)
REFLECT_PROPERTY(m_Position)
REFLECT_PROPERTY(m_StartPositionOffset)
REFLECT_PROPERTY(m_MinVelocity)
REFLECT_PROPERTY(m_MaxVelocity)
REFLECT_PROPERTY(m_MinTime)
REFLECT_PROPERTY(m_MaxTime)
REFLECT_END()

REFLECT_INIT(CircleParticleEmitter)
REFLECT_PARENT(ParticleEmitter)
REFLECT_PROPERTY(m_RadiusX)
REFLECT_PROPERTY(m_RadiusY)
REFLECT_PROPERTY(m_RadiusZ)
REFLECT_PROPERTY(m_Center)
REFLECT_PROPERTY(m_StartPositionOffset)
REFLECT_PROPERTY(m_MinVelocity)
REFLECT_PROPERTY(m_MaxVelocity)
REFLECT_PROPERTY(m_MinTime)
REFLECT_PROPERTY(m_MaxTime)
REFLECT_END()

REFLECT_INIT(Attractor)
REFLECT_PROPERTY(m_Force)
REFLECT_END()
