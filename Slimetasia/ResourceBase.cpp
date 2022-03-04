#include "ResourceBase.h"

#include <utility>

/*
Constructor for ResourceBase.
*/
ResourceBase::ResourceBase(const std::string& resourceName, const filesystem::path& filepath)
    : m_Name(resourceName)
    , m_FilePath(filepath)
    , m_GUID(0)
    , m_LoadStatus(ResourceStatus::Unloaded)
{
}

const std::string& ResourceBase::GetName() const
{
    return m_Name;
}

/*
Function to get fullName_.
*/
const filesystem::path& ResourceBase::GetFilePath() const
{
    return m_FilePath;
}

/*
Function to get status_.
*/
ResourceStatus ResourceBase::GetStatus() const
{
    return m_LoadStatus;
}

/*
Function to get the guid_.
*/
ResourceGUID ResourceBase::GetGUID() const
{
    return m_GUID;
}

void ResourceBase::GenerateGUID()
{
    static unsigned short count = 0;
    std::time_t t = std::time(nullptr);
    char text[100] = "";

    // Print format time into 'text'
    std::strftime(text, sizeof(text), "%Y%m%d%H%M%S", std::localtime(&t));

    m_GUID = static_cast<ResourceGUID>((std::stoll(text) << 6) + count++);
}

void ResourceBase::Serialize(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentElem)
{
    tinyxml2::XMLElement* resourceElem = doc->NewElement("Resource");
    tinyxml2::XMLElement* nameElem = doc->NewElement("Name");

    resourceElem->SetAttribute("GUID", static_cast<int64_t>(m_GUID));
    nameElem->SetValue(m_Name.c_str());

    parentElem->InsertEndChild(resourceElem);
    resourceElem->InsertEndChild(nameElem);
}

void ResourceBase::Unserialize(tinyxml2::XMLElement* currElem)
{
    m_GUID = (ResourceGUID)currElem->Int64Attribute("GUID");

    m_Name = currElem->FirstChildElement("Name")->GetText();
}
