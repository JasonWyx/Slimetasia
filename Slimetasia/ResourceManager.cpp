#include "ResourceManager.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>

#include "CorePrerequisites.h"
#include "PhysicsSystem.h"
#include "PlayerPref.h"

// ===========================================================================|
// STATIC VARIABLES                                                           |
// ===========================================================================|

const filesystem::path ResourceManager::s_DirectoryPath = filesystem::current_path();
const filesystem::path ResourceManager::s_ResourcePath = "Resources";
const filesystem::path ResourceManager::s_ResourcePathAudio = ResourceManager::s_ResourcePath / "Audio";
const filesystem::path ResourceManager::s_ResourcePathTexture = ResourceManager::s_ResourcePath / "Texture";
const filesystem::path ResourceManager::s_ResourcePathModel = ResourceManager::s_ResourcePath / "Models";
const filesystem::path ResourceManager::s_ResourcePathFonts = ResourceManager::s_ResourcePath / "Fonts";
const filesystem::path ResourceManager::s_ResourcePathScripts = ResourceManager::s_DirectoryPath / "Scripts";
const filesystem::path ResourceManager::s_ResourceDataDocument = s_ResourcePath / "Resource.xml";
const filesystem::path ResourceManager::s_PhysicsWorldSettingsDataDocument = s_ResourcePath / "PhysicsWorldSettings.xml";

/*
Constructor for ResourceManager.
*/
ResourceManager::ResourceManager()
    : m_ResourceEntries{MAX_RESOURCE_ENTRIES}
    , m_ResourceNameMap{}
    , m_AcceptableFileTypes{
          std::set<std::string>{".ogg", ".OGG"},                                                                  // Audio
          std::set<std::string>{".dds", ".jpg", ".png", ".tga", ".bmp", ".DDS", ".JPG", ".PNG", ".TGA", ".BMP"},  // Texture
          std::set<std::string>{".fbx", ".dae", ".FBX", ".DAE"},                                                  // Mesh
          std::set<std::string>{".ttf", ".TTF"},                                                                  // Font
      }
{
    /// Create directories if not created
    std::vector<filesystem::path> paths = {s_ResourcePath, s_ResourcePathAudio, s_ResourcePathModel, s_ResourcePathTexture, s_ResourcePathFonts, s_ResourcePathScripts};

    for (const auto& path : paths)
    {
        if (!filesystem::exists(path))
        {
            filesystem::create_directory(path);
        }
    }

    /// Create resource information
    tinyxml2::XMLError error = m_ResourceXML.LoadFile(s_ResourceDataDocument.string().c_str());
    if (error == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND)
    {
        /// Create new data file
        std::ofstream newFile(s_ResourceDataDocument);
        newFile.close();

        /// Add root elements
        tinyxml2::XMLElement* newElem;
        tinyxml2::XMLElement* tmp;
        newElem = m_ResourceXML.NewElement("ResourceData");
        m_ResourceXML.InsertFirstChild(newElem);
        tmp = m_ResourceXML.NewElement("Confiq");
        newElem->InsertEndChild(tmp);
        tmp = m_ResourceXML.NewElement("Resources");
        newElem->InsertEndChild(tmp);

        m_ResourceXML.SaveFile(s_ResourceDataDocument.string().c_str());
    }
    RefreshResources();

    /// Load all player pref
    PlayerPref::Deserialize();
}

/*
Destructor for ResourceManager.
*/
ResourceManager::~ResourceManager()
{
    /// Serialize any changes made
    Serialize();
    PlayerPref::Serialize();
    Clear();
}

ResourceBase* ResourceManager::GetResourceInstance(ResourceHandleBase& handle)
{
    auto it = m_ResourceIDMap.find(static_cast<ResourceGUID>(handle));
    if (it != m_ResourceIDMap.end())
    {
        return m_ResourceEntries[it->second].m_Entry;
    }
    return nullptr;
}

/*
Function to resolve name conflict
*/
std::string ResourceManager::ResolveNameConflict(std::string const& name)
{
    std::string assignName = name;
    int duplicateNum = 1;

    /// Append duplicate name if already exist
    while (CheckNameExists(assignName))
    {
        assignName = name + "(" + std::to_string(duplicateNum) + ")";
        duplicateNum++;
    }

    return assignName;
}

/*
Function to check if a resource with a name exist.
*/
bool ResourceManager::CheckNameExists(std::string const& name)
{
    return m_ResourceNameMap.find(name) != m_ResourceNameMap.end();
}

/*
Function to rename a paticular resource.
*/
void ResourceManager::RenameResource(std::string const& oldName, std::string newName)
{
    auto it = m_ResourceNameMap.find(oldName);

    // If name exists...
    if (it != m_ResourceNameMap.end())
    {
        unsigned index = it->second;

        // Replace name index map
        m_ResourceNameMap.erase(oldName);
        m_ResourceNameMap.try_emplace(newName, index);

        m_ResourceEntries[it->second].m_Entry->m_Name = newName;
    }
}

/*
Function for serialization.
*/
void ResourceManager::Serialize()
{
    /// Record configurations
    tinyxml2::XMLElement* root;

    root = FindElement(&m_ResourceXML, "Confiq", true);

    /// Record exisiting resource
    root = FindElement(&m_ResourceXML, "Resources", true);

    root->DeleteChildren();

    for (auto const& pair : m_ResourceIDMap)
    {
        ResourceEntry& handleEntry = m_ResourceEntries[pair.second];

        if (handleEntry.m_Entry && handleEntry.m_Entry->m_FilePath.string().size() > 0)
        {
            handleEntry.m_Entry->Serialize(&m_ResourceXML, root);
        }
    }

    m_ResourceXML.SaveFile(s_ResourceDataDocument.string().c_str());
}

/*
Function for unserialization.
*/
void ResourceManager::Unserialize()
{
    tinyxml2::XMLElement* root;
    tinyxml2::XMLElement* curr;

    /// Load resources
    root = FindElement(&m_ResourceXML, "Resources", true);

    if (root)
    {
        /// Iterate through all resource
        curr = root->FirstChildElement();

        while (curr)
        {
            if (strcmp("Audio", curr->Name()) == 0)
            {
                AudioResource* resource = new AudioResource("", "");
                resource->Unserialize(curr);
                CreateResource<AudioResource>(resource);
            }
            else if (strcmp("AnimationSet", curr->Name()) == 0)
            {
                AnimationSet* resource = new AnimationSet("", "");
                resource->Unserialize(curr);
                CreateResource<AnimationSet>(resource);
            }
            else if (strcmp("Mesh", curr->Name()) == 0)
            {
                Mesh* resource = new Mesh("", "");
                resource->Unserialize(curr);
                CreateResource<Mesh>(resource);
            }
            else if (strcmp("Texture", curr->Name()) == 0)
            {
                Texture* resource = new Texture("", "");
                resource->Unserialize(curr);
                CreateResource<Texture>(resource);
            }
            else if (strcmp("Font", curr->Name()) == 0)
            {
                Font* resource = new Font("", "");
                resource->Unserialize(curr);
                CreateResource<Font>(resource);
            }

            curr = curr->NextSiblingElement();
        }
    }
}

/*
Function to clear all resources in the resource manager.
*/
void ResourceManager::Clear()
{
    /// Unload all resource
    for (auto& pair : m_ResourceIDMap)
    {
        // Unload and delete resource
        m_ResourceEntries[pair.second].m_Entry->Unload();
        delete m_ResourceEntries[pair.second].m_Entry;

        // Update resource entry
        m_ResourceEntries[pair.second].m_Entry = nullptr;
    }

    m_FreeIndices = std::stack<unsigned short>();

    for (int i = MAX_RESOURCE_ENTRIES - 1; i >= 0; --i)
        m_FreeIndices.push(i);
}

/*
Function to refresh all resources.
*/
void ResourceManager::RefreshResources()
{
    /// Clear current resources
    Clear();

    /// Unserialize from data file
    Unserialize();

    for (auto& pair : m_ResourceIDMap)
    {
        m_ResourceEntries[pair.second].m_Entry->Load();
    }
}

/*
Function to add a new file into the resource folder.
*/
bool ResourceManager::CopyNewResource(filesystem::path& filePath, HWND hwnd)
{
    filesystem::path srcFilePath = filePath;
    filesystem::path dstFilePath;

    if (srcFilePath.has_extension())
    {
        filesystem::path folderPath;
        if (!GetResourceFolderPath(srcFilePath, folderPath))
        {
            return false;
        }

        dstFilePath = folderPath / srcFilePath.filename();
        if (filesystem::copy_file(srcFilePath, dstFilePath, filesystem::copy_options::skip_existing))
        {
            std::string errorMsg = "RESOURCE MANAGER : File already exist \nOverwrite (" + filePath.filename().string() + ") ?";
            if (MessageBox(hwnd, LPCSTR(errorMsg.c_str()), NULL, MB_YESNO) == 6)
            {
                filesystem::copy_file(filePath, dstFilePath, filesystem::copy_options::overwrite_existing);
            }
            else
            {
                return false;
            }
        }
        filePath = dstFilePath;
        return true;
    }
    return false;
}

/*
Function to assign the new resource to a folder.
*/
bool ResourceManager::GetResourceFolderPath(const filesystem::path& filePath, filesystem::path& folderPath) const
{
    /// Identify resource type
    ResourceType type = GetResourceType(filePath);

    /// Assign resource folder name
    if (type == ResourceType::Invalid) return false;

    switch (type)
    {
        case ResourceType::Audio: folderPath = s_ResourcePathAudio; break;
        case ResourceType::Texture: folderPath = s_ResourcePathTexture; break;
        case ResourceType::Model: folderPath = s_ResourcePathModel; break;
        case ResourceType::Font: folderPath = s_ResourcePathFonts; break;
    }

    return true;
}

/*
Function to identify the type of resource.
*/
ResourceType ResourceManager::GetResourceType(filesystem::path filePath) const
{
    if (filePath.has_extension())
    {
        const auto& extension = filePath.extension().string();

        if (m_AcceptableFileTypes[0].find(extension) != m_AcceptableFileTypes[0].end()) return ResourceType::Audio;
        if (m_AcceptableFileTypes[1].find(extension) != m_AcceptableFileTypes[1].end()) return ResourceType::Texture;
        if (m_AcceptableFileTypes[2].find(extension) != m_AcceptableFileTypes[2].end()) return ResourceType::Model;
        if (m_AcceptableFileTypes[3].find(extension) != m_AcceptableFileTypes[3].end()) return ResourceType::Font;
    }
    return ResourceType::Invalid;
}

/*
Function to get all the file types accepted by the system.
*/
const std::array<std::set<std::string>, (int)ResourceType::Invalid>& ResourceManager::GetAllAcceptableFileTypes() const
{
    return m_AcceptableFileTypes;
}

void ResourceManager::SavePhysicsWorldSettings(std::string filename)
{
    // save to file.
    /// Record configurations
    tinyxml2::XMLElement* root;

    root = m_PhysicsWorldSettingsXML.NewElement("Physics_World_Settings");
    m_PhysicsWorldSettingsXML.InsertEndChild(root);
    root->SetAttribute("Default_Friction_Coefficient", PhysicsSystem::s_PhyWorldSettings.m_DefaultFrictCoefficient);
    root->SetAttribute("Default_Restitution", PhysicsSystem::s_PhyWorldSettings.m_DefaultRestitution);
    root->SetAttribute("Restitution_Velocity_Threshold", PhysicsSystem::s_PhyWorldSettings.m_RestitutionVelThreshold);
    root->SetAttribute("Default_Rolling_Resistance", PhysicsSystem::s_PhyWorldSettings.m_DefaultRollResis);
    root->SetAttribute("Sleeping_Enabled", PhysicsSystem::s_PhyWorldSettings.m_IsSleepingEnabled);
    root->SetAttribute("Default_Time_Before_Sleep", PhysicsSystem::s_PhyWorldSettings.m_DefaultTimeBeforeSleep);
    root->SetAttribute("Gravity_x", PhysicsSystem::s_PhyWorldSettings.m_Gravity.x);
    root->SetAttribute("Gravity_y", PhysicsSystem::s_PhyWorldSettings.m_Gravity.y);
    root->SetAttribute("Gravity_z", PhysicsSystem::s_PhyWorldSettings.m_Gravity.z);
    m_PhysicsWorldSettingsXML.SaveFile(filename.c_str());
}

void ResourceManager::LoadPhysicsWorldSettings(std::string filename)
{
    tinyxml2::XMLError error = m_PhysicsWorldSettingsXML.LoadFile(filename.c_str());

    // if the settings file is not found.
    if (error == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND)
    {
        /// Create new data file
        // std::ofstream newFile(filename);
        // newFile.close();
        //
        // /// Add root elements
        // tinyxml2::XMLElement * newElem;
        // //tinyxml2::XMLElement * tmp    ;
        // newElem = m_PhysicsWorldSettingsData.NewElement("Physics_World_Settings");
        // m_PhysicsWorldSettingsData.InsertFirstChild(newElem);
        // m_PhysicsWorldSettingsData.SaveFile(filename.c_str());
        return;
    }
    // if the settings file is not found.
    /*if (error == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND)
    {
            std::cout<<"PhysicsWorldSettings.xml is unable to be found in the Resource Folder!\n";
            return;
    }*/

    tinyxml2::XMLElement* root;

    // Load resources
    root = FindElement(&m_PhysicsWorldSettingsXML, "Physics_World_Settings", true);

    if (root)
    {
        PhysicsSystem::s_PhyWorldSettings.m_DefaultFrictCoefficient = root->FloatAttribute("Default_Friction_Coefficient");
        PhysicsSystem::s_PhyWorldSettings.m_DefaultRestitution = root->FloatAttribute("Default_Restitution");
        PhysicsSystem::s_PhyWorldSettings.m_RestitutionVelThreshold = root->FloatAttribute("Restitution_Velocity_Threshold");
        PhysicsSystem::s_PhyWorldSettings.m_DefaultRollResis = root->FloatAttribute("Default_Rolling_Resistance");
        PhysicsSystem::s_PhyWorldSettings.m_IsSleepingEnabled = root->FloatAttribute("Sleeping_Enabled");
        PhysicsSystem::s_PhyWorldSettings.m_DefaultTimeBeforeSleep = root->FloatAttribute("Default_Time_Before_Sleep");
        PhysicsSystem::s_PhyWorldSettings.m_Gravity.x = root->FloatAttribute("Gravity_x");
        PhysicsSystem::s_PhyWorldSettings.m_Gravity.y = root->FloatAttribute("Gravity_y");
        PhysicsSystem::s_PhyWorldSettings.m_Gravity.z = root->FloatAttribute("Gravity_z");
    }
}

/*
Function to find an element in an xml document.
*/
template <typename XML> tinyxml2::XMLElement* ResourceManager::FindElement(XML* doc, std::string name, bool recursive)
{
    tinyxml2::XMLElement* curr = doc->FirstChildElement(name.c_str());

    if (!curr && recursive)
    {
        curr = doc->FirstChildElement();
        while (curr)
        {
            tinyxml2::XMLElement* potentialResult = FindElement(curr, name, recursive);
            if (potentialResult) return potentialResult;

            curr = curr->NextSibling()->ToElement();
        }
    }

    return curr;
}

/*
Function to insert an element into an xml document.
*/
template <typename T> tinyxml2::XMLElement* ResourceManager::InsertElement(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* root, std::string name, const T& val)
{
    tinyxml2::XMLElement* newElement = FindElement(root, name);
    if (newElement)
        newElement->SetText(val);
    else
    {
        newElement = doc->NewElement(name.c_str());
        newElement->SetText(val);
        root->InsertEndChild(newElement);
    }

    return newElement;
}

tinyxml2::XMLElement* ResourceManager::InsertElement(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* root, std::string name, std::string val)
{
    tinyxml2::XMLElement* newElement = FindElement(root, name);
    if (newElement)
        newElement->SetText(val.c_str());
    else
    {
        newElement = doc->NewElement(name.c_str());
        newElement->SetText(val.c_str());
        root->InsertEndChild(newElement);
    }

    return newElement;
}
