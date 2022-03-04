#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Reflection.h"

class ComponentReflection
{
public:
    virtual std::vector<registration::variant> getProperties() = 0;
    virtual std::vector<registration::class_> getParents() = 0;
    virtual bool hasVtable() = 0;
    virtual registration::class_ getType() = 0;
};

class ComponentFactory
{
public:
    virtual void* create(GameObject* go) = 0;
    virtual void remove(GameObject* go) = 0;
    virtual void createIfDoesntExist(GameObject* go) = 0;
};

class EnumReflection
{
public:
    virtual std::vector<const char*> getEnums() = 0;
};

class Factory
{
public:
    Factory();
    ~Factory();

    static void Initialize();
    static void Shutdown();

    static std::map<std::string, ComponentReflection*>* m_Reflection;
    static std::map<std::string, ComponentFactory*>* m_Factories;
    static std::map<std::string, EnumReflection*>* m_Enums;
    static std::map<std::string, ComponentReflection*>* m_Resources;

    static void registerType(const std::string& name, ComponentFactory* factory);
    static void registerType(const std::string& name, ComponentReflection* reflect);
    static void registerType(const std::string& name, EnumReflection* reflect);
    static void registerResource(const std::string& name, ComponentReflection* reflect);
};
