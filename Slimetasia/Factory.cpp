#include "Factory.h"

std::map<std::string, ComponentReflection*>* Factory::m_Reflection;
std::map<std::string, ComponentReflection*>* Factory::m_Resources;
std::map<std::string, ComponentFactory*>* Factory::m_Factories;
std::map<std::string, EnumReflection*>* Factory::m_Enums;

Factory::Factory() {}

Factory::~Factory() {}

void Factory::Initialize()
{
    if (m_Reflection == nullptr) m_Reflection = new std::map<std::string, ComponentReflection*> {};
    if (m_Factories == nullptr) m_Factories = new std::map<std::string, ComponentFactory*> {};
    if (m_Enums == nullptr) m_Enums = new std::map<std::string, EnumReflection*> {};
    if (m_Resources == nullptr) m_Resources = new std::map<std::string, ComponentReflection*> {};
}

void Factory::Shutdown()
{
    delete m_Reflection;
    delete m_Factories;
    delete m_Enums;
    delete m_Resources;
}

void Factory::registerType(const std::string& name, ComponentFactory* factory)
{
    if (m_Factories == nullptr) m_Factories = new std::map<std::string, ComponentFactory*> {};
    m_Factories->insert(std::pair<std::string, ComponentFactory*>(name, factory));
}

void Factory::registerType(const std::string& name, ComponentReflection* reflect)
{
    if (m_Reflection == nullptr) m_Reflection = new std::map<std::string, ComponentReflection*> {};
    m_Reflection->insert(std::pair<std::string, ComponentReflection*>(name, reflect));
}

void Factory::registerType(const std::string& name, EnumReflection* reflect)
{
    if (m_Enums == nullptr) m_Enums = new std::map<std::string, EnumReflection*> {};
    m_Enums->insert(std::pair<std::string, EnumReflection*>(name, reflect));
}

void Factory::registerResource(const std::string& name, ComponentReflection* reflect)
{
    if (m_Resources == nullptr) m_Resources = new std::map<std::string, ComponentReflection*> {};
    m_Resources->insert(std::pair<std::string, ComponentReflection*>(name, reflect));
}
