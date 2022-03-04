#pragma once
#include "CorePrerequisites.h"
#define GetInstance(name) name::Instance()

// Singleton class wrapper
template <typename System> class ISystem
{
public:
    static System& Instance()
    {
        p_assert(IsInitialized());
        return *_Instance();
    }

    static System* InstancePtr()
    {
        // p_assert(IsInitialized());
        return _Instance();
    }

    template <typename... Args> static void Initialize(Args... args)
    {
        p_assert(!IsInitialized());
        IsInitializedInternally() = true;
        _Instance() = new System(args...);
        IsInitialized() = true;
        IsInitializedInternally() = false;
    }

    static void Shutdown()
    {
        p_assert(IsInitialized() && !IsShutdown());
        delete _Instance();
        IsShutdown() = true;
    }

protected:
    // Default constructor & destructor
    ISystem() { p_assert(IsInitializedInternally()); }
    virtual ~ISystem() = default;

    // Prevent remove copy construct/assignment
    ISystem(ISystem&&) = delete;
    ISystem(ISystem const&) = delete;
    ISystem& operator=(ISystem&&) = delete;
    ISystem& operator=(ISystem const&) = delete;

    static System*& _Instance()
    {
        static System* _instance = nullptr;
        return _instance;
    }

    static bool& IsInitializedInternally()
    {
        static bool _isInitializedInternally = false;
        return _isInitializedInternally;
    }

    static bool& IsInitialized()
    {
        static bool _isInitialized = false;
        return _isInitialized;
    }

    static bool& IsShutdown()
    {
        static bool _isShutdown = false;
        return _isShutdown;
    }
};
