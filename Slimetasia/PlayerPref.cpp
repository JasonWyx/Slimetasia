#include "PlayerPref.h"

#include "ResourceManager.h"

// ===========================================================================|
// VARIABLES NAMES                                                            |
// ===========================================================================|
DEFINE_TYPE_NAME(int, "Integer");
DEFINE_TYPE_NAME(float, "Float");
DEFINE_TYPE_NAME(bool, "Bool");
DEFINE_TYPE_NAME(Vector3, "Vector3");
DEFINE_TYPE_NAME(std::string, "String");
DEFINE_TYPE_NAME(std::vector<int>, "ArrayInteger");
DEFINE_TYPE_NAME(std::vector<float>, "ArrayFloat");
DEFINE_TYPE_NAME(std::vector<bool>, "ArrayBool");
DEFINE_TYPE_NAME(std::vector<std::string>, "ArrayString");
DEFINE_TYPE_NAME(std::vector<Vector3>, "ArrayVector3");

// ===========================================================================|
// STATIC VARIABLES                                                           |
// ===========================================================================|
std::unordered_map<std::string, PlayerPref::table> PlayerPref::variables_ = {};
bool PlayerPref::loadedFromXML_ = false;
tinyxml2::XMLDocument PlayerPref::playerPrefXML_;
const std::filesystem::path PlayerPref::dataDoc_ = "Resources/PlayerPref.xml";

// ===========================================================================|
// CONSTRUCTOR                                                                |
// ===========================================================================|
PlayerPref::PlayerPref()
{
    /// Load from xml
    Deserialize();
}

PlayerPref::~PlayerPref()
{
    /// Write into xml and clear XML
    Serialize();
    DeleteAllVariables();
}

// ===========================================================================|
// PLUBLIC FUNCTIONS                                                          |
// ===========================================================================|
void PlayerPref::DeleteAllVariables()
{
    variables_.clear();
    variables_.insert(std::make_pair<std::string, table>("Variables", table()));
}

void PlayerPref::DeleteAllVariablesInTable(std::string tableName)
{
    variables_.erase(tableName);
}

void PlayerPref::DeleteVariable(std::string name, std::string tableName)
{
    auto tmp = variables_.find(tableName);
    if (tmp != variables_.end()) tmp->second.erase(name);
}

bool PlayerPref::CheckExist(std::string name, std::string tableName)
{
    auto table = variables_.find(tableName);
    if (table != variables_.end())
    {
        auto entry = table->second.find(name);
        if (entry != table->second.end()) return true;
    }

    return false;
}

// ===========================================================================|
// SERIALIZE FUNCTIONS                                                        |
// ===========================================================================|
void PlayerPref::Serialize()
{
    tinyxml2::XMLElement* root = playerPrefXML_.FirstChildElement("PlayerPref");

    /// Clear all values
    root->DeleteChildren();

    /// Add all variables
    for (const auto& table : variables_)
    {
        /// Register the table
        tinyxml2::XMLElement* newTable = playerPrefXML_.NewElement("Table");
        newTable->SetAttribute("TableName", table.first.c_str());

        for (const auto& variable : table.second)
        {
            tinyxml2::XMLElement* newElem = playerPrefXML_.NewElement("Variable");
            const char* typeName = variable.second->type_.c_str();
            newElem->SetAttribute("Name", variable.first.c_str());
            unsigned i = 0;

            /// Single value
            if (strcmp("Integer", typeName) == 0)
                newElem->SetAttribute("Value", dynamic_cast<Type<int>*>(variable.second.get())->variable_);
            else if (strcmp("Float", typeName) == 0)
                newElem->SetAttribute("Value", dynamic_cast<Type<float>*>(variable.second.get())->variable_);
            else if (strcmp("Bool", typeName) == 0)
                newElem->SetAttribute("Value", dynamic_cast<Type<bool>*>(variable.second.get())->variable_);
            else if (strcmp("String", typeName) == 0)
                newElem->SetAttribute("Value", dynamic_cast<Type<std::string>*>(variable.second.get())->variable_.c_str());
            else if (strcmp("Vector3", typeName) == 0)
            {
                Vector3 elem = dynamic_cast<Type<Vector3>*>(variable.second.get())->variable_;
                // tinyxml2::XMLElement* entry = playerPrefXML_.NewElement("Value");
                newElem->SetAttribute("x", elem.x);
                newElem->SetAttribute("y", elem.y);
                newElem->SetAttribute("z", elem.z);

                // newElem->InsertEndChild(entry);
            }
            /// Array values
            else if (strcmp("ArrayInteger", typeName) == 0)
                for (const auto& elem : dynamic_cast<Type<std::vector<int>>*>(variable.second.get())->variable_)
                {
                    tinyxml2::XMLElement* entry = playerPrefXML_.NewElement(("Index" + std::to_string(i++)).c_str());
                    entry->SetAttribute("Value", elem);

                    newElem->InsertEndChild(entry);
                }
            else if (strcmp("ArrayFloat", typeName) == 0)
                for (const auto& elem : dynamic_cast<Type<std::vector<float>>*>(variable.second.get())->variable_)
                {
                    tinyxml2::XMLElement* entry = playerPrefXML_.NewElement(("Index" + std::to_string(i++)).c_str());
                    entry->SetAttribute("Value", elem);

                    newElem->InsertEndChild(entry);
                }
            else if (strcmp("ArrayBool", typeName) == 0)
                for (const auto& elem : dynamic_cast<Type<std::vector<bool>>*>(variable.second.get())->variable_)
                {
                    tinyxml2::XMLElement* entry = playerPrefXML_.NewElement(("Index" + std::to_string(i++)).c_str());
                    entry->SetAttribute("Value", elem);

                    newElem->InsertEndChild(entry);
                }
            else if (strcmp("ArrayString", typeName) == 0)
                for (const auto& elem : dynamic_cast<Type<std::vector<std::string>>*>(variable.second.get())->variable_)
                {
                    tinyxml2::XMLElement* entry = playerPrefXML_.NewElement(("Index" + std::to_string(i++)).c_str());
                    entry->SetAttribute("Value", elem.c_str());

                    newElem->InsertEndChild(entry);
                }
            else if (strcmp("ArrayVector3", typeName) == 0)
                for (const auto& elem : dynamic_cast<Type<std::vector<Vector3>>*>(variable.second.get())->variable_)
                {
                    tinyxml2::XMLElement* entry = playerPrefXML_.NewElement(("Index" + std::to_string(i++)).c_str());
                    entry->SetAttribute("x", elem.x);
                    entry->SetAttribute("y", elem.y);
                    entry->SetAttribute("z", elem.z);

                    newElem->InsertEndChild(entry);
                }

            newElem->SetAttribute("Type", typeName);
            newTable->InsertEndChild(newElem);
        }

        root->InsertEndChild(newTable);
    }

    /// Save to file
    playerPrefXML_.SaveFile(dataDoc_.string().c_str());
}

void PlayerPref::Deserialize()
{
    if (loadedFromXML_) return;
    loadedFromXML_ = true;
    variables_.insert(std::make_pair<std::string, table>("Variables", table()));

    /// Create XML if doesnt exist
    tinyxml2::XMLError error = playerPrefXML_.LoadFile(dataDoc_.string().c_str());
    if (error == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND)
    {
        /// Create new data file
        std::ofstream newFile(dataDoc_);
        newFile.close();

        /// Add root elements
        tinyxml2::XMLElement* newElem = playerPrefXML_.NewElement("PlayerPref");
        tinyxml2::XMLElement* newElem2 = playerPrefXML_.NewElement("Table");
        newElem2->SetAttribute("TableName", "Variables");

        playerPrefXML_.InsertFirstChild(newElem);
        newElem->InsertEndChild(newElem2);
        playerPrefXML_.SaveFile(dataDoc_.string().c_str());
    }
    /// Read from XML
    else
    {
        tinyxml2::XMLElement* root = playerPrefXML_.FirstChildElement("PlayerPref");
        for (tinyxml2::XMLElement* currTable = root->FirstChildElement(); currTable; currTable = currTable->NextSiblingElement())
        {
            const char* tableName;
            currTable->QueryStringAttribute("TableName", &tableName);
            for (tinyxml2::XMLElement* curr = currTable->FirstChildElement(); curr; curr = curr->NextSiblingElement())
            {
                const char* variableType;
                const char* variableName;
                curr->QueryStringAttribute("Type", &variableType);
                curr->QueryStringAttribute("Name", &variableName);

                /// Single value
                if (strcmp("Integer", variableType) == 0)
                {
                    int value;
                    curr->QueryIntAttribute("Value", &value);
                    SaveVariable<int>(variableName, value, tableName);
                }
                else if (strcmp("Float", variableType) == 0)
                {
                    float value;
                    curr->QueryFloatAttribute("Value", &value);
                    SaveVariable<float>(variableName, value, tableName);
                }
                else if (strcmp("Bool", variableType) == 0)
                {
                    bool value;
                    curr->QueryBoolAttribute("Value", &value);
                    SaveVariable<bool>(variableName, value, tableName);
                }
                else if (strcmp("String", variableType) == 0)
                {
                    const char* value;
                    curr->QueryStringAttribute("Value", &value);
                    SaveVariable<std::string>(variableName, value, tableName);
                }
                else if (strcmp("Vector3", variableType) == 0)
                {
                    Vector3 value;
                    curr->QueryFloatAttribute("x", &value.x);
                    curr->QueryFloatAttribute("y", &value.y);
                    curr->QueryFloatAttribute("z", &value.z);
                    SaveVariable<Vector3>(variableName, value, tableName);
                }

                /// Arrays
                else if (strcmp("ArrayInteger", variableType) == 0)
                {
                    std::vector<int> values;
                    for (tinyxml2::XMLElement* currElem = curr->FirstChildElement(); currElem; currElem = currElem->NextSiblingElement())
                    {
                        int value;
                        currElem->QueryIntAttribute("Value", &value);
                        values.push_back(value);
                    }
                    SaveVariable<std::vector<int>>(variableName, values, tableName);
                }
                else if (strcmp("ArrayFloat", variableType) == 0)
                {
                    std::vector<float> values;
                    for (tinyxml2::XMLElement* currElem = curr->FirstChildElement(); currElem; currElem = currElem->NextSiblingElement())
                    {
                        float value;
                        currElem->QueryFloatAttribute("Value", &value);
                        values.push_back(value);
                    }
                    SaveVariable<std::vector<float>>(variableName, values, tableName);
                }
                else if (strcmp("ArrayBool", variableType) == 0)
                {
                    std::vector<bool> values;
                    for (tinyxml2::XMLElement* currElem = curr->FirstChildElement(); currElem; currElem = currElem->NextSiblingElement())
                    {
                        bool value;
                        currElem->QueryBoolAttribute("Value", &value);
                        values.push_back(value);
                    }
                    SaveVariable<std::vector<bool>>(variableName, values, tableName);
                }
                else if (strcmp("ArrayString", variableType) == 0)
                {
                    std::vector<std::string> values;
                    for (tinyxml2::XMLElement* currElem = curr->FirstChildElement(); currElem; currElem = currElem->NextSiblingElement())
                    {
                        const char* value;
                        currElem->QueryStringAttribute("Value", &value);
                        values.push_back(std::string { value });
                    }
                    SaveVariable<std::vector<std::string>>(variableName, values, tableName);
                }
                else if (strcmp("ArrayVector3", variableType) == 0)
                {
                    std::vector<Vector3> values;
                    for (tinyxml2::XMLElement* currElem = curr->FirstChildElement(); currElem; currElem = currElem->NextSiblingElement())
                    {
                        Vector3 value;
                        currElem->QueryFloatAttribute("x", &value.x);
                        currElem->QueryFloatAttribute("y", &value.y);
                        currElem->QueryFloatAttribute("z", &value.z);
                        values.push_back(value);
                    }
                    SaveVariable<std::vector<Vector3>>(variableName, values, tableName);
                }
            }
        }
    }
}