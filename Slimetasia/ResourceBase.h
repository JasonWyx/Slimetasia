#pragma once
#include <ntverp.h>

#include <fstream>
#include <string>
#define NOMINMAX
#if VER_PRODUCTBUILD >= 10011
// Windows 10+ SDK code goes here
#include <filesystem>
#define filesystem std::filesystem
#else
#include <experimental/filesystem>
#define filesystem std::experimental::filesystem
// Windows 8.1- SDK code goes here
#endif

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
public:                           // Members
    std::string m_Name;           // Name as referenced in the editor
    filesystem::path m_FilePath;  // File path

public:  // Functions
    explicit ResourceBase(const std::string& name = "", const filesystem::path& filepath = "");
    virtual ~ResourceBase() = default;

    const std::string& GetName() const;
    const filesystem::path& GetFilePath() const;
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

protected:                        // Members
    ResourceGUID m_GUID;          // Resource ID
    ResourceStatus m_LoadStatus;  // Status of the resource

    friend class ResourceManager;
};
