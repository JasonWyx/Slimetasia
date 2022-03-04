#include "External Libraries\imgui\imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <iostream>

#include "CorePrerequisites.h"
#include "Dock.h"
#include "External Libraries\imgui\imgui_internal.h"

// TODO: Partion multiple docks area in one dockspace
// TODO: Can use the below the if not empty then create the child
// Now that we got the rectangle, we can make got four and check collision
// with them instead

void ImGui::CreateDockContext()
{
    context = new DockContext();
}

void ImGui::DestroyDockContext()
{
    if (context)
    {
        delete context;
        context = nullptr;
    }
}

bool ImGui::BeginDock(const char* name, bool* open, ImGuiWindowFlags flags, const ImVec2& default_size, const ImVec2& default_pos)
{
    auto start = context->mDocks.begin();
    while (start != context->mDocks.end())
    {
        if (start->name == name)
        {
            currentDock.push(&*start);
            if (start->status == Floating)
            {
                Begin(name, open, flags);
                return true;
            }
            else  // docked
            {
                // Search for parent here
                DockSpace* parent = start->parent;
                std::string name = parent->root_window->Name;
                if (start->slot == Top) name += "_Topworkspace";
                if (start->slot == Bottom) name += "_Btmworkspace";
                if (start->slot == Left) name += "_Leftworkspace";
                if (start->slot == Right) name += "_Rightworkspace";
                GImGui->CurrentWindow = parent->root_window;
                // ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
                if (start->slot == Left)
                {
                    float y = parent->size.y - parent->titleBarSz;
                    ImVec2 sz{parent->leftSize.x, y};
                    BeginChild(name.c_str(), sz, false, 0);
                }
                if (start->slot == Top)
                {
                    float x = parent->size.x - parent->leftSize.x - parent->rightSize.x;
                    ImVec2 sz{x, parent->topSize.y};
                    BeginChild(name.c_str(), sz, false, 0);
                }
                if (start->slot == Bottom)
                {
                    float x = parent->size.x - parent->leftSize.x - parent->rightSize.x;
                    ImVec2 sz{x, parent->btmSize.y};
                    BeginChild(name.c_str(), sz, false, 0);
                }
                if (start->slot == Right)
                {
                    float y = parent->size.y - parent->titleBarSz;
                    ImVec2 sz{parent->rightSize.x, y};
                    BeginChild(name.c_str(), sz, false, 0);
                    if (!parent->top.empty() || !parent->btm.empty()) Indent(15.f);
                }
                if (!start->active) GImGui->CurrentWindow->SkipItems = true;
                // context->mDockSpaces.back().child_window = GImGui->CurrentWindow;
                return true;
            }
        }
        ++start;
    }
    Dock dock;
    dock.name = name;
    dock.pos = default_pos;
    dock.size = default_size;
    dock.opened = open;
    dock.active = true;
    dock.status = Floating;
    context->mDocks.push_back(dock);
    currentDock.push(&context->mDocks.back());
    if (dock.status == Floating) Begin(name, open, flags);
    return true;
}

void ImGui::EndDock()
{
    if (currentDock.empty()) return;
    Dock* current = currentDock.front();
    currentDock.pop();
    if (!current->active) GImGui->CurrentWindow->SkipItems = false;
    if (current->pos.x != GImGui->CurrentWindow->Pos.x && current->pos.y != GImGui->CurrentWindow->Pos.y && current->status == Floating)
    {
        if (IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && IsMouseDown(0))
        {
            if (IntersectionTest(current))
            {
                current->triggered_Status = true;
                // std::cout << "In" << std::endl;
            }
            else
                current->triggered_Status = false;
        }
    }
    current->pos = GImGui->CurrentWindow->Pos;
    if (current->status == Floating)
        End();
    else
    {
        ImDrawList* draw_list = GetWindowDrawList();
        float line_height = GetTextLineHeightWithSpacing();
        ImU32 color = GetColorU32(ImGuiCol_Button);
        ImU32 color_hovered = GetColorU32(ImGuiCol_ButtonHovered);
        ImVec2 sz;
        ImVec2 mouse_pos = GetIO().MousePos;
        bool morethanone = false;
        switch (current->slot)
        {
            case Top: sz = current->parent->topSize; break;
            case Bottom: sz = ImVec2(0, 0); break;
            case Left: sz = current->parent->leftSize; break;
            case Right: sz = current->parent->rightSize; break;
        }
        // this whole chunk is for the top one
        if (current->slot == Top)
        {
            ImVec2 cp;
            if (sz.y == 0.f)
            {
                sz.y = current->parent->size.y - 10.f;
                cp = ImVec2(current->parent->pos.x, current->parent->pos.y + sz.y);
            }
            else
            {
                float offset = GetTextLineHeightWithSpacing();
                // if (current->slot == Bottom) offset *= 1.6f;
                cp = ImVec2(current->parent->pos.x + current->parent->leftSize.x, current->parent->pos.y + sz.y + offset);
                morethanone = true;
            }
            float x = current->parent->size.x - current->parent->leftSize.x - current->parent->rightSize.x;
            ImRect r = ImRect(cp, cp + ImVec2(x, 5.f));
            bool hovered = r.Contains(mouse_pos);
            if (morethanone && !current->parent->once)
            {
                current->parent->once = !current->parent->once;
                if (IsMouseClicked(0) && hovered && !current->parent->mousedown) current->parent->mousedown = true;
                if (current->parent->mousedown)
                {
                    float y = GetIO().MouseDelta.y;
                    if (current->parent->topSize.y + y > 25.f && current->parent->btmSize.y - y > 0.f)
                    {
                        current->parent->topSize.y += y;
                        current->parent->btmSize.y -= y;
                    }
                }
                if (!IsMouseDown(0) && current->parent->mousedown)
                {
                    current->parent->mousedown = false;
                }
            }
            if (!current->parent->btm.empty()) draw_list->AddRectFilled(r.Min, r.Max, hovered ? color_hovered : color);
        }
        else if (current->slot == Left)
        {
            if (!current->parent->top.empty() || !current->parent->btm.empty())
            {
                ImVec2 cp;
                cp.x = current->parent->pos.x + current->parent->leftSize.x - 5.f;
                cp.y = current->parent->pos.y - GetTextLineHeightWithSpacing() + current->parent->titleBarSz * 0.75f;
                ImRect r = ImRect(cp, cp + ImVec2(5.f, current->parent->size.y - current->parent->titleBarSz * 1.5f));
                bool hovered = r.Contains(mouse_pos);
                if (!current->parent->lonce)
                {
                    current->parent->lonce = true;
                    if (IsMouseClicked(0) && hovered && !current->parent->lmousedown) current->parent->lmousedown = true;
                    if (current->parent->lmousedown)
                    {
                        float x = GetIO().MouseDelta.x;
                        if (current->parent->leftSize.x + x > 15.f && current->parent->leftSize.x + x < current->parent->size.x - 5.f)
                        {
                            if ((!current->parent->right.empty()) && current->parent->leftSize.x + x < current->parent->size.x - current->parent->rightSize.x - 25.f)
                                current->parent->leftSize.x += x;
                            else if (current->parent->right.empty())
                                current->parent->leftSize.x += x;
                        }
                    }
                    if (!IsMouseDown(0) && current->parent->lmousedown) current->parent->lmousedown = false;
                }
                draw_list->AddRectFilled(r.Min, r.Max, hovered ? color_hovered : color);
            }
            else if (!current->parent->right.empty())
            {
                ImVec2 cp;
                cp.x = current->parent->pos.x + current->parent->leftSize.x - 5.f;
                cp.y = current->parent->pos.y - GetTextLineHeightWithSpacing() + current->parent->titleBarSz * 0.75f;
                ImRect r = ImRect(cp, cp + ImVec2(5.f, current->parent->size.y - current->parent->titleBarSz * 1.5f));
                bool hovered = r.Contains(mouse_pos);
                if (!current->parent->lonce)
                {
                    current->parent->lonce = true;
                    if (IsMouseClicked(0) && hovered && !current->parent->lmousedown) current->parent->lmousedown = true;
                    if (current->parent->lmousedown)
                    {
                        float x = GetIO().MouseDelta.x;
                        if (current->parent->rightSize.x - x > 5.f && current->parent->leftSize.x + x > 15.f)
                        {
                            current->parent->leftSize.x += x;
                            current->parent->rightSize.x -= x;
                        }
                    }
                    if (!IsMouseDown(0) && current->parent->lmousedown) current->parent->lmousedown = false;
                }
                draw_list->AddRectFilled(r.Min, r.Max, hovered ? color_hovered : color);
            }
        }
        else if (current->slot == Right)
        {
            if (!current->parent->top.empty() || !current->parent->btm.empty())
            {
                ImVec2 cp;
                cp.x = current->parent->pos.x + current->parent->size.x - current->parent->rightSize.x + ImGuiStyleVar_FramePadding * 0.1f;
                if (!current->parent->left.empty()) cp.x += ImGuiStyleVar_FramePadding * 1.75f;
                cp.y = current->parent->pos.y - GetTextLineHeightWithSpacing() + current->parent->titleBarSz * 0.75f;
                ImRect r = ImRect(cp, cp + ImVec2(5.f, current->parent->size.y - current->parent->titleBarSz * 1.5f));
                bool hovered = r.Contains(mouse_pos);
                if (!current->parent->ronce)
                {
                    current->parent->ronce = true;
                    if (IsMouseClicked(0) && hovered && !current->parent->rmousedown) current->parent->rmousedown = true;
                    if (current->parent->rmousedown)
                    {
                        float x = GetIO().MouseDelta.x;
                        if (current->parent->rightSize.x - x > 35.f && current->parent->rightSize.x - x + 25.f < current->parent->size.x - current->parent->leftSize.x)
                            current->parent->rightSize.x -= x;
                    }
                    if (!IsMouseDown(0) && current->parent->rmousedown) current->parent->rmousedown = false;
                }
                draw_list->AddRectFilled(r.Min, r.Max, hovered ? color_hovered : color);
                Unindent(15.f);
            }
        }
        EndChild();
    }
    if (current->triggered_Status && !IsMouseDown(0))
    {
        if (current->status == Floating)
        {
            current->parent->drawRectFilled = false;
            if (current->parent->slot == Top)
            {
                auto& vec = current->parent->top;
                vec.push_back(current);
                current->active = true;
                for (int i = (int)vec.size() - 2; i >= 0; --i)
                    vec[i]->active = false;
            }
            if (current->parent->slot == Bottom)
            {
                auto& vec = current->parent->btm;
                vec.push_back(current);
                current->active = true;
                for (int i = (int)vec.size() - 2; i >= 0; --i)
                    vec[i]->active = false;
            }
            if (current->parent->slot == Left)
            {
                auto& vec = current->parent->left;
                vec.push_back(current);
                current->active = true;
                for (int i = (int)vec.size() - 2; i >= 0; --i)
                    vec[i]->active = false;
            }
            if (current->parent->slot == Right)
            {
                auto& vec = current->parent->right;
                vec.push_back(current);
                current->active = true;
                for (int i = (int)vec.size() - 2; i >= 0; --i)
                    vec[i]->active = false;
            }
            current->status = Docked;
            current->slot = current->parent->slot;
            current->parent->slot = None;
            if (current->slot == None)
            {
                current->status = Floating;
                current->parent = nullptr;
            }
            SetDock(current);
        }
        else
        {
            if (current->slot == Top)
            {
                auto it = current->parent->top.begin();
                while (it != current->parent->top.end())
                {
                    if (*it == current) break;
                    ++it;
                }
                if (it == current->parent->top.begin())
                {
                    if (current->parent->top.size() != 1) (*(it + 1))->active = true;
                }
                else
                    (*(it - 1))->active = true;
                current->parent->top.erase(it);
                UnDock(current);
                current->parent = nullptr;
            }
            if (current->slot == Bottom)
            {
                auto it = current->parent->btm.begin();
                while (it != current->parent->btm.end())
                {
                    if (*it == current) break;
                    ++it;
                }
                if (it == current->parent->btm.begin())
                {
                    if (current->parent->btm.size() != 1) (*(it + 1))->active = true;
                }
                else
                    (*(it - 1))->active = true;
                current->parent->btm.erase(it);
                UnDock(current);
                current->parent = nullptr;
            }
            if (current->slot == Left)
            {
                auto it = current->parent->left.begin();
                while (it != current->parent->left.end())
                {
                    if (*it == current) break;
                    ++it;
                }
                if (it == current->parent->left.begin())
                {
                    if (current->parent->left.size() != 1) (*(it + 1))->active = true;
                }
                else
                    (*(it - 1))->active = true;
                current->parent->left.erase(it);
                UnDock(current);
                current->parent = nullptr;
            }
            if (current->slot == Right)
            {
                auto it = current->parent->right.begin();
                while (it != current->parent->right.end())
                {
                    if (*it == current) break;
                    ++it;
                }
                if (it == current->parent->right.begin())
                {
                    if (current->parent->right.size() != 1) (*(it + 1))->active = true;
                }
                else
                    (*(it - 1))->active = true;
                current->parent->right.erase(it);
                UnDock(current);
                current->parent = nullptr;
            }
            current->status = Floating;
            current->slot = None;
        }
        current->triggered_Status = false;
    }
}

void ImGui::InitDockSpace()
{
    DockSpace* currSpace = nullptr;
    ImGuiWindow* win = GImGui->CurrentWindow;
    // win->Flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove;
    auto start = context->mDockSpaces.begin();
    while (start != context->mDockSpaces.end())
    {
        if (start->name == win->Name)
        {
            ImDrawList* draw_list = GetWindowDrawList();
            ImU32 color_hovered = GetColorU32(ImGuiCol_ButtonHovered, 0.5f);
            start->pos = win->Pos;
            start->diff = win->Size - start->size;
            start->size = win->Size;
            start->once = false;
            start->lonce = false;
            start->ronce = false;
            start->name = win->Name;
            start->root_window = win;
            if (!IsMouseDown(0)) start->drawRectFilled = false;
            if (start->drawRectFilled)
            {
                // draw_list->AddRectFilled(start->pos, start->pos + start->size, color_hovered, 16);
                // top
                ImVec2 halfRectsz = win->Size / 9.f;
                ImVec2 center = win->Pos + ImVec2((win->Size / 2.f).x, 30.f);

                draw_list->AddRectFilled(center - ImVec2(halfRectsz.x, 0.f), center + ImVec2(halfRectsz.x, halfRectsz.y), color_hovered, 0);
                if (IsMouseHoveringRect(center - ImVec2(halfRectsz.x, 0.f), center + ImVec2(halfRectsz.x, halfRectsz.y))) start->slot = Top;
                // btm
                center = win->Pos + win->Size - ImVec2((win->Size / 2.f).x, (win->Size / 5.f).y);
                draw_list->AddRectFilled(center - ImVec2(halfRectsz.x, 0.f), center + ImVec2(halfRectsz.x, halfRectsz.y), color_hovered, 0);
                if (IsMouseHoveringRect(center - ImVec2(halfRectsz.x, 0.f), center + ImVec2(halfRectsz.x, halfRectsz.y))) start->slot = Bottom;
                // left
                center = win->Pos + ImVec2(30.f, (win->Size / 2.f).y);
                draw_list->AddRectFilled(center - ImVec2(0.f, halfRectsz.y), center + ImVec2(halfRectsz.x, halfRectsz.y), color_hovered, 0);
                if (IsMouseHoveringRect(center - ImVec2(0.f, halfRectsz.y), center + ImVec2(halfRectsz.x, halfRectsz.y))) start->slot = Left;
                // right
                center = win->Pos + win->Size - ImVec2((win->Size / 5.f).x, (win->Size / 2.f).y);
                draw_list->AddRectFilled(center - ImVec2(0.f, halfRectsz.y), center + ImVec2(halfRectsz.x, halfRectsz.y), color_hovered, 0);
                if (IsMouseHoveringRect(center - ImVec2(0.f, halfRectsz.y), center + ImVec2(halfRectsz.x, halfRectsz.y))) start->slot = Right;
            }

            currSpace = &*start;
            // start->drawRectFilled = false;
            break;
        }
        ++start;
    }

    if (start == context->mDockSpaces.end())
    {
        DockSpace tmp;
        tmp.size = win->Size;
        tmp.pos = win->Pos;
        tmp.root_window = win;
        tmp.diff = ImVec2(0, 0);
        tmp.once = false;
        tmp.lonce = false;
        tmp.ronce = false;
        tmp.name = win->Name;
        tmp.prevSize = win->Size;
        context->mDockSpaces.push_back(tmp);
        currSpace = &context->mDockSpaces.back();
    }
    currSpace->UpdateSizes();
    if (currSpace && !currSpace->left.empty())
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        std::string name = win->Name;
        name += "_Leftworkspace";
        float y = currSpace->size.y - currSpace->titleBarSz;
        ImVec2 sz{currSpace->leftSize.x, y};
        BeginChild(name.c_str(), sz, false, 0);
        currSpace->Tabs(win->Size, Left);
        EndChild();
        SameLine();
    }
    if (!currSpace->top.empty())
    {
        BeginGroup();
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        std::string name = win->Name;
        name += "_Topworkspace";
        float x = currSpace->size.x - currSpace->leftSize.x - currSpace->rightSize.x - ImGuiStyleVar_FramePadding * 2;
        ImVec2 sz{x, currSpace->topSize.y};
        BeginChild(name.c_str(), sz, false, 0);
        currSpace->Tabs(win->Size, Top);
        // Text("Here");
        EndChild();
        if (currSpace->btm.empty())
        {
            EndGroup();
            SameLine();
        }
    }
    if (!currSpace->btm.empty())
    {
        if (currSpace->top.empty()) BeginGroup();
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        std::string name = win->Name;
        name += "_Btmworkspace";
        float x = currSpace->size.x - currSpace->leftSize.x - currSpace->rightSize.x - ImGuiStyleVar_FramePadding * 2;
        ImVec2 sz{x, currSpace->btmSize.y};
        BeginChild(name.c_str(), sz, false, 0);
        currSpace->Tabs(win->Size, Bottom);
        // Text("Here");
        EndChild();
        EndGroup();
        SameLine();
    }
    if (!currSpace->right.empty())
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        std::string name = win->Name;
        name += "_Rightworkspace";
        float y = currSpace->size.y - currSpace->titleBarSz;
        ImVec2 sz{currSpace->rightSize.x - ImGuiStyleVar_FramePadding * 2.f, y};
        BeginChild(name.c_str(), sz, false, 0);
        currSpace->Tabs(win->Size, Right);
        if (!currSpace->top.empty() || !currSpace->btm.empty()) Indent(15.f);
        if (!currSpace->top.empty() || !currSpace->btm.empty()) Unindent(15.f);
        EndChild();
    }
    currSpace->prevSize = currSpace->size;
}

void ImGui::ChangeDockStatus()
{
    if (!currentDock.empty()) currentDock.front()->changeStatus();
}

void ImGui::DebugDockSpaces()
{
    Begin("Debug Dock Spaces");
    auto tmp = context->mDockSpaces.begin();
    for (int i = 0; i < context->mDockSpaces.size(); ++i)
    {
        ImGuiWindow* win = tmp->root_window;
        if (win && TreeNode(win->Name))
        {
            Text("Name: %s", win->Name);
            Text("Size: %f, %f", tmp->size.x, tmp->size.y);
            Text("Pos: %f, %f", tmp->pos.x, tmp->pos.y);
            Text("LeftSz: %f, %f", tmp->leftSize.x, tmp->leftSize.y);
            Text("RightSz: %f, %f", tmp->rightSize.x, tmp->rightSize.y);
            Text("TopSz: %f, %f", tmp->topSize.x, tmp->topSize.y);
            Text("BtmSz: %f, %f", tmp->btmSize.x, tmp->btmSize.y);
            TreePop();
        }
        ++tmp;
    }
    End();
}

bool ImGui::IntersectionTest(Dock* d)
{
    ImVec2 mousePos = GetMousePos();
    ImDrawList* draw_list = GetWindowDrawList();
    ImU32 color_hovered = GetColorU32(ImGuiCol_ButtonHovered, 0.5f);
    for (auto& dockspace : context->mDockSpaces)
    {
        ImVec2 min = dockspace.pos;
        ImVec2 max = dockspace.pos + dockspace.size;
        if ((mousePos.y > max.y || mousePos.y < min.y) || (mousePos.x > max.x || mousePos.x < min.x))
        {
            dockspace.drawRectFilled = false;
            continue;
        }
        else
        {
            dockspace.drawRectFilled = true;
            d->parent = &dockspace;
            return true;
        }
    }
    d->parent = nullptr;
    return false;
}

ImGui::Dock* ImGui::GetCurrentDock()
{
    return currentDock.front();
}

void ImGui::SetDock(Dock* current)
{
    Slot dock_slot = current->slot;
    // std::cout << dock_slot << std::endl;
    DockSpace* parent = current->parent;
    switch (dock_slot)
    {
        case Top:
        {
            if (!parent->btm.empty() && parent->topSize.y == 0.f)
            {
                parent->btmSize.y = parent->topSize.y = (parent->size.y - parent->titleBarSz) / 2.f;
            }
            if (parent->btm.empty() && parent->top.size() == 1)
            {
                if (!parent->left.empty() && !parent->right.empty())
                {
                    parent->leftSize.x = parent->leftSize.x / 2.f;
                    parent->rightSize.x = parent->rightSize.x / 2.f;
                }
                else if (!parent->left.empty() && parent->right.empty())
                {
                    parent->leftSize.x = parent->size.x / 2.f;
                }
                else if (parent->left.empty() && !parent->right.empty())
                {
                    parent->rightSize.x = parent->size.x / 2.f;
                }
            }
            break;
        }
        case Left:
        {
            if ((!parent->btm.empty() || !parent->top.empty()) && parent->leftSize.x == 0.f)
            {
                // case 2
                if (parent->right.empty())
                {
                    parent->leftSize.x = parent->size.x / 2.f;
                }
                else  // case 1
                {
                    float x = parent->size.x - parent->rightSize.x;
                    parent->leftSize.x = x / 2.f;
                }
            }
            else if (parent->leftSize.x == 0.f && !parent->right.empty())  // case 3
            {
                parent->rightSize.x = parent->leftSize.x = parent->size.x / 2.f;
            }
            break;
        }
        case Right:
        {
            if ((!parent->btm.empty() || !parent->top.empty()) && parent->rightSize.x == 0.f)
            {
                if (parent->left.empty())
                {
                    parent->rightSize.x = parent->size.x / 2.f;
                }
                else
                {
                    float x = parent->size.x - parent->leftSize.x;
                    parent->rightSize.x = x / 2.f;
                }
            }
            else if (parent->rightSize.x == 0.f && !parent->left.empty())
            {
                parent->rightSize.x = parent->leftSize.x = parent->size.x / 2.f;
            }
            break;
        }
        case Bottom:
        {
            if (!parent->top.empty() && parent->btmSize.y == 0.f)
            {
                parent->btmSize.y = parent->topSize.y = (parent->size.y - parent->titleBarSz) / 2.f;
            }
            if (parent->top.empty() && parent->btm.size() == 1)
            {
                if (!parent->left.empty() && !parent->right.empty())
                {
                    parent->leftSize.x = parent->leftSize.x / 2.f;
                    parent->rightSize.x = parent->rightSize.x / 2.f;
                }
                else if (!parent->left.empty() && parent->right.empty())
                {
                    parent->leftSize.x = parent->size.x / 2.f;
                }
                else if (parent->left.empty() && !parent->right.empty())
                {
                    parent->rightSize.x = parent->size.x / 2.f;
                }
            }
            break;
        }
        case None: break;
    }
}

void ImGui::UnDock(Dock* current)
{
    Slot dock_slot = current->slot;
    DockSpace* parent = current->parent;
    switch (dock_slot)
    {
        case Top:
        {
            if (parent->top.empty())
            {
                parent->btmSize.y = parent->topSize.y = 0.f;
                if (parent->btm.empty())
                {
                    if (!parent->left.empty() && !parent->right.empty())
                    {
                        parent->leftSize.x = parent->rightSize.x = parent->size.x / 2.f;
                    }
                    else if (parent->left.empty() && !parent->right.empty())
                    {
                        parent->rightSize.x = 0.f;
                    }
                    else if (!parent->left.empty() && parent->right.empty())
                    {
                        parent->leftSize.x = 0.f;
                    }
                }
            }
            break;
        }
        case Left:
        {
            if (parent->left.empty())
            {
                parent->leftSize.x = 0.f;
                if ((!parent->btm.empty() || !parent->top.empty()) && !parent->right.empty())
                {
                    parent->rightSize.x = parent->size.x / 2.f;
                }
                if (parent->btm.empty() && parent->top.empty() && !parent->right.empty())
                {
                    parent->rightSize.x = 0.f;
                }
            }
            break;
        }
        case Right:
        {
            if (parent->right.empty())
            {
                parent->rightSize.x = 0.f;
                if ((!parent->btm.empty() || !parent->top.empty()) && !parent->left.empty())
                {
                    parent->leftSize.x = parent->size.x / 2.f;
                }
                if (parent->btm.empty() && parent->top.empty() && !parent->left.empty())
                {
                    parent->leftSize.x = 0.f;
                }
            }
            break;
        }
        case Bottom:
        {
            if (parent->btm.empty())
            {
                parent->btmSize.y = parent->topSize.y = 0.f;
                if (parent->top.empty())
                {
                    if (!parent->left.empty() && !parent->right.empty())
                    {
                        parent->leftSize.x = parent->rightSize.x = parent->size.x / 2.f;
                    }
                    else if (parent->left.empty() && !parent->right.empty())
                    {
                        parent->rightSize.x = 0.f;
                    }
                    else if (!parent->left.empty() && parent->right.empty())
                    {
                        parent->leftSize.x = 0.f;
                    }
                }
            }
            break;
        }
    }
}

void ImGui::BeginDockChild(const char* str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
    BeginChild(str_id, size_arg, border, extra_flags);
    if (!currentDock.front()->active) GImGui->CurrentWindow->SkipItems = true;
}

void ImGui::EndDockChild()
{
    if (!currentDock.front()->active) GImGui->CurrentWindow->SkipItems = true;
    EndChild();
}

void ImGui::Dock::changeStatus()
{
    triggered_Status = true;
}

void ImGui::DockSpace::Tabs(const ImVec2& sz, Slot slot)
{
    std::vector<Dock*>* vec = nullptr;
    if (slot == Top) vec = &top;
    if (slot == Bottom) vec = &btm;
    if (slot == Left) vec = &left;
    if (slot == Right) vec = &right;

    if (vec && vec->empty()) return;
    // tabbar in lumix
    // using top for now
    auto it = vec->begin();
    std::string tmp(GetCurrentWindow()->Name);
    tmp += "_tabs";
    if (BeginChild(tmp.c_str(), ImVec2(sz.x, GetTextLineHeightWithSpacing())))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImDrawList* draw_list = GetWindowDrawList();
        ImU32 color = GetColorU32(ImGuiCol_FrameBg);
        ImU32 color_active = GetColorU32(ImGuiCol_FrameBgActive);
        ImU32 color_hovered = GetColorU32(ImGuiCol_FrameBgHovered);
        ImU32 button_hovered = GetColorU32(ImGuiCol_ButtonHovered);
        ImU32 text_color = GetColorU32(ImGuiCol_Text);
        float tab_base;
        float line_height = GetTextLineHeightWithSpacing();
        TabList(slot);
        // Drawtablist button lumix
        while (it != vec->end())
        {
            std::string label = (*it)->name;
            if ((*it)->active) label += " ";
            SameLine(0, 15);
            const char* text_end = FindRenderedTextEnd(label.c_str());
            InvisibleButton(label.c_str(), ImVec2(CalcTextSize(label.c_str(), text_end).x, line_height));
            float offset1 = 0;
            float offset2 = 10;
            float offset3 = 15;
            if ((*it)->active)
            {
                label += "X";
                offset1 = 7;
                offset2 = 15;
                offset3 = 25;
            }
            ImVec2 size(CalcTextSize(label.c_str(), text_end).x, line_height);
            text_end = FindRenderedTextEnd(label.c_str());
            bool hovered = IsItemHovered();
            if (IsItemClicked(0))
            {
                auto tmp = vec->begin();
                while (tmp != vec->end())
                {
                    if (tmp != it)
                        (*tmp)->active = false;
                    else
                        (*tmp)->active = true;
                    ++tmp;
                }
            }
            ImVec2 pos = GetItemRectMin();
            tab_base = pos.y;
            SameLine(0, 0);
            if ((*it)->active)
                if (InvisibleButton((label + "close").c_str(), CalcTextSize("X") + ImVec2(2, 0))) (*it)->changeStatus();
            draw_list->PathClear();
            draw_list->PathLineTo(pos + ImVec2(-15, size.y));
            draw_list->PathBezierCurveTo(pos + ImVec2(-10, size.y), pos + ImVec2(-5, 0), pos + ImVec2(0, 0), 10);
            draw_list->PathLineTo(pos + ImVec2(size.x + offset1, 0));
            draw_list->PathBezierCurveTo(pos + ImVec2(size.x + offset2, 0), pos + ImVec2(size.x + offset2, size.y), pos + ImVec2(size.x + offset3, size.y), 10);
            draw_list->PathFillConvex((*it)->active ? color_active : (hovered ? color_hovered : color));
            draw_list->AddText(pos, text_color, label.c_str(), text_end);
            ++it;
        }
        PopStyleVar();
    }
    EndChild();
    Separator();
}

void ImGui::DockSpace::TabList(Slot slot)
{
    std::vector<Dock*>* vec = nullptr;
    if (slot == Top) vec = &top;
    if (slot == Bottom) vec = &btm;
    if (slot == Left) vec = &left;
    if (slot == Right) vec = &right;

    if (vec && vec->size() <= 1) return;
    ImDrawList* draw_list = GetWindowDrawList();
    if (InvisibleButton("list", ImVec2(16, 16))) OpenPopup("tab_list_popup");
    if (BeginPopup("tab_list_popup"))
    {
        auto tmp = vec->begin();
        while (tmp != vec->end())
        {
            bool active = (*tmp)->active;
            if (active)
                TextDisabled((*tmp)->name.c_str());
            else
            {
                if (Selectable((*tmp)->name.c_str()))
                {
                    auto start = vec->begin();
                    while (start != vec->end())
                    {
                        if (start != tmp)
                            (*start)->active = false;
                        else
                            (*start)->active = true;
                        ++start;
                    }
                }
            }
            ++tmp;
        }
        EndPopup();
    }
    bool hovered = IsItemHovered();
    ImVec2 min = GetItemRectMin();
    ImVec2 max = GetItemRectMax();
    ImVec2 center = (min + max) * 0.5f;
    ImU32 text_color = GetColorU32(ImGuiCol_Text);
    ImU32 color_active = GetColorU32(ImGuiCol_FrameBgActive);
    draw_list->AddRectFilled(ImVec2(min.x, min.y + 3), ImVec2(min.x + 8, min.y + 5), hovered ? color_active : text_color);
    draw_list->AddTriangleFilled(ImVec2(min.x, min.y + 7), ImVec2(min.x + 8, min.y + 7), ImVec2(min.x + 4, min.y + 12), hovered ? color_active : text_color);
}

void ImGui::DockSpace::UpdateSizes()
{
    ImVec2 total = topSize + btmSize + leftSize + rightSize;
    if (prevSize.x != size.x)
    {
        if (total.x != 0.f)
        {
            float x = leftSize.x / prevSize.x;
            leftSize.x += x * diff.x;
            x = rightSize.x / prevSize.x;
            rightSize.x += x * diff.x;
        }
        // std::cout << "not same" << std::endl;
    }
    if (total.y != 0.f)
    {
        float x = topSize.y / total.y;
        topSize.y += x * diff.y;
        x = btmSize.y / total.y;
        btmSize.y += x * diff.y;
        x = leftSize.y / total.y;
    }
}

ImGui::DockContext::DockContext()
{
    TX::XMLDocument file;
    m_currwdir = _getcwd(0, 0);
    if (file.LoadFile((std::string(m_currwdir) + "\\" + "ImGuiDockLayout.xml").c_str()) == TX::XMLError::XML_SUCCESS)
    {
        TX::XMLNode* root = file.FirstChild();
        TX::XMLElement* pDockspace = root->FirstChildElement();
        for (; pDockspace; pDockspace = pDockspace->NextSiblingElement())
        {
            DockSpace tmp;
            ImVec2 sizes;
            std::string x = pDockspace->Attribute("Size.x");
            std::string y = pDockspace->Attribute("Size.y");
            sizes.x = std::stof(x);
            sizes.y = std::stof(y);
            tmp.size = sizes;
            tmp.prevSize = tmp.size;
            x = pDockspace->Attribute("Pos.x");
            y = pDockspace->Attribute("Pos.y");
            sizes.x = std::stof(x);
            sizes.y = std::stof(y);
            tmp.pos = sizes;
            x = pDockspace->Attribute("topSize.x");
            y = pDockspace->Attribute("topSize.y");
            sizes.x = std::stof(x);
            sizes.y = std::stof(y);
            tmp.topSize = sizes;
            x = pDockspace->Attribute("btmSize.x");
            y = pDockspace->Attribute("btmSize.y");
            sizes.x = std::stof(x);
            sizes.y = std::stof(y);
            tmp.btmSize = sizes;
            x = pDockspace->Attribute("leftSize.x");
            y = pDockspace->Attribute("leftSize.y");
            sizes.x = std::stof(x);
            sizes.y = std::stof(y);
            tmp.leftSize = sizes;
            x = pDockspace->Attribute("RightSize.x");
            y = pDockspace->Attribute("RightSize.y");
            sizes.x = std::stof(x);
            sizes.y = std::stof(y);
            tmp.rightSize = sizes;
            tmp.diff = ImVec2(0, 0);
            tmp.once = false;
            tmp.lonce = false;
            tmp.ronce = false;
            tmp.name = pDockspace->Value();
            mDockSpaces.push_back(tmp);
        }
        // Docks
        root = root->NextSibling();
        TX::XMLElement* pDock = root->FirstChildElement();
        for (; pDock; pDock = pDock->NextSiblingElement())
        {
            Dock dock;
            ImVec2 sizes;
            dock.name = pDock->Value();
            std::string x = pDock->Attribute("Size.x");
            std::string y = pDock->Attribute("Size.y");
            sizes.x = std::stof(x);
            sizes.y = std::stof(y);
            dock.size = sizes;
            x = pDock->Attribute("Pos.x");
            y = pDock->Attribute("Pos.y");
            sizes.x = std::stof(x);
            sizes.y = std::stof(y);
            dock.pos = sizes;
            bool active;
            pDock->QueryBoolAttribute("Active", &active);
            dock.active = active;
            int status;
            pDock->QueryIntAttribute("Status", &status);
            dock.status = static_cast<Status>(status);
            pDock->QueryIntAttribute("Slot", &status);
            dock.slot = static_cast<Slot>(status);
            mDocks.push_back(dock);
            if (pDock->Attribute("parent") != NULL)
            {
                auto start = mDockSpaces.begin();
                for (; start != mDockSpaces.end(); ++start)
                {
                    if (pDock->Attribute("parent") == start->name)
                    {
                        mDocks.back().parent = &*start;
                        switch (dock.slot)
                        {
                            case Top:
                            {
                                start->top.push_back(&mDocks.back());
                                start->top.back()->name = pDock->Value();
                                break;
                            }
                            case Bottom:
                            {
                                start->btm.push_back(&mDocks.back());
                                start->btm.back()->name = pDock->Value();
                                break;
                            }
                            case Left:
                            {
                                start->left.push_back(&mDocks.back());
                                start->left.back()->name = pDock->Value();
                                break;
                            }
                            case Right:
                            {
                                start->right.push_back(&mDocks.back());
                                start->right.back()->name = pDock->Value();
                                break;
                            }
                            case None: break;
                        }
                    }
                }
            }
        }
    }
}

ImGui::DockContext::~DockContext()
{
    std::string filename = std::string(m_currwdir) + "\\ImGuiDockLayout.xml";
    TX::XMLDocument doc;
    TX::XMLNode* pRoot = doc.NewElement("DockSpaces");
    doc.InsertEndChild(pRoot);
    for (DockSpace& ds : mDockSpaces)
    {
        TX::XMLElement* pDockSpace = doc.NewElement(ds.name.c_str());
        pRoot->InsertEndChild(pDockSpace);
        pDockSpace->SetAttribute("Size.x", ds.size.x);
        pDockSpace->SetAttribute("Size.y", ds.size.y);
        pDockSpace->SetAttribute("Pos.x", ds.pos.x);
        pDockSpace->SetAttribute("Pos.y", ds.pos.y);
        pDockSpace->SetAttribute("topSize.x", ds.topSize.x);
        pDockSpace->SetAttribute("topSize.y", ds.topSize.y);
        pDockSpace->SetAttribute("btmSize.x", ds.btmSize.x);
        pDockSpace->SetAttribute("btmSize.y", ds.btmSize.y);
        pDockSpace->SetAttribute("leftSize.x", ds.leftSize.x);
        pDockSpace->SetAttribute("leftSize.y", ds.leftSize.y);
        pDockSpace->SetAttribute("RightSize.x", ds.rightSize.x);
        pDockSpace->SetAttribute("RightSize.y", ds.rightSize.y);
    }
    TX::XMLNode* pDocks = doc.NewElement("Docks");
    doc.InsertEndChild(pDocks);
    for (Dock& dock : mDocks)
    {
        TX::XMLElement* pDock = doc.NewElement(dock.name.c_str());
        pDocks->InsertEndChild(pDock);
        pDock->SetAttribute("Pos.x", dock.pos.x);
        pDock->SetAttribute("Pos.y", dock.pos.y);
        pDock->SetAttribute("Size.x", dock.size.x);
        pDock->SetAttribute("Size.y", dock.size.y);
        if (dock.parent) pDock->SetAttribute("parent", dock.parent->name.c_str());
        pDock->SetAttribute("Active", dock.active);
        pDock->SetAttribute("Status", dock.status);
        pDock->SetAttribute("Slot", dock.slot);
    }
    TX::XMLError result = doc.SaveFile(filename.c_str());
    free(m_currwdir);
}
