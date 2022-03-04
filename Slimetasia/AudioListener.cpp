#include "AudioListener.h"

#include "AudioSystem.h"
#include "Transform.h"

// ===========================================================================|
// AUDIOLISTENER                                                              |
// ===========================================================================|

/*
Constructor for AudioListener.
*/
AudioListener::AudioListener(GameObject* parentObject)
    : IComponent{parentObject, "AudioListener"}
    , isMain_()
{
    if (!m_OwnerObject->GetParentLayer()) return;

    /// Add to AudioSystem
    AUDIOSYSTEM.audioListeners_.push_back(this);

    /// Make this the main listener
    AUDIOSYSTEM.MakeMainListener(this);
}

/*
Destructor for AudioListener.
*/
AudioListener::~AudioListener()
{
    if (!m_OwnerObject->GetParentLayer()) return;
    /// Remove from AudioSystem
    auto position = std::find(AUDIOSYSTEM.audioListeners_.begin(), AUDIOSYSTEM.audioListeners_.end(), this);
    AUDIOSYSTEM.audioListeners_.erase(position);

    /// Reset main listener if required
    if (this == AUDIOSYSTEM.mainListener_) AUDIOSYSTEM.FindNewListener();
}

/*
Function to make this listener the main listener in the scene.
state : Should this listener be the main listener of not.
*/
void AudioListener::MakeMain(bool state)
{
    /// Check if the listener is already in the same state
    // if (isMain_ == state)
    //   return;
    isMain_ = state;

    /// Check if making it main
    isMain_ ? AUDIOSYSTEM.MakeMainListener(this) : AUDIOSYSTEM.FindNewListener(this);
}

/*
Function to check if this listener is the main listener.
*/
bool AudioListener::IsMain() const
{
    return isMain_;
}

/*
Function to retrieve the position of the listener.
*/
Vector3 AudioListener::GetPosition() const
{
    /// Returns a legit position if the transform componenet exists
    auto transformRef_ = m_OwnerObject->GetComponent<Transform>();
    if (transformRef_) return transformRef_->GetWorldPosition();

    return Vector3{0.0f, 0.0f, 0.0f};
}

/*
Function to get the forward vector of the audio listener.
*/
Vector3 AudioListener::GetForwardVector() const
{
    auto transformRef_ = m_OwnerObject->GetComponent<Transform>();
    return transformRef_ ? transformRef_->GetForwardVector() : Transform::worldForward;
}

/*
Function to get the upward vector of the audio listener
*/
Vector3 AudioListener::GetUpwardVector() const
{
    auto transformRef_ = m_OwnerObject->GetComponent<Transform>();
    return transformRef_ ? transformRef_->GetUpwardVector() : Transform::worldUpward;
}

/*
Reflection.
*/
REFLECT_INIT(AudioListener)
REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(isMain_)
REFLECT_END()