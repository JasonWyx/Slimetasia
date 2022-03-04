/******************************************************************************/
/*!

*/
/******************************************************************************/

#pragma once

#include <string>

#include "ResourceBase.h"

enum class FileResourceStatus
{
    LOADING = 0,
    LOADED,
    UNLOADED
};

// Base class for resources that depends on file system
class FileResourceBase : public ResourceBase
{
private:
    std::string m_FilePath;
    FileResourceStatus m_Status;

public:
    FileResourceBase();
    ~FileResourceBase();
    FileResourceBase(FileResourceBase const&) = delete;
    FileResourceBase(FileResourceBase&&) = delete;

    FileResourceBase& operator=(FileResourceBase const&) = delete;
    FileResourceBase& operator=(FileResourceBase&&) = delete;

    virtual void Load() = 0;
    virtual void Unload() = 0;
    void Reload();

    std::string const& GetFilePath() const;
    void SetFilePath(std::string const& filePath);

    FileResourceStatus GetStatus() const;
};