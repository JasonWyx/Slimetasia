#pragma once
// ===========================================================================|
// INCLUDES                                                                   |
// ===========================================================================|
#include <array>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

/// Others
#include "AudioResource.h"
#include "CorePrerequisites.h"
#include "External Libraries/tinyxml2/tinyxml2.h"
#include "Font.h"
#include "ISystem.h"
#include "Mesh.h"
#include "ResourceBase.h"
#include "ResourceEntry.h"
#include "ResourceHandle.h"
#include "Texture.h"

// ===========================================================================|
// DEFINE                                                                     |
// ===========================================================================|
static constexpr unsigned MAX_RESOURCE_ENTRIES = 1 << 16;

enum class ResourceType
{
    Audio = 0,
    Texture,
    Model,
    Font,
    Invalid
};

// ===========================================================================|
// RESOURCE MANAGER                                                           |
// ===========================================================================|
class ResourceManager : public ISystem<ResourceManager>
{
private:  // Typedefs ----------------------------------------------------------
    using ResourceVector = std::vector<ResourceEntry>;
    using ResourceNamesMap = std::unordered_map<std::string, unsigned>;
    using ResourceIDsMap = std::unordered_map<ResourceGUID, unsigned>;

    /// Variables ---------------------------------------------------------------
    ResourceVector m_ResourceEntries;
    ResourceNamesMap m_ResourceNameMap;
    ResourceIDsMap m_ResourceIDMap;

    std::array<std::set<std::string>, (int)ResourceType::Invalid> m_AcceptableFileTypes;

    std::stack<unsigned short> m_FreeIndices;

    tinyxml2::XMLDocument m_ResourceXML;
    tinyxml2::XMLDocument m_PhysicsWorldSettingsXML;

public:
    static const std::filesystem::path s_DirectoryPath;
    static const std::filesystem::path s_ResourcePath;
    static const std::filesystem::path s_ResourcePathAudio;
    static const std::filesystem::path s_ResourcePathTexture;
    static const std::filesystem::path s_ResourcePathModel;
    static const std::filesystem::path s_ResourcePathFonts;
    static const std::filesystem::path s_ResourcePathScripts;
    static const std::filesystem::path s_ResourceDataDocument;
    static const std::filesystem::path s_PhysicsWorldSettingsDataDocument;

    /// Functions ---------------------------------------------------------------
private:
    friend class ResourceHandleBase;
    ResourceBase* GetResourceInstance(ResourceHandleBase& handle);

public:
    ResourceManager();
    ~ResourceManager();

    std::string ResolveNameConflict(std::string const& name);
    bool CheckNameExists(std::string const& name);
    void RenameResource(std::string const& oldName, std::string newName);
    void Serialize();
    void Unserialize();
    void Clear();

    /// Add new file to the resource folder
    const std::array<std::set<std::string>, (int)ResourceType::Invalid>& GetAllAcceptableFileTypes() const;
    void RefreshResources();
    bool CopyNewResource(std::filesystem::path& filePath, HWND hwnd);
    ResourceType GetResourceType(std::filesystem::path filePath) const;
    bool GetResourceFolderPath(const std::filesystem::path& filePath, std::filesystem::path& folderPath) const;
    void SavePhysicsWorldSettings(std::string filename);
    void LoadPhysicsWorldSettings(std::string filename);

    /// Data serialization
    template <typename XML> tinyxml2::XMLElement* FindElement(XML* doc, std::string name, bool recusive = false);
    template <typename T> tinyxml2::XMLElement* InsertElement(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* root, std::string name, const T& val);
    tinyxml2::XMLElement* InsertElement(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* root, std::string name, std::string val);

    // template<typename ResourceType> ResourceHandle<ResourceType> GetResource     (std::string const & name);
    // template<typename ResourceType> ResourceHandle<ResourceType> GetResource     (unsigned id);
    // template<typename ResourceType> ResourceType *               GetRawResource  (ResourceHandle<ResourceType> const & handle);
    // template<typename ResourceType> ResourceHandle<ResourceType> CreateResource  (std::string name = std::string{}, std::string fullname = std::string{});
    // template<typename ResourceType> ResourceHandle<ResourceType> CreateResource  (ResourceType * resource);
    // template<typename ResourceType> void                         DestroyResource (ResourceHandle<ResourceType> handle);

    template <typename ResourceType> std::vector<ResourceHandle<ResourceType>> GetResources()
    {
        std::vector<ResourceHandle<ResourceType>> results;

        for (const auto& entry : m_ResourceIDMap)
        {
            if (dynamic_cast<ResourceType*>(m_ResourceEntries[entry.second].m_Entry) != nullptr)
            {
                results.emplace_back(&m_ResourceEntries[entry.second]);
            }
        }

        return results;
    }

    /*
    Function to get a resource via name.
    */
    template <typename ResourceType> ResourceHandle<ResourceType> GetResource(std::string const& name)
    {
        /// Find resource
        auto entryIndex = m_ResourceNameMap.find(name);
        if (entryIndex != m_ResourceNameMap.end())
        {
            auto& resource = m_ResourceEntries[entryIndex->second];
            if (dynamic_cast<ResourceType*>(resource.m_Entry))
            {
                return ResourceHandle<ResourceType>(&resource);
            }
        }

        /// No resource of name is found
        std::cout << "RESOURCE MANAGER : FAIL - No (" << typeid(ResourceType).name() << ") resource with name " << name << std::endl;
        return ResourceHandle<ResourceType>{};
    }

    /*
    Function to get a resource via ID.
    */
    template <typename ResourceType> ResourceHandle<ResourceType> GetResource(ResourceGUID guid)
    {
        /// Find resource
        // std::cout << "ID IS " << id << std::endl;
        auto entryIndex = m_ResourceIDMap.find(guid);
        if (entryIndex != m_ResourceIDMap.end())
        {
            auto& resource = m_ResourceEntries[entryIndex->second];
            if (dynamic_cast<ResourceType*>(resource.m_Entry))
            {
                return ResourceHandle<ResourceType>(&resource);
            }
        }

        /// No resource of GUID is found
        std::cout << "RESOURCE MANAGER : FAIL - No (" << typeid(ResourceType).name() << ") resource with GUID : " << guid << std::endl;
        return ResourceHandle<ResourceType>{};
    }

    /*
    Function to create a new resource.
    Provide a unique ID if load set to true.
    */
    template <typename ResourceType> ResourceHandle<ResourceType> CreateResource(const std::string resourceName = "", const std::filesystem::path& filePath = "", const ResourceGUID guid = 0)
    {
        // No more free slots
        ASSERT(!m_FreeIndices.empty());

        unsigned short index = m_FreeIndices.top();
        m_FreeIndices.pop();

        std::string assignedName = resourceName;

        // Resolve name conflicts
        if (CheckNameExists(assignedName))
        {
            assignedName = ResolveNameConflict(resourceName);
        }

        m_ResourceEntries[index].m_Entry = new ResourceType(resourceName, filePath);
        if (guid == 0)
        {
            m_ResourceEntries[index].m_Entry->GenerateGUID();
        }
        m_ResourceIDMap[m_ResourceEntries[index].m_Entry->GetGUID()] = index;
        m_ResourceNameMap[m_ResourceEntries[index].m_Entry->m_Name] = index;
        return ResourceHandle<ResourceType>(&m_ResourceEntries[index]);
    }

    /*
    Function to create a new resouce. Used during importing.
    */
    template <typename ResourceType> ResourceHandle<ResourceType> CreateResource(ResourceType* resource)
    {
        // No more free slots
        ASSERT(!m_FreeIndices.empty());

        unsigned short index = m_FreeIndices.top();
        m_FreeIndices.pop();

        m_ResourceEntries[index].m_Entry = resource;
        m_ResourceIDMap[resource->GetGUID()] = index;
        m_ResourceNameMap[resource->GetName()] = index;

        return ResourceHandle<ResourceType>(&m_ResourceEntries[index]);
    }

    /*
    Funtion to destroy a resource.
    */

    template <typename ResourceType> void DestroyResource(ResourceHandle<ResourceType>& handle)
    {
        if (handle.Validate())
        {
            auto entryIndex = m_ResourceIDMap.find(handle->GetGUID());
            if (entryIndex != m_ResourceIDMap.end())
            {
                // Update handles and cleanup maps
                m_ResourceEntries[entryIndex->second].InvalidateHandles();
                m_FreeIndices.push(entryIndex->second);
                m_ResourceNameMap.erase(handle->m_Name);
                m_ResourceIDMap.erase(handle->GetGUID());

                delete m_ResourceEntries[entryIndex->second].m_Entry;
            }
        }
    }
};

#define RESOURCEMANAGER ResourceManager::Instance()
