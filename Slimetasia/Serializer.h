#pragma once
#include <Algorithm>
#include <cstring>
#include <string>

#include "External Libraries\tinyxml2\tinyxml2.h"
#include "Layer.h"
#include "Reflection.h"
#include "Scene.h"

namespace TX = tinyxml2;

class Serializer
{
    std::string filename;
    TX::XMLDocument doc;
    void RecursionParent(const char* name, unsigned char* base, TX::XMLElement* pComponent);
    void RecursionStruct(registration::variant prop, unsigned char* base, TX::XMLElement* pComponent);
    void RecursionLoadParent(TX::XMLElement* attribute, unsigned char* base, const char* comp);
    void RecursionLoadStruct(TX::XMLElement* attribute, unsigned char* base);

public:
    Serializer(std::string fn)
        : filename(fn)
        , doc{}
    {
    }

    void SaveScene(const Scene* sn);
    void LoadScene();
};
