#include "FileResourceBase.h"

FileResourceBase::FileResourceBase()
    : ResourceBase()
    , m_FilePath()
    , m_Status(FileResourceStatus::UNLOADED)
{
}

FileResourceBase::~FileResourceBase() {}

void FileResourceBase::Reload()
{
    if (m_Status == FileResourceStatus::LOADED) Unload();
    Load();
}

std::string const& FileResourceBase::GetFilePath() const
{
    return m_FilePath;
}

void FileResourceBase::SetFilePath(std::string const& filePath)
{
    m_FilePath = filePath;
    // TODO: Handle invalid path/failure to load
}

FileResourceStatus FileResourceBase::GetStatus() const
{
    return m_Status;
}
