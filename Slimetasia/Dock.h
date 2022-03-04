#pragma once
#include <algorithm>
#include <list>
#include <queue>
#include <string>
#include <vector>

#include "External Libraries\imgui\imgui.h"
#include "External Libraries\tinyxml2\tinyxml2.h"

struct ImGuiWindow;

namespace TX = tinyxml2;

namespace ImGui
{
    enum Status
    {
        Docked = 0,
        Floating = 1
    };

    enum Slot
    {
        Top = 0,
        Bottom = 1,
        Left = 2,
        Right = 3,
        None = 4
    };

    struct DockSpace;

    struct Dock
    {
        std::string name;
        ImVec2 pos;
        ImVec2 size;
        bool* opened;
        DockSpace* parent;
        bool active;
        Status status;
        bool triggered_Status;
        Slot slot;

        void changeStatus();

        Dock()
            : name()
            , pos()
            , size()
            , opened()
            , parent()
            , active()
            , status(Docked)
            , triggered_Status()
            , slot(None)
        {
        }
    };

    struct DockSpace
    {
        std::vector<Dock*> top;
        std::vector<Dock*> btm;
        std::vector<Dock*> left;
        std::vector<Dock*> right;
        bool* opened;
        ImVec2 size;
        ImVec2 pos;
        ImVec2 topSize;
        ImVec2 btmSize;
        ImVec2 leftSize;
        ImVec2 rightSize;
        ImGuiWindow* root_window;
        bool drawRectFilled;
        Slot slot;
        float titleBarSz;
        ImVec2 diff;
        bool mousedown;
        bool once;
        std::string name;
        ImVec2 prevSize;
        bool lonce;
        bool ronce;
        bool lmousedown;
        bool rmousedown;
        // need name for referencing loading

        DockSpace()
            : top()
            , btm()
            , left()
            , right()
            , opened()
            , size()
            , pos()
            , topSize()
            , btmSize()
            , leftSize()
            , rightSize()
            , root_window()
            , drawRectFilled()
            , slot(None)
            , titleBarSz()
            , diff()
            , mousedown()
            , once()
            , name()
            , prevSize()
            , lonce()
            , ronce()
            , lmousedown()
            , rmousedown()
        {
            titleBarSz = ImGuiStyleVar_FramePadding * 2 + GetFontSize() + 15.f;
        }
        void Tabs(const ImVec2& sz, Slot slot);
        void TabList(Slot slot);
        void UpdateSizes();
    };

    struct DockContext
    {
        std::list<Dock> mDocks;
        std::list<DockSpace> mDockSpaces;
        DockContext();
        ~DockContext();
        char* m_currwdir;
    };

    static DockContext* context = nullptr;
    static std::queue<Dock*> currentDock;            // why i put stack, C++ is sequential so always will have one only
    static std::queue<DockSpace*> currentDockSpace;  // why i put stack, C++ is sequential so always will have one only

    void CreateDockContext();
    void DestroyDockContext();

    bool BeginDock(const char* name, bool* open = NULL, ImGuiWindowFlags flags = 0, const ImVec2& default_size = ImVec2(30, 30), const ImVec2& default_pos = ImVec2(0, 0));
    void EndDock();
    void InitDockSpace();
    void ChangeDockStatus();

    void DebugDockSpaces();
    bool IntersectionTest(Dock* d);
    Dock* GetCurrentDock();
    void SetDock(Dock* current);
    void UnDock(Dock* current);
    void BeginDockChild(const char* str_id, const ImVec2& size_arg = ImVec2(0, 0), bool border = false, ImGuiWindowFlags extra_flags = 0);
    void EndDockChild();
}  // namespace ImGui