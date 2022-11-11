#pragma once
// ===========================================================================|
// INCLUDES                                                                   |
// ===========================================================================|
#include <map>
#include <memory>
#include <string>

#include "CorePrerequisites.h"

// ===========================================================================|
// HELPER                                                                     |
// ===========================================================================|
template <typename T> const char* GetTypeName();
#define DEFINE_TYPE_NAME(type, name) \
    template <> const char* GetTypeName<type>() { return name; }

// ===========================================================================|
// PLAYERPREF                                                                 |
// ===========================================================================|
class PlayerPref
{
    /// Classes -----------------------------------------------------------------
    struct TypeBase
    {
        std::string type_;
        virtual ~TypeBase(){};
    };

    template <typename t> struct Type : public TypeBase
    {
        t variable_;

        Type(t variable = t{})
            : variable_{variable} {};
    };

    /// Variables ---------------------------------------------------------------
    using table = std::unordered_map<std::string, std::shared_ptr<TypeBase>>;

    static std::unordered_map<std::string, table> variables_;
    static bool loadedFromXML_;
    static tinyxml2::XMLDocument playerPrefXML_;
    const static std::filesystem::path dataDoc_;

    /// Functions ---------------------------------------------------------------
public:
    PlayerPref();
    ~PlayerPref();

    template <typename t> static bool SaveVariable(std::string name, t value, std::string tableName = "Variables")
    {
        Deserialize();

        //// Prevent overwriting of types
        // auto variable = variables_.find(name);
        // if (variable != variables_.end())
        //{
        //  auto tmp = dynamic_cast<Type<t>*>(variable->second.get());
        //  if (tmp)
        //  {
        //    *tmp = value;
        //    return true;
        //  }
        //
        //  return false;
        //}

        variables_[tableName][name] = std::make_shared<Type<t>>(value);
        variables_[tableName][name]->type_ = GetTypeName<t>();

        return true;
    }

    template <typename t> static t GetVariable(std::string name, std::string tableName = "Variables")
    {
        Deserialize();

        auto table = variables_.find(tableName);
        if (table != variables_.end())
        {
            auto variable = table->second.find(name);
            if (variable != table->second.end())
            {
                Type<t>* vPtr = dynamic_cast<Type<t>*>(variable->second.get());
                if (vPtr) return vPtr->variable_;
            }
        }

        return t{};
    }

    static void DeleteAllVariables();
    static void DeleteAllVariablesInTable(std::string tableName);
    static void DeleteVariable(std::string name, std::string tableName = "Variables");
    static bool CheckExist(std::string name, std::string tableName = "Variables");

    static void Serialize();
    static void Deserialize();
};
