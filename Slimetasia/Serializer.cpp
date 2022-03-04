#include "Serializer.h"

#include "AISystem.h"
#include "Application.h"
#include "GameObject.h"
#include "IComponent.h"
#include "ParticleSystem.h"
#include "Transform.h"

// The Vtable thing is not tested, need to test and see if the offset of vtable is correct
// std::string if we want can serialize and test out some stuff, try use cast the memory into std::string
// see can call copy construct it anot
// Find out what is reference type and continue it

// debuging we can use for loops to create 100 GameObjects and set the localPosition with rand numbers
// and add different component, then save it, make a copy and then load the saved filed and save it to
// generate copy2 then diff the copy and copy2

void Serializer::RecursionParent(const char* name, unsigned char* base, TX::XMLElement* pComponent)
{
    auto parents = (*Factory::m_Reflection).at(name)->getParents();
    for (auto pare : parents)
    {
        if (!pare.parents.empty()) RecursionParent(pare.key.c_str(), base, pComponent);
        auto pare_comp = pare.get_properties();
        for (auto comp : pare_comp)
        {
            if (comp.type == typeid(std::string).name())
            {
                // std::cout << comp.name << std::endl;
                TX::XMLElement* pElem = doc.NewElement(comp.name.c_str());
                pComponent->InsertEndChild(pElem);
                std::string value = *reinterpret_cast<std::string*>(base + comp.offset);
                pElem->SetAttribute("Value", value.c_str());
                pElem->SetAttribute("Type", comp.type.c_str());
            }
            else if (comp.type.find("ptr") != std::string::npos)
                continue;
            else if (comp.type.find("std::") != std::string::npos)
                continue;
            else if (comp.type.find("*") != std::string::npos)
                continue;
            else
            {
                std::string value;
                // std::cout << comp.name << std::endl;
                TX::XMLElement* pElem = doc.NewElement(comp.name.c_str());
                pComponent->InsertEndChild(pElem);
                unsigned char* comp_addr = base + comp.offset;
                unsigned i = 0;
                if ((comp.type.find("struct") != std::string::npos || comp.type.find("class") != std::string::npos) && comp.type.find("std::") == std::string::npos)
                {
                    try
                    {
                        RecursionStruct(comp, comp_addr, pElem);
                    }
                    catch (...)
                    {
                        // std::cout << comp.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                        for (; i < comp.size; ++i)
                        {
                            unsigned int val = static_cast<unsigned int>(*(comp_addr + i));
                            value += (std::to_string(val) + ",");
                        }
                        pElem->SetAttribute("Value", value.c_str());
                        pElem->SetAttribute("Type", comp.type.c_str());
                    }
                }
                else
                {
                    for (; i < comp.size; ++i)
                    {
                        unsigned int val = static_cast<unsigned int>(*(comp_addr + i));
                        value += (std::to_string(val) + ",");
                    }
                    pElem->SetAttribute("Value", value.c_str());
                    pElem->SetAttribute("Type", comp.type.c_str());
                }
            }
        }
    }
}

void Serializer::RecursionStruct(registration::variant prop, unsigned char* base, TX::XMLElement* pComponent)
{
    try
    {
        size_t pos = prop.type.find(" ");
        std::string type = prop.type.substr(pos + 1);
        auto reflectionClass = (*Factory::m_Reflection).at(type.c_str());
        auto recursionProp = reflectionClass->getProperties();
        pComponent->SetAttribute("Type", prop.type.c_str());
        RecursionParent(type.c_str(), base, pComponent);
        for (auto rProp : recursionProp)
        {
            if (rProp.type == typeid(std::string).name())
            {
                // std::cout << rProp.name << std::endl;
                TX::XMLElement* pElem = doc.NewElement(rProp.name.c_str());
                pComponent->InsertEndChild(pElem);
                std::string value = *reinterpret_cast<std::string*>(base + rProp.offset);
                pElem->SetAttribute("Value", value.c_str());
                pElem->SetAttribute("Type", rProp.type.c_str());
            }
            else if (rProp.type.find("ptr") != std::string::npos)
                continue;
            else if (rProp.type.find("std::") != std::string::npos)
                continue;
            else if (rProp.type.find("*") != std::string::npos)
                continue;
            else
            {
                std::string value;
                // std::cout << rProp.name << std::endl;
                TX::XMLElement* pElem = doc.NewElement(rProp.name.c_str());
                pComponent->InsertEndChild(pElem);
                unsigned char* rProp_addr = base + rProp.offset;
                unsigned i = 0;
                if ((rProp.type.find("struct") != std::string::npos || rProp.type.find("class") != std::string::npos) && rProp.type.find("std::") == std::string::npos)
                {
                    try
                    {
                        RecursionStruct(rProp, rProp_addr, pElem);
                    }
                    catch (...)
                    {
                        // std::cout << rProp.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                        for (; i < rProp.size; ++i)
                        {
                            unsigned int val = static_cast<unsigned int>(*(rProp_addr + i));
                            value += (std::to_string(val) + ",");
                        }
                        pElem->SetAttribute("Value", value.c_str());
                        pElem->SetAttribute("Type", rProp.type.c_str());
                    }
                }
                else
                {
                    for (; i < rProp.size; ++i)
                    {
                        unsigned int val = static_cast<unsigned int>(*(rProp_addr + i));
                        value += (std::to_string(val) + ",");
                    }
                    pElem->SetAttribute("Value", value.c_str());
                    pElem->SetAttribute("Type", rProp.type.c_str());
                }
            }
        }
    }
    catch (...)
    {
        throw("Not In");
    }
}

void Serializer::SaveScene(const Scene* sn)
{
    const Scene* CurrentScene = sn;
    std::cout << "Saving Scene : " << CurrentScene->GetName() << std::endl;
    // Saving Scene
    TX::XMLNode* pRoot = doc.NewElement(sn->GetName().c_str());
    doc.InsertEndChild(pRoot);
    // for every layer
    for (Layer* ly : sn->GetLayers())
    {
        TX::XMLElement* pLayer = doc.NewElement(ly->GetName().c_str());
        pRoot->InsertEndChild(pLayer);
        for (GameObject* go : ly->GetObjectsList())
        {
            if (go->GetName() == "EditorCamera") continue;
            TX::XMLElement* pElement = doc.NewElement(go->GetName().c_str());
            pLayer->InsertEndChild(pElement);
            pElement->SetAttribute("Archetype", go->GetArchetype().c_str());
            pElement->SetAttribute("Tag", go->GetTag().c_str());
            pElement->SetAttribute("Active", go->GetActive());
            pElement->SetAttribute("IsChildren", go->GetIsChildren());
            pElement->SetAttribute("ParentObj", go->GetParentObject());
            pElement->SetAttribute("Id", go->GetID());
            auto childrens = go->GetChildrenObjects();
            std::string childrenData;
            for (auto& child : childrens)
                childrenData += (std::to_string(child) + ",");
            pElement->SetAttribute("Childrens", childrenData.c_str());
            // Storing all the children
            for (IComponent* comp : go->GetComponentList())
            {
                TX::XMLElement* pComponent = doc.NewElement(comp->GetName().c_str());
                pElement->InsertEndChild(pComponent);
                if (comp->GetName() == "BoxParticleEmitter" || comp->GetName() == "CircleParticleEmitter")
                {
                    auto attractors = dynamic_cast<ParticleEmitter*>(comp)->m_Attractors;
                    std::string attract;
                    for (auto& a : attractors)
                        attract += (std::to_string(a) + ",");
                    pComponent->SetAttribute("m_attractors", attract.c_str());
                }
                std::vector<registration::variant> props;
                try
                {
                    props = (*Factory::m_Reflection).at(comp->GetName().c_str())->getProperties();
                }
                catch (...)
                {
                    // std::cout << comp->GetName() << " not in Reflection Registration" << std::endl;
                    continue;
                }
                unsigned char* base = reinterpret_cast<unsigned char*>(comp);
                // Get the list of parents here, may do a reccursion call for inheriting and inheriting
                RecursionParent(comp->GetName().c_str(), base, pComponent);

                for (auto prop : props)
                {
                    if (prop.type == typeid(std::string).name())
                    {
                        // std::cout << prop.name << std::endl;
                        TX::XMLElement* pElem = doc.NewElement(prop.name.c_str());
                        pComponent->InsertEndChild(pElem);
                        std::string value = *reinterpret_cast<std::string*>(base + prop.offset);
                        pElem->SetAttribute("Value", value.c_str());
                        pElem->SetAttribute("Type", prop.type.c_str());
                    }
                    else if (prop.type.find("ptr") != std::string::npos)
                        continue;
                    else if (prop.type.find("std::") != std::string::npos)
                        continue;
                    else if (prop.type.find("*") != std::string::npos)
                        continue;
                    else
                    {
                        std::string value;
                        // std::cout << prop.name << std::endl;
                        TX::XMLElement* pElem = doc.NewElement(prop.name.c_str());
                        pComponent->InsertEndChild(pElem);
                        unsigned char* prop_addr = base + prop.offset;
                        unsigned i = 0;
                        if ((prop.type.find("struct") != std::string::npos || prop.type.find("class") != std::string::npos) && prop.type.find("std::") == std::string::npos)
                        {
                            try
                            {
                                RecursionStruct(prop, prop_addr, pElem);
                            }
                            catch (...)
                            {
                                // std::cout << prop.name << " is a struct/class which is not registered in the Reflection." << std::endl;
                                for (; i < prop.size; ++i)
                                {
                                    unsigned int val = static_cast<unsigned int>(*(prop_addr + i));
                                    value += (std::to_string(val) + ",");
                                }
                                pElem->SetAttribute("Value", value.c_str());
                                pElem->SetAttribute("Type", prop.type.c_str());
                            }
                        }
                        else
                        {
                            for (; i < prop.size; ++i)
                            {
                                unsigned int val = static_cast<unsigned int>(*(prop_addr + i));
                                value += (std::to_string(val) + ",");
                            }
                            pElem->SetAttribute("Value", value.c_str());
                            pElem->SetAttribute("Type", prop.type.c_str());
                        }
                    }
                }
            }
        }
    }
    TX::XMLError result = doc.SaveFile(filename.c_str());
    if (result != TX::XMLError::XML_SUCCESS) throw("ERROR");  // Replace with proper throw and assert
    std::cout << "Saving " << filename << " completed!" << std::endl;
}

void Serializer::RecursionLoadParent(TX::XMLElement* attribute, unsigned char* base, const char* comp)
{
    try
    {
        auto cParents = (*Factory::m_Reflection).at(comp)->getParents();
        for (auto cParent : cParents)
        {
            auto pProperties = cParent.get_properties();
            // not std::string
            std::string pType = attribute->Attribute("Type");
            if (pType != typeid(std::string).name())
            {
                auto pProperty = std::find(pProperties.begin(), pProperties.end(), attribute->Name());
                if (pProperty == pProperties.end())
                    RecursionLoadParent(attribute, base, cParent.key.c_str());
                else if (!attribute->NoChildren())
                {
                    if (pType == pProperty->type)
                    {
                        // std::cout << "parent has a struct" << std::endl;
                        unsigned char* attributePtr = base + pProperty->offset;
                        RecursionLoadStruct(attribute, attributePtr);
                    }
                }
                else if (pType == pProperty->type)
                {
                    // std::cout << attribute->Attribute("Type") << std::endl;
                    std::string attributeValue = attribute->Attribute("Value");
                    // std::cout << attributeValue << std::endl;
                    unsigned char* attributePtr = base + pProperty->offset;
                    char* attributeString = const_cast<char*>(attributeValue.c_str());
                    char* attributeToken = std::strtok(attributeString, ",");
                    unsigned i = 0;
                    for (; i < pProperty->size; ++i)
                    {
                        if (!attributeToken) break;
                        unsigned int value = std::stoi(attributeToken);
                        *(attributePtr + i) = static_cast<unsigned char>(value);
                        attributeToken = std::strtok(NULL, ",");
                    }
                }
            }
            // is std::string
            else
            {
                auto pProperty = std::find(pProperties.begin(), pProperties.end(), attribute->Name());
                if (pProperty == pProperties.end())
                    RecursionLoadParent(attribute, base, cParent.key.c_str());
                else if (pType == pProperty->type)
                {
                    // std::cout << attribute->Attribute("Type") << std::endl;
                    std::string& attribute_str = *reinterpret_cast<std::string*>(base + pProperty->offset);
                    std::string attributeValue{attribute->Attribute("Value")};
                    attribute_str = attributeValue;
                    // std::cout << attribute_str << std::endl;
                }
            }
        }
    }
    catch (...)
    {
        return;
    }
}

void Serializer::RecursionLoadStruct(TX::XMLElement* attribute, unsigned char* base)
{
    std::string aType = attribute->Attribute("Type");
    size_t pos = aType.find(" ");
    std::string type = aType.substr(pos + 1);
    auto sProperties = (*Factory::m_Reflection).at(type)->getProperties();
    TX::XMLElement* sAttribute = attribute->FirstChildElement();
    for (; sAttribute; sAttribute = sAttribute->NextSiblingElement())
    {
        std::string sType = sAttribute->Attribute("Type");
        if (sType != typeid(std::string).name())
        {
            auto sProperty = std::find(sProperties.begin(), sProperties.end(), sAttribute->Name());
            if (sProperty == sProperties.end())
                RecursionLoadParent(sAttribute, base, attribute->Value());
            else if (!sAttribute->NoChildren())
            {
                if (sType == sProperty->type)
                {
                    // std::cout << "struct has a struct" << std::endl;
                    unsigned char* attributePtr = base + sProperty->offset;
                    RecursionLoadStruct(sAttribute, attributePtr);
                }
            }
            else if (sType == sProperty->type)
            {
                // std::cout << sAttribute->Attribute("Type") << std::endl;
                std::string attributeValue = sAttribute->Attribute("Value");
                // std::cout << attributeValue << std::endl;
                unsigned char* attributePtr = base + sProperty->offset;
                char* attributeString = const_cast<char*>(attributeValue.c_str());
                char* attributeToken = std::strtok(attributeString, ",");
                unsigned i = 0;
                for (; i < sProperty->size; ++i)
                {
                    if (!attributeToken) break;
                    unsigned int value = std::stoi(attributeToken);
                    *(attributePtr + i) = static_cast<unsigned char>(value);
                    attributeToken = std::strtok(NULL, ",");
                }
            }
        }
        // is std::string
        else
        {
            auto sProperty = std::find(sProperties.begin(), sProperties.end(), sAttribute->Name());
            if (sProperty == sProperties.end())
                RecursionLoadParent(sAttribute, base, type.c_str());
            else if (sType == sProperty->type)
            {
                // std::cout << sAttribute->Attribute("Type") << std::endl;
                std::string& attribute = *reinterpret_cast<std::string*>(base + sProperty->offset);
                std::string attributeValue{sAttribute->Attribute("Value")};
                attribute = attributeValue;
                // std::cout << attribute << std::endl;
            }
        }
    }
}

void Serializer::LoadScene()
{
    // when we load, we check if the scene if offset matches, if matches we can copy the values over string to int, then cast to unsigned char
    // mental note, try the saving and loading of std library classes, the layout I am not very sure

    TX::XMLDocument file;
    if (file.LoadFile(filename.c_str()) == TX::XMLError::XML_SUCCESS)
    {
        std::cout << "Loading : " << filename << std::endl;

        TX::XMLNode* root = file.FirstChild();
        std::string value(root->Value());
        Application::Instance().NewScene(value.c_str());

        // For each Layer
        TX::XMLElement* pLayer = root->FirstChildElement();
        for (; pLayer; pLayer = pLayer->NextSiblingElement())
        {
            // std::cout << pLayer->Value() << std::endl;
            Layer* Ly = Application::Instance().GetCurrentScene()->CreateLayerWithoutCamera(pLayer->Value());
            TX::XMLElement* pGo = pLayer->FirstChildElement();
            // For each GameObject
            for (; pGo; pGo = pGo->NextSiblingElement())
            {
                // std::cout << pGo->Value() << std::endl;
                GameObject* Go = nullptr;
                if (pGo->Attribute("Id"))
                {
                    unsigned id = pGo->UnsignedAttribute("Id");
                    Go = Ly->CreateObjectWithId(pGo->Value(), id);
                }
                else
                    Go = Ly->CreateObject(pGo->Value());
                if (pGo->Attribute("Archetype")) Go->SetArchetype(pGo->Attribute("Archetype"));
                if (pGo->Attribute("Tag")) Go->SetTag(pGo->Attribute("Tag"));
                if (pGo->Attribute("Active")) Go->SetActive(pGo->BoolAttribute("Active"));
                if (pGo->Attribute("IsChildren")) Go->SetIsChildren(pGo->BoolAttribute("IsChildren"));
                if (pGo->Attribute("ParentObj")) Go->SetParentObject(pGo->UnsignedAttribute("ParentObj"));
                // Reload all the childrens
                if (pGo->Attribute("Childrens"))
                {
                    std::string childrens = pGo->Attribute("Childrens");
                    char* children = const_cast<char*>(childrens.c_str());
                    char* child = std::strtok(children, ",");
                    std::list<unsigned> childrenData;
                    while (child)
                    {
                        childrenData.push_back(std::stoi(child));
                        child = std::strtok(NULL, ",");
                    }
                    Go->SetChildrenObjects(childrenData);
                }
                TX::XMLElement* pComp = pGo->FirstChildElement();
                // For each Component
                for (; pComp; pComp = pComp->NextSiblingElement())
                {
                    // Needs a recursion to check all the parent values
                    // std::cout << pComp->Value() << std::endl;
                    // Can put Factory::m_Factories.at(pComp->Value()) here then try and catch
                    try
                    {
                        auto tmp = (*Factory::m_Reflection).at(pComp->Value());
                    }
                    catch (...)
                    {
                        continue;
                    }

                    unsigned char* base = reinterpret_cast<unsigned char*>((*Factory::m_Factories)[pComp->Value()]->create(Go));
                    if (pComp->Value() == std::string("BoxParticleEmitter") || pComp->Value() == std::string("CircleParticleEmitter"))
                    {
                        auto& attractors = dynamic_cast<ParticleEmitter*>(reinterpret_cast<IComponent*>(base))->m_Attractors;
                        if (pComp->Attribute("m_attractors"))
                        {
                            std::string attract = pComp->Attribute("m_attractors");
                            char* attrac = const_cast<char*>(attract.c_str());
                            char* a = std::strtok(attrac, ",");
                            while (a)
                            {
                                attractors.push_back(std::stoi(a));
                                a = std::strtok(NULL, ",");
                            }
                        }
                    }
                    auto cProperties = (*Factory::m_Reflection).at(pComp->Value())->getProperties();
                    // TODO: Change the Loading Scene according to the save scene, use hasChildren, if has children then take the type and
                    //       Recursion and so on and so forth
                    TX::XMLElement* pAttribute = pComp->FirstChildElement();
                    for (; pAttribute; pAttribute = pAttribute->NextSiblingElement())
                    {
                        // not std::string
                        std::string pType = pAttribute->Attribute("Type");
                        if (pType != typeid(std::string).name())
                        {
                            auto cProperty = std::find(cProperties.begin(), cProperties.end(), pAttribute->Name());
                            if (cProperty == cProperties.end())
                                RecursionLoadParent(pAttribute, base, pComp->Value());
                            else if (!pAttribute->NoChildren())
                            {
                                if (pType == cProperty->type)
                                {
                                    // std::cout << "base has a struct" << std::endl;
                                    unsigned char* attributePtr = base + cProperty->offset;
                                    RecursionLoadStruct(pAttribute, attributePtr);
                                }
                            }
                            else if (pType == cProperty->type)
                            {
                                // std::cout << pAttribute->Attribute("Type") << std::endl;
                                std::string attributeValue = pAttribute->Attribute("Value");
                                // std::cout << attributeValue << std::endl;
                                unsigned char* attributePtr = base + cProperty->offset;
                                char* attributeString = const_cast<char*>(attributeValue.c_str());
                                char* attributeToken = std::strtok(attributeString, ",");
                                unsigned i = 0;
                                for (; i < cProperty->size; ++i)
                                {
                                    if (!attributeToken) break;
                                    unsigned int value = std::stoi(attributeToken);
                                    *(attributePtr + i) = static_cast<unsigned char>(value);
                                    attributeToken = std::strtok(NULL, ",");
                                }
                            }
                        }
                        // is std::string
                        else
                        {
                            auto cProperty = std::find(cProperties.begin(), cProperties.end(), pAttribute->Name());
                            if (cProperty == cProperties.end())
                                RecursionLoadParent(pAttribute, base, pComp->Value());
                            else if (pType == cProperty->type)
                            {
                                // std::cout << pAttribute->Attribute("Type") << std::endl;
                                std::string& attribute = *reinterpret_cast<std::string*>(base + cProperty->offset);
                                std::string attributeValue{pAttribute->Attribute("Value")};
                                attribute = attributeValue;
                                // std::cout << attribute << std::endl;
                            }
                        }
                    }
                }
            }

            for (GameObject* go : Ly->GetObjectsList())
            {
                go->UpdateComponents();
            }
        }
        AISystem::Instance().Init();
        if (Application::InstancePtr())
        {
            for (auto ly : Application::Instance().GetCurrentScene()->GetLayers())
                for (auto go : ly->GetObjectsList())
                    for (auto c : go->GetLuaScripts())
                        c->InitScript();
        }
        std::cout << filename << " Loaded!" << std::endl;
    }
    else
        throw("File Cannot Be Loaded");
}
