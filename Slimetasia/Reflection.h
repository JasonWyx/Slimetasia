#pragma once
#include <cstdarg>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#include "Any.h"

class GameObject;

#define REFLECT_PARENT(A) self->parents.push_back(A::reflection);

#define REFLECT()                           \
public:                                     \
                                            \
    static registration::class_ reflection; \
    static void initReflection(registration::class_*);

#define REFLECT_VIRTUAL(A)                                                  \
    class A##Reflection : public ComponentReflection                        \
    {                                                                       \
    public:                                                                 \
                                                                            \
        A##Reflection()                                                     \
        {                                                                   \
            Factory::registerType(#A, this);                                \
        }                                                                   \
        virtual std::vector<registration::variant> getProperties() override \
        {                                                                   \
            return A::reflection.get_properties();                          \
        }                                                                   \
        virtual std::vector<registration::class_> getParents() override     \
        {                                                                   \
            return A::reflection.get_parents();                             \
        }                                                                   \
        virtual bool hasVtable() override                                   \
        {                                                                   \
            return A::reflection.Vtable;                                    \
        }                                                                   \
        virtual registration::class_ getType() override                     \
        {                                                                   \
            return A::reflection;                                           \
        }                                                                   \
    };                                                                      \
    static A##Reflection global_##A##Reflection;                            \
    registration::class_ A::reflection { A::initReflection };               \
    void A::initReflection(registration::class_* self)                      \
    {                                                                       \
        using T = A;                                                        \
        self->key = #A;

#define REFLECT_TEMPLATE(A, B)                                              \
    class A##B##Reflection : public ComponentReflection                     \
    {                                                                       \
    public:                                                                 \
                                                                            \
        A##B##Reflection()                                                  \
        {                                                                   \
            Factory::registerType(#A #B, this);                             \
        }                                                                   \
        virtual std::vector<registration::variant> getProperties() override \
        {                                                                   \
            return A<B>::reflection.get_properties();                       \
        }                                                                   \
        virtual std::vector<registration::class_> getParents() override     \
        {                                                                   \
            return A<B>::reflection.get_parents();                          \
        }                                                                   \
        virtual bool hasVtable() override                                   \
        {                                                                   \
            return A<B>::reflection.Vtable;                                 \
        }                                                                   \
        virtual registration::class_ getType() override                     \
        {                                                                   \
            return A<B>::reflection;                                        \
        }                                                                   \
    };                                                                      \
    class A##B##Factory : public ComponentFactory                           \
    {                                                                       \
    public:                                                                 \
                                                                            \
        A##B##Factory()                                                     \
        {                                                                   \
            Factory::registerType(#A #B, this);                             \
        }                                                                   \
        virtual void* create(GameObject* go) override                       \
        {                                                                   \
            return reinterpret_cast<void*>(go->AddComponent<A<B>>());       \
        }                                                                   \
        virtual void remove(GameObject* go) override                        \
        {                                                                   \
            go->RemoveComponent<A<B>>();                                    \
        }                                                                   \
        virtual void createIfDoesntExist(GameObject* go) override           \
        {                                                                   \
            go->AddIfDoesntExist<A<B>>();                                   \
        }                                                                   \
    };                                                                      \
    static A##B##Factory global_##A##B##Factory;                            \
    static A##B##Reflection global_##A##Reflection;                         \
    registration::class_ A<B>::reflection { A<B>::initReflection };         \
    void A<B>::initReflection(registration::class_* self)                   \
    {                                                                       \
        using T = A;                                                        \
        self->key = #A #B;                                                  \
        self->Vtable = std::is_polymorphic<T>::value;

#define REFLECT_INIT(A)                                                     \
    class A##Reflection : public ComponentReflection                        \
    {                                                                       \
    public:                                                                 \
                                                                            \
        A##Reflection()                                                     \
        {                                                                   \
            Factory::registerType(#A, this);                                \
        }                                                                   \
        virtual std::vector<registration::variant> getProperties() override \
        {                                                                   \
            return A::reflection.get_properties();                          \
        }                                                                   \
        virtual std::vector<registration::class_> getParents() override     \
        {                                                                   \
            return A::reflection.get_parents();                             \
        }                                                                   \
        virtual bool hasVtable() override                                   \
        {                                                                   \
            return A::reflection.Vtable;                                    \
        }                                                                   \
        virtual registration::class_ getType() override                     \
        {                                                                   \
            return A::reflection;                                           \
        }                                                                   \
    };                                                                      \
    class A##Factory : public ComponentFactory                              \
    {                                                                       \
    public:                                                                 \
                                                                            \
        A##Factory()                                                        \
        {                                                                   \
            Factory::registerType(#A, this);                                \
        }                                                                   \
        virtual void* create(GameObject* go) override                       \
        {                                                                   \
            return reinterpret_cast<void*>(go->AddComponent<A>());          \
        }                                                                   \
        virtual void remove(GameObject* go) override                        \
        {                                                                   \
            go->RemoveComponent<A>();                                       \
        }                                                                   \
        virtual void createIfDoesntExist(GameObject* go) override           \
        {                                                                   \
            go->AddIfDoesntExist<A>();                                      \
        }                                                                   \
    };                                                                      \
    static A##Factory global_##A##Factory;                                  \
    static A##Reflection global_##A##Reflection;                            \
    registration::class_ A::reflection { A::initReflection };               \
    void A::initReflection(registration::class_* self)                      \
    {                                                                       \
        using T = A;                                                        \
        self->key = #A;                                                     \
        self->Vtable = std::is_polymorphic<T>::value;

#define REFLECT_PROPERTY(N) self->properties.push_back(registration::variant(#N, typeid(N).name(), offsetof(T, N), sizeof(N)));

#define REFLECT_FUNCTION(F) self->functions.push_back(registration::method { #F, &T::F });

#define REFLECT_END() }

#define t_invoke(O, N, ...) O.N(__VA_ARGS__);

#define REFLECT_ENUM(A)                                                               \
    static_assert(std::is_enum<A>::value, "ERROR: " #A " is not a valid enum type!"); \
    class A##Enum : public EnumReflection                                             \
    {                                                                                 \
        std::vector<const char*> enums_;                                              \
                                                                                      \
    public:                                                                           \
                                                                                      \
        A##Enum()                                                                     \
        {                                                                             \
            Factory::registerType(typeid(A).name(), this);                            \
            for (int i = 0; i < e##A##_count; i++)                                    \
                enums_.push_back(A##Array[i]);                                        \
        }                                                                             \
        virtual std::vector<const char*> getEnums() override                          \
        {                                                                             \
            return enums_;                                                            \
        }                                                                             \
    };                                                                                \
    static A##Enum global_##A##Enum;

#define RESOURCE_INIT(A)                                                    \
    class A##Resource : public ComponentReflection                          \
    {                                                                       \
    public:                                                                 \
                                                                            \
        A##Resource()                                                       \
        {                                                                   \
            Factory::registerResource(#A, this);                            \
        }                                                                   \
        virtual std::vector<registration::variant> getProperties() override \
        {                                                                   \
            return A::reflection.get_properties();                          \
        }                                                                   \
        virtual std::vector<registration::class_> getParents() override     \
        {                                                                   \
            return A::reflection.get_parents();                             \
        }                                                                   \
        virtual bool hasVtable() override                                   \
        {                                                                   \
            return A::reflection.Vtable;                                    \
        }                                                                   \
        virtual registration::class_ getType() override                     \
        {                                                                   \
            return A::reflection;                                           \
        }                                                                   \
    };                                                                      \
    static A##Resource global_##A##Resource;                                \
    registration::class_ A::reflection { A::initReflection };               \
    void A::initReflection(registration::class_* self)                      \
    {                                                                       \
        using T = A;                                                        \
        self->key = #A;

#define RESOURCE_PROPERTY(N) self->properties.push_back(registration::variant(#N, typeid(N).name(), offsetof(T, N), sizeof(N)));

#define RESOURCE_END() }

namespace registration
{
    // assuming all chars are 1 byte in everywhere
    struct property
    {
        std::string name;
        std::string type;
        unsigned offset;
        unsigned size;
        void* base;

        property(std::string n, std::string t, unsigned o, unsigned s, void* b)
            : name(n)
            , type(t)
            , offset(o)
            , size(s)
            , base(b)
        {
        }

        template <typename T>
        void set_value(T val)
        {
            char* b = reinterpret_cast<char*>(base);
            b += offset;
            memcpy_s(reinterpret_cast<void*>(b), size, reinterpret_cast<void*>(&val), size);
        }

        int& to_int()
        {
            char* b = reinterpret_cast<char*>(base);
            b += size;
            return *reinterpret_cast<int*>(b);
        }
    };

    struct variant
    {
        std::string name;
        std::string type;
        unsigned offset;
        unsigned size;

        variant(std::string n, std::string t, unsigned o, unsigned s)
            : name(n)
            , type(t)
            , offset(o)
            , size(s)
        {
        }

        bool operator==(std::string rhs) { return name == rhs; }
        friend std::ostream& operator<<(std::ostream& os, const variant& prop);
    };

    struct method
    {
        std::string name;
        Any function;

        method(std::string n, Any f)
            : name(n)
            , function(f)
        {
        }

        bool operator==(std::string rhs) { return name == rhs; }

        template <typename T, typename... Args>
        void invoke(T& obj, Args... a)
        {
        }
    };

    struct Node
    {
        // may want to private it
        std::string key;
        std::vector<variant> properties;
        std::vector<method> functions;
        bool Vtable;

        Node(std::string name)
            : key(name)
            , properties()
            , functions()
            , Vtable(false)
        {
        }
    };

    struct class_ : public Node
    {
        std::vector<class_> parents;
        class_()
            : Node("NIL")
        {
        }

        class_(void (*init)(class_*))
            : Node("name")
        {
            init(this);
        }

        template <typename T>
        registration::property get_property(std::string n, T& obj)
        {
            std::vector<variant>::iterator pos = std::find(properties.begin(), properties.end(), n);
            if (pos == properties.end()) throw "Property does not exists";
            variant p = *pos;
            return registration::property(p.name, p.type, p.offset, p.size, reinterpret_cast<void*>(&obj));
        }

        registration::method get_function(std::string n)
        {
            std::vector<method>::iterator pos = std::find(functions.begin(), functions.end(), n);
            if (pos == functions.end()) throw "Function does not exists";
            method func = *pos;
            return func;
        }

        std::vector<variant> get_properties() { return properties; }

        std::vector<method> get_functions() { return functions; }
        std::vector<class_> get_parents() { return parents; }
    };

    template <typename T>
    auto GetType()
    {
        return T::reflection;
    }

    std::ostream& operator<<(std::ostream& os, const variant& prop);
}  // namespace registration