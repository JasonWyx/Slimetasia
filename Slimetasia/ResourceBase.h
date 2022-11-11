#pragma once
#include <ntverp.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "External Libraries/tinyxml2/tinyxml2.h"

// ===========================================================================|
// Foward declaration                                                         |
// ===========================================================================|
class ResourceManager;

enum class ResourceStatus
{
    Unloaded = 0,
    Loading,
    Loaded
};

using ResourceGUID = unsigned long long;

// ===========================================================================|
// ResourceBase                                                               |
// ===========================================================================|
class ResourceBase
{
public:  // Members

    std::string m_Name;                // Name as referenced in the editor
    std::filesystem::path m_FilePath;  // File path

public:  // Functions

    explicit ResourceBase(const std::string& name = "", const std::filesystem::path& filepath = "");
    virtual ~ResourceBase() = default;

    const std::string& GetName() const;
    const std::filesystem::path& GetFilePath() const;
    ResourceStatus GetStatus() const;
    ResourceGUID GetGUID() const;

    void GenerateGUID();

    virtual void Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem);
    virtual void Unserialize(tinyxml2::XMLElement* currElem);
    virtual void Load() {}
    virtual void Unload() {}

    // Disabled Functions
    ResourceBase(ResourceBase const&) = delete;
    ResourceBase(ResourceBase&&) = delete;
    ResourceBase& operator=(ResourceBase const&) = delete;
    ResourceBase& operator=(ResourceBase&&) = delete;

protected:  // Members

    ResourceGUID m_GUID;          // Resource ID
    ResourceStatus m_LoadStatus;  // Status of the resource

    friend class ResourceManager;
};
