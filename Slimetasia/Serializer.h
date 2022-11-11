#pragma once
#include <Algorithm>
#include <cstring>
#include <string>

#include "External Libraries\tinyxml2\tinyxml2.h"
#include "Layer.h"
#include "Reflection.h"
#include "Scene.h"

class Serializer
{
    std::string filename;
    tinyxml2::XMLDocument doc;
    void RecursionParent(const char* name, unsigned char* base, tinyxml2::XMLElement* pComponent);
    void RecursionStruct(registration::variant prop, unsigned char* base, tinyxml2::XMLElement* pComponent);
    void RecursionLoadParent(tinyxml2::XMLElement* attribute, unsigned char* base, const char* comp);
    void RecursionLoadStruct(tinyxml2::XMLElement* attribute, unsigned char* base);

public:

    Serializer(std::string fn)
        : filename(fn)
        , doc {}
    {
    }

    void SaveScene(const Scene* sn);
    void LoadScene();
};
