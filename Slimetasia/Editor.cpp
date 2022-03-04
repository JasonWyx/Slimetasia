#include "Editor.h"

#include <comdef.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>

#include "AudioEmitter.h"
#include "AudioSystem.h"
#include "External Libraries/ImGuizmo/ImGuizmo.h"
#include "MeshAnimator.h"
#include "MeshRenderer.h"
#include "ParticleSystem.h"
#include "PhysicsSystem.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "RigidbodyComponent.h"
#include "Timer.h"
#include "luascript.h"
#define MAX_STRING_LENGTH 100

bool Editor::s_ShowBV = false;
bool Editor::s_ShowDebug = false;
bool Editor::s_lockMousePosition = false;
bool Editor::s_isPlaying = false;

static auto vector_getter = [](void* vec, int idx, const char** out_text)
{
    auto& vector = *static_cast<std::vector<std::string>*>(vec);
    if (idx < 0 || idx >= static_cast<int>(vector.size()))
    {
        return false;
    }
    *out_text = vector.at(idx).c_str();
    return true;
};

static auto enums_getter = [](void* vec, int idx, const char** out_text)
{
    auto& vector = *static_cast<std::vector<const char*>*>(vec);
    if (idx < 0 || idx >= static_cast<int>(vector.size()))
    {
        return false;
    }
    *out_text = vector.at(idx);
    return true;
};

void Editor::Undo()
{
    if (m_Undo.empty()) return;
    std::cout << "Undo" << std::endl;
    Actions* act = m_Undo.back();
    if (dynamic_cast<Creation_Action*>(act))
    {
        m_CurrentObject = nullptr;
        m_SelectedObjects.clear();
        Renderer::Instance().SetSelectedObjects({0});
    }
    if (dynamic_cast<CreateObjectArchetype_Action*>(act))
    {
        m_CurrentObject = nullptr;
        m_SelectedObjects.clear();
        Renderer::Instance().SetSelectedObjects({0});
    }
    m_Undo.pop_back();
    act->UnExecute();
    m_Redo.push_back(act);
}

void Editor::Redo()
{
    if (m_Redo.empty()) return;
    std::cout << "Redo" << std::endl;
    Actions* act = m_Redo.back();
    m_Redo.pop_back();
    act->Execute();
    m_Undo.push_back(act);
}

void Editor::Update_Redo_Undo()
{
    while (m_Redo.size() > m_Redo_Undo_Size)
    {
        Actions* tmp = m_Redo.front();
        m_Redo.pop_front();
        delete tmp;
    }

    while (m_Undo.size() > m_Redo_Undo_Size)
    {
        Actions* tmp = m_Undo.front();
        m_Undo.pop_front();
        delete tmp;
    }
}

void Editor::Clear_Redo_Undo()
{
    while (!m_Redo.empty())
    {
        Actions* tmp = m_Redo.front();
        m_Redo.pop_front();
        delete tmp;
    }
    while (!m_Undo.empty())
    {
        Actions* tmp = m_Undo.front();
        m_Undo.pop_front();
        delete tmp;
    }
}

void Editor::MainMenu()
{
    bool b = false;
    bool style = false;
    bool help = false;
    BeginMainMenuBar();
    if (BeginMenu("File"))
    {
        if (MenuItem("New", "CTRL+N")) b = true;
        if (MenuItem("Save", "CTRL+S")) Save();
        if (MenuItem("Load", "CTRL+O")) Load();
        if (MenuItem("Exit", "ALT+X")) Application::Instance().QuitProgram();
        ImGui::EndMenu();
    }
    if ((Input::Instance().GetKeyDown(KEY_LALT) || Input::Instance().GetKeyDown(KEY_RALT)) && Input::Instance().GetKeyDown(KEY_X)) Application::Instance().QuitProgram();
    if (BeginMenu("Edit"))
    {
        if (m_Undo.empty())
            MenuItem("Undo", "CTRL+Z", false, false);
        else if (MenuItem("Undo", "CTRL+Z"))
            Undo();
        if (m_Redo.empty())
            MenuItem("Redo", "CTRL+Y", false, false);
        else if (MenuItem("Redo", "CTRL+Y"))
            Redo();
        if (m_SelectedObjects.empty())
            MenuItem("Duplicate", "CTRL+D", false, false);
        else if (MenuItem("Duplicate", "CTRL+D"))
            Duplicate();
        ImGui::EndMenu();
    }
    if (BeginMenu("Windows"))
    {
        if (MenuItem("Outliner", NULL, m_ActiveWindow[WindowStates::Outliner_State], !m_ActiveWindow[WindowStates::Outliner_State])) m_ActiveWindow[WindowStates::Outliner_State] = true;
        if (MenuItem("Inspector", NULL, m_ActiveWindow[WindowStates::Inspector_State], !m_ActiveWindow[WindowStates::Inspector_State])) m_ActiveWindow[WindowStates::Inspector_State] = true;
        if (MenuItem("Console", NULL, m_Console.ActiveWindow, !m_Console.ActiveWindow)) m_Console.ActiveWindow = true;
        if (MenuItem("Viewport", NULL, m_ActiveWindow[WindowStates::Viewport_State], !m_ActiveWindow[WindowStates::Viewport_State])) m_ActiveWindow[WindowStates::Viewport_State] = true;
        if (MenuItem("Archetype", NULL, m_ActiveWindow[WindowStates::Archetype_State], !m_ActiveWindow[WindowStates::Archetype_State])) m_ActiveWindow[WindowStates::Archetype_State] = true;
        if (MenuItem("Text Editor", NULL, m_ActiveWindow[WindowStates::TextEditor_State], !m_ActiveWindow[WindowStates::TextEditor_State])) m_ActiveWindow[WindowStates::TextEditor_State] = true;
        if (MenuItem("Layer Editor", NULL, m_ActiveWindow[WindowStates::LayerEditor_State], !m_ActiveWindow[WindowStates::LayerEditor_State])) m_ActiveWindow[WindowStates::LayerEditor_State] = true;
        if (MenuItem("Profiler", NULL, m_ActiveWindow[WindowStates::Profiler_State], !m_ActiveWindow[WindowStates::Profiler_State])) m_ActiveWindow[WindowStates::Profiler_State] = true;
        if (MenuItem("Tags Editor", NULL, m_ActiveWindow[WindowStates::Tags_State], !m_ActiveWindow[WindowStates::Tags_State])) m_ActiveWindow[WindowStates::Tags_State] = true;
        if (MenuItem("Physics Editor", NULL, m_ActiveWindow[WindowStates::Physics_State], !m_ActiveWindow[WindowStates::Physics_State])) m_ActiveWindow[WindowStates::Physics_State] = true;
        if (MenuItem("Resource Manager", NULL, m_ActiveWindow[WindowStates::Resource_State], !m_ActiveWindow[WindowStates::Resource_State])) m_ActiveWindow[WindowStates::Resource_State] = true;
        ImGui::EndMenu();
    }
    if (BeginMenu("GameObject"))
    {
        if (MenuItem("Empty Object"))
        {
            Creation_Action* act = new Creation_Action(m_CurrentLayer, "GameObject");
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
        ImGui::EndMenu();
    }
    if (BeginMenu("Options"))
    {
        // TextDisabled("Editor Style");
        // if (MenuItem("Style Editor")) style = true;
        Separator();
        TextDisabled("Viewport");
        if (MenuItem("FullScreen VP on Play", "F8", m_viewportFullScreen)) m_viewportFullScreen = !m_viewportFullScreen;
        // if (!m_GameCameraInVP){
        // 	if (MenuItem("Enable Game Camera in Viewport")) m_GameCameraInVP = !m_GameCameraInVP;
        // }
        // else {
        // 	if (MenuItem("Disable Game Camera in Viewport")) m_GameCameraInVP = !m_GameCameraInVP;
        // }
        Separator();
        TextDisabled("Object Transformation");
        if (!m_Local)
        {
            if (MenuItem("Local Transform")) m_Local = true;
        }
        else
        {
            if (MenuItem("World Transform")) m_Local = false;
        }
        Separator();
        // TextDisabled("Help and Troubleshoot");
        // if (MenuItem("Help", "F1")) help = true;
        ImGui::EndMenu();
    }
    Text("Frame time : %f", 1.0 / Application::Instance().GetGameTimer().GetScaledFrameTime());
    EndMainMenuBar();
    if (style) OpenPopup("StyleEditor");
    if (b) OpenPopup("NewScene");
    if (help) OpenPopup("HelpScreen");
}

void Editor::ShortcutButtons()
{
    if (m_editorFocus || !Application::Instance().GetGameTimer().IsEditorPaused()) return;
    auto& io = GetIO();
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;
    if (IsKeyDown(17))
    {
        if (IsKeyPressed('Z'))
            Undo();
        else if (IsKeyPressed('Y'))
            Redo();
        else if (IsKeyPressed('S'))
        {
            GetIO().KeysDown[17] = false;
            GetIO().KeysDown['S'] = false;
            Save();
        }
        else if (IsKeyPressed('O'))
        {
            GetIO().KeysDown[17] = false;
            GetIO().KeysDown['O'] = false;
            Load();
        }
        else if (IsKeyPressed('N'))
            OpenPopup("NewScene");
    }
    // if (IsKeyPressed(112)) OpenPopup("HelpScreen");
    if (IsKeyPressed(46) && m_CurrentObject && m_CurrentObject->GetName() != "EditorCamera")
    {
        DeleteObject_Action* act = new DeleteObject_Action(m_CurrentObject, m_CurrentLayer);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
        m_CurrentObject = nullptr;
        m_SelectedObjects.clear();
        Renderer::Instance().SetSelectedObjects({0});
    }
}

void Editor::StyleEditor()
{
    if (BeginPopupModal("StyleEditor", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
    {
        SetItemDefaultFocus();
        ShowStyleEditor();
        NewLine();
        if (Button("Exit")) CloseCurrentPopup();
        NewLine();
        EndPopup();
    }
}

void Editor::Help()
{
    if (BeginPopupModal("HelpScreen", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImDrawList* draw_list = GetWindowDrawList();
        ImU32 color = GetColorU32(ImGuiCol_Text);
        ImVec2 pos = GetCursorScreenPos();
        draw_list->AddText(GetFont(), GetFontSize() * 1.5f, ImVec2(pos.x, pos.y + 5.f), color, "Welcome to PEngine's Troubleshoot/Help Screen.");
        NewLine();
        NewLine();
        Text("Please navigate the table below see the troubleshoot the problem you are currently having");
        if (TreeNode("The book of Troubleshoot"))
        {
            NewLine();
            NewLine();
            NewLine();
            NewLine();
            NewLine();
            draw_list->AddText(GetFont(), GetFontSize() * 7.f, ImVec2(pos.x + 50.f, pos.y + 75.f), IM_COL32(255, 0, 0, 255), "User Error!");
            TreePop();
            TreePop();
        }
        NewLine();
        if (Button("Exit")) CloseCurrentPopup();
        NewLine();
        EndPopup();
    }
}

void Editor::Viewport()
{
    if (Input::Instance().GetKeyPressed(KEY_F8)) m_viewportFullScreen = !m_viewportFullScreen;
    static bool deltaGizmoState = false;
    static bool over = false;
    static bool state = 0;
    static ImGuizmo::OPERATION currOperation = (ImGuizmo::OPERATION)3;
    ImGuizmo::SetDrawlist();
    auto editorCamera = m_CurrentLayer->GetEditorCamera();
    //  if (m_CurrentObject && m_CurrentObject->GetComponent<Transform>()) Renderer::Instance().SetSelectedObjects({ m_CurrentObject->GetID() });
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 windowOffset = ImGui::GetWindowPos();

    float minusy = GetWindowPos().y;
    float minusx = GetWindowPos().x;
    if (Application::Instance().GetGameTimer().IsEditorPaused())
    {
        if (Button("Play") || IsKeyPressed(116))
        {
            std::cout << "============================================================" << std::endl;
            std::cout << "PLAY MODE START" << std::endl;
            std::cout << "============================================================" << std::endl;
            s_isPlaying = true;

            // Set system instance
            Application::Instance().GetGameTimer().SetEditorPaused(false);
            PhysicsSystem::Instance().Init();

            // Load all objects in all layers
            auto scene = Application::Instance().GetCurrentScene();
            auto layers = scene->GetLayers();
            for (auto& ly : layers)
            {
                auto l = new Layer(nullptr, ly->GetId(), ly->GetName(), false);
                m_SavedLayers.emplace_back(l);
                ly->Clone(m_SavedLayers.back());
            }

            // Change to game camera
            // Renderer::Instance().ChangeCamera(true);
            // int cx = Application::Instance().GetMaxWindowWidth() / 2;
            // int cy = Application::Instance().GetMaxWindowHeight() / 2;
            // cx -= Application::Instance().GetWindowWidth() / 2;
            // cy -= Application::Instance().GetWindowHeight() / 2;

            // SetCursorPos(cx, cy);
            // if(!m_viewportFullScreen)
            //   SetCursorPos(cx + (int)minusx + (int)windowSize.x / 2, cy + (int)minusy + (int)windowSize.y / 2);
            // else
            //   SetCursorPos(cx + Application::Instance().GetWindowWidth() / 2, cy + Application::Instance().GetWindowHeight() / 2);

            // Initialize all lua scripts
            if (Application::InstancePtr())
            {
                for (auto& ly : Application::Instance().GetCurrentScene()->GetLayers())
                    for (auto go : ly->GetObjectsList())
                        for (auto c : go->GetLuaScripts())
                            c->InitScript();
            }
        }
        if (Input::Instance().GetKeyDown(KEY_LCTRL) && Input::Instance().GetKeyPressed(KEY_D))
        {
            if (!m_SelectedObjects.empty())
            {
                Duplicate();
            }
        }
    }
    else
    {
        if (Application::Instance().GetGameTimer().IsPlayModePaused())
        {
            if (Button("Play"))
            {
                s_isPlaying = true;
                while (ShowCursor(false) > 0)
                    ;
                AudioSystem::Instance().UnPauseAudio();
                Application::Instance().GetGameTimer().SetPlayModePaused(false);
            }
        }
        else
        {
            if (Button("Pause") || Input::Instance().GetKeyPressed(KEY_F6))
            {
                s_isPlaying = false;
                while (ShowCursor(true) < 0)
                    ;
                AudioSystem::Instance().PauseAudio();
                Application::Instance().GetGameTimer().SetPlayModePaused(true);
            }
        }
        SameLine();
        if (Button("Stop") || Input::Instance().GetKeyPressed(KEY_F5))
        {
            Application::Instance().GetGameTimer().SetPlayModePaused(false);
            while (ShowCursor(true) < 0)
                ;
            std::cout << "============================================================" << std::endl;
            std::cout << "PLAY MODE END" << std::endl;
            std::cout << "============================================================" << std::endl;
            s_isPlaying = false;

            Application::Instance().GetGameTimer().SetEditorPaused(true);
            PhysicsSystem::Instance().Close();
            PhysicsSystem::Instance().RepopulateDTree();
            if (!m_SavedLayers.empty())
            {
                Application::Instance().GetCurrentScene()->ClearLayer();
                auto Scene = Application::Instance().GetCurrentScene();
                // PhysicsSystem::s_DynamicsWorld.m_ColDetection.m_BroadPhase.m_Tree.ResetTree();
                for (auto& ly : m_SavedLayers)
                {
                    Scene->CreateLayerWithoutCamera(ly->GetName());
                    ly->Clone(Scene->GetLayers().back());
                    delete ly;
                }
                m_SavedLayers.clear();
                m_CurrentLayer = Scene->GetLayers().back();
                Renderer::Instance().SetCurrentLayer(m_CurrentLayer);
            }
            m_CurrentObject = nullptr;
            m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayers().back();
            m_SelectedObjects.clear();
            Renderer::Instance().SetSelectedObjects({0});
            Renderer::Instance().ChangeCamera(false);
            ParticleSystem::Instance().Reset();
            AISystem::Instance().RevertBase();
        }
    }
    // Text("Window Size : %f, %f", GetWindowSize().x, GetWindowSize().y);
    if (GetCurrentDock()->status == Docked)
    {
        windowSize.y -= GetCurrentDock()->parent->titleBarSz + 33.f;
        minusy += GetCurrentDock()->parent->titleBarSz - 36.f - 9.f - 13.f;
    }
    else
    {
        windowSize.y -= 36.f - 33.f;
        minusx += 7.f;
        minusy -= (9.f - 53.f);  // += ImGuiStyleVar_FramePadding * 2 + GetFontSize() - 5.f;
    }
    if (IsWindowHovered() && (Application::Instance().GetGameTimer().IsEditorPaused() || Application::Instance().GetGameTimer().IsPlayModePaused()))
    {
        static Vector3 tmp;
        static Vector3 current;
        if ((GetCurrentDock()->status == Floating) || (GetCurrentDock()->status == Docked && GetCurrentDock()->active))
        {
            if (IsKeyPressed(70) && !Input::Instance().GetKeyDown(KEY_LALT) && m_CurrentObject) m_CurrentLayer->GetEditorCamera()->LookAt(m_CurrentObject);
            m_CurrentLayer->GetEditorCamera()->SetUpdate(true);
            m_CurrentLayer->GetEditorCamera()->OnUpdate(1 / 60.f);
            m_CurrentLayer->GetEditorCamera()->SetUpdate(false);
            float y = abs(GetIO().MousePos.y - GetWindowSize().y - minusy);
            /*std::cout << "Input::Instance().GetKeyPressed(VK_LBUTTON) = " << Input::Instance().GetKeyPressed(VK_LBUTTON) << "\n";
            std::cout << "!Input::Instance().GetKeyDown(KEY_LALT) = " << !Input::Instance().GetKeyDown(KEY_LALT) << "\n";
            std::cout << "!ImGuizmo::IsUsing() = " << !ImGuizmo::IsUsing() << "\n";
            std::cout << "!ImGuizmo::IsOver() = " << !ImGuizmo::IsOver() << "\n";*/
            // if (/*IsMouseClicked(0)*/ Input::Instance().GetKeyPressed(VK_LBUTTON) && !Input::Instance().GetKeyDown(KEY_LALT) && !ImGuizmo::IsUsing() && ImGuizmo::IsOver())

            if (state)
                over = ImGuizmo::IsOver();
            else
                over = false;

            // std::cout << over << std::endl;

            if ((IsMouseClicked(0) && !Input::Instance().GetKeyDown(KEY_LALT) && !ImGuizmo::IsUsing() && !deltaGizmoState && !over) ||
                (IsMouseClicked(0) && !Input::Instance().GetKeyDown(KEY_LALT) && !m_CurrentObject))
            {
                unsigned picked = Renderer::Instance().GetPickedObject(iVector2((int)(GetIO().MousePos.x - minusx), (int)y));
                if (picked != 0)
                {
                    m_CurrentObject = m_CurrentLayer->GetObjectById(picked);
                    if (Input::Instance().GetKeyDown(KEY_LSHIFT))
                    {
                        auto it = std::find(m_SelectedObjects.begin(), m_SelectedObjects.end(), m_CurrentObject);
                        if (it == m_SelectedObjects.end())
                            m_SelectedObjects.push_back(m_CurrentObject);
                        else
                        {
                            m_SelectedObjects.erase(it);
                            if (!m_SelectedObjects.empty())
                            {
                                m_CurrentObject = m_SelectedObjects.front();
                                std::vector<unsigned> tmp;
                                for (auto& o : m_SelectedObjects)
                                    tmp.push_back(o->GetID());
                                Renderer::Instance().SetSelectedObjects(tmp);
                            }
                            else
                            {
                                Renderer::Instance().SetSelectedObjects({0});
                                m_CurrentObject = nullptr;
                            }
                        }
                    }
                    else
                    {
                        m_SelectedObjects.clear();
                        m_SelectedObjects.push_back(m_CurrentObject);
                        Renderer::Instance().SetSelectedObjects({m_CurrentObject->GetID()});
                    }
                    std::vector<unsigned> selecteds;
                    for (auto& obj : m_SelectedObjects)
                        selecteds.push_back(obj->GetID());
                    Renderer::Instance().SetSelectedObjects(selecteds);
                }
                else
                {
                    Renderer::Instance().SetSelectedObjects({0});
                    m_CurrentObject = nullptr;
                    m_SelectedObjects.clear();
                }
            }
        }
    }
    else
    {
        m_CurrentLayer->GetEditorCamera()->Cancel();
    }

    Renderer::Instance().SetWindowSize(iVector2((int)windowSize.x, (int)windowSize.y));
    Renderer::Instance().GetCurrentEditorLayer()->GetEditorCamera()->SetViewportSize(iVector2((int)windowSize.x, (int)windowSize.y));

    ImVec2 GameCameraSize = ImVec2((float)Application::Instance().GetWindowHeight(), (float)Application::Instance().GetWindowHeight());
    GameCameraSize.x /= 10.f;
    GameCameraSize.y /= 10.f;

    Image((ImTextureID)((__int64)Renderer::Instance().GetRenderTexture()), windowSize, ImVec2(0, 1), ImVec2(1, 0));

    if (m_GameCameraInVP)
    {
        ImDrawList* draw_list = GetWindowDrawList();
        ImVec2 start = GetWindowPos();
        if (GetCurrentDock()->status == Docked)
        {
            start.x += 20.f;
            start.y += 60.f;
        }
        else
        {
            start.y += 70.f;
            start.x += 20.f;
        }
        GameCameraSize.x += start.x;
        GameCameraSize.y += start.y;
        draw_list->AddImage((ImTextureID)((__int64)Renderer::Instance().GetRenderTexture()), start, GameCameraSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    if (Application::Instance().GetGameTimer().IsEditorPaused())
    {
        if ((GetCurrentDock()->status == Floating) || (GetCurrentDock()->status == Docked && GetCurrentDock()->active))
        {
            if (m_CurrentObject)
            {
                if (!m_SelectedObjects.empty())
                {
                    std::vector<unsigned> selecteds;
                    for (auto& obj : m_SelectedObjects)
                        selecteds.push_back(obj->GetID());
                    Renderer::Instance().SetSelectedObjects(selecteds);
                }
                Transform* t = m_CurrentObject->GetComponent<Transform>();
                if (t)
                {
                    static Vector3 savedTrans;
                    static Vector3 savedRot;
                    static Vector3 savedScale;
                    float m[16];
                    static float trans[3];
                    static float rot[3];
                    static float scale[3];

                    if (IsKeyPressed(81))
                    {
                        currOperation = (ImGuizmo::OPERATION)3;
                        state = 0;
                    }
                    if (IsKeyPressed(87))
                    {
                        state = 1;
                        currOperation = ImGuizmo::TRANSLATE;
                    }
                    if (IsKeyPressed(69))
                    {
                        state = 1;
                        currOperation = ImGuizmo::ROTATE;
                    }
                    if (IsKeyPressed(82))
                    {
                        state = 1;
                        currOperation = ImGuizmo::SCALE;
                    }

                    memcpy(trans, t->GetWorldPosition().GetVector(), sizeof(float) * 3);
                    memcpy(rot, t->GetWorldRotation().GetVector(), sizeof(float) * 3);
                    memcpy(scale, t->GetWorldScale().GetVector(), sizeof(float) * 3);

                    bool useSnap(false);
                    if (Input::Instance().GetKeyDown(KEY_LSHIFT)) useSnap = true;
                    float snap[3];
                    switch (currOperation)
                    {
                        case ImGuizmo::TRANSLATE: snap[0] = snap[1] = snap[2] = 1.f; break;
                        case ImGuizmo::ROTATE: snap[0] = snap[1] = snap[2] = 45.f; break;
                        case ImGuizmo::SCALE: snap[0] = snap[1] = snap[2] = 1.f; break;
                    }

                    if (GetCurrentDock()->status == Docked)
                    {
                        if (GetCurrentDock()->slot == Slot::Right && (!GetCurrentDock()->parent->btm.empty() || !GetCurrentDock()->parent->top.empty()))
                            ImGuizmo::SetRect(GetCurrentDock()->pos.x + 15, GetCurrentDock()->pos.y + 49, windowSize.x, windowSize.y);
                        else
                            ImGuizmo::SetRect(GetCurrentDock()->pos.x, GetCurrentDock()->pos.y + 49, windowSize.x, windowSize.y);
                    }
                    else
                        ImGuizmo::SetRect(windowOffset.x + 8, windowOffset.y + 50, windowSize.x, windowSize.y);

                    ImGuizmo::RecomposeMatrixFromComponents(trans, rot, scale, m);
                    if (currOperation != (ImGuizmo::OPERATION)3)
                    {
                        if (!m_Local)
                            ImGuizmo::Manipulate(editorCamera->GetViewTransform().GetMatrix(), editorCamera->GetProjTransform().GetMatrix(), currOperation, ImGuizmo::WORLD, m, NULL,
                                                 useSnap ? snap : NULL);
                        else
                            ImGuizmo::Manipulate(editorCamera->GetViewTransform().GetMatrix(), editorCamera->GetProjTransform().GetMatrix(), currOperation, ImGuizmo::LOCAL, m, NULL,
                                                 useSnap ? snap : NULL);
                    }
                    ImGuizmo::DecomposeMatrixToComponents(m, trans, rot, scale);
                    if (!deltaGizmoState && ImGuizmo::IsUsing())
                    {
                        savedTrans = t->GetWorldPosition();
                        savedRot = t->GetWorldRotation();
                        savedScale = t->GetWorldScale();
                    }
                    if (deltaGizmoState && ImGuizmo::IsUsing())
                    {
                        // for one and for many
                        if (m_SelectedObjects.size() == 1)
                        {
                            Vector3 tmpTrans{trans[0], trans[1], trans[2]};
                            Vector3 tmpRot{rot[0], rot[1], rot[2]};
                            Vector3 tmpScale{scale[0], scale[1], scale[2]};
                            t->SetWorldPosition(tmpTrans);
                            t->SetWorldRotation(tmpRot);
                            t->SetWorldScale(tmpScale);
                        }
                        else if (m_SelectedObjects.size() > 1)
                        {
                            Vector3 tmpTrans{trans[0], trans[1], trans[2]};
                            Vector3 tmpRot{rot[0], rot[1], rot[2]};
                            Vector3 tmpScale{scale[0], scale[1], scale[2]};
                            Vector3 m_Trans = t->GetWorldPosition();
                            Vector3 m_Rot = t->GetWorldRotation();
                            Vector3 m_Scale = t->GetWorldScale();
                            tmpTrans -= m_Trans;
                            tmpRot -= m_Rot;
                            tmpScale -= m_Scale;
                            for (auto& obj : m_SelectedObjects)
                            {
                                Transform* pos = obj->GetComponent<Transform>();
                                if (pos)
                                {
                                    Vector3 w_trans = pos->GetWorldPosition();
                                    Vector3 w_rot = pos->GetWorldRotation();
                                    Vector3 w_scale = pos->GetWorldScale();
                                    pos->SetWorldPosition(w_trans + tmpTrans);
                                    pos->SetWorldRotation(w_rot + tmpRot);
                                    pos->SetWorldScale(w_scale + tmpScale);
                                }
                            }
                        }
                    }
                    if (deltaGizmoState && !ImGuizmo::IsUsing())
                    {
                        if (m_SelectedObjects.size() == 1)
                        {
                            t->SetWorldPosition(savedTrans);
                            t->SetWorldRotation(savedRot);
                            t->SetWorldScale(savedScale);
                            if ((trans[0] != savedTrans.x || trans[1] != savedTrans.y || trans[2] != savedTrans.z) && currOperation == ImGuizmo::TRANSLATE)
                            {
                                Vector3 newValue = Vector3(trans[0], trans[1], trans[2]);
                                Vector3 oldValue = savedTrans;
                                Input_Action<Vector3>* act = new Input_Action<Vector3>(oldValue, newValue, m_CurrentObject->GetName(), "Transform", "m_WorldPosition", m_CurrentLayer, m_CurrentObject);
                                act->Execute();
                                m_Undo.push_back(std::move(act));
                                ClearRedo();
                            }
                            else if ((scale[0] != savedScale.x || scale[1] != savedScale.y || scale[2] != savedScale.z) && currOperation == ImGuizmo::SCALE)
                            {
                                Vector3 newValue = Vector3(scale[0], scale[1], scale[2]);
                                Vector3 oldValue = savedScale;
                                Input_Action<Vector3>* act = new Input_Action<Vector3>(oldValue, newValue, m_CurrentObject->GetName(), "Transform", "m_WorldScale", m_CurrentLayer, m_CurrentObject);
                                act->Execute();
                                m_Undo.push_back(std::move(act));
                                ClearRedo();
                            }
                            else if ((rot[0] != savedRot.x || rot[1] != savedRot.y || rot[2] != savedRot.z) && currOperation == ImGuizmo::ROTATE)
                            {
                                Vector3 newValue = Vector3(rot[0], rot[1], rot[2]);
                                Vector3 oldValue = savedRot;
                                Input_Action<Vector3>* act = new Input_Action<Vector3>(oldValue, newValue, m_CurrentObject->GetName(), "Transform", "m_WorldRotation", m_CurrentLayer, m_CurrentObject);
                                act->Execute();
                                m_Undo.push_back(std::move(act));
                                ClearRedo();
                            }
                        }
                        else if (m_SelectedObjects.size() > 1)
                        {
                            Vector3 tmpTrans{trans[0], trans[1], trans[2]};
                            Vector3 tmpRot{rot[0], rot[1], rot[2]};
                            Vector3 tmpScale{scale[0], scale[1], scale[2]};
                            Vector3 m_Trans = tmpTrans - savedTrans;
                            Vector3 m_Rot = tmpRot - savedRot;
                            Vector3 m_Scale = tmpScale - savedScale;
                            std::vector<unsigned> ids;
                            for (auto& obj : m_SelectedObjects)
                            {
                                Transform* pos = obj->GetComponent<Transform>();
                                if (pos)
                                {
                                    Vector3 w_trans = pos->GetWorldPosition();
                                    Vector3 w_rot = pos->GetWorldRotation();
                                    Vector3 w_scale = pos->GetWorldScale();
                                    pos->SetWorldPosition(w_trans - m_Trans);
                                    pos->SetWorldRotation(w_rot - m_Rot);
                                    pos->SetWorldScale(w_scale - m_Scale);
                                }
                                ids.push_back(obj->GetID());
                            }
                            if ((m_Trans[0] != 0.f || m_Trans[1] != 0.f || m_Trans[2] != 0.f) && currOperation == ImGuizmo::TRANSLATE)
                            {
                                MultipleTransformation_Action* act = new MultipleTransformation_Action(m_Trans, ids, "Transform", "m_WorldPosition", m_CurrentLayer);
                                act->Execute();
                                m_Undo.push_back(std::move(act));
                                ClearRedo();
                            }
                            else if ((m_Scale[0] != 0.f || m_Scale[1] != 0.f || m_Scale[2] != 0.f) && currOperation == ImGuizmo::SCALE)
                            {
                                MultipleTransformation_Action* act = new MultipleTransformation_Action(m_Scale, ids, "Transform", "m_WorldScale", m_CurrentLayer);
                                act->Execute();
                                m_Undo.push_back(std::move(act));
                                ClearRedo();
                            }
                            else if ((m_Rot[0] != 0.f || m_Rot[1] != 0.f || m_Rot[2] != 0.f) && currOperation == ImGuizmo::ROTATE)
                            {
                                MultipleTransformation_Action* act = new MultipleTransformation_Action(m_Rot, ids, "Transform", "m_WorldRotation", m_CurrentLayer);
                                act->Execute();
                                m_Undo.push_back(std::move(act));
                                ClearRedo();
                            }
                        }
                    }
                    deltaGizmoState = ImGuizmo::IsUsing();
                }
            }
        }
    }
}

void Editor::Archetype()
{
    static int selected = -1;
    m_Global_Spaces += " ";
    std::vector<std::string> archetypes;
    for (auto a : m_Archetypes)
        archetypes.push_back(a.first);
    PushItemWidth(200.f);
    ListBox(m_Global_Spaces.c_str(), &selected, vector_getter, static_cast<void*>(&archetypes), (int)archetypes.size(), 5);
    PopItemWidth();
    static char string[MAX_STRING_LENGTH];
    // SecureZeroMemory(string, MAX_STRING_LENGTH);
    if (Button("Create Base Archetype")) OpenPopup("ArchetypeName");

    Separator();

    if (selected > -1 && m_Archetypes.size())
    {
        GameObject* currentArchetype = m_UpdatedArchetypes[archetypes[selected]];
        Text("Archetype Name : %s", currentArchetype->GetName().c_str());
        // Text("Archetype ID : %d", currentArchetype->GetID());
        // Add Archetype Inspector here
        // Idea is to have the similar thing to the inspector
        // only save changes and will affect all archetypes, and need clone

        // Add Tags here
        Text("Tag:");
        SameLine();
        if (Selectable(currentArchetype->GetTag() == std::string{} ? "Click me to add Tag" : currentArchetype->GetTag().c_str())) OpenPopup("ArcheTagsPopUp");
        if (BeginPopup("ArcheTagsPopUp"))
        {
            Text("Tags");
            ImGui::Separator();
            for (auto& tag : m_tags)
            {
                if (Selectable(tag.c_str()))
                {
                    currentArchetype->SetTag(tag.c_str());
                }
            }
            if (Selectable(""))
            {
                if (currentArchetype->GetTag() != "")
                {
                    currentArchetype->SetTag("");
                }
            }
            ImGui::EndPopup();
        }

        std::vector<std::string> AddComp;
        std::vector<std::string> DelComp;
        auto list = currentArchetype->GetComponentList();
        auto start = (*Factory::m_Factories).begin();
        for (; start != (*Factory::m_Factories).end(); ++start)
        {
            auto comp = list.begin();
            for (; comp != list.end(); ++comp)
            {
                if ((*comp)->GetName() == start->first)
                {
                    DelComp.push_back((*comp)->GetName());
                    break;
                }
            }
            if (comp == list.end())
            {
                if (start->first != "LuaScript") AddComp.push_back(start->first);
            }
        }
        AddComp.push_back("LuaScript");
        Text("Num of Components : %d", list.size());
        Separator();
        if (Button("Add Component")) OpenPopup("AddComponentPopup");
        if (BeginPopup("AddComponentPopup"))
        {
            Text("Components");
            Separator();
            for (int i = 0; i < AddComp.size(); ++i)
                if (Selectable(AddComp[i].c_str())) Factory::m_Factories->at(AddComp[i])->create(currentArchetype);
            EndPopup();
        }
        SameLine();
        if (Button("Remove Component")) OpenPopup("RemoveComponentPopup");
        if (BeginPopup("RemoveComponentPopup"))
        {
            Text("Components");
            Separator();
            for (int i = 0; i < DelComp.size(); ++i)
            {
                if (DelComp[i] == "LuaScript")
                {
                    auto scripts = currentArchetype->GetScripts();
                    for (size_t j = 0; j < scripts.size(); ++j)
                    {
                        if (Selectable((std::string("Script:") + scripts[j]).c_str())) currentArchetype->RemoveScript(scripts[j]);
                    }
                }
                else if (Selectable(DelComp[i].c_str()))
                    Factory::m_Factories->at(DelComp[i])->remove(currentArchetype);
            }
            EndPopup();
        }
        if (Button("Revert"))
        {
            GameObject* tmp = new GameObject(nullptr, 0);
            tmp->SetName(currentArchetype->GetName());
            m_Archetypes[archetypes[selected]]->Clone(tmp);
            RevertArchetype_Action* act = new RevertArchetype_Action(tmp);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            return;
        }
        SameLine();
        if (Button("Make Changes"))
        {
            GameObject* tmp = new GameObject(nullptr, 0);
            tmp->SetName(currentArchetype->GetName());
            currentArchetype->Clone(tmp);
            MakeChanges_Action* act = new MakeChanges_Action(tmp);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            return;
        }
        SameLine();
        if (Button("Create Archetype"))
        {
            Layer* ly = m_CurrentLayer;
            CreateObjectArchetype_Action* act = new CreateObjectArchetype_Action(ly, currentArchetype->GetName(), currentArchetype->GetName());
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            return;
        }
        if (Button("Delete Archetype"))
        {
            DeleteArchetype_Action* act = new DeleteArchetype_Action(m_Archetypes[archetypes[selected]], m_UpdatedArchetypes[archetypes[selected]]);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            selected = -1;
            return;
        }
        Separator();
        int i = 0;
        for (auto& comp : list)
        {
            if (TreeNode((std::string("Comp ") + std::to_string(i) + std::string(" : ") + comp->GetName()).c_str()))
            {
                char* address = reinterpret_cast<char*>(comp);
                // Text("Component Num : %d", i);
                // Text("Component Name : %s", comp->GetName().c_str());
                try
                {
                    if (!Factory::m_Reflection->at(comp->GetName())->getParents().empty())
                        ParentArchetypeInspector(address, Factory::m_Reflection->at(comp->GetName())->getParents().back().key, currentArchetype);
                    auto component = (*Factory::m_Reflection).at(comp->GetName().c_str());
                    auto properties = component->getProperties();
                    Indent(5.f);
                    for (int i = 0; i < properties.size(); ++i)
                    {
                        Text("%s : ", properties[i].name.c_str());
                        // if else for each type
                        if (properties[i].type == typeid(std::string).name())
                        {
                            char tmp[MAX_STRING_LENGTH];
                            SecureZeroMemory(tmp, MAX_STRING_LENGTH);
                            SameLine();
                            m_Global_Spaces += " ";
                            std::string* string = reinterpret_cast<std::string*>(address + properties[i].offset);
                            std::copy(string->begin(), string->end(), tmp);
                            // here for textrenderer
                            if (comp->GetName() == "TextRenderer" && properties[i].name == "m_Text")
                            {
                                InputTextMultiline(m_Global_Spaces.c_str(), tmp, MAX_STRING_LENGTH);
                                if (!IsItemActive() && *string != tmp) *string = std::string(tmp);
                            }
                            else
                            {
                                InputText(m_Global_Spaces.c_str(), tmp, MAX_STRING_LENGTH);
                                if (IsKeyPressed(KEY_RETURN) && *string != tmp) *string = std::string(tmp);
                            }
                        }
                        else if (properties[i].type == typeid(iVector2).name())
                        {
                            iVector2* vec3 = reinterpret_cast<iVector2*>(address + properties[i].offset);
                            int tmp[2];
                            tmp[0] = vec3->x;
                            tmp[1] = vec3->y;
                            m_Global_Spaces += " ";
                            InputInt2(m_Global_Spaces.c_str(), tmp);
                            vec3->x = tmp[0];
                            vec3->y = tmp[1];
                        }
                        else if (properties[i].type == typeid(Vector2).name())
                        {
                            Vector2* vec3 = reinterpret_cast<Vector2*>(address + properties[i].offset);
                            float tmp[2];
                            tmp[0] = vec3->x;
                            tmp[1] = vec3->y;
                            m_Global_Spaces += " ";
                            InputFloat2(m_Global_Spaces.c_str(), tmp);
                            vec3->x = tmp[0];
                            vec3->y = tmp[1];
                        }
                        else if (properties[i].type == typeid(Vector3).name())
                        {
                            Vector3* vec3 = reinterpret_cast<Vector3*>(address + properties[i].offset);
                            float tmp[3];
                            tmp[0] = vec3->x;
                            tmp[1] = vec3->y;
                            tmp[2] = vec3->z;
                            m_Global_Spaces += " ";
                            InputFloat3(m_Global_Spaces.c_str(), tmp);
                            vec3->x = tmp[0];
                            vec3->y = tmp[1];
                            vec3->z = tmp[2];
                        }
                        else if (properties[i].type == typeid(float).name())
                        {
                            SameLine();
                            float* tmp = reinterpret_cast<float*>(address + properties[i].offset);
                            m_Global_Spaces += " ";
                            InputFloat(m_Global_Spaces.c_str(), tmp);
                        }
                        else if (properties[i].type == typeid(int).name())
                        {
                            SameLine();
                            int* tmp = reinterpret_cast<int*>(address + properties[i].offset);
                            m_Global_Spaces += " ";
                            InputInt(m_Global_Spaces.c_str(), tmp);
                        }
                        else if (properties[i].type == typeid(bool).name())
                        {
                            SameLine();
                            bool* tmp = reinterpret_cast<bool*>(address + properties[i].offset);
                            m_Global_Spaces += " ";
                            Checkbox(m_Global_Spaces.c_str(), tmp);
                        }
                        else if (properties[i].type == typeid(Vector4).name())
                        {
                            static std::string hashes{"##"};
                            ImVec4 backup_color;
                            static ImVec4 tmp_color;
                            Color4* clr = reinterpret_cast<Color4*>(address + properties[i].offset);

                            float tmp[4];
                            tmp[0] = clr->x;
                            tmp[1] = clr->y;
                            tmp[2] = clr->z;
                            tmp[3] = clr->w;
                            ImVec4 color = ImVec4(tmp[0], tmp[1], tmp[2], tmp[3]);
                            m_Global_Spaces += " ";
                            InputFloat4(m_Global_Spaces.c_str(), tmp, 3);
                            if (IsKeyPressed(KEY_RETURN))
                            {
                                if (tmp[0] != clr->x || tmp[1] != clr->y || tmp[2] != clr->z || tmp[3] != clr->w)
                                {
                                    Color4 newValue(tmp[0], tmp[1], tmp[2], tmp[3]);
                                    *clr = newValue;
                                    TreePop();
                                    return;
                                }
                            }
                            // ImGui::SameLine();
                            bool open_popup = ColorButton(("Current Color" + hashes + properties[i].name).c_str(), color);
                            if (open_popup)
                            {
                                OpenPopup(("ColorPicker" + hashes + properties[i].name).c_str());
                                backup_color = ImVec4(tmp[0], tmp[1], tmp[2], tmp[3]);
                                tmp_color = backup_color;
                            }
                            if (ImGui::BeginPopup(("ColorPicker" + hashes + properties[i].name).c_str()))
                            {
                                ImGui::Text("Color4 Picker");
                                ImGui::Separator();
                                ImGui::ColorPicker4(("##picker" + properties[i].name).c_str(), (float*)&tmp_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
                                if (Input::Instance().GetKeyUp(MOUSE_LEFT))
                                {
                                    if (tmp_color.x != clr->x || tmp_color.y != clr->y || tmp_color.z != clr->z || tmp_color.w != clr->w)
                                    {
                                        Color4 newValue(tmp_color.x, tmp_color.y, tmp_color.z, tmp_color.w);
                                        *clr = newValue;
                                        CloseCurrentPopup();
                                    }
                                }
                                ImGui::SameLine();
                                ImGui::BeginGroup();
                                ImGui::Text("Current");
                                ImGui::ColorButton(("##current" + properties[i].name).c_str(), tmp_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
                                ImGui::Text("Previous");
                                if (ImGui::ColorButton(("##previous" + properties[i].name).c_str(), backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
                                    tmp_color = backup_color;
                                ImGui::EndGroup();
                                ImGui::EndPopup();
                            }
                        }
                        else if (properties[i].type.find("enum") != std::string::npos)
                        {
                            auto enumsFactory = Factory::m_Enums;
                            try
                            {
                                auto enums = enumsFactory->at(properties[i].type)->getEnums();
                                int* selected = reinterpret_cast<int*>(address + properties[i].offset);
                                m_Global_Spaces += " ";
                                Combo(m_Global_Spaces.c_str(), selected, enums_getter, &enums, (int)enums.size());
                            }
                            catch (...)
                            {
                                Text("Enum not in Enum Reflection");
                            }
                        }
                        else if (properties[i].type == typeid(HTexture).name())
                        {
                            HTexture texture = *reinterpret_cast<HTexture*>(address + properties[i].offset);
                            std::string name;
                            int selected = -1;
                            ResourceGUID currId = 0;
                            if (texture.Validate()) currId = (texture)->GetGUID();
                            for (int i = 0; i < m_texture_ids.size(); ++i)
                                if (currId == m_texture_ids[i])
                                {
                                    selected = i;
                                    name = m_texture_names[i];
                                }
                            int tmp = selected;
                            SameLine();
                            Text("%s", name.c_str());
                            m_Global_Spaces += " ";
                            Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_texture_names, (int)m_texture_names.size());
                            if (selected != tmp)
                            {
                                *reinterpret_cast<HTexture*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<Texture>(m_texture_ids[selected]);
                                // ResourceGUID newIndex = m_texture_ids[selected];
                                // auto renderer = currentArchetype->GetComponent<MeshRenderer>();
                                // if (renderer)
                                // {
                                //   if (properties[i].name == "m_DiffuseTexture")
                                //     renderer->SetDiffuseTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
                                //   else if (properties[i].name == "m_NormalTexture")
                                //     renderer->SetNormalTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
                                //   else if (properties[i].name == "m_SpecularTexture")
                                //     renderer->SetSpecularTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
                                // }
                                // else
                                // {
                                //   ParticleEmitter* particleEmitter = currentArchetype->GetComponent<BoxParticleEmitter>();
                                //   if (!particleEmitter) currentArchetype->GetComponent<CircleParticleEmitter>();
                                //   if (particleEmitter)
                                //     particleEmitter->SetTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
                                // }
                            }
                        }
                        else if (properties[i].type == typeid(HMesh).name())
                        {
                            HMesh mesh = *reinterpret_cast<HMesh*>(address + properties[i].offset);
                            ResourceGUID currId = 0;
                            int selected = -1;
                            if (mesh.Validate()) currId = (mesh)->GetGUID();
                            std::string name;
                            for (int i = 0; i < m_mesh_ids.size(); ++i)
                                if (currId == m_mesh_ids[i])
                                {
                                    selected = i;
                                    name = m_mesh_names[i];
                                }
                            int tmp = selected;
                            SameLine();
                            Text("%s", name.c_str());
                            m_Global_Spaces += " ";
                            Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_mesh_names, (int)m_mesh_names.size());
                            if (selected != tmp)
                            {
                                *reinterpret_cast<HMesh*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<Mesh>(m_mesh_ids[selected]);
                                // ResourceGUID newIndex = m_mesh_ids[selected];
                                // auto renderer = currentArchetype->GetComponent<MeshRenderer>();
                                // if (renderer)
                                // {
                                //   renderer->SetMesh(ResourceManager::Instance().GetResource<Mesh>(newIndex));
                                // }
                            }
                        }
                        else if (properties[i].type == typeid(HAnimationSet).name())
                        {
                            HAnimationSet anim = *reinterpret_cast<HAnimationSet*>(address + properties[i].offset);
                            ResourceGUID currId = 0;
                            int selected = -1;
                            if (anim.Validate()) currId = (anim)->GetGUID();
                            std::string name;
                            for (int i = 0; i < m_anim_ids.size(); ++i)
                                if (currId == m_anim_ids[i])
                                {
                                    selected = i;
                                    name = m_anim_names[i];
                                }
                            int tmp = selected;
                            SameLine();
                            Text("%s", name.c_str());
                            m_Global_Spaces += " ";
                            Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_anim_names, (int)m_anim_names.size());
                            if (selected != tmp)
                            {
                                *reinterpret_cast<HAnimationSet*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<AnimationSet>(m_anim_ids[selected]);
                                // ResourceGUID newIndex = m_anim_ids[selected];
                                // auto animator = currentArchetype->GetComponent<MeshAnimator>();
                                // if (animator)
                                // {
                                //   animator->SetAnimationSet(ResourceManager::Instance().GetResource<AnimationSet>(newIndex));
                                // }
                            }
                        }
                        else if (properties[i].type == typeid(HFont).name())
                        {
                            HFont font = *reinterpret_cast<HFont*>(address + properties[i].offset);
                            ResourceGUID currId = 0;
                            int selected = -1;
                            if (font.Validate()) currId = font->GetGUID();
                            std::string name;
                            for (int i = 0; i < m_font_ids.size(); ++i)
                                if (currId == m_font_ids[i])
                                {
                                    selected = i;
                                    name = m_font_names[i];
                                }
                            int tmp = selected;
                            SameLine();
                            Text("%s", name.c_str());
                            m_Global_Spaces += " ";
                            Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_font_names, (int)m_font_names.size());
                            if (selected != tmp) *reinterpret_cast<HFont*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<Font>(m_font_ids[selected]);
                        }
                    }
                    if (comp->GetName() == "AudioEmitter")
                    {
                        auto sound = dynamic_cast<AudioEmitter*>(comp);
                        std::vector<std::string> SoundTracks;
                        auto start = AudioSystem::Instance().GetSoundMap().begin();
                        int currentSound = -1;
                        for (int i = 0; start != AudioSystem::Instance().GetSoundMap().end(); ++start)
                        {
                            if (start->first == dynamic_cast<AudioEmitter*>(comp)->GetSoundName()) currentSound = i;
                            SoundTracks.push_back(start->first);
                            ++i;
                        }
                        int s = currentSound;
                        ListBox("SoundTracks", &currentSound, vector_getter, &SoundTracks, (int)SoundTracks.size(), 4);
                        if (currentSound != s)
                        {
                            std::string newValue = SoundTracks[currentSound];
                            sound->SetAudioClip(newValue);
                        }
                    }
                    Unindent(5.f);
                }
                catch (...)
                {
                    Text("Component %s not in reflection factory!", comp->GetName().c_str());
                }
                TreePop();
            }
            Separator();
            ++i;
        }
    }

    if (BeginPopupModal("ArchetypeName", NULL, ImGuiWindowFlags_NoResize))
    {
        m_Global_Spaces += " ";
        Text("Archetype Name : ");
        SameLine();
        InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
        NewLine();
        InvisibleButton("OKCANCELSPACING1", ImVec2(60, 1));
        SameLine();
        if (Button("OK", ImVec2(120, 0)) || IsKeyPressed(KEY_RETURN))
        {
            if (std::string{} != string)
            {
                auto go = CreateArchetype(string);
                CreateArchetype_Action* act = new CreateArchetype_Action(go, m_UpdatedArchetypes[string]);
                m_Undo.push_back(std::move(act));
                ClearRedo();
                SecureZeroMemory(string, MAX_STRING_LENGTH);
                CloseCurrentPopup();
            }
        }
        SameLine();
        InvisibleButton("OKCANCELSPACING2", ImVec2(120, 1));
        SameLine();
        if (Button("Cancel", ImVec2(120, 0)))
        {
            SecureZeroMemory(string, MAX_STRING_LENGTH);
            CloseCurrentPopup();
        }
        SameLine();
        InvisibleButton("OKCANCELSPACING3", ImVec2(60, 1));
        EndPopup();
    }
}

void Editor::ParentArchetypeInspector(char* address, std::string parent, GameObject* currentArchetype)
{
    if (parent == "IComponent" || parent == "") return;
    if (!Factory::m_Reflection->at(parent)->getParents().empty()) ParentArchetypeInspector(address, Factory::m_Reflection->at(parent)->getParents().back().key, currentArchetype);
    auto properties = Factory::m_Reflection->at(parent)->getProperties();
    Indent(5.f);
    if (TreeNode((std::string("Parent ") + std::string(" : ") + parent).c_str()))
    {
        for (int i = 0; i < properties.size(); ++i)
        {
            Text("%s : ", properties[i].name.c_str());
            // if else for each type
            if (properties[i].type == typeid(std::string).name())
            {
                char tmp[MAX_STRING_LENGTH];
                SecureZeroMemory(tmp, MAX_STRING_LENGTH);
                SameLine();
                m_Global_Spaces += " ";
                std::string* string = reinterpret_cast<std::string*>(address + properties[i].offset);
                std::copy(string->begin(), string->end(), tmp);
                InputText(m_Global_Spaces.c_str(), tmp, MAX_STRING_LENGTH);
                if (IsKeyPressed(KEY_RETURN) && *string != tmp) *string = std::string(tmp);
            }
            else if (properties[i].type == typeid(iVector2).name())
            {
                iVector2* vec3 = reinterpret_cast<iVector2*>(address + properties[i].offset);
                int tmp[2];
                tmp[0] = vec3->x;
                tmp[1] = vec3->y;
                m_Global_Spaces += " ";
                InputInt2(m_Global_Spaces.c_str(), tmp);
                vec3->x = tmp[0];
                vec3->y = tmp[1];
            }
            else if (properties[i].type == typeid(Vector2).name())
            {
                Vector2* vec3 = reinterpret_cast<Vector2*>(address + properties[i].offset);
                float tmp[2];
                tmp[0] = vec3->x;
                tmp[1] = vec3->y;
                m_Global_Spaces += " ";
                InputFloat2(m_Global_Spaces.c_str(), tmp);
                vec3->x = tmp[0];
                vec3->y = tmp[1];
            }
            else if (properties[i].type == typeid(Vector3).name())
            {
                Vector3* vec3 = reinterpret_cast<Vector3*>(address + properties[i].offset);
                float tmp[3];
                tmp[0] = vec3->x;
                tmp[1] = vec3->y;
                tmp[2] = vec3->z;
                m_Global_Spaces += " ";
                InputFloat3(m_Global_Spaces.c_str(), tmp);
                vec3->x = tmp[0];
                vec3->y = tmp[1];
                vec3->z = tmp[2];
            }
            else if (properties[i].type == typeid(float).name())
            {
                SameLine();
                float* tmp = reinterpret_cast<float*>(address + properties[i].offset);
                m_Global_Spaces += " ";
                InputFloat(m_Global_Spaces.c_str(), tmp);
            }
            else if (properties[i].type == typeid(int).name())
            {
                SameLine();
                int* tmp = reinterpret_cast<int*>(address + properties[i].offset);
                m_Global_Spaces += " ";
                InputInt(m_Global_Spaces.c_str(), tmp);
            }
            else if (properties[i].type == typeid(bool).name())
            {
                SameLine();
                bool* tmp = reinterpret_cast<bool*>(address + properties[i].offset);
                m_Global_Spaces += " ";
                Checkbox(m_Global_Spaces.c_str(), tmp);
            }
            else if (properties[i].type == typeid(Vector4).name())
            {
                static std::string hashes{"##"};
                ImVec4 backup_color;
                static ImVec4 tmp_color;
                Color4* clr = reinterpret_cast<Color4*>(address + properties[i].offset);

                float tmp[4];
                tmp[0] = clr->x;
                tmp[1] = clr->y;
                tmp[2] = clr->z;
                tmp[3] = clr->w;
                ImVec4 color = ImVec4(tmp[0], tmp[1], tmp[2], tmp[3]);
                m_Global_Spaces += " ";
                InputFloat4(m_Global_Spaces.c_str(), tmp, 3);
                if (IsKeyPressed(KEY_RETURN))
                {
                    if (tmp[0] != clr->x || tmp[1] != clr->y || tmp[2] != clr->z || tmp[3] != clr->w)
                    {
                        Color4 newValue(tmp[0], tmp[1], tmp[2], tmp[3]);
                        *clr = newValue;
                        TreePop();
                        return;
                    }
                }
                // ImGui::SameLine();
                bool open_popup = ColorButton(("Current Color" + hashes + properties[i].name).c_str(), color);
                if (open_popup)
                {
                    OpenPopup(("ColorPicker" + hashes + properties[i].name).c_str());
                    backup_color = ImVec4(tmp[0], tmp[1], tmp[2], tmp[3]);
                    tmp_color = backup_color;
                }
                if (ImGui::BeginPopup(("ColorPicker" + hashes + properties[i].name).c_str()))
                {
                    ImGui::Text("Color4 Picker");
                    ImGui::Separator();
                    ImGui::ColorPicker4(("##picker" + properties[i].name).c_str(), (float*)&tmp_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
                    if (Input::Instance().GetKeyUp(MOUSE_LEFT))
                    {
                        if (tmp_color.x != clr->x || tmp_color.y != clr->y || tmp_color.z != clr->z || tmp_color.w != clr->w)
                        {
                            Color4 newValue(tmp_color.x, tmp_color.y, tmp_color.z, tmp_color.w);
                            *clr = newValue;
                            CloseCurrentPopup();
                        }
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::Text("Current");
                    ImGui::ColorButton(("##current" + properties[i].name).c_str(), tmp_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
                    ImGui::Text("Previous");
                    if (ImGui::ColorButton(("##previous" + properties[i].name).c_str(), backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
                        tmp_color = backup_color;
                    ImGui::EndGroup();
                    ImGui::EndPopup();
                }
            }
            else if (properties[i].type.find("enum") != std::string::npos)
            {
                auto enumsFactory = Factory::m_Enums;
                try
                {
                    auto enums = enumsFactory->at(properties[i].type)->getEnums();
                    int* selected = reinterpret_cast<int*>(address + properties[i].offset);
                    m_Global_Spaces += " ";
                    Combo(m_Global_Spaces.c_str(), selected, enums_getter, &enums, (int)enums.size());
                }
                catch (...)
                {
                    Text("Enum not in Enum Reflection");
                }
            }
            else if (properties[i].type == typeid(HTexture).name())
            {
                HTexture texture = *reinterpret_cast<HTexture*>(address + properties[i].offset);
                std::string name;
                int selected = -1;
                ResourceGUID currId = 0;
                if (texture.Validate()) currId = (texture)->GetGUID();
                for (int i = 0; i < m_texture_ids.size(); ++i)
                    if (currId == m_texture_ids[i])
                    {
                        selected = i;
                        name = m_texture_names[i];
                    }
                int tmp = selected;
                SameLine();
                Text("%s", name.c_str());
                m_Global_Spaces += " ";
                Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_texture_names, (int)m_texture_names.size());
                if (selected != tmp)
                {
                    *reinterpret_cast<HTexture*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<Texture>(m_texture_ids[selected]);
                    // ResourceGUID newIndex = m_texture_ids[selected];
                    // auto renderer = currentArchetype->GetComponent<MeshRenderer>();
                    // if (renderer && parent == "MeshRenderer")
                    // {
                    //   if (properties[i].name == "m_DiffuseTexture")
                    //     renderer->SetDiffuseTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
                    //   else if (properties[i].name == "m_NormalTexture")
                    //     renderer->SetNormalTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
                    //   else if (properties[i].name == "m_SpecularTexture")
                    //     renderer->SetSpecularTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
                    // }
                    // else if (parent == "ParticleEmitter")
                    // {
                    //   *reinterpret_cast<HTexture*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<Texture>(m_texture_ids[selected]);
                    //   // if (properties[i].name == "m_texture")
                    //   // {
                    //   //   ParticleEmitter* particleEmitter = currentArchetype->GetComponent<BoxParticleEmitter>();
                    //   //   if (!particleEmitter) particleEmitter = currentArchetype->GetComponent<CircleParticleEmitter>();
                    //   //   if (particleEmitter)
                    //   //     particleEmitter->SetTexture(ResourceManager::Instance().GetResource<Texture>(newIndex));
                    //   // }
                    // }
                }
            }
        }
        TreePop();
    }
    Unindent(5.f);
}

void Editor::ParentInspector(char* address, std::string comp, std::string parent)
{
    if (parent == "IComponent" || parent == "") return;
    if (!Factory::m_Reflection->at(parent)->getParents().empty()) ParentInspector(address, comp, Factory::m_Reflection->at(parent)->getParents().back().key);
    auto properties = Factory::m_Reflection->at(parent)->getProperties();
    // Indent(5.f);
    // if (TreeNode((std::string("Parent ") + std::string(" : ") + parent).c_str()))
    // {
    for (auto& prop : properties)
    {
        Text("%s : ", prop.name.c_str());
        if (prop.type == typeid(std::string).name())
        {
            SameLine();
            ParentStructOptions(reinterpret_cast<std::string*>(address + prop.offset), m_CurrentObject, comp);
        }
        else if (prop.type == typeid(float).name())
        {
            SameLine();
            ParentStructOptions(reinterpret_cast<float*>(address + prop.offset), m_CurrentObject, comp);
        }
        else if (prop.type == typeid(Vector3).name())
            ParentStructOptions(reinterpret_cast<Vector3*>(address + prop.offset), m_CurrentObject, comp);
        else if (prop.type == typeid(Vector2).name())
            ParentStructOptions(reinterpret_cast<Vector2*>(address + prop.offset), m_CurrentObject, comp);
        else if (prop.type == typeid(TVector2<int>).name())
            ParentStructOptions(reinterpret_cast<TVector2<int>*>(address + prop.offset), m_CurrentObject, comp);
        else if (prop.type == typeid(float).name())
        {
            SameLine();
            ParentStructOptions(reinterpret_cast<float*>(address + prop.offset), m_CurrentObject, comp);
        }
        else if (prop.type == typeid(int).name())
        {
            SameLine();
            ParentStructOptions(reinterpret_cast<int*>(address + prop.offset), m_CurrentObject, comp);
        }
        else if (prop.type == typeid(bool).name())
        {
            SameLine();
            ParentStructOptions(reinterpret_cast<bool*>(address + prop.offset), m_CurrentObject, comp);
        }
        else if (prop.type == typeid(Color4).name())
        {
            SameLine();
            ParentStructOptions(reinterpret_cast<Color4*>(address + prop.offset), m_CurrentObject, comp, prop.name);
        }
        else if (prop.type.find("enum") != std::string::npos)
        {
            ParentStructEnumOptions(reinterpret_cast<int*>(address + prop.offset), prop.type, m_CurrentObject, comp);
        }
        else if (prop.type == typeid(HTexture).name())
        {
            ParentStructOptions(reinterpret_cast<HTexture*>(address + prop.offset), m_CurrentObject, comp);
        }
        else if ((prop.type.find("struct") != std::string::npos || prop.type.find("class") != std::string::npos) && prop.type.find("std::") == std::string::npos)
        {
            StructInspector(address + prop.offset, prop.type);
        }
        if (comp == "BoxParticleEmitter" || comp == "CircleParticleEmitter")
        {
            if (prop.name == "m_affectedByAllAttractor")
            {
                bool b = *reinterpret_cast<bool*>(address + prop.offset);
                if (!b)
                {
                    Text("m_attractors : ");
                    std::vector<unsigned> attractor_id;
                    std::vector<std::string> attractor_name;
                    auto& m_attractor = dynamic_cast<ParticleEmitter*>(reinterpret_cast<IComponent*>(address))->m_Attractors;
                    auto attractors = ParticleSystem::Instance().GetAttractorUpdater()->get();
                    for (auto& a : attractors)
                    {
                        unsigned id = a->GetOwner()->GetID();
                        auto iterator = std::find(m_attractor.begin(), m_attractor.end(), id);
                        if (iterator == m_attractor.end())
                        {
                            attractor_id.push_back(id);
                            attractor_name.push_back(a->GetOwner()->GetName());
                        }
                    }
                    if (Button("Add Attractor")) OpenPopup("AddAttractorPopUp");
                    if (BeginPopup("AddAttractorPopUp"))
                    {
                        Text("Attractors");
                        Separator();
                        for (int i = 0; i < attractor_name.size(); ++i)
                            if (Selectable(attractor_name[i].c_str()))
                            {
                                AddAttractor_Action* act = new AddAttractor_Action(m_CurrentObject, m_attractor, attractor_id[i]);
                                act->Execute();
                                m_Undo.push_back(std::move(act));
                                ClearRedo();
                            }
                        EndPopup();
                    }
                    for (int i = 0; i < m_attractor.size(); ++i)
                    {
                        Bullet();
                        Text("%s", m_CurrentLayer->GetObjectById(m_attractor[i]));
                        SameLine();
                        if (SmallButton((std::string("Delete") + "##" + std::to_string(i)).c_str()))
                        {
                            DeleteAttractor_Action* act = new DeleteAttractor_Action(m_CurrentObject, m_attractor, m_attractor[i]);
                            act->Execute();
                            m_Undo.push_back(std::move(act));
                            ClearRedo();
                        }
                    }
                }
            }
        }
    }
    // TreePop();
    //}
    // Unindent(5.f);
}

void Editor::StructInspector(char* address, std::string comp)
{
    size_t pos = comp.find(" ");
    std::string type = comp.substr(pos + 1);
    try
    {
        if (TreeNode((std::string("Struct ") + std::string(" : ") + type).c_str()))
        {
            if (!Factory::m_Reflection->at(type)->getParents().empty()) ParentInspector(address, type, Factory::m_Reflection->at(type.c_str())->getParents().back().key);
            auto component = (*Factory::m_Reflection).at(type);
            auto properties = component->getProperties();
            Indent(5.f);
            for (int i = 0; i < properties.size(); ++i)
            {
                Text("%s : ", properties[i].name.c_str());
                // if else for each type
                if (properties[i].type == typeid(std::string).name())
                {
                    SameLine();
                    ParentStructOptions(reinterpret_cast<std::string*>(address + properties[i].offset), m_CurrentObject, type);
                }
                else if (properties[i].type == typeid(Vector3).name())
                    ParentStructOptions(reinterpret_cast<Vector3*>(address + properties[i].offset), m_CurrentObject, type);
                else if (properties[i].type == typeid(Vector2).name())
                    ParentStructOptions(reinterpret_cast<Vector2*>(address + properties[i].offset), m_CurrentObject, comp);
                else if (properties[i].type == typeid(TVector2<int>).name())
                    ParentStructOptions(reinterpret_cast<TVector2<int>*>(address + properties[i].offset), m_CurrentObject, comp);
                else if (properties[i].type == typeid(float).name())
                {
                    SameLine();
                    ParentStructOptions(reinterpret_cast<float*>(address + properties[i].offset), m_CurrentObject, type);
                }
                else if (properties[i].type == typeid(int).name())
                {
                    SameLine();
                    ParentStructOptions(reinterpret_cast<int*>(address + properties[i].offset), m_CurrentObject, type);
                }
                else if (properties[i].type == typeid(bool).name())
                {
                    SameLine();
                    ParentStructOptions(reinterpret_cast<bool*>(address + properties[i].offset), m_CurrentObject, type);
                }
                else if (properties[i].type == typeid(Color4).name())
                {
                    SameLine();
                    ParentStructOptions(reinterpret_cast<Color4*>(address + properties[i].offset), m_CurrentObject, type, properties[i].name);
                }
                else if (properties[i].type.find("enum") != std::string::npos)
                {
                    ParentStructEnumOptions(reinterpret_cast<int*>(address + properties[i].offset), properties[i].type, m_CurrentObject, type);
                }
                else if ((properties[i].type.find("struct") != std::string::npos || properties[i].type.find("class") != std::string::npos) && properties[i].type.find("std::") == std::string::npos)
                {
                    StructInspector(address + properties[i].offset, properties[i].type);
                }
            }
            Unindent(5.f);
            TreePop();
        }
    }
    catch (...)
    {
        TreePop();
    }
}

void Editor::ClearRedo()
{
    while (!m_Redo.empty())
    {
        Actions* tmp = m_Redo.front();
        m_Redo.pop_front();
        delete tmp;
    }
}

void Editor::TextEditor()
{
    // if (IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) m_editorFocus = true;
    // else m_editorFocus = false;
    // if (!GetCurrentDock()->active) m_editorFocus = false;
    if (Button("New File"))
    {
        ZeroMemory(&m_editorFile, sizeof(m_editorFile));
        SecureZeroMemory(m_text, 2 << 23);
        m_editorFileName.clear();
    }
    SameLine();
    if (Button("Open File"))
    {
        OPENFILENAME ofn;
        ZeroMemory(&m_editorFile, sizeof(m_editorFile));
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = Application::Instance().GetWindowHandle();
        ofn.lpstrFilter = "Lua Files\0*.lua\0All Files\0*.*\0";
        ofn.lpstrFile = m_editorFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFileTitle = (LPSTR) "Load File";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameA(&ofn))
        {
            std::ifstream input(m_editorFile);
            if (input.is_open())
            {
                SecureZeroMemory(m_text, 2 << 23);
                std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
                std::copy(contents.begin(), contents.end(), m_text);
                input.close();
                m_editorFileName = m_editorFile;
                auto pos = m_editorFileName.find_last_of("\\");
                m_editorFileName.erase(0, pos + 1);
            }
        }
    }
    SameLine();
    if (!m_editorFileName.empty())
    {
        if (Button("Save"))
        {
            std::fstream pFile;
            pFile.open(m_editorFile, std::fstream::out);
            pFile << m_text;
            pFile.close();
            std::cout << "File Saved" << std::endl;
        }
        SameLine();
    }
    if (Button("Save File"))
    {
        OPENFILENAME ofn;
        ZeroMemory(&m_editorFile, sizeof(m_editorFile));
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = Application::Instance().GetWindowHandle();
        ofn.lpstrFilter = "Lua Files\0*.lua\0All Files\0*.*\0";
        ofn.lpstrFile = m_editorFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFileTitle = (LPSTR) "Save File";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetSaveFileNameA(&ofn))
        {
            std::string file(m_editorFile);
            auto dir = file.find_last_of('\\');
            auto ext = file.find_first_of('.', dir);
            if (ext == file.npos)
            {
                file += ".lua";
                std::copy(file.begin(), file.end(), m_editorFile);
            }
            std::fstream pFile;
            pFile.open(file, std::fstream::out);
            pFile << m_text;
            pFile.close();
            m_editorFileName = m_editorFile;
            auto pos = m_editorFileName.find_last_of("\\");
            m_editorFileName.erase(0, pos + 1);
        }
    }
    SameLine();
    ImVec2 sz = GetWindowSize();
    ImVec2 winSz = ImVec2((float)Application::Instance().GetWindowWidth(), (float)Application::Instance().GetWindowHeight());
    Text("Filename : %s", m_editorFileName.c_str());
    BeginDockChild("EditorText", ImVec2(-1, -1), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    InputTextMultiline("##source", m_text, IM_ARRAYSIZE(m_text), ImVec2(winSz.x, -1), ImGuiInputTextFlags_AllowTabInput);
    EndDockChild();
}

void Editor::LayerEditor()
{
    if (Button("New Layer"))
    {
        OpenPopup("AddLayer");
    }
    if (BeginPopupModal("AddLayer"))
    {
        SetItemDefaultFocus();
        static char AddLayerName[MAX_STRING_LENGTH];
        Text("New Layer Name : ");
        m_Global_Spaces += " ";
        SameLine();
        InputText(m_Global_Spaces.c_str(), AddLayerName, MAX_STRING_LENGTH);
        std::string name(AddLayerName);
        NewLine();
        InvisibleButton("OKCANCELSPACING1", ImVec2(60, 1));
        SameLine();
        if (Button("OK", ImVec2(120, 0)) && !name.empty())
        {
            CreateLayer_Action* act = new CreateLayer_Action(name, m_CurrentLayer);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            SecureZeroMemory(AddLayerName, MAX_STRING_LENGTH);
            CloseCurrentPopup();
        }
        SameLine();
        InvisibleButton("OKCANCELSPACING2", ImVec2(120, 1));
        SameLine();
        if (Button("Cancel", ImVec2(120, 0)))
        {
            SecureZeroMemory(AddLayerName, MAX_STRING_LENGTH);
            CloseCurrentPopup();
        }
        SameLine();
        InvisibleButton("OKCANCELSPACING3", ImVec2(60, 1));
        EndPopup();
    }
    if (Application::Instance().GetCurrentScene()->GetLayers().size() > 1)
    {
        SameLine();
        if (Button("Delete Layer"))
        {
            DeleteLayer_Action* act = new DeleteLayer_Action(m_CurrentLayer);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            m_CurrentObject = nullptr;
            m_SelectedObjects.clear();
            Renderer::Instance().SetSelectedObjects({0});
            m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayers().back();
            Renderer::Instance().SetCurrentLayer(m_CurrentLayer);
        }
    }
    auto lys = Application::Instance().GetCurrentScene()->GetLayers();
    for (auto& ly : lys)
    {
        if (ly == m_CurrentLayer)
            Selectable(ly->GetName().c_str(), true);
        else if (Selectable(ly->GetName().c_str()))
        {
            m_CurrentLayer = ly;
            Renderer::Instance().SetCurrentLayer(ly);
            m_CurrentObject = nullptr;
            m_SelectedObjects.clear();
            Renderer::Instance().SetSelectedObjects({0});
        }
    }
}

void Editor::Profiler()
{
    float real = 1.f / Application::Instance().GetGameTimer().GetActualFrameTime();
    float rdt = Application::Instance().GetGameTimer().GetActualFrameTime();
    Text("Current Frame Time : %f", real);
    // input sys
    Text("Input System : %f", m_timings[0]);
    static float input_t[60] = {0};
    if (m_timer < 60)
        input_t[m_timer] = (m_timings[0] / rdt) * 100.f;
    else
    {
        memcpy(input_t, input_t + 1, 59 * sizeof(float));
        input_t[59] = (m_timings[0] / rdt) * 100.f;
    }
    m_Global_Spaces += " ";
    PlotLines(m_Global_Spaces.c_str(), input_t, IM_ARRAYSIZE(input_t), 0, "Input", 0, 100, ImVec2(0, 50));
    // physics sys
    Text("Physics System : %f", m_timings[1]);
    static float physics_t[60] = {0};
    if (m_timer < 60)
        physics_t[m_timer] = (m_timings[1] / rdt) * 100.f;
    else
    {
        memcpy(physics_t, physics_t + 1, 59 * sizeof(float));
        physics_t[59] = (m_timings[1] / rdt) * 100.f;
    }
    m_Global_Spaces += " ";
    PlotLines(m_Global_Spaces.c_str(), physics_t, IM_ARRAYSIZE(input_t), 0, "Physics", 0, 100, ImVec2(0, 50));
    // scene sys
    Text("Scene System : %f", m_timings[2]);
    static float Logic_t[60] = {0};
    if (m_timer < 60)
        Logic_t[m_timer] = (m_timings[2] / rdt) * 100.f;
    else
    {
        memcpy(Logic_t, Logic_t + 1, 59 * sizeof(float));
        Logic_t[59] = (m_timings[2] / rdt) * 100.f;
    }
    m_Global_Spaces += " ";
    PlotLines(m_Global_Spaces.c_str(), Logic_t, IM_ARRAYSIZE(input_t), 0, "Logic", 0, 100, ImVec2(0, 50));
    // render sys
    Text("Renderer System : %f", m_timings[3]);
    static float Render_t[60] = {0};
    if (m_timer < 60)
        Render_t[m_timer] = (m_timings[3] / rdt) * 100.f;
    else
    {
        memcpy(Render_t, Render_t + 1, 59 * sizeof(float));
        Render_t[59] = (m_timings[3] / rdt) * 100.f;
    }
    m_Global_Spaces += " ";
    PlotLines(m_Global_Spaces.c_str(), Render_t, IM_ARRAYSIZE(input_t), 0, "Renderer", 0, 100, ImVec2(0, 50));
    // editor sys
    Text("Editor System : %f", m_timings[4]);
    static float Editor_t[60] = {0};
    if (m_timer < 60)
        Editor_t[m_timer] = (m_timings[4] / rdt) * 100.f;
    else
    {
        memcpy(Editor_t, Editor_t + 1, 59 * sizeof(float));
        Editor_t[59] = (m_timings[4] / rdt) * 100.f;
    }
    m_Global_Spaces += " ";
    PlotLines(m_Global_Spaces.c_str(), Editor_t, IM_ARRAYSIZE(input_t), 0, "Editor", 0, 100, ImVec2(0, 50));
    // audio sys
    Text("Audio System : %f", m_timings[5]);
    static float Audio_t[60] = {0};
    if (m_timer < 60)
        Audio_t[m_timer] = (m_timings[5] / rdt) * 100.f;
    else
    {
        memcpy(Audio_t, Audio_t + 1, 59 * sizeof(float));
        Audio_t[59] = (m_timings[5] / rdt) * 100.f;
    }
    m_Global_Spaces += " ";
    PlotLines(m_Global_Spaces.c_str(), Audio_t, IM_ARRAYSIZE(input_t), 0, "Audio", 0, 100, ImVec2(0, 50));
    if (m_timer < 60) ++m_timer;
    // animation sys
    Text("Animation System : %f", m_timings[6]);
    static float Anim_t[60] = {0};
    if (m_timer < 60)
        Anim_t[m_timer] = (m_timings[6] / rdt) * 100.f;
    else
    {
        memcpy(Anim_t, Anim_t + 1, 59 * sizeof(float));
        Anim_t[59] = (m_timings[6] / rdt) * 100.f;
    }
    m_Global_Spaces += " ";
    PlotLines(m_Global_Spaces.c_str(), Anim_t, IM_ARRAYSIZE(input_t), 0, "Anim", 0, 100, ImVec2(0, 50));
    if (m_timer < 60) ++m_timer;

    // Particle System
    Text("Particle System : %f", m_timings[7]);
    static float Part_t[60] = {0};
    if (m_timer < 60)
        Part_t[m_timer] = (m_timings[7] / rdt) * 100.f;
    else
    {
        memcpy(Part_t, Part_t + 1, 59 * sizeof(float));
        Part_t[59] = (m_timings[7] / rdt) * 100.f;
    }
    m_Global_Spaces += " ";
    PlotLines(m_Global_Spaces.c_str(), Part_t, IM_ARRAYSIZE(input_t), 0, "Particle", 0, 100, ImVec2(0, 50));
    if (m_timer < 60) ++m_timer;

    // AI System
    Text("AI System : %f", m_timings[8]);
    static float AI_t[60] = {0};
    if (m_timer < 60)
        AI_t[m_timer] = (m_timings[8] / rdt) * 100.f;
    else
    {
        memcpy(AI_t, AI_t + 1, 59 * sizeof(float));
        AI_t[59] = (m_timings[8] / rdt) * 100.f;
    }
    m_Global_Spaces += " ";
    PlotLines(m_Global_Spaces.c_str(), AI_t, IM_ARRAYSIZE(input_t), 0, "AI", 0, 100, ImVec2(0, 50));
    if (m_timer < 60) ++m_timer;
}

void Editor::FullScreenVP()
{
    if (Input::Instance().GetKeyPressed(KEY_F8)) m_viewportFullScreen = !m_viewportFullScreen;
    SetNextWindowPos(ImVec2(0, 0));
    SetNextWindowSize(ImVec2((float)Application::Instance().GetWindowWidth(), (float)Application::Instance().GetWindowHeight()));
    Begin("FullScreen ViewPort", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    if (Application::Instance().GetGameTimer().IsPlayModePaused())
    {
        if (Button("Play"))
        {
            s_isPlaying = true;
            while (ShowCursor(false) > 0)
                ;
            AudioSystem::Instance().UnPauseAudio();
            Application::Instance().GetGameTimer().SetPlayModePaused(false);
        }
    }
    else
    {
        if (Button("Pause") || Input::Instance().GetKeyPressed(KEY_F6))
        {
            while (ShowCursor(true) < 0)
                ;
            AudioSystem::Instance().PauseAudio();
            Application::Instance().GetGameTimer().SetPlayModePaused(true);
        }
    }
    SameLine();
    if (Button("Stop") || Input::Instance().GetKeyPressed(KEY_F5))
    {
        Application::Instance().GetGameTimer().SetPlayModePaused(false);
        while (ShowCursor(true) < 0)
            ;
        // Show cursor
        // ShowCursor(true);
        s_isPlaying = false;

        Application::Instance().GetGameTimer().SetEditorPaused(true);
        PhysicsSystem::Instance().Close();
        PhysicsSystem::Instance().RepopulateDTree();
        if (!m_SavedLayers.empty())
        {
            Application::Instance().GetCurrentScene()->ClearLayer();
            auto Scene = Application::Instance().GetCurrentScene();
            // PhysicsSystem::s_DynamicsWorld.m_ColDetection.m_BroadPhase.m_Tree.ResetTree();
            for (auto& ly : m_SavedLayers)
            {
                Scene->CreateLayerWithoutCamera(ly->GetName());
                ly->Clone(Scene->GetLayers().back());
                delete ly;
            }
            m_SavedLayers.clear();
            m_CurrentLayer = Scene->GetLayers().back();
            Renderer::Instance().SetCurrentLayer(m_CurrentLayer);
        }
        m_CurrentObject = nullptr;
        m_SelectedObjects.clear();
        Renderer::Instance().SetSelectedObjects({0});
        Renderer::Instance().ChangeCamera(false);
        ParticleSystem::Instance().Reset();
        AISystem::Instance().RevertBase();
    }

    if (IsWindowHovered() && (Application::Instance().GetGameTimer().IsEditorPaused() || Application::Instance().GetGameTimer().IsPlayModePaused()))
    {
        static Vector3 tmp;
        static Vector3 current;
        if ((GetCurrentDock()->status == Floating) || (GetCurrentDock()->status == Docked && GetCurrentDock()->active))
        {
            if (IsKeyPressed(70) && !Input::Instance().GetKeyDown(KEY_LALT) && m_CurrentObject) m_CurrentLayer->GetEditorCamera()->LookAt(m_CurrentObject);
            m_CurrentLayer->GetEditorCamera()->SetUpdate(true);
            m_CurrentLayer->GetEditorCamera()->OnUpdate(1 / 60.f);
            m_CurrentLayer->GetEditorCamera()->SetUpdate(false);
        }
    }
    else
    {
        m_CurrentLayer->GetEditorCamera()->Cancel();
    }

    ImVec2 windowSize = GetWindowSize();
    windowSize.y -= ImGuiStyleVar_FramePadding * 2 + GetFontSize() + 13.f;

    Renderer::Instance().SetWindowSize(iVector2((int)windowSize.x, (int)windowSize.y));
    Renderer::Instance().GetCurrentEditorLayer()->GetEditorCamera()->SetViewportSize(iVector2((int)windowSize.x, (int)windowSize.y));

    Image((ImTextureID)((__int64)Renderer::Instance().GetRenderTexture()), windowSize, ImVec2(0, 1), ImVec2(1, 0));
    End();
}

void Editor::TagsEditor()
{
    static int selected_tag = -1;
    m_Global_Spaces += " ";
    std::vector<std::string> tags;
    for (auto& s : m_tags)
        tags.push_back(s);
    int height = (int)tags.size() > 33 ? 33 : (int)tags.size();
    PushItemWidth(300.f);
    ListBox(m_Global_Spaces.c_str(), &selected_tag, vector_getter, static_cast<void*>(&tags), (int)tags.size(), height);
    PopItemWidth();
    Separator();
    if (selected_tag >= 0)
    {
        Text("Selected Tag : %s", tags[selected_tag].c_str());
        NewLine();
        Text("New Name : ");
        SameLine();
        m_Global_Spaces += " ";
        char newString[MAX_STRING_LENGTH];
        SecureZeroMemory(newString, MAX_STRING_LENGTH);
        InputText(m_Global_Spaces.c_str(), newString, MAX_STRING_LENGTH);
        if (IsKeyPressed(KEY_RETURN))
        {
            std::string s = newString;
            if (!s.empty() && std::find(tags.begin(), tags.end(), s) == tags.end())
            {
                ChangeTag_Action* act = new ChangeTag_Action(tags[selected_tag], s);
                act->Execute();
                m_Undo.push_back(std::move(act));
                ClearRedo();
                selected_tag = -1;
            }
        }
        if (Button("Delete Tag"))
        {
            DeleteTag_Action* act = new DeleteTag_Action(tags[selected_tag]);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            selected_tag = -1;
        }
        Separator();
    }
    // Add tags here

    Text("Tag Name : ");
    SameLine();
    m_Global_Spaces += " ";
    static char string[MAX_STRING_LENGTH];
    InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
    if (Button("Add Tag") || IsKeyPressed(KEY_RETURN))
    {
        std::string s = string;
        SecureZeroMemory(string, MAX_STRING_LENGTH);
        if (!s.empty() && std::find(tags.begin(), tags.end(), s) == tags.end())
        {
            AddTag_Action* act = new AddTag_Action(s);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            selected_tag = -1;
        }
    }
}

void Editor::PhysicsEditor()
{
#define PhysicsWorld PhysicsSystem::Instance().s_PhyWorldSettings
    if (Button("Save Physics World"))
    {
        char filename[MAX_PATH];
        OPENFILENAME ofn;
        ZeroMemory(&filename, sizeof(filename));
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = Application::Instance().GetWindowHandle();
        ofn.lpstrFilter = "XML Files\0*.xml\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFileTitle = (LPSTR) "Save File";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetSaveFileNameA(&ofn))
        {
            std::string file(filename);
            auto dir = file.find_last_of('\\');
            auto ext = file.find_first_of('.', dir);
            if (ext == file.npos) file += ".xml";
            PhysicsWorld.SaveToFile(file);
        }
    }
    SameLine();
    if (Button("Load Physics World"))
    {
        char filename[MAX_PATH];
        OPENFILENAME ofn;
        ZeroMemory(&filename, sizeof(filename));
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = Application::Instance().GetWindowHandle();
        ofn.lpstrFilter = "XML Files\0*.xml\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFileTitle = (LPSTR) "Load File";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameA(&ofn)) PhysicsWorld.LoadFromFile(filename);
    }
    Separator();
    Text("World Name : %s", PhysicsWorld.m_WorldName.c_str());
    Text("Persistant Contact Distance Threshold");
    SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_PersistantContactDistThreshold);

    Text("Friction Coefficient");
    SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultFrictCoefficient);

    Text("Restitution");
    SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultRestitution);

    Text("Restitution Velocity Threshold");
    SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_RestitutionVelThreshold);

    Text("Rolling Resistance");
    SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultRollResis);

    Text("Sleeping Enabled");
    SameLine();
    PhysicsOptionsBool(&PhysicsWorld.m_IsSleepingEnabled);

    Text("Time Before Sleep");
    SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultTimeBeforeSleep);

    Text("Sleep Linear Velocity");
    SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultSleepLinearVel);

    Text("Sleep Angular Velocity");
    SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultSleepAngularVel);

    Text("Gravity");
    PhysicsOptionsVector3(&PhysicsWorld.m_Gravity);

    Text("Velocity Solver Iterations");
    SameLine();
    PhysicsOptionsUint(&PhysicsWorld.m_DefaultVelSolverIteration);

    Text("Position Solver Iterations");
    SameLine();
    PhysicsOptionsUint(&PhysicsWorld.m_DefaultPosSolverIteration);

    Text("Max Convex Contact Manifold");
    PhysicsOptionsUint(&PhysicsWorld.m_MaxConvexContactManifoldCount);

    Text("Max Concave Contact Manifold");
    PhysicsOptionsUint(&PhysicsWorld.m_MaxConcaveContactManifoldCount);

    Text("Contact Manifold Similar Angle");
    PhysicsOptionsFloat(&PhysicsWorld.m_ContactManifoldSimilarAngle);
}

void Editor::ResourceManager()
{
    static bool b = false;
    Checkbox("Debugging Resources", &b);
    Button("Add Resource");
    if (b)
    {
        SameLine();
        if (Button("Clean Resources XML"))
        {
            // Mesh
            std::string path("Resources\\Models");
            unsigned i = 1;

            for (i = 1; i < m_mesh_ids.size(); ++i)
            {
                bool b = false;
                auto resource = ResourceManager::Instance().GetResource<Mesh>(m_mesh_ids[i]);
                if (resource.Validate())
                {
                    std::wstring res = resource->GetFilePath().stem().c_str();
                    for (auto const& entry : filesystem::directory_iterator(path))
                    {
                        auto path = filesystem::path(entry.path());
                        std::wstring file = path.stem().c_str();
                        if (res == file)
                        {
                            b = true;
                            break;
                        }
                    }
                    if (!b)
                    {
                        ResourceManager::Instance().DestroyResource<Mesh>(resource);
                        std::cout << m_mesh_names[i] << std::endl;
                    }
                }
            }

            // Animation Set
            path = "Resources\\Models";

            for (i = 1; i < m_anim_ids.size(); ++i)
            {
                bool b = false;
                auto resource = ResourceManager::Instance().GetResource<AnimationSet>(m_anim_ids[i]);
                if (resource.Validate())
                {
                    std::wstring res = resource->GetFilePath().stem().c_str();
                    for (auto const& entry : filesystem::directory_iterator(path))
                    {
                        auto path = filesystem::path(entry.path());
                        std::wstring file = path.stem().c_str();
                        if (res == file)
                        {
                            b = true;
                            break;
                        }
                    }
                    if (!b)
                    {
                        ResourceManager::Instance().DestroyResource<AnimationSet>(resource);
                        std::cout << m_anim_names[i] << std::endl;
                    }
                }
            }

            // texture
            path = "Resources\\Texture";

            for (i = 1; i < m_texture_ids.size(); ++i)
            {
                bool b = false;
                auto resource = ResourceManager::Instance().GetResource<Texture>(m_texture_ids[i]);
                if (resource.Validate())
                {
                    std::wstring res = resource->GetFilePath().stem().c_str();
                    for (auto const& entry : filesystem::directory_iterator(path))
                    {
                        auto path = filesystem::path(entry.path());
                        std::wstring file = path.stem().c_str();
                        if (res == file)
                        {
                            b = true;
                            break;
                        }
                    }
                    if (!b)
                    {
                        ResourceManager::Instance().DestroyResource<Texture>(resource);
                        std::cout << m_texture_names[i] << std::endl;
                    }
                }
            }

            // Font
            path = "Resources\\Fonts";

            for (i = 1; i < m_font_ids.size(); ++i)
            {
                bool b = false;
                auto resource = ResourceManager::Instance().GetResource<Font>(m_font_ids[i]);
                if (resource.Validate())
                {
                    std::wstring res = resource->GetFilePath().stem().c_str();
                    for (auto const& entry : filesystem::directory_iterator(path))
                    {
                        auto path = filesystem::path(entry.path());
                        std::wstring file = path.stem().c_str();
                        if (res == file)
                        {
                            b = true;
                            break;
                        }
                    }
                    if (!b)
                    {
                        ResourceManager::Instance().DestroyResource<Font>(resource);
                        std::cout << m_font_names[i] << std::endl;
                    }
                }
            }
        }
        SameLine();
        if (Button("Clean Resources Source"))
        {
            HWND handle = Application::Instance().GetWindowHandle();
            std::string message("YOU ARE DELETING ALL THE SOURCE FILES, EVANGELION 3.5 YOU CAN(NOT) UNDO, P.S. SHINJI IS A BITCH");
            int result = MessageBox(handle, LPCSTR(message.c_str()), "WARNING", MB_OKCANCEL);
            if (result == IDOK)
            {
                std::vector<filesystem::path> filesToBeDeleted;
                std::cout << "SELECTED OK" << std::endl;
                // Mesh
                std::string path("Resources\\Models");
                for (auto const& entry : filesystem::directory_iterator(path))
                {
                    auto path = filesystem::path(entry.path());
                    std::wstring file = path.stem().c_str();

                    unsigned i = 1;

                    for (i = 1; i < m_mesh_ids.size(); ++i)
                    {
                        auto resource = ResourceManager::Instance().GetResource<Mesh>(m_mesh_ids[i]);
                        if (resource.Validate())
                        {
                            std::wstring res = resource->GetFilePath().stem().c_str();
                            if (res == file) break;
                        }
                    }

                    if (i != m_mesh_ids.size()) continue;

                    for (i = 1; i < m_anim_ids.size(); ++i)
                    {
                        auto resource = ResourceManager::Instance().GetResource<AnimationSet>(m_anim_ids[i]);
                        if (resource.Validate())
                        {
                            std::wstring res = resource->GetFilePath().stem().c_str();
                            if (res == file) break;
                        }
                    }

                    if (i != m_anim_ids.size()) continue;

                    filesToBeDeleted.push_back(entry.path());
                }

                // Texture
                path = "Resources\\Texture";
                for (auto const& entry : filesystem::directory_iterator(path))
                {
                    auto path = filesystem::path(entry.path());
                    std::wstring file = path.stem().c_str();

                    unsigned i = 1;

                    for (i = 1; i < m_texture_ids.size(); ++i)
                    {
                        auto resource = ResourceManager::Instance().GetResource<Texture>(m_texture_ids[i]);
                        if (resource.Validate())
                        {
                            std::wstring res = resource->GetFilePath().stem().c_str();
                            if (res == file) break;
                        }
                    }

                    if (i != m_texture_ids.size()) continue;

                    filesToBeDeleted.push_back(entry.path());
                }

                // Font
                path = "Resources\\Fonts";
                for (auto const& entry : filesystem::directory_iterator(path))
                {
                    auto path = filesystem::path(entry.path());
                    std::wstring file = path.stem().c_str();

                    unsigned i = 1;

                    for (i = 1; i < m_font_ids.size(); ++i)
                    {
                        auto resource = ResourceManager::Instance().GetResource<Font>(m_font_ids[i]);
                        if (resource.Validate())
                        {
                            std::wstring res = resource->GetFilePath().stem().c_str();
                            if (res == file) break;
                        }
                    }

                    if (i != m_font_ids.size()) continue;

                    filesToBeDeleted.push_back(entry.path());
                }

                for (const auto& entry : filesToBeDeleted)
                {
                    std::cout << entry << std::endl;
                    if (!filesystem::is_directory(entry))
                    {
                        filesystem::remove(entry);
                    }
                    else
                    {
                        filesystem::remove_all(entry);
                    }
                }
            }
        }
    }
    Separator();
    PushFont(m_fontBold);
    Text("Meshes");
    PopFont();
    static int mesh_selected = -1;
    static int mesh_height = static_cast<int>(m_mesh_names.size()) > 6 ? 6 : static_cast<int>(m_mesh_names.size());
    if (Button("Delete Mesh"))
    {
        if (!m_mesh_names[mesh_selected].empty())
        {
            HMesh tmp = ResourceManager::Instance().GetResource<Mesh>(m_mesh_ids[mesh_selected]);
            ResourceManager::Instance().DestroyResource<Mesh>(tmp);
        }
    }
    SameLine();
    Text("Mesh List Box Height : ");
    PushItemWidth(100);
    SameLine();
    InputInt("##meshHeight", &mesh_height);
    PopItemWidth();
    // int mesh_tmp = mesh_selected;
    PushItemWidth(300);
    ListBox("##Mesh", &mesh_selected, vector_getter, &m_mesh_names, (int)m_mesh_names.size(), mesh_height);
    PopItemWidth();
    Separator();

    PushFont(m_fontBold);
    Text("Animations");
    PopFont();
    static int anim_selected = -1;
    static int anim_height = static_cast<int>(m_anim_names.size()) > 6 ? 6 : static_cast<int>(m_anim_names.size());
    if (Button("Delete Animation"))
    {
        if (!m_anim_names[anim_selected].empty())
        {
            HAnimationSet tmp = ResourceManager::Instance().GetResource<AnimationSet>(m_anim_ids[anim_selected]);
            ResourceManager::Instance().DestroyResource<AnimationSet>(tmp);
        }
    }
    SameLine();
    Text("Animation List Box Height : ");
    PushItemWidth(100);
    SameLine();
    InputInt("##animHeight", &anim_height);
    PopItemWidth();
    PushItemWidth(300);
    ListBox("##Animation", &anim_selected, vector_getter, &m_anim_names, (int)m_anim_names.size(), anim_height);
    PopItemWidth();
    Separator();

    PushFont(m_fontBold);
    Text("Textures");
    PopFont();
    static int tex_selected = -1;
    static int tex_height = static_cast<int>(m_texture_names.size()) > 6 ? 6 : static_cast<int>(m_texture_names.size());
    if (Button("Delete Texture"))
    {
        if (!m_texture_names[tex_selected].empty())
        {
            HTexture tmp = ResourceManager::Instance().GetResource<Texture>(m_texture_ids[tex_selected]);
            ResourceManager::Instance().DestroyResource<Texture>(tmp);
        }
    }
    SameLine();
    Text("Texture List Box Height : ");
    PushItemWidth(100);
    SameLine();
    InputInt("##textHeight", &tex_height);
    PopItemWidth();
    PushItemWidth(300);
    ListBox("##Texture", &tex_selected, vector_getter, &m_texture_names, (int)m_texture_names.size(), tex_height);
    PopItemWidth();
    Separator();

    PushFont(m_fontBold);
    Text("Fonts");
    PopFont();
    static int font_selected = -1;
    static int font_height = static_cast<int>(m_font_names.size()) > 6 ? 6 : static_cast<int>(m_font_names.size());
    if (Button("Delete Font"))
    {
        if (!m_font_names[font_selected].empty())
        {
            HFont tmp = ResourceManager::Instance().GetResource<Font>(m_font_ids[font_selected]);
            ResourceManager::Instance().DestroyResource<Font>(tmp);
        }
    }
    SameLine();
    Text("Font List Box Height : ");
    PushItemWidth(100);
    SameLine();
    InputInt("##fontHeight", &font_height);
    PopItemWidth();
    PushItemWidth(300);
    ListBox("##Font", &font_selected, vector_getter, &m_font_names, (int)m_font_names.size(), font_height);
    PopItemWidth();
}

void Editor::PhysicsOptionsFloat(float* f)
{
    float tmp = *f;
    m_Global_Spaces += " ";
    InputFloat(m_Global_Spaces.c_str(), &tmp);
    if (IsKeyPressed(KEY_RETURN) && tmp != *f)
    {
        // Physics Action here
        float oldValue = *f;
        float newValue = tmp;
        PhysicsInput_Action<float>* act = new PhysicsInput_Action<float>(oldValue, newValue, f);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::PhysicsOptionsBool(bool* b)
{
    bool tmp = *b;
    m_Global_Spaces += " ";
    Checkbox(m_Global_Spaces.c_str(), &tmp);
    if (tmp != *b)
    {
        bool oldValue = *b;
        bool newValue = tmp;
        PhysicsInput_Action<bool>* act = new PhysicsInput_Action<bool>(oldValue, newValue, b);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::PhysicsOptionsVector3(Vector3* v)
{
    Vector3 tmp = *v;
    m_Global_Spaces += " ";
    InputFloat3(m_Global_Spaces.c_str(), &tmp.x);
    if (IsKeyPressed(KEY_RETURN))
    {
        if (tmp.x != v->x || tmp.y != v->y || tmp.z != v->z)
        {
            Vector3 newValue = tmp;
            Vector3 oldValue = *v;
            PhysicsInput_Action<Vector3>* act = new PhysicsInput_Action<Vector3>(oldValue, newValue, v);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
}

void Editor::PhysicsOptionsUint(uint* u)
{
    int tmp = (int)*u;
    m_Global_Spaces += " ";
    InputInt(m_Global_Spaces.c_str(), &tmp, 0);
    if (IsKeyPressed(KEY_RETURN) && tmp != (int)*u && tmp >= 0.f)
    {
        uint newValue = (uint)tmp;
        uint oldValue = *u;
        PhysicsInput_Action<uint>* act = new PhysicsInput_Action<uint>(oldValue, newValue, u);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::Options(std::string* s, GameObject* go, std::string c, std::string p)
{
    char string[MAX_STRING_LENGTH];
    SecureZeroMemory(string, MAX_STRING_LENGTH);
    std::copy(s->c_str(), s->c_str() + s->size(), string);
    m_Global_Spaces += " ";
    if (c != "TextRenderer" && p != "m_Text")
        InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
    else
        InputTextMultiline(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
    if (c != "TextRenderer" && p != "m_Text")
    {
        if (IsKeyPressed(KEY_RETURN) && *s != string)
        {
            std::string oldValue = *s;
            std::string newValue = string;
            Input_Action<std::string>* act = new Input_Action<std::string>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
    else
    {
        if (!IsItemActive() && *s != string)
        {
            std::string oldValue = *s;
            std::string newValue = string;
            Input_Action<std::string>* act = new Input_Action<std::string>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
}

void Editor::Options(std::string* s)
{
    char string[MAX_STRING_LENGTH];
    SecureZeroMemory(string, MAX_STRING_LENGTH);
    std::copy(s->c_str(), s->c_str() + s->size(), string);
    m_Global_Spaces += " ";
    InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
    if (IsKeyPressed(KEY_RETURN) && *s != string)
    {
        std::string oldValue = *s;
        std::string newValue = string;
        Name_Action* act = new Name_Action(oldValue, newValue, m_CurrentLayer, m_CurrentObject);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::Options(Vector2* vec2, GameObject* go, std::string c, std::string p)
{
    float tmp[2];
    tmp[0] = vec2->x;
    tmp[1] = vec2->y;
    m_Global_Spaces += " ";
    InputFloat2(m_Global_Spaces.c_str(), tmp);
    if (IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec2->x || tmp[1] != vec2->y)
        {
            Vector2 newValue(tmp[0], tmp[1]);
            Vector2 oldValue = *vec2;
            Input_Action<Vector2>* act = new Input_Action<Vector2>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
}

void Editor::Options(Vector3* vec3, GameObject* go, std::string c, std::string p)
{
    float tmp[3];
    tmp[0] = vec3->x;
    tmp[1] = vec3->y;
    tmp[2] = vec3->z;
    m_Global_Spaces += " ";
    InputFloat3(m_Global_Spaces.c_str(), tmp);
    if (IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec3->x || tmp[1] != vec3->y || tmp[2] != vec3->z)
        {
            Vector3 newValue(tmp[0], tmp[1], tmp[2]);
            Vector3 oldValue = *vec3;
            Input_Action<Vector3>* act = new Input_Action<Vector3>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
}

void Editor::Options(float* f, GameObject* go, std::string c, std::string p)
{
    float tmp = *f;
    m_Global_Spaces += " ";
    InputFloat(m_Global_Spaces.c_str(), &tmp);
    if (IsKeyPressed(KEY_RETURN) && tmp != *f)
    {
        float oldValue = *f;
        float newValue = tmp;
        Input_Action<float>* act = new Input_Action<float>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::Options(int* i, GameObject* go, std::string c, std::string p)
{
    int tmp = *i;
    m_Global_Spaces += " ";
    InputInt(m_Global_Spaces.c_str(), &tmp);
    if (IsKeyPressed(KEY_RETURN) && tmp != *i)
    {
        int oldValue = *i;
        int newValue = tmp;
        Input_Action<int>* act = new Input_Action<int>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::Options(bool* b, GameObject* go, std::string c, std::string p)
{
    bool tmp = *b;
    m_Global_Spaces += " ";
    Checkbox(m_Global_Spaces.c_str(), &tmp);
    if (tmp != *b)
    {
        int oldValue = *b;
        int newValue = tmp;
        Input_Action<bool>* act = new Input_Action<bool>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::Options(Color4* clr, GameObject* go, std::string c, std::string p)
{
    static std::string hashes = "##";
    ImVec4 backup_color;
    static ImVec4 tmp_color;

    float tmp[4];
    tmp[0] = clr->x;
    tmp[1] = clr->y;
    tmp[2] = clr->z;
    tmp[3] = clr->w;
    ImVec4 color = ImVec4(tmp[0], tmp[1], tmp[2], tmp[3]);
    m_Global_Spaces += " ";
    InputFloat4(m_Global_Spaces.c_str(), tmp, 3);
    if (IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != clr->x || tmp[1] != clr->y || tmp[2] != clr->z || tmp[3] != clr->w)
        {
            Color4 newValue(tmp[0], tmp[1], tmp[2], tmp[3]);
            Color4 oldValue = *clr;
            Input_Action<Color4>* act = new Input_Action<Color4>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            return;
        }
    }
    // ImGui::SameLine();
    bool open_popup = ColorButton(("Current Color" + hashes + p).c_str(), color);
    if (open_popup)
    {
        OpenPopup(("ColorPicker" + hashes + p).c_str());
        backup_color = ImVec4(tmp[0], tmp[1], tmp[2], tmp[3]);
        tmp_color = backup_color;
    }
    if (ImGui::BeginPopup(("ColorPicker" + hashes + p).c_str()))
    {
        ImGui::Text("Color4 Picker");
        ImGui::Separator();
        ImGui::ColorPicker4(("##picker" + p).c_str(), (float*)&tmp_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        if (Input::Instance().GetKeyUp(MOUSE_LEFT))
        {
            if (tmp_color.x != clr->x || tmp_color.y != clr->y || tmp_color.z != clr->z || tmp_color.w != clr->w)
            {
                Color4 newValue(tmp_color.x, tmp_color.y, tmp_color.z, tmp_color.w);
                Color4 oldValue = *clr;
                Input_Action<Color4>* act = new Input_Action<Color4>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
                act->Execute();
                m_Undo.push_back(std::move(act));
                ClearRedo();
                CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Current");
        ImGui::ColorButton(("##current" + p).c_str(), tmp_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
        ImGui::Text("Previous");
        if (ImGui::ColorButton(("##previous" + p).c_str(), backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40))) tmp_color = backup_color;
        ImGui::EndGroup();
        ImGui::EndPopup();
    }
}

void Editor::Options(HMesh mesh, GameObject* go, std::string c, std::string p)
{
    std::string name;
    ResourceGUID currId = 0;
    int selected = -1;
    if (mesh.Validate()) currId = mesh->GetGUID();
    for (int i = 0; i < m_mesh_ids.size(); ++i)
        if (currId == m_mesh_ids[i])
        {
            selected = i;
            name = m_mesh_names[i];
        }
    int tmp = selected;
    SameLine();
    Text("%s", name.c_str());
    m_Global_Spaces += " ";
    Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_mesh_names, (int)m_mesh_names.size());
    if (selected != tmp)
    {
        ResourceGUID oldIndex = tmp == -1 ? 0 : m_mesh_ids[tmp];
        ResourceGUID newIndex = m_mesh_ids[selected];
        HMesh_InputAction* act = new HMesh_InputAction(m_CurrentObject, c, m_CurrentLayer, oldIndex, newIndex);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::Options(HTexture texture, GameObject* go, std::string c, std::string p)
{
    std::string name;
    int selected = -1;
    ResourceGUID currId = 0;
    if (texture.Validate()) currId = texture->GetGUID();
    for (int i = 0; i < m_texture_ids.size(); ++i)
        if (currId == m_texture_ids[i])
        {
            selected = i;
            name = m_texture_names[i];
        }
    int tmp = selected;
    SameLine();
    Text("%s", name.c_str());
    m_Global_Spaces += " ";
    Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_texture_names, (int)m_texture_names.size());
    if (selected != tmp)
    {
        ResourceGUID oldIndex = tmp == -1 ? 0 : m_texture_ids[tmp];
        ResourceGUID newIndex = m_texture_ids[selected];
        TextureInput_Action* act = new TextureInput_Action(m_CurrentObject, c, m_CurrentLayer, oldIndex, newIndex, p);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::Options(HAnimationSet anim, GameObject* go, std::string c, std::string p)
{
    std::string name;
    int selected = -1;
    ResourceGUID currId = 0;
    if (anim.Validate()) currId = anim->GetGUID();
    for (int i = 0; i < m_anim_ids.size(); ++i)
        if (currId == m_anim_ids[i])
        {
            selected = i;
            name = m_anim_names[i];
        }
    int tmp = selected;
    SameLine();
    Text("%s", name.c_str());
    m_Global_Spaces += " ";
    Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_anim_names, (int)m_anim_names.size());
    if (selected != tmp)
    {
        ResourceGUID oldIndex = tmp == -1 ? 0 : m_anim_ids[tmp];
        ResourceGUID newIndex = m_anim_ids[selected];
        AnimationInput_Action* act = new AnimationInput_Action(m_CurrentObject, c, m_CurrentLayer, oldIndex, newIndex, p);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::Options(TVector2<int>* vec2, GameObject* go, std::string c, std::string p)
{
    int tmp[2];
    tmp[0] = vec2->x;
    tmp[1] = vec2->y;
    m_Global_Spaces += " ";
    InputInt2(m_Global_Spaces.c_str(), tmp);
    if (IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec2->x || tmp[1] != vec2->y)
        {
            TVector2<int> newValue(tmp[0], tmp[1]);
            TVector2<int> oldValue = *vec2;
            Input_Action<TVector2<int>>* act = new Input_Action<TVector2<int>>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
}

void Editor::Options(HFont font, GameObject* go, std::string c, std::string p)
{
    std::string name;
    int selected = -1;
    ResourceGUID currId = 0;
    if (font.Validate()) currId = font->GetGUID();
    for (int i = 0; i < m_font_ids.size(); ++i)
        if (currId == m_font_ids[i])
        {
            selected = i;
            name = m_font_names[i];
        }
    int tmp = selected;
    SameLine();
    Text("%s", name.c_str());
    m_Global_Spaces += " ";
    Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_font_names, (int)m_font_names.size());
    if (selected != tmp)
    {
        ResourceGUID oldIndex = tmp == -1 ? 0 : m_font_ids[tmp];
        ResourceGUID newIndex = m_font_ids[selected];
        HFont_InputAction* act = new HFont_InputAction(m_CurrentObject, c, m_CurrentLayer, oldIndex, newIndex);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::EnumOptions(int* i, std::string t, GameObject* go, std::string c, std::string p)
{
    auto enumsFactory = Factory::m_Enums;
    try
    {
        auto enums = enumsFactory->at(t)->getEnums();
        int selected = *i;
        int tmp = selected;
        m_Global_Spaces += " ";
        Combo(m_Global_Spaces.c_str(), &tmp, enums_getter, &enums, (int)enums.size());
        if (tmp != selected)
        {
            int oldValue = selected;
            int newValue = tmp;
            Input_Action<int>* act = new Input_Action<int>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
    catch (...)
    {
        Text("Enum not in Enum Reflection");
    }
}

void Editor::ParentStructOptions(std::string* s, GameObject* go, std::string c)
{
    char string[MAX_STRING_LENGTH];
    SecureZeroMemory(string, MAX_STRING_LENGTH);
    std::copy(s->c_str(), s->c_str() + s->size(), string);
    m_Global_Spaces += " ";
    InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
    if (IsKeyPressed(KEY_RETURN) && *s != string)
    {
        ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
        *s = string;
        go->UpdateComponents();
        act->SetNew(go);
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::ParentStructOptions(float* f, GameObject*& go, std::string c)
{
    float tmp = *f;
    m_Global_Spaces += " ";
    InputFloat(m_Global_Spaces.c_str(), &tmp);
    if (IsKeyPressed(KEY_RETURN) && tmp != *f)
    {
        ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
        *f = tmp;
        go->UpdateComponents();
        act->SetNew(go);
        m_Undo.push_back(std::move(act));
        ClearRedo();
        return;
    }
}

void Editor::ParentStructOptions(int* i, GameObject*& go, std::string c)
{
    int tmp = *i;
    m_Global_Spaces += " ";
    InputInt(m_Global_Spaces.c_str(), &tmp);
    if (IsKeyPressed(KEY_RETURN) && tmp != *i)
    {
        ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
        *i = tmp;
        go->UpdateComponents();
        act->SetNew(go);
        m_Undo.push_back(std::move(act));
        ClearRedo();
        return;
    }
}

void Editor::ParentStructOptions(bool* b, GameObject*& go, std::string c)
{
    bool tmp = *b;
    m_Global_Spaces += " ";
    Checkbox(m_Global_Spaces.c_str(), &tmp);
    if (tmp != *b)
    {
        ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
        *b = tmp;
        go->UpdateComponents();
        act->SetNew(go);
        m_Undo.push_back(std::move(act));
        ClearRedo();
        return;
    }
}

void Editor::ParentStructOptions(Vector2* vec2, GameObject*& go, std::string c)
{
    float tmp[2];
    tmp[0] = vec2->x;
    tmp[1] = vec2->y;
    m_Global_Spaces += " ";
    InputFloat2(m_Global_Spaces.c_str(), tmp);
    if (IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec2->x || tmp[1] != vec2->y)
        {
            Vector2 newValue(tmp[0], tmp[1]);
            ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
            *vec2 = newValue;
            go->UpdateComponents();
            act->SetNew(go);
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
}

void Editor::ParentStructOptions(Vector3* vec3, GameObject*& go, std::string c)
{
    float tmp[3];
    tmp[0] = vec3->x;
    tmp[1] = vec3->y;
    tmp[2] = vec3->z;
    m_Global_Spaces += " ";
    InputFloat3(m_Global_Spaces.c_str(), tmp);
    if (IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec3->x || tmp[1] != vec3->y || tmp[2] != vec3->z)
        {
            Vector3 newValue(tmp[0], tmp[1], tmp[2]);
            ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
            *vec3 = newValue;
            go->UpdateComponents();
            act->SetNew(go);
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
}

void Editor::ParentStructOptions(TVector2<int>* vec2, GameObject*& go, std::string c)
{
    int tmp[2];
    tmp[0] = vec2->x;
    tmp[1] = vec2->y;
    m_Global_Spaces += " ";
    InputInt2(m_Global_Spaces.c_str(), tmp);
    if (IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec2->x || tmp[1] != vec2->y)
        {
            TVector2<int> newValue(tmp[0], tmp[1]);
            ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
            *vec2 = newValue;
            go->UpdateComponents();
            act->SetNew(go);
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
}

void Editor::ParentStructOptions(Vector4* clr, GameObject*& go, std::string c, std::string p)
{
    static std::string hashes{"##"};
    ImVec4 backup_color;
    static ImVec4 tmp_color;

    float tmp[4];
    tmp[0] = clr->x;
    tmp[1] = clr->y;
    tmp[2] = clr->z;
    tmp[3] = clr->w;
    ImVec4 color = ImVec4(tmp[0], tmp[1], tmp[2], tmp[3]);
    m_Global_Spaces += " ";
    InputFloat4(m_Global_Spaces.c_str(), tmp, 3);
    if (IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != clr->x || tmp[1] != clr->y || tmp[2] != clr->z || tmp[3] != clr->w)
        {
            ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
            Color4 newValue(tmp[0], tmp[1], tmp[2], tmp[3]);
            *clr = newValue;
            go->UpdateComponents();
            act->SetNew(go);
            m_Undo.push_back(std::move(act));
            ClearRedo();
            return;
        }
    }
    // ImGui::SameLine();
    bool open_popup = ColorButton(("Current Color" + hashes + p).c_str(), color);
    if (open_popup)
    {
        OpenPopup(("ColorPicker" + hashes + p).c_str());
        backup_color = ImVec4(tmp[0], tmp[1], tmp[2], tmp[3]);
        tmp_color = backup_color;
    }
    if (ImGui::BeginPopup(("ColorPicker" + hashes + p).c_str()))
    {
        ImGui::Text("Color4 Picker");
        ImGui::Separator();
        ImGui::ColorPicker4(("##picker" + p).c_str(), (float*)&tmp_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        if (Input::Instance().GetKeyUp(MOUSE_LEFT))
        {
            if (tmp_color.x != clr->x || tmp_color.y != clr->y || tmp_color.z != clr->z || tmp_color.w != clr->w)
            {
                ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
                Color4 newValue(tmp_color.x, tmp_color.y, tmp_color.z, tmp_color.w);
                *clr = newValue;
                go->UpdateComponents();
                act->SetNew(go);
                m_Undo.push_back(std::move(act));
                ClearRedo();
                CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Current");
        ImGui::ColorButton(("##current" + p).c_str(), tmp_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
        ImGui::Text("Previous");
        if (ImGui::ColorButton(("##previous" + p).c_str(), backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40))) tmp_color = backup_color;
        ImGui::EndGroup();
        ImGui::EndPopup();
    }
}

void Editor::ParentStructOptions(HTexture* texture, GameObject*& go, std::string c)
{
    std::string name;
    int selected = -1;
    ResourceGUID currId = 0;
    if (texture->Validate()) currId = (*texture)->GetGUID();
    for (int i = 0; i < m_texture_ids.size(); ++i)
        if (currId == m_texture_ids[i])
        {
            selected = i;
            name = m_texture_names[i];
        }
    int tmp = selected;
    SameLine();
    Text("%s", name.c_str());
    m_Global_Spaces += " ";
    Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_texture_names, (int)m_texture_names.size());
    if (selected != tmp)
    {
        ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
        *texture = ResourceManager::Instance().GetResource<Texture>(m_texture_ids[selected]);
        go->UpdateComponents();
        act->SetNew(go);
        m_Undo.push_back(std::move(act));
        ClearRedo();
        return;
    }
}

void Editor::ParentStructEnumOptions(int* i, std::string t, GameObject*& go, std::string c)
{
    auto enumsFactory = Factory::m_Enums;
    try
    {
        auto enums = enumsFactory->at(t)->getEnums();
        int selected = *i;
        int tmp = selected;
        m_Global_Spaces += " ";
        Combo(m_Global_Spaces.c_str(), &tmp, enums_getter, &enums, (int)enums.size());
        if (tmp != selected)
        {
            ParentStruct_InputAction* act = new ParentStruct_InputAction(go, c, m_CurrentLayer);
            *i = tmp;
            go->UpdateComponents();
            act->SetNew(go);
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
    catch (...)
    {
        Text("Enum not in Enum Reflection");
    }
}

void Editor::Save()
{
    char filename[MAX_PATH];
    OPENFILENAME ofn;
    ZeroMemory(&filename, sizeof(filename));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = Application::Instance().GetWindowHandle();
    ofn.lpstrFilter = "XML Files\0*.xml\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = (LPSTR) "Save File";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetSaveFileNameA(&ofn))
    {
        std::string file(filename);
        auto dir = file.find_last_of('\\');
        auto ext = file.find_first_of('.', dir);
        if (ext == file.npos) file += ".xml";
        Serializer{file}.SaveScene(Application::Instance().GetCurrentScene());
    }
}

void Editor::Load()
{
    char filename[MAX_PATH];
    OPENFILENAME ofn;
    ZeroMemory(&filename, sizeof(filename));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = Application::Instance().GetWindowHandle();
    ofn.lpstrFilter = "XML Files\0*.xml\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = (LPSTR) "Load File";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&ofn))
    {
        m_CurrentObject = nullptr;
        m_SelectedObjects.clear();
        Renderer::Instance().SetSelectedObjects({0});
        Clear_Redo_Undo();
        Serializer{filename}.LoadScene();
    }
    m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayers().back();
    Renderer::Instance().SetCurrentLayer(m_CurrentLayer);
    Renderer::Instance().ChangeCamera(false);
    Application::Instance().GetGameTimer().SetEditorPaused(true);
}

void Editor::New()
{
    if (ImGui::BeginPopupModal("NewScene", 0, ImGuiWindowFlags_NoResize))
    {
        SetItemDefaultFocus();
        static char LayerName[MAX_STRING_LENGTH];
        Text("New Layer Name : ");
        m_Global_Spaces += " ";
        SameLine();
        InputText(m_Global_Spaces.c_str(), LayerName, MAX_STRING_LENGTH);
        std::string name(LayerName);
        NewLine();
        InvisibleButton("OKCANCELSPACING1", ImVec2(60, 1));
        SameLine();
        if (Button("OK", ImVec2(120, 0)) && !name.empty())
        {
            m_CurrentObject = nullptr;
            m_SelectedObjects.clear();
            Renderer::Instance().SetSelectedObjects({0});
            Clear_Redo_Undo();
            Application::Instance().NewScene("Scene");
            Application::Instance().GetCurrentScene()->CreateLayer(name);
            m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayers().back();
            Renderer::Instance().SetCurrentLayer(m_CurrentLayer);
            SecureZeroMemory(LayerName, MAX_STRING_LENGTH);
            CloseCurrentPopup();
        }
        SameLine();
        InvisibleButton("OKCANCELSPACING2", ImVec2(120, 1));
        SameLine();
        if (Button("Cancel", ImVec2(120, 0)))
        {
            SecureZeroMemory(LayerName, MAX_STRING_LENGTH);
            CloseCurrentPopup();
        }
        SameLine();
        InvisibleButton("OKCANCELSPACING3", ImVec2(60, 1));
        EndPopup();
    }
}

void Editor::Duplicate()
{
    if (!m_CurrentLayer) return;
    if (m_SelectedObjects.empty()) return;
    Duplicate_Action* act = new Duplicate_Action(m_CurrentLayer, m_SelectedObjects);
    act->Execute();
    m_Undo.push_back(std::move(act));
    ClearRedo();
}

void Editor::ChildrenDuplicate(GameObject* original, GameObject* Clone)
{
    if (original->GetChildrenObjects().empty()) return;
    ChildrenList m_Childrens;
    ChildrenList correctChildren = original->GetChildrenObjects();
    for (auto& id : original->GetChildrenObjects())
    {
        GameObject* go = m_CurrentLayer->GetObjectById(id);
        GameObject* tmp = m_CurrentLayer->CreateObject(go->GetName());
        go->Clone(tmp);
        m_Childrens.push_back(tmp->GetID());
        tmp->SetParentObject(Clone->GetID());
        ChildrenDuplicate(go, tmp);
    }
    Clone->SetChildrenObjects(m_Childrens);
    original->SetChildrenObjects(correctChildren);
}

void Editor::LoadData()
{
    TX::XMLDocument file;
    if (file.LoadFile((std::string(m_currwdir) + "\\ImGUI_Docks.xml").c_str()) == TX::XMLError::XML_SUCCESS)
    {
        TX::XMLNode* root = file.FirstChild();
        TX::XMLElement* Data = root->FirstChildElement();
        std::string boolean = Data->Attribute("AppConsole");
        m_Console.ActiveWindow = (bool)std::stoi(boolean);
        for (int i = 0; i < NUM_OF_WINDOWS; ++i)
        {
            if (!Data) return;
            if (!Data->Attribute(("Window" + std::to_string(i)).c_str())) break;
            std::string boolean = Data->Attribute(("Window" + std::to_string(i)).c_str());
            m_ActiveWindow[i] = (bool)std::stoi(boolean);
        }
    }
}

void Editor::SaveData()
{
    TX::XMLDocument doc;
    TX::XMLNode* pRoot = doc.NewElement("Activeness");
    doc.InsertEndChild(pRoot);
    TX::XMLElement* Data = doc.NewElement("Windows");
    pRoot->InsertEndChild(Data);
    Data->SetAttribute("AppConsole", (int)m_Console.ActiveWindow);
    for (int i = 0; i < NUM_OF_WINDOWS; ++i)
        Data->SetAttribute(("Window" + std::to_string(i)).c_str(), (int)m_ActiveWindow[i]);
    TX::XMLError result = doc.SaveFile((std::string(m_currwdir) + "\\ImGUI_Docks.xml").c_str());
}

void Editor::SetUndoRedoSize(const unsigned& sz)
{
    m_Redo_Undo_Size = sz;
}

AppConsole& Editor::GetConsole()
{
    return m_Console;
}

void Editor::KeyDown(WPARAM key, bool pressed)
{
    GetIO().KeysDown[key] = pressed;
}

void Editor::CharInput(char c)
{
    GetIO().AddInputCharacter(c);
}

void Editor::MouseScroll(short s)
{
    GetIO().MouseWheel = s;
}

GameObject* Editor::CreateArchetype(const std::string& n)
{
    GameObject* go = new GameObject(nullptr, 0);
    go->SetName(n);
    GameObject* ugo = new GameObject(nullptr, 0);
    ugo->SetName(n);
    m_Archetypes.insert(std::pair<std::string, GameObject*>(n, go));
    m_UpdatedArchetypes.insert(std::pair<std::string, GameObject*>(n, ugo));
    return go;
}

void Editor::RecursionSaveArchetypeParent(const char* name, unsigned char* base, TX::XMLElement* pComponent, TX::XMLDocument& doc)
{
    auto parents = (*Factory::m_Reflection).at(name)->getParents();
    for (auto pare : parents)
    {
        if (!pare.parents.empty()) RecursionSaveArchetypeParent(pare.key.c_str(), base, pComponent, doc);
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
                        RecursionSaveArchetypeStruct(comp, comp_addr, pElem, doc);
                    }
                    catch (...)
                    {
                        std::cout << comp.name << " is a struct/class which is not registered in the Reflection." << std::endl;
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

void Editor::RecursionSaveArchetypeStruct(registration::variant prop, unsigned char* base, TX::XMLElement* pComponent, TX::XMLDocument& doc)
{
    try
    {
        size_t pos = prop.type.find(" ");
        std::string type = prop.type.substr(pos + 1);
        auto reflectionClass = (*Factory::m_Reflection).at(type.c_str());
        auto recursionProp = reflectionClass->getProperties();
        pComponent->SetAttribute("Type", prop.type.c_str());
        RecursionSaveArchetypeParent(type.c_str(), base, pComponent, doc);
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
                        RecursionSaveArchetypeStruct(rProp, rProp_addr, pElem, doc);
                    }
                    catch (...)
                    {
                        std::cout << rProp.name << " is a struct/class which is not registered in the Reflection." << std::endl;
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

void Editor::SerializeArchetypes()
{
    std::string filename = "Archetypes.xml";
    TX::XMLDocument doc;

    TX::XMLNode* pRoot = doc.NewElement("Archetypes");
    doc.InsertEndChild(pRoot);

    for (auto& pair : m_Archetypes)
    {
        GameObject* go = pair.second;
        if (go->GetName() == "EditorCamera") continue;
        TX::XMLElement* pElement = doc.NewElement(go->GetName().c_str());
        pElement->SetAttribute("Tag", go->GetTag().c_str());
        pRoot->InsertEndChild(pElement);
        for (IComponent* comp : go->GetComponentList())
        {
            TX::XMLElement* pComponent = doc.NewElement(comp->GetName().c_str());
            pElement->InsertEndChild(pComponent);
            std::vector<registration::variant> props;
            try
            {
                props = (*Factory::m_Reflection).at(comp->GetName().c_str())->getProperties();
            }
            catch (...)
            {
                std::cout << comp->GetName() << " not in Reflection Registration" << std::endl;
                continue;
            }
            unsigned char* base = reinterpret_cast<unsigned char*>(comp);
            // Get the list of parents here, may do a reccursion call for inheriting and inheriting
            RecursionSaveArchetypeParent(comp->GetName().c_str(), base, pComponent, doc);

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
                            RecursionSaveArchetypeStruct(prop, prop_addr, pElem, doc);
                        }
                        catch (...)
                        {
                            std::cout << prop.name << " is a struct/class which is not registered in the Reflection." << std::endl;
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
    TX::XMLError result = doc.SaveFile(filename.c_str());
    if (result != TX::XMLError::XML_SUCCESS) throw("ERROR");  // Replace with proper throw and assert
    std::cout << "Saving " << filename << " completed!" << std::endl;
}

void Editor::RecursionLoadArchetypeStruct(TX::XMLElement* attribute, unsigned char* base)
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
                RecursionLoadArchetypeParent(sAttribute, base, attribute->Value());
            else if (!sAttribute->NoChildren())
            {
                if (sType == sProperty->type)
                {
                    // std::cout << "struct has a struct" << std::endl;
                    unsigned char* attributePtr = base + sProperty->offset;
                    RecursionLoadArchetypeStruct(sAttribute, attributePtr);
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
                RecursionLoadArchetypeParent(sAttribute, base, type.c_str());
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

void Editor::RecursionLoadArchetypeParent(TX::XMLElement* attribute, unsigned char* base, const char* comp)
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
                    RecursionLoadArchetypeParent(attribute, base, cParent.key.c_str());
                else if (!attribute->NoChildren())
                {
                    if (pType == pProperty->type)
                    {
                        // std::cout << "parent has a struct" << std::endl;
                        unsigned char* attributePtr = base + pProperty->offset;
                        RecursionLoadArchetypeStruct(attribute, attributePtr);
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
                    RecursionLoadArchetypeParent(attribute, base, cParent.key.c_str());
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

void Editor::DeSerializeArchetypes()
{
    std::string filename = "Archetypes.xml";
    TX::XMLDocument file;
    if (file.LoadFile(filename.c_str()) == TX::XMLError::XML_SUCCESS)
    {
        TX::XMLNode* root = file.FirstChild();

        TX::XMLElement* pGo = root->FirstChildElement();
        for (; pGo; pGo = pGo->NextSiblingElement())
        {
            GameObject* Go = new GameObject(nullptr, 0);
            if (pGo->Attribute("Tag"))
            {
                Go->SetTag(pGo->Attribute("Tag"));
            }
            Go->SetName(pGo->Value());
            TX::XMLElement* pComp = pGo->FirstChildElement();
            // For each Component
            for (; pComp; pComp = pComp->NextSiblingElement())
            {
                // Needs a recursion to check all the parent values
                // std::cout << pComp->Value() << std::endl;
                // Can put Factory::m_Factories.at(pComp->Value()) here then try and catch
                unsigned char* base = reinterpret_cast<unsigned char*>((*Factory::m_Factories)[pComp->Value()]->create(Go));
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
                            RecursionLoadArchetypeParent(pAttribute, base, pComp->Value());
                        else if (!pAttribute->NoChildren())
                        {
                            if (pType == cProperty->type)
                            {
                                // std::cout << "base has a struct" << std::endl;
                                unsigned char* attributePtr = base + cProperty->offset;
                                RecursionLoadArchetypeStruct(pAttribute, attributePtr);
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
                            RecursionLoadArchetypeParent(pAttribute, base, pComp->Value());
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
            Go->UpdateComponents();
            GameObject* uGo = new GameObject(nullptr, 0);
            uGo->SetName(Go->GetName());
            Go->Clone(uGo);
            AddArchetype(Go, uGo);
        }
        std::cout << filename << " Loaded!" << std::endl;
    }
    else
        std::cout << "No Archetype File" << std::endl;
}

void Editor::UpdateMeshArray()
{
    m_texture_ids.clear();
    m_texture_names.clear();
    m_mesh_ids.clear();
    m_mesh_names.clear();
    m_anim_ids.clear();
    m_anim_names.clear();
    m_font_ids.clear();
    m_font_names.clear();

    m_font_ids.push_back(0);
    m_font_names.push_back(std::string{});
    m_texture_ids.push_back(0);
    m_texture_names.push_back(std::string{});
    m_mesh_ids.push_back(0);
    m_mesh_names.push_back(std::string{});
    m_anim_ids.push_back(0);
    m_anim_names.push_back(std::string{});

    std::vector<HTexture> textures = ResourceManager::Instance().GetResources<Texture>();
    std::vector<HMesh> meshes = ResourceManager::Instance().GetResources<Mesh>();
    std::vector<HAnimationSet> anims = ResourceManager::Instance().GetResources<AnimationSet>();
    std::vector<HFont> fonts = ResourceManager::Instance().GetResources<Font>();

    std::sort(textures.begin(), textures.end(),
              [](HTexture& first, HTexture& second)
              {
                  std::string tmp_first, tmp_second;
                  for (auto& elem : first->m_Name)
                  {
                      tmp_first += std::toupper(elem);
                  }

                  for (auto& elem : second->m_Name)
                  {
                      tmp_second += std::toupper(elem);
                  }
                  return tmp_first < tmp_second;
              });
    std::sort(meshes.begin(), meshes.end(),
              [](HMesh& first, HMesh& second)
              {
                  std::string tmp_first, tmp_second;
                  for (auto& elem : first->m_Name)
                  {
                      tmp_first += std::toupper(elem);
                  }

                  for (auto& elem : second->m_Name)
                  {
                      tmp_second += std::toupper(elem);
                  }
                  return tmp_first < tmp_second;
              });
    std::sort(anims.begin(), anims.end(),
              [](HAnimationSet& first, HAnimationSet& second)
              {
                  std::string tmp_first, tmp_second;
                  for (auto& elem : first->m_Name)
                  {
                      tmp_first += std::toupper(elem);
                  }

                  for (auto& elem : second->m_Name)
                  {
                      tmp_second += std::toupper(elem);
                  }
                  return tmp_first < tmp_second;
              });
    std::sort(fonts.begin(), fonts.end(),
              [](HFont& first, HFont& second)
              {
                  std::string tmp_first, tmp_second;
                  for (auto& elem : first->m_Name)
                  {
                      tmp_first += std::toupper(elem);
                  }

                  for (auto& elem : second->m_Name)
                  {
                      tmp_second += std::toupper(elem);
                  }
                  return tmp_first < tmp_second;
              });

    for (auto& r : textures)
    {
        m_texture_ids.push_back(r->GetGUID());
        m_texture_names.push_back(r->m_Name);
    }

    for (auto& r : meshes)
    {
        m_mesh_ids.push_back(r->GetGUID());
        m_mesh_names.push_back(r->m_Name);
    }

    for (auto& r : anims)
    {
        m_anim_ids.push_back(r->GetGUID());
        m_anim_names.push_back(r->m_Name);
    }

    for (auto& f : fonts)
    {
        m_font_ids.push_back(f->GetGUID());
        m_font_names.push_back(f->m_Name);
    }
}
void Editor::RecursiveParentAndChildObject(GameObject* go, std::string s, int& selected)
{
    unsigned parent = m_CurrentObject ? m_CurrentObject->GetParentObject() : 0;
    while (parent)
    {
        if (parent == go->GetID())
        {
            SetNextTreeNodeOpen(true);
            break;
        }
        parent = m_CurrentLayer->GetObjectById(parent)->GetParentObject();
    }

    if (TreeNode(s.c_str()))
    {
        if (IsItemClicked())
        {
            m_CurrentObject = go;
            if (Input::Instance().GetKeyDown(KEY_LSHIFT))
            {
                auto it = std::find(m_SelectedObjects.begin(), m_SelectedObjects.end(), m_CurrentObject);
                if (it == m_SelectedObjects.end())
                    m_SelectedObjects.push_back(m_CurrentObject);
                else
                {
                    m_SelectedObjects.erase(it);
                    if (!m_SelectedObjects.empty())
                    {
                        m_CurrentObject = m_SelectedObjects.front();
                        std::vector<unsigned> tmp;
                        for (auto& o : m_SelectedObjects)
                            tmp.push_back(o->GetID());
                        Renderer::Instance().SetSelectedObjects(tmp);
                    }
                    else
                    {
                        Renderer::Instance().SetSelectedObjects({0});
                        m_CurrentObject = nullptr;
                    }
                }
            }
            else
            {
                m_SelectedObjects.clear();
                m_SelectedObjects.push_back(m_CurrentObject);
                Renderer::Instance().SetSelectedObjects({m_CurrentObject->GetID()});
            }
            selected = go->GetID();
        }
        auto childrens = go->GetChildrenObjects();
        auto end = childrens.end();
        for (auto begin = childrens.begin(); begin != end; ++begin)
        {
            auto ly = go->GetParentLayer();
            auto gameObject = ly->GetObjectById(*begin);
            if (!gameObject->GetChildrenObjects().size())
            {
                if (Selectable((gameObject->GetName() + "##" + std::to_string(*begin)).c_str(), selected == *begin))
                {
                    m_CurrentObject = gameObject;
                    if (Input::Instance().GetKeyDown(KEY_LSHIFT))
                    {
                        auto it = std::find(m_SelectedObjects.begin(), m_SelectedObjects.end(), m_CurrentObject);
                        if (it == m_SelectedObjects.end())
                            m_SelectedObjects.push_back(m_CurrentObject);
                        else
                        {
                            m_SelectedObjects.erase(it);
                            if (!m_SelectedObjects.empty())
                                m_CurrentObject = m_SelectedObjects.front();
                            else
                                m_CurrentObject = nullptr;
                        }
                    }
                    else
                    {
                        m_SelectedObjects.clear();
                        m_SelectedObjects.push_back(m_CurrentObject);
                        Renderer::Instance().SetSelectedObjects({m_CurrentObject->GetID()});
                    }
                    selected = *begin;
                }
                if (IsItemHovered() && Input::Instance().GetMousePressed(MOUSE_MID) && !m_selectedOutliner)
                {
                    m_selectedOutliner = gameObject;
                }
                if (Input::Instance().GetKeyUp(MOUSE_MID) && m_selectedOutliner && IsItemHovered())
                {
                    if (m_selectedOutliner != gameObject)
                    {
                        bool tmp = false;
                        if (m_selectedOutliner && m_selectedOutliner->GetIsChildren())
                        {
                            unsigned parent = m_selectedOutliner->GetParentObject();
                            if (parent == gameObject->GetID()) tmp = true;
                            m_CurrentLayer->GetObjectById(parent)->RemoveChild(m_selectedOutliner->GetID());
                            m_selectedOutliner->SetIsChildren(false);
                            m_selectedOutliner->SetParentObject(0);
                        }
                        if (!tmp)
                        {
                            gameObject->AttachChild(m_selectedOutliner->GetID());
                            m_selectedOutliner->SetIsChildren(true);
                            m_selectedOutliner->SetParentObject(gameObject->GetID());
                            m_selectedOutliner = nullptr;
                        }
                    }
                }
            }
            else
            {
                RecursiveParentAndChildObject(gameObject, (gameObject->GetName() + "##" + std::to_string(*begin)), selected);
            }
        }
        TreePop();
    }
    if (IsItemHovered() && Input::Instance().GetMousePressed(MOUSE_MID) && !m_selectedOutliner) m_selectedOutliner = go;
    if (Input::Instance().GetKeyUp(MOUSE_MID) && m_selectedOutliner && IsItemHovered())
    {
        if (m_selectedOutliner != go)
        {
            bool tmp = false;
            if (m_selectedOutliner && m_selectedOutliner->GetIsChildren())
            {
                unsigned parent = m_selectedOutliner->GetParentObject();
                if (go->GetID() == parent) tmp = true;
                m_CurrentLayer->GetObjectById(parent)->RemoveChild(m_selectedOutliner->GetID());
                m_selectedOutliner->SetIsChildren(false);
                m_selectedOutliner->SetParentObject(0);
            }
            if (!tmp)
            {
                go->AttachChild(m_selectedOutliner->GetID());
                m_selectedOutliner->SetIsChildren(true);
                m_selectedOutliner->SetParentObject(go->GetID());
                m_selectedOutliner = nullptr;
            }
        }
    }
}

void Editor::LoadTags()
{
    TX::XMLDocument file;
    if (file.LoadFile((std::string(m_currwdir) + "\\tags.xml").c_str()) == TX::XMLError::XML_SUCCESS)
    {
        TX::XMLNode* root = file.FirstChild();
        TX::XMLElement* Data = root->FirstChildElement();
        auto a = Data->FirstAttribute();
        while (a)
        {
            m_tags.insert(a->Name());
            a = a->Next();
        }
    }
}

void Editor::SaveTags()
{
    TX::XMLDocument doc;
    TX::XMLNode* pRoot = doc.NewElement("Tags");
    doc.InsertEndChild(pRoot);
    TX::XMLElement* Data = doc.NewElement("Name");
    pRoot->InsertEndChild(Data);
    for (auto& s : m_tags)
        Data->SetAttribute(s.c_str(), 1);
    TX::XMLError result = doc.SaveFile((std::string(m_currwdir) + "\\tags.xml").c_str());
}

void Editor::AddArchetype(GameObject* go, GameObject* ugo)
{
    m_Archetypes.insert(std::pair<std::string, GameObject*>(go->GetName(), go));
    m_UpdatedArchetypes.insert(std::pair<std::string, GameObject*>(go->GetName(), ugo));
}

void Editor::RemoveArchetype(GameObject* go, GameObject* ugo)
{
    m_Archetypes.erase(go->GetName());
    m_UpdatedArchetypes.erase(ugo->GetName());
}

GameObject* Editor::RevertArchetype(GameObject* go)
{
    std::string name = go->GetName();
    GameObject* tmp = m_UpdatedArchetypes[name];
    m_UpdatedArchetypes[name] = go;
    return tmp;
}

GameObject* Editor::MakeChangesArchetype(GameObject* go)
{
    std::string name = go->GetName();
    GameObject* tmp = m_Archetypes[name];
    m_Archetypes[name] = go;
    return tmp;
}

GameObject* Editor::GetArchetype(std::string n)
{
    return m_Archetypes[n];
}

GameObject* Editor::CreateArchetypeObject(std::string n)
{
    try
    {
        auto& archetype = m_Archetypes.at(n);
        GameObject* go = m_CurrentLayer->CreateObject(n);
        archetype->Clone(go);
        go->SetArchetype(n);
        return go;
    }
    catch (...)
    {
        return nullptr;
    }
}

GameObject* Editor::LuaCreateArchetypeObject(std::string n, Layer* layer)
{
    try
    {
        auto& archetype = m_Archetypes.at(n);
        GameObject* go = layer->CreateObject(n);
        archetype->Clone(go);
        go->SetArchetype(n);
        return go;
    }
    catch (...)
    {
        return nullptr;
    }
}

void Editor::SetLayer(Layer* ly)
{
    m_CurrentLayer = ly;
    Renderer::Instance().SetCurrentLayer(ly);
}

void Editor::SystemTimer(float input, float phy, float Comps, float renderer, float editor, float audio, float anim, float particle, float ai)
{
    m_timings[0] = input;
    m_timings[1] = phy;
    m_timings[2] = Comps;
    m_timings[3] = renderer;
    m_timings[4] = editor;
    m_timings[5] = audio;
    m_timings[6] = anim;
    m_timings[7] = particle;
    m_timings[8] = ai;
}

void Editor::AddTag(std::string t)
{
    m_tags.insert(t);
}

void Editor::RemoveTag(std::string t)
{
    m_tags.erase(t);
}

void Editor::Outliner()
{
    // if (m_selectedOutliner) std::cout << m_selectedOutliner->GetName() << std::endl;
    // if (m_selectedOutliner) Text("Selected Outliner : %s", m_selectedOutliner->GetName());
    // LayerList ly = Application::Instance().GetCurrentScene()->GetLayers();
    static int selected = -1;
    if (m_CurrentObject)
    {
        selected = m_CurrentObject->GetID();
        SetNextTreeNodeOpen(true);
    }
    int i = 0;
    auto lyStart = m_CurrentLayer;
    if (TreeNode(((lyStart)->GetName() + "##" + std::to_string(i)).c_str()))
    {
        for (auto& go : (lyStart)->GetObjectsList())
        {
            if (go->GetIsChildren()) continue;
            if (!go->GetChildrenObjects().size())
            {
                if (Selectable((go->GetName() + "##" + std::to_string(i)).c_str(), selected == go->GetID()))
                {
                    m_CurrentObject = go;
                    if (Input::Instance().GetKeyDown(KEY_LSHIFT))
                    {
                        auto it = std::find(m_SelectedObjects.begin(), m_SelectedObjects.end(), m_CurrentObject);
                        if (it == m_SelectedObjects.end())
                        {
                            m_SelectedObjects.push_back(m_CurrentObject);
                            std::vector<unsigned> tmp;
                            for (auto& o : m_SelectedObjects)
                                tmp.push_back(o->GetID());
                            Renderer::Instance().SetSelectedObjects(tmp);
                        }
                        else
                        {
                            m_SelectedObjects.erase(it);
                            if (!m_SelectedObjects.empty())
                            {
                                m_CurrentObject = m_SelectedObjects.front();
                                std::vector<unsigned> tmp;
                                for (auto& o : m_SelectedObjects)
                                    tmp.push_back(o->GetID());
                                Renderer::Instance().SetSelectedObjects(tmp);
                            }
                            else
                            {
                                m_CurrentObject = nullptr;
                                Renderer::Instance().SetSelectedObjects({0});
                            }
                        }
                    }
                    else
                    {
                        m_SelectedObjects.clear();
                        m_SelectedObjects.push_back(go);
                        Renderer::Instance().SetSelectedObjects({go->GetID()});
                    }
                    selected = go->GetID();
                }
                if (Input::Instance().GetKeyUp(MOUSE_MID) && m_selectedOutliner && IsItemHovered())
                {
                    if (m_selectedOutliner != go)
                    {
                        bool tmp = false;
                        if (m_selectedOutliner && m_selectedOutliner->GetIsChildren())
                        {
                            unsigned parent = m_selectedOutliner->GetParentObject();
                            if (go->GetID() == parent) tmp = true;
                            m_CurrentLayer->GetObjectById(parent)->RemoveChild(m_selectedOutliner->GetID());
                            m_selectedOutliner->SetIsChildren(false);
                            m_selectedOutliner->SetParentObject(0);
                        }
                        if (!tmp)
                        {
                            go->AttachChild(m_selectedOutliner->GetID());
                            m_selectedOutliner->SetIsChildren(true);
                            m_selectedOutliner->SetParentObject(go->GetID());
                            m_selectedOutliner = nullptr;
                        }
                    }
                }
                if (IsItemHovered() && Input::Instance().GetMousePressed(MOUSE_MID) && !m_selectedOutliner) m_selectedOutliner = go;
            }
            else
            {
                RecursiveParentAndChildObject(go, (go->GetName() + "##" + std::to_string(i)), selected);
            }

            ++i;
        }
        TreePop();
    }
    if (Input::Instance().GetKeyUp(MOUSE_MID))
    {
        if (m_selectedOutliner && m_selectedOutliner->GetIsChildren())
        {
            unsigned parent = m_selectedOutliner->GetParentObject();
            m_CurrentLayer->GetObjectById(parent)->RemoveChild(m_selectedOutliner->GetID());
            m_selectedOutliner->SetIsChildren(false);
            m_selectedOutliner->SetParentObject(0);
        }
        m_selectedOutliner = nullptr;
    }
}

void Editor::Inspector()
{
    if (m_CurrentObject)
    {
        // TODO: Reflection of Components and editable and also the pushing into the
        // deque for undo
        Text("Chinese");
        // Text("Name of Malay : %s", m_CurrentObject->GetName().c_str());
        Text("Name : ");
        // Need to change when we implement the destroy and undo
        Options(const_cast<std::string*>(&m_CurrentObject->GetName()));
        Text("GuID : %d", m_CurrentObject->GetID());
        if (!m_CurrentObject->GetArchetype().empty()) Text("Archetype : %s", m_CurrentObject->GetArchetype().c_str());
        Text("Tag : ");
        ImGui::SameLine();
        if (Selectable(m_CurrentObject->GetTag() == std::string{} ? "Click me to add Tag" : m_CurrentObject->GetTag().c_str())) OpenPopup("TagsPopUp");
        if (BeginPopup("TagsPopUp"))
        {
            Text("Tags");
            ImGui::Separator();
            for (auto& tag : m_tags)
            {
                if (Selectable(tag.c_str()))
                {
                    ObjectTag_Action* act = new ObjectTag_Action(m_CurrentObject->GetTag(), tag, m_CurrentObject);
                    act->Execute();
                    m_Undo.push_back(std::move(act));
                    ClearRedo();
                }
            }
            if (Selectable(""))
            {
                if (m_CurrentObject->GetTag() != "")
                {
                    ObjectTag_Action* act = new ObjectTag_Action(m_CurrentObject->GetTag(), "", m_CurrentObject);
                    act->Execute();
                    m_Undo.push_back(std::move(act));
                    ClearRedo();
                }
            }
            ImGui::EndPopup();
        }
        std::vector<std::string> AddComp;
        std::vector<std::string> DelComp;
        auto list = m_CurrentObject->GetComponentList();
        auto start = (*Factory::m_Factories).begin();
        for (; start != (*Factory::m_Factories).end(); ++start)
        {
            auto comp = list.begin();
            for (; comp != list.end(); ++comp)
            {
                if ((*comp)->GetName() == start->first)
                {
                    DelComp.push_back((*comp)->GetName());
                    break;
                }
            }
            if (comp == list.end())
            {
                if (start->first != "LuaScript") AddComp.push_back(start->first);
            }
        }
        AddComp.push_back("LuaScript");
        Text("Num of Components : %d", list.size());
        Separator();
        if (Button("Add Component")) OpenPopup("AddComponentPopup");
        if (BeginPopup("AddComponentPopup"))
        {
            Text("Components");
            Separator();
            for (int i = 0; i < AddComp.size(); ++i)
                if (Selectable(AddComp[i].c_str()))
                {
                    Add_Component_Action* act = new Add_Component_Action(m_CurrentObject, AddComp[i], m_CurrentLayer);
                    act->Execute();
                    m_Undo.push_back(std::move(act));
                    ClearRedo();
                }
            EndPopup();
        }
        SameLine();
        if (Button("Remove Component")) OpenPopup("RemoveComponentPopup");
        if (BeginPopup("RemoveComponentPopup"))
        {
            Text("Components");
            Separator();
            for (int i = 0; i < DelComp.size(); ++i)
            {
                if (DelComp[i] == "LuaScript")
                {
                    auto scripts = m_CurrentObject->GetScripts();
                    for (size_t j = 0; j < scripts.size(); ++j)
                    {
                        if (Selectable((std::string("Script:") + scripts[j]).c_str()))
                        {
                            RemoveScript_Action* act = new RemoveScript_Action(m_CurrentObject, scripts[j], m_CurrentLayer);
                            act->Execute();
                            m_Undo.push_back(std::move(act));
                            ClearRedo();
                        }
                    }
                }
                else if (Selectable(DelComp[i].c_str()))
                {
                    Remove_Component_Action* act = new Remove_Component_Action(m_CurrentObject, DelComp[i], m_CurrentLayer);
                    act->Execute();
                    m_Undo.push_back(std::move(act));
                    ClearRedo();
                }
            }
            EndPopup();
        }
        SameLine();
        if (Button("Refresh")) m_CurrentObject->UpdateComponents();
        Separator();
        int i = 0;
        for (auto& comp : list)
        {
            if (TreeNode((std::string("Comp ") + std::to_string(i) + std::string(" : ") + comp->GetName()).c_str()))
            {
                char* address = reinterpret_cast<char*>(comp);
                // Text("Component Num : %d", i);
                // Text("Component Name : %s", comp->GetName().c_str());
                try
                {
                    if (!Factory::m_Reflection->at(comp->GetName().c_str())->getParents().empty())
                        ParentInspector(address, comp->GetName(), Factory::m_Reflection->at(comp->GetName().c_str())->getParents().back().key);
                    auto component = (*Factory::m_Reflection).at(comp->GetName().c_str());
                    auto properties = component->getProperties();
                    Indent(5.f);
                    for (int i = 0; i < properties.size(); ++i)
                    {
                        Text("%s : ", properties[i].name.c_str());
                        // if else for each type
                        if (properties[i].type == typeid(std::string).name())
                        {
                            SameLine();
                            Options(reinterpret_cast<std::string*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if (properties[i].type == typeid(Vector3).name())
                            Options(reinterpret_cast<Vector3*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        else if (properties[i].type == typeid(Vector2).name())
                            Options(reinterpret_cast<Vector2*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        else if (properties[i].type == typeid(TVector2<int>).name())
                            Options(reinterpret_cast<TVector2<int>*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        else if (properties[i].type == typeid(Vector4).name())
                            Options(reinterpret_cast<Vector4*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        else if (properties[i].type == typeid(float).name())
                        {
                            SameLine();
                            Options(reinterpret_cast<float*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if (properties[i].type == typeid(int).name())
                        {
                            SameLine();
                            Options(reinterpret_cast<int*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if (properties[i].type == typeid(bool).name())
                        {
                            SameLine();
                            Options(reinterpret_cast<bool*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if (properties[i].type.find("enum") != std::string::npos)
                        {
                            EnumOptions(reinterpret_cast<int*>(address + properties[i].offset), properties[i].type, m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if (properties[i].type == typeid(HMesh).name())
                        {
                            Options(*reinterpret_cast<HMesh*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if (properties[i].type == typeid(HTexture).name())
                        {
                            Options(*reinterpret_cast<HTexture*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if (properties[i].type == typeid(HAnimationSet).name())
                        {
                            Options(*reinterpret_cast<HAnimationSet*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if (properties[i].type == typeid(HFont).name())
                        {
                            Options(*reinterpret_cast<HFont*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if ((properties[i].type.find("struct") != std::string::npos || properties[i].type.find("class") != std::string::npos) &&
                                 properties[i].type.find("std::") == std::string::npos)
                        {
                            StructInspector(address + properties[i].offset, properties[i].type);
                        }
                    }
                    if (comp->GetName() == "AudioEmitter")
                    {
                        auto sound = dynamic_cast<AudioEmitter*>(comp);
                        std::vector<std::string> SoundTracks;
                        auto start = AudioSystem::Instance().GetSoundMap().begin();
                        int currentSound = -1;
                        for (int i = 0; start != AudioSystem::Instance().GetSoundMap().end(); ++start)
                        {
                            if (start->first == dynamic_cast<AudioEmitter*>(comp)->GetSoundName()) currentSound = i;
                            SoundTracks.push_back(start->first);
                            ++i;
                        }
                        int s = currentSound;
                        ListBox("SoundTracks", &currentSound, vector_getter, &SoundTracks, (int)SoundTracks.size(), 4);
                        if (currentSound != s)
                        {
                            if (!sound->IsPlaying())
                                sound->SetAudioClip(SoundTracks[currentSound]);
                            else
                                sound->SetAndPlayAudioClip(SoundTracks[currentSound]);
                            std::string oldValue = s < 0 ? std::string{} : SoundTracks[s];
                            std::string newValue = SoundTracks[currentSound];
                            Input_Action<std::string>* act =
                                new Input_Action<std::string>(oldValue, newValue, m_CurrentObject->GetName(), "AudioEmitter", "audioClipName_", m_CurrentLayer, m_CurrentObject);
                            // act->Execute();
                            m_Undo.push_back(std::move(act));
                            ClearRedo();
                        }
                        std::string p("Play");
                        p += "##";
                        p += std::to_string(m_CurrentObject->GetID());
                        std::string stop("Stop");
                        stop += "##";
                        stop += std::to_string(m_CurrentObject->GetID());
                        if (Button(p.c_str())) sound->Play();
                        SameLine();
                        if (Button(stop.c_str())) sound->Stop();
                    }
                    if (comp->GetName() == "LuaScript")
                    {
                        // struct stat result;
                        static time_t d_m_time = 0;
                        auto script = dynamic_cast<LuaScript*>(comp);
                        auto filename = script->GetScript();
                        // if (stat((ResourceManager::m_ResourcePathScripts + filename).c_str(), &result) == 0)
                        // {
                        //   if (d_m_time != result.st_mtime)
                        //   {
                        //     d_m_time = result.st_mtime;
                        //     script->InitScript();
                        //   }
                        // }
                        auto vars = script->GetVariables();
                        for (auto& var : vars)
                        {
                            if (var.second == "bool")
                            {
                                bool b = script->get<bool>(var.first);
                                bool tmp = b;
                                Text("%s : ", var.first.c_str());
                                m_Global_Spaces += " ";
                                SameLine();
                                Checkbox(m_Global_Spaces.c_str(), &tmp);
                                if (b != tmp)
                                {
                                    int oldValue = b;
                                    int newValue = tmp;
                                    ScriptInput_Action<bool>* act = new ScriptInput_Action<bool>(m_CurrentObject, filename, oldValue, newValue, var.first);
                                    act->Execute();
                                    m_Undo.push_back(std::move(act));
                                    ClearRedo();
                                }
                            }
                            else if (var.second == "float")
                            {
                                float f = script->get<float>(var.first);
                                float tmp = f;
                                Text("%s : ", var.first.c_str());
                                m_Global_Spaces += " ";
                                SameLine();
                                InputFloat(m_Global_Spaces.c_str(), &tmp);
                                if (IsKeyPressed(KEY_RETURN) && f != tmp)
                                {
                                    float oldValue = f;
                                    float newValue = tmp;
                                    ScriptInput_Action<float>* act = new ScriptInput_Action<float>(m_CurrentObject, filename, oldValue, newValue, var.first);
                                    act->Execute();
                                    m_Undo.push_back(std::move(act));
                                    ClearRedo();
                                }
                            }
                            else if (var.second == "int")
                            {
                                int i = script->get<int>(var.first);
                                int tmp = i;
                                Text("%s : ", var.first.c_str());
                                m_Global_Spaces += " ";
                                SameLine();
                                InputInt(m_Global_Spaces.c_str(), &tmp);
                                if (IsKeyPressed(KEY_RETURN) && i != tmp)
                                {
                                    int oldValue = i;
                                    int newValue = tmp;
                                    ScriptInput_Action<int>* act = new ScriptInput_Action<int>(m_CurrentObject, filename, oldValue, newValue, var.first);
                                    act->Execute();
                                    m_Undo.push_back(std::move(act));
                                    ClearRedo();
                                }
                            }
                            else if (var.second == "string")
                            {
                                std::string s = script->get<std::string>(var.first);
                                char string[MAX_STRING_LENGTH];
                                SecureZeroMemory(string, MAX_STRING_LENGTH);
                                std::copy(s.c_str(), s.c_str() + s.size(), string);
                                Text("%s : ", var.first.c_str());
                                m_Global_Spaces += " ";
                                SameLine();
                                InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
                                if (IsKeyPressed(KEY_RETURN) && s != string)
                                {
                                    std::string oldValue = s;
                                    std::string newValue = string;
                                    ScriptInput_Action<std::string>* act = new ScriptInput_Action<std::string>(m_CurrentObject, filename, oldValue, newValue, var.first);
                                    act->Execute();
                                    m_Undo.push_back(std::move(act));
                                    ClearRedo();
                                }
                            }
                        }
                    }
                    Unindent(5.f);
                }
                catch (...)
                {
                    Text("Component %s not in reflection factory!", comp->GetName().c_str());
                }
                TreePop();
            }
            Separator();
            ++i;
        }
    }
}

void Editor::SetEditorMouse()
{
    ImGuiIO& io = GetIO();
    // set up the imgui io
    Vector2 mousePos = GetInstance(Input).GetMousePosition();
    io.MousePos = ImVec2(mousePos.x, io.DisplaySize.y - mousePos.y);
    io.MouseDown[0] = GetInstance(Input).GetKeyDown(MOUSE_LEFT);
    io.MouseDown[1] = GetInstance(Input).GetKeyDown(MOUSE_RIGHT);
    io.MouseClickedPos[0] = ImVec2(mousePos.x, io.DisplaySize.y - mousePos.y);
    io.MouseClickedPos[1] = ImVec2(mousePos.x, io.DisplaySize.y - mousePos.y);
}

Editor::Editor(float x, float y)
    : m_Console{}
    , m_Redo_Undo_Size{20}
    , m_CurrentObject{nullptr}
    , m_Global_Spaces{}
    , m_GlobalIDCounter{0}
    , m_CurrentLayer(nullptr)
    , m_editorFocus(false)
    , m_currwdir(_getcwd(0, 0))
    , m_ActiveWindow()
    , m_timer(0)
    , m_selectedOutliner(nullptr)
    , m_viewportFullScreen(false)
    , m_Local(false)
    , m_GameCameraInVP(false)
{
    CreateContext();
    CreateDockContext();
    ImGuiIO& io = GetIO();
    io.DisplaySize.x = x;
    io.DisplaySize.y = y;
    io.DeltaTime = 1.f / 60.f;
    io.RenderDrawListsFn = ImGuiRenderDrawLists;

    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    unsigned char* pixels;
    int width, height;

    // for (int i = 0; i < MAX_ARCHETYPES; ++i) m_Archetypes.emplace_back(nullptr, i);
    for (int i = 0; i < NUM_OF_WINDOWS; ++i)
        m_ActiveWindow[i] = false;
    LoadData();

    for (int n = 0; n < IM_ARRAYSIZE(m_saved_palette); n++)
    {
        ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f, m_saved_palette[n].x, m_saved_palette[n].y, m_saved_palette[n].z);
        m_saved_palette[n].w = 1.0f;  // Alpha
    }
    SecureZeroMemory(m_text, 2 << 23);
    DeSerializeArchetypes();
    LoadTags();
    m_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\calibri.ttf", 13.f);
    m_fontBold = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\calibrib.ttf", 23.f);
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
}

Editor::~Editor()
{
    SerializeArchetypes();
    SaveData();
    SaveTags();
    Clear_Redo_Undo();
    for (auto& a : m_Archetypes)
        delete a.second;
    for (auto& a : m_UpdatedArchetypes)
        delete a.second;
    // Need to save the archetype
    DestroyDockContext();
    DestroyContext();
    free(m_currwdir);
}

void Editor::Update(float dt)
{
    // GetIO().Framerate = dt;
    GetIO().DeltaTime = dt;
    // GetIO().FontGlobalScale = 0.95f;
    // PushFont(m_font);
    // GetIO().FontDefault = m_font;
    // auto & timer = Application::Instance().GetGameTimer();
    // if (!timer.IsEditorPaused() && timer.IsPlayModePaused())
    //	ShowCursor(true);
    // if (timer.IsEditorPaused())
    //	ShowCursor(true);

    // If not in play mode
    if (!s_isPlaying)
    {
        // ShowCursor(true);
        // std::cout << "EDITOR NOT PLAYING" << std::endl;
        // HCURSOR cursor = LoadCursor(Application::Instance().GetAppInstance(), IDC_ARROW);
        // SetCursor(cursor);
        // SetClassLongPtr(Application::Instance().GetWindowHandle(), GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor));
        //
        // s_lockMousePosition = false;
    }

    if (s_lockMousePosition)
    {
        // float  minusy     = GetWindowPos().y      ;
        // float  minusx     = GetWindowPos().x      ;
        // ImVec2 windowSize = ImGui::GetWindowSize();
        //
        //// Get center position of current camera
        // int cx = Application::Instance().GetCx() / 2;
        // int cy = Application::Instance().GetCy() / 2;
        // cx -= Application::Instance().GetWindowWidth() / 2;
        // cy -= Application::Instance().GetWindowHeight() / 2;
        //
        // if (!m_viewportFullScreen)
        //  SetCursorPos(cx + (int)minusx + (int)windowSize.x / 2, cy + (int)minusy + (int)windowSize.y / 2);
        // else
        //  SetCursorPos(cx + Application::Instance().GetWindowWidth() / 2, cy + Application::Instance().GetWindowHeight() / 2);
    }

    ImGuiWindowFlags flag = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    if (Input::Instance().GetKeyDown(KEY_LCTRL))
    {
        flag = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
    }
    UpdateMeshArray();
    if (!m_CurrentLayer) m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayers().back();
    NewFrame();
    if (m_ActiveWindow[WindowStates::Viewport_State])
    {
        if (Application::Instance().GetGameTimer().IsEditorPaused() || !m_viewportFullScreen) ImGuizmo::BeginFrame();
    }
    GetIO().DisplaySize.x = (float)Application::Instance().GetWindowWidth();
    GetIO().DisplaySize.y = (float)Application::Instance().GetWindowHeight();
    SetEditorMouse();
    MainMenu();
    ShortcutButtons();
    New();
    StyleEditor();
    Help();
    // if (Input::Instance().GetKeyPressed(KEY_RCTRL))
    // {
    SetNextWindowPos(ImVec2(0, 18.f));
    SetNextWindowSize(ImVec2(0.25f * GetIO().DisplaySize.x, 0.65f * GetIO().DisplaySize.y));
    // }
    Begin("Docking1", NULL, flag);
    InitDockSpace();
    End();

    // if (Input::Instance().GetKeyPressed(KEY_RCTRL))
    // {
    SetNextWindowPos(ImVec2(0.8725f * GetIO().DisplaySize.x, 18.f));
    SetNextWindowSize(ImVec2(0.1275f * GetIO().DisplaySize.x, 0.985f * GetIO().DisplaySize.y));
    // }
    Begin("Docking2", NULL, flag);
    InitDockSpace();
    End();

    // if (Input::Instance().GetKeyPressed(KEY_RCTRL))
    // {
    SetNextWindowPos(ImVec2(0.25f * GetIO().DisplaySize.x, 18.f));
    SetNextWindowSize(ImVec2(0.6225f * GetIO().DisplaySize.x, 0.65f * GetIO().DisplaySize.y));
    // }
    Begin("Docking3", NULL, flag);
    InitDockSpace();
    End();

    // if (Input::Instance().GetKeyPressed(KEY_RCTRL))
    // {
    SetNextWindowPos(ImVec2(0, 0.65f * GetIO().DisplaySize.y + 18.f));
    SetNextWindowSize(ImVec2(0.8725f * GetIO().DisplaySize.x, 0.335f * GetIO().DisplaySize.y));
    // }
    Begin("Docking4", NULL, flag);
    InitDockSpace();
    End();

    if (Application::Instance().GetGameTimer().IsEditorPaused() || !m_viewportFullScreen)
    {
        if (m_ActiveWindow[WindowStates::Viewport_State])
        {
            ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
            if (GetIO().KeyAlt) flag = flag | ImGuiWindowFlags_NoMove;
            BeginDock("ViewPort", &m_ActiveWindow[2], flag);
            Viewport();
            EndDock();
        }
    }
    else
    {
        // FullScreen Viewport here
        FullScreenVP();
    }

    if (m_ActiveWindow[WindowStates::Physics_State])
    {
        ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        BeginDock("Physics", &m_ActiveWindow[WindowStates::Physics_State], flag);
        PhysicsEditor();
        EndDock();
    }

    if (m_ActiveWindow[WindowStates::Outliner_State])
    {
        ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        BeginDock("Outliner", &m_ActiveWindow[0], flag, ImVec2(60, 180));
        Outliner();
        NewLine();
        EndDock();
    }

    if (m_ActiveWindow[WindowStates::Inspector_State])
    {
        ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        BeginDock("Inspector", &m_ActiveWindow[1], flag);
        Inspector();
        NewLine();
        EndDock();
    }

    if (m_ActiveWindow[WindowStates::Archetype_State])
    {
        ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        BeginDock("Archetype", &m_ActiveWindow[3], flag);
        Archetype();
        NewLine();
        EndDock();
    }

    if (m_ActiveWindow[WindowStates::LayerEditor_State])
    {
        ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        BeginDock("LayerEditor", &m_ActiveWindow[WindowStates::LayerEditor_State], flag);
        LayerEditor();
        NewLine();
        EndDock();
    }

    if (m_ActiveWindow[WindowStates::Profiler_State])
    {
        ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        BeginDock("Profiler", &m_ActiveWindow[WindowStates::Profiler_State], flag);
        Profiler();
        EndDock();
    }

    if (m_ActiveWindow[WindowStates::Tags_State])
    {
        ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        BeginDock("Tags", &m_ActiveWindow[WindowStates::Profiler_State], flag);
        TagsEditor();
        EndDock();
    }

    if (m_ActiveWindow[WindowStates::TextEditor_State])
    {
        ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        BeginDock("TextEditor", &m_ActiveWindow[WindowStates::TextEditor_State], flag);
        TextEditor();
        NewLine();
        EndDock();
    }

    if (m_ActiveWindow[WindowStates::Resource_State])
    {
        ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        BeginDock("ResourceManager", &m_ActiveWindow[WindowStates::Resource_State], flag);
        ResourceManager();
        EndDock();
    }

    // Begin("Test Selected Objects");
    // Text("Total number of Selected Objects : %d", m_SelectedObjects.size());
    // Separator();
    // for (auto & obj : m_SelectedObjects)
    // {
    //   if (Selectable(obj->GetName().c_str())) m_CurrentObject = obj;
    //   Text("ID : %d", obj->GetID());
    //   Separator();
    // }
    // End();
    // Text("Mouse Pos : %f, %f", GetIO().MousePos.x, GetIO().MousePos.y);

    // DebugDockSpaces();
    m_Console.Draw();
    Update_Redo_Undo();
    m_Global_Spaces.clear();
    ImGui::ShowDemoWindow();
    // Call PostFrameFunction for all archetypes
    Draw();
    m_DeltaMousePos = GetMousePos();
}

void Editor::Draw()
{
    Render();
}
