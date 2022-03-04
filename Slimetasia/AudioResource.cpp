#include "AudioResource.h"

#include "AudioSystem.h"

/*
Constructor for AudioResource.
*/
AudioResource::AudioResource(const std::string& resourceName, const filesystem::path& filepath)
    : ResourceBase{resourceName, filepath}
    , is3D_{true}
    , loop_{false}
    , stream_{false}
{
}

/*
Function to serialize the AudioResource.
*/
void AudioResource::Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem)
{
    tinyxml2::XMLElement* resourceElem = doc->NewElement("Audio");
    tinyxml2::XMLElement* nameElem = doc->NewElement("Name");
    tinyxml2::XMLElement* filepathElem = doc->NewElement("Filepath");

    resourceElem->SetAttribute("GUID", static_cast<int64_t>(m_GUID));
    nameElem->SetText(m_Name.c_str());
    filepathElem->SetText(m_FilePath.string().c_str());

    parentElem->InsertEndChild(resourceElem);
    resourceElem->InsertEndChild(nameElem);
    resourceElem->InsertEndChild(filepathElem);
}

/*
Function to unserialize the AudioResource.
*/
void AudioResource::Unserialize(tinyxml2::XMLElement* currElem)
{
    m_GUID = (ResourceGUID)currElem->Int64Attribute("GUID");

    m_Name = currElem->FirstChildElement("Name")->GetText();
    m_FilePath = currElem->FirstChildElement("Filepath")->GetText();
}

/*
Function to load the AudioResource.
*/
void AudioResource::Load()
{
    /// Adds sound to audio system
    AUDIOSYSTEM.LoadAudio(m_Name, m_FilePath.string(), is3D_, loop_, stream_);

    /// Set status
    m_LoadStatus = ResourceStatus::Loaded;
}

/*
Function to unload the AudioResource.
*/
void AudioResource::Unload()
{
    /// Removes sound from audio system
    AUDIOSYSTEM.UnLoadAudio(m_Name);

    /// Set status
    m_LoadStatus = ResourceStatus::Unloaded;
}

/*
Function to set the 3D option for the AudioResource.
*/
bool AudioResource::Set3D(bool state)
{
    is3D_ = state;
    return is3D_;
}

/*
Function to set the loop option for the AudioResource.
*/
bool AudioResource::SetLoop(bool state)
{
    loop_ = state;
    return loop_;
}

/*
Function to set the stream option for the AudioResource.
*/
bool AudioResource::SetStream(bool state)
{
    stream_ = state;
    return stream_;
}