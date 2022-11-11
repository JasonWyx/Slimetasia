#pragma once
#include "AudioSystem.h"
#include "ResourceBase.h"

// ===========================================================================|
// AUDIO RESOURCE                                                             |
// ===========================================================================|
class AudioResource : public ResourceBase
{
    /// Variables ---------------------------------------------------------------
private:
    bool is3D_;
    bool loop_;
    bool stream_;

    /// Functions ---------------------------------------------------------------
public:
    AudioResource(const std::string& resourceName = "AudioResource", const std::filesystem::path& filepath = "");

    void Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem);
    void Unserialize(tinyxml2::XMLElement* currElem);
    void Load();
    void Unload();

    bool Set3D(bool state);
    bool SetLoop(bool state);
    bool SetStream(bool state);
};