#include "Editor.h"

#include <comdef.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>

#include "AudioEmitter.h"
#include "AudioSystem.h"
#include "External Libraries/ImGuizmo/ImGuizmo.h"
#include "External Libraries/imgui/backends/imgui_impl_opengl3.h"
#include "External Libraries/imgui/backends/imgui_impl_win32.h"
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

bool Editor::ms_ShowBoundingVolume = false;
bool Editor::ms_ShowDebug = false;
bool Editor::ms_ShouldLockMousePosition = false;
bool Editor::ms_IsGameRunning = false;

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
    Action* act = m_Undo.back();
    if (dynamic_cast<ActionCreate*>(act))
    {
        m_CurrentObject = nullptr;
        m_SelectedObjects.clear();
        Renderer::Instance().SetSelectedObjects({ 0 });
    }
    if (dynamic_cast<ActionCreateObjectArchetype*>(act))
    {
        m_CurrentObject = nullptr;
        m_SelectedObjects.clear();
        Renderer::Instance().SetSelectedObjects({ 0 });
    }
    m_Undo.pop_back();
    act->Revert();
    m_Redo.push_back(act);
}

void Editor::Redo()
{
    if (m_Redo.empty()) return;
    std::cout << "Redo" << std::endl;
    Action* act = m_Redo.back();
    m_Redo.pop_back();
    act->Execute();
    m_Undo.push_back(act);
}

void Editor::UpdateRedoUndo()
{
    while (m_Redo.size() > m_RedoUndoCount)
    {
        Action* tmp = m_Redo.front();
        m_Redo.pop_front();
        delete tmp;
    }

    while (m_Undo.size() > m_RedoUndoCount)
    {
        Action* tmp = m_Undo.front();
        m_Undo.pop_front();
        delete tmp;
    }
}

void Editor::ClearRedoUndo()
{
    while (!m_Redo.empty())
    {
        Action* tmp = m_Redo.front();
        m_Redo.pop_front();
        delete tmp;
    }
    while (!m_Undo.empty())
    {
        Action* tmp = m_Undo.front();
        m_Undo.pop_front();
        delete tmp;
    }
}

void Editor::DrawMenuBar()
{
    bool b = false;
    bool style = false;
    bool help = false;

    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New", "CTRL+N")) b = true;
        if (ImGui::MenuItem("Save", "CTRL+S")) Save();
        if (ImGui::MenuItem("Load", "CTRL+O")) Load();
        if (ImGui::MenuItem("Exit", "ALT+X")) Application::Instance().QuitProgram();
        ImGui::EndMenu();
    }

    if ((Input::Instance().GetKeyDown(KEY_LALT) || Input::Instance().GetKeyDown(KEY_RALT)) && Input::Instance().GetKeyDown(KEY_X))
    {
        Application::Instance().QuitProgram();
    }

    if (ImGui::BeginMenu("Edit"))
    {
        if (m_Undo.empty())
        {
            ImGui::MenuItem("Undo", "CTRL+Z", false, false);
        }
        else if (ImGui::MenuItem("Undo", "CTRL+Z"))
        {
            Undo();
        }

        if (m_Redo.empty())
        {
            ImGui::MenuItem("Redo", "CTRL+Y", false, false);
        }
        else if (ImGui::MenuItem("Redo", "CTRL+Y"))
        {
            Redo();
        }

        if (m_SelectedObjects.empty())
        {
            ImGui::MenuItem("Duplicate", "CTRL+D", false, false);
        }
        else if (ImGui::MenuItem("Duplicate", "CTRL+D"))
        {
            Duplicate();
        }

        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Windows"))
    {
        if (ImGui::MenuItem("Outliner", NULL, m_WindowStates[(int)EditorWindowType::Outliner], !m_WindowStates[(int)EditorWindowType::Outliner]))
        {
            m_WindowStates[(int)EditorWindowType::Outliner] = true;
        }
        if (ImGui::MenuItem("Inspector", NULL, m_WindowStates[(int)EditorWindowType::Inspector], !m_WindowStates[(int)EditorWindowType::Inspector]))
        {
            m_WindowStates[(int)EditorWindowType::Inspector] = true;
        }
        if (ImGui::MenuItem("Console", NULL, m_Console.m_IsActiveWindow, !m_Console.m_IsActiveWindow))
        {
            m_Console.m_IsActiveWindow = true;
        }
        if (ImGui::MenuItem("Viewport", NULL, m_WindowStates[(int)EditorWindowType::Viewport], !m_WindowStates[(int)EditorWindowType::Viewport]))
        {
            m_WindowStates[(int)EditorWindowType::Viewport] = true;
        }
        if (ImGui::MenuItem("Archetype", NULL, m_WindowStates[(int)EditorWindowType::Archetype], !m_WindowStates[(int)EditorWindowType::Archetype]))
        {
            m_WindowStates[(int)EditorWindowType::Archetype] = true;
        }
        if (ImGui::MenuItem("ImGui::Text Editor", NULL, m_WindowStates[(int)EditorWindowType::TextEditor], !m_WindowStates[(int)EditorWindowType::TextEditor]))
        {
            m_WindowStates[(int)EditorWindowType::TextEditor] = true;
        }
        if (ImGui::MenuItem("Layer Editor", NULL, m_WindowStates[(int)EditorWindowType::LayerEditor], !m_WindowStates[(int)EditorWindowType::LayerEditor]))
        {
            m_WindowStates[(int)EditorWindowType::LayerEditor] = true;
        }
        if (ImGui::MenuItem("Profiler", NULL, m_WindowStates[(int)EditorWindowType::Profiler], !m_WindowStates[(int)EditorWindowType::Profiler]))
        {
            m_WindowStates[(int)EditorWindowType::Profiler] = true;
        }
        if (ImGui::MenuItem("Tags Editor", NULL, m_WindowStates[(int)EditorWindowType::Tags], !m_WindowStates[(int)EditorWindowType::Tags]))
        {
            m_WindowStates[(int)EditorWindowType::Tags] = true;
        }
        if (ImGui::MenuItem("Physics Editor", NULL, m_WindowStates[(int)EditorWindowType::Physics], !m_WindowStates[(int)EditorWindowType::Physics]))
        {
            m_WindowStates[(int)EditorWindowType::Physics] = true;
        }
        if (ImGui::MenuItem("Resource Manager", NULL, m_WindowStates[(int)EditorWindowType::Resource], !m_WindowStates[(int)EditorWindowType::Resource]))
        {
            m_WindowStates[(int)EditorWindowType::Resource] = true;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("GameObject"))
    {
        if (ImGui::MenuItem("Empty Object"))
        {
            ActionCreate* act = new ActionCreate(m_CurrentLayer, "GameObject");
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Options"))
    {
        // TextDisabled("Editor Style");
        // if (ImGui::MenuItem("Style Editor")) style = true;
        ImGui::Separator();
        ImGui::TextDisabled("Viewport");
        if (ImGui::MenuItem("FullScreen VP on Play", "F8", m_IsViewportFullScreen)) m_IsViewportFullScreen = !m_IsViewportFullScreen;
        // if (!m_GameCameraInVP){
        // 	if (ImGui::MenuItem("Enable Game Camera in Viewport")) m_GameCameraInVP = !m_GameCameraInVP;
        // }
        // else {
        // 	if (ImGui::MenuItem("Disable Game Camera in Viewport")) m_GameCameraInVP = !m_GameCameraInVP;
        // }
        ImGui::Separator();
        ImGui::TextDisabled("Object Transformation");
        if (!m_IsTransformInLocalSpace)
        {
            if (ImGui::MenuItem("Local Transform")) m_IsTransformInLocalSpace = true;
        }
        else
        {
            if (ImGui::MenuItem("World Transform")) m_IsTransformInLocalSpace = false;
        }
        ImGui::Separator();
        // TextDisabled("Help and Troubleshoot");
        // if (ImGui::MenuItem("Help", "F1")) help = true;
        ImGui::EndMenu();
    }

    ImGui::Text("Frame time : %f", 1.0 / Application::Instance().GetGameTimer().GetScaledFrameTime());
    ImGui::EndMainMenuBar();

    if (style)
    {
        ImGui::OpenPopup("StyleEditor");
    }
    if (b)
    {
        ImGui::OpenPopup("NewScene");
    }
    if (help)
    {
        ImGui::OpenPopup("HelpScreen");
    }
}

void Editor::ShortcutButtons()
{
    if (m_IsEditorInFocus || !Application::Instance().GetGameTimer().IsEditorPaused())
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    // io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    // io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    // io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    // io.KeySuper = false;

    if (ImGui::IsKeyDown(17))
    {
        if (ImGui::IsKeyPressed('Z'))
        {
            Undo();
        }
        else if (ImGui::IsKeyPressed('Y'))
        {
            Redo();
        }
        else if (ImGui::IsKeyPressed('S'))
        {
            io.KeysDown[17] = false;
            io.KeysDown['S'] = false;

            Save();
        }
        else if (ImGui::IsKeyPressed('O'))
        {
            io.KeysDown[17] = false;
            io.KeysDown['O'] = false;

            Load();
        }
        else if (ImGui::IsKeyPressed('N'))
        {
            ImGui::OpenPopup("NewScene");
        }
    }
    // if (ImGui::IsKeyPressed(112)) ImGui::OpenPopup("HelpScreen");
    if (ImGui::IsKeyPressed(46) && m_CurrentObject && m_CurrentObject->GetName() != "EditorCamera")
    {
        ActionDeleteObject* act = new ActionDeleteObject(m_CurrentObject, m_CurrentLayer);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
        m_CurrentObject = nullptr;
        m_SelectedObjects.clear();

        Renderer::Instance().SetSelectedObjects({ 0 });
    }
}

void Editor::DrawStyleEditor()
{
    if (ImGui::BeginPopupModal("StyleEditor", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
    {
        ImGui::SetItemDefaultFocus();
        ImGui::ShowStyleEditor();
        ImGui::NewLine();

        if (ImGui::Button("Exit"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::NewLine();
        ImGui::EndPopup();
    }
}

void Editor::DrawHelp()
{
    if (ImGui::BeginPopupModal("HelpScreen", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 1.5f, ImVec2(pos.x, pos.y + 5.0f), color, "Welcome to Slimetasia's Troubleshoot/Help Screen.");

        ImGui::NewLine();
        ImGui::NewLine();
        ImGui::Text("Please navigate the table below see the troubleshoot the problem you are currently having");

        if (ImGui::TreeNode("The book of Troubleshoot"))
        {
            ImGui::NewLine();
            ImGui::NewLine();
            ImGui::NewLine();
            ImGui::NewLine();
            ImGui::NewLine();
            draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 7.0f, ImVec2(pos.x + 50.0f, pos.y + 75.0f), IM_COL32(255, 0, 0, 255), "User Error!");
            ImGui::TreePop();
            ImGui::TreePop();
        }

        ImGui::NewLine();

        if (ImGui::Button("Exit"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::NewLine();
        ImGui::EndPopup();
    }
}

void Editor::DrawViewport()
{
    if (Input::Instance().GetKeyPressed(KEY_F8))
    {
        m_IsViewportFullScreen = !m_IsViewportFullScreen;
    }

    static bool deltaGizmoState = false;
    static bool over = false;
    static bool state = 0;

    static ImGuizmo::OPERATION currOperation = (ImGuizmo::OPERATION)3;
    ImGuizmo::SetDrawlist();

    auto editorCamera = m_CurrentLayer->GetEditorCamera();

    const ImVec2 windowSize = ImGui::GetWindowSize();
    const ImVec2 windowOffset = ImGui::GetWindowPos();

    float minusy = ImGui::GetWindowPos().y;
    float minusx = ImGui::GetWindowPos().x;

    if (Application::Instance().GetGameTimer().IsEditorPaused())
    {
        if (ImGui::Button("Play") || ImGui::IsKeyPressed(116))
        {
            std::cout << "============================================================" << std::endl;
            std::cout << "PLAY MODE START" << std::endl;
            std::cout << "============================================================" << std::endl;
            ms_IsGameRunning = true;

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
            if (ImGui::Button("Play"))
            {
                ms_IsGameRunning = true;
                while (ShowCursor(false) > 0)
                    ;
                AudioSystem::Instance().UnPauseAudio();
                Application::Instance().GetGameTimer().SetPlayModePaused(false);
            }
        }
        else
        {
            if (ImGui::Button("Pause") || Input::Instance().GetKeyPressed(KEY_F6))
            {
                ms_IsGameRunning = false;
                while (ShowCursor(true) < 0)
                    ;
                AudioSystem::Instance().PauseAudio();
                Application::Instance().GetGameTimer().SetPlayModePaused(true);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop") || Input::Instance().GetKeyPressed(KEY_F5))
        {
            Application::Instance().GetGameTimer().SetPlayModePaused(false);
            while (ShowCursor(true) < 0)
                ;
            std::cout << "============================================================" << std::endl;
            std::cout << "PLAY MODE ImGui::End" << std::endl;
            std::cout << "============================================================" << std::endl;
            ms_IsGameRunning = false;

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
            Renderer::Instance().SetSelectedObjects({ 0 });
            Renderer::Instance().ChangeCamera(false);
            ParticleSystem::Instance().Reset();
            AISystem::Instance().RevertBase();
        }
    }

    if (ImGui::IsWindowHovered() && (Application::Instance().GetGameTimer().IsEditorPaused() || Application::Instance().GetGameTimer().IsPlayModePaused()))
    {
        static Vector3 tmp;
        static Vector3 current;

        if (ImGui::IsKeyPressed(70) && !Input::Instance().GetKeyDown(KEY_LALT) && m_CurrentObject)
        {
            m_CurrentLayer->GetEditorCamera()->LookAt(m_CurrentObject);
        }

        m_CurrentLayer->GetEditorCamera()->SetUpdate(true);
        m_CurrentLayer->GetEditorCamera()->OnUpdate(1 / 60.0f);
        m_CurrentLayer->GetEditorCamera()->SetUpdate(false);

        float y = abs(ImGui::GetIO().MousePos.y - ImGui::GetWindowSize().y - minusy);

        // std::cout << "Input::Instance().GetKeyPressed(VK_LBUTTON) = " << Input::Instance().GetKeyPressed(VK_LBUTTON) << "\n";
        // std::cout << "!Input::Instance().GetKeyDown(KEY_LALT) = " << !Input::Instance().GetKeyDown(KEY_LALT) << "\n";
        // std::cout << "!ImGuizmo::IsUsing() = " << !ImGuizmo::IsUsing() << "\n";
        // std::cout << "!ImGuizmo::IsOver() = " << !ImGuizmo::IsOver() << "\n";
        //  if (/*IsMouseClicked(0)*/ Input::Instance().GetKeyPressed(VK_LBUTTON) && !Input::Instance().GetKeyDown(KEY_LALT) && !ImGuizmo::IsUsing() && ImGuizmo::IsOver())

        if (state)
        {
            over = ImGuizmo::IsOver();
        }
        else
        {
            over = false;
        }

        // std::cout << over << std::endl;

        if ((ImGui::IsMouseClicked(0) && !Input::Instance().GetKeyDown(KEY_LALT) && !ImGuizmo::IsUsing() && !deltaGizmoState && !over) ||
            (ImGui::IsMouseClicked(0) && !Input::Instance().GetKeyDown(KEY_LALT) && !m_CurrentObject))
        {
            unsigned picked = Renderer::Instance().GetPickedObject(iVector2((int)(ImGui::GetIO().MousePos.x - minusx), (int)y));
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
                            Renderer::Instance().SetSelectedObjects({ 0 });
                            m_CurrentObject = nullptr;
                        }
                    }
                }
                else
                {
                    m_SelectedObjects.clear();
                    m_SelectedObjects.push_back(m_CurrentObject);
                    Renderer::Instance().SetSelectedObjects({ m_CurrentObject->GetID() });
                }
                std::vector<unsigned> selecteds;
                for (auto& obj : m_SelectedObjects)
                    selecteds.push_back(obj->GetID());
                Renderer::Instance().SetSelectedObjects(selecteds);
            }
            else
            {
                Renderer::Instance().SetSelectedObjects({ 0 });
                m_CurrentObject = nullptr;
                m_SelectedObjects.clear();
            }
        }
    }

    Renderer::Instance().SetWindowSize(iVector2((int)windowSize.x, (int)windowSize.y));
    Renderer::Instance().GetCurrentEditorLayer()->GetEditorCamera()->SetViewportSize(iVector2((int)windowSize.x, (int)windowSize.y));

    ImVec2 GameCameraSize = ImVec2((float)Application::Instance().GetWindowHeight(), (float)Application::Instance().GetWindowHeight());
    GameCameraSize.x /= 10.0f;
    GameCameraSize.y /= 10.0f;

    ImGui::Image((ImTextureID)((__int64)Renderer::Instance().GetRenderTexture()), windowSize, ImVec2(0, 1), ImVec2(1, 0));

    if (m_IsGameCameraAcitve)
    {
        ImGui::Image((ImTextureID)((__int64)Renderer::Instance().GetRenderTexture()), GameCameraSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    if (Application::Instance().GetGameTimer().IsEditorPaused())
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

                if (ImGui::IsKeyPressed(81))
                {
                    currOperation = (ImGuizmo::OPERATION)3;
                    state = 0;
                }
                if (ImGui::IsKeyPressed(87))
                {
                    state = 1;
                    currOperation = ImGuizmo::TRANSLATE;
                }
                if (ImGui::IsKeyPressed(69))
                {
                    state = 1;
                    currOperation = ImGuizmo::ROTATE;
                }
                if (ImGui::IsKeyPressed(82))
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
                    case ImGuizmo::TRANSLATE: snap[0] = snap[1] = snap[2] = 1.0f; break;
                    case ImGuizmo::ROTATE: snap[0] = snap[1] = snap[2] = 45.0f; break;
                    case ImGuizmo::SCALE: snap[0] = snap[1] = snap[2] = 1.0f; break;
                }

                ImGuizmo::RecomposeMatrixFromComponents(trans, rot, scale, m);
                if (currOperation != (ImGuizmo::OPERATION)3)
                {
                    if (!m_IsTransformInLocalSpace)
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
                        Vector3 tmpTrans { trans[0], trans[1], trans[2] };
                        Vector3 tmpRot { rot[0], rot[1], rot[2] };
                        Vector3 tmpScale { scale[0], scale[1], scale[2] };
                        t->SetWorldPosition(tmpTrans);
                        t->SetWorldRotation(tmpRot);
                        t->SetWorldScale(tmpScale);
                    }
                    else if (m_SelectedObjects.size() > 1)
                    {
                        Vector3 tmpTrans { trans[0], trans[1], trans[2] };
                        Vector3 tmpRot { rot[0], rot[1], rot[2] };
                        Vector3 tmpScale { scale[0], scale[1], scale[2] };
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
                            ActionInput<Vector3>* act = new ActionInput<Vector3>(oldValue, newValue, m_CurrentObject->GetName(), "Transform", "m_WorldPosition", m_CurrentLayer, m_CurrentObject);
                            act->Execute();
                            m_Undo.push_back(std::move(act));
                            ClearRedo();
                        }
                        else if ((scale[0] != savedScale.x || scale[1] != savedScale.y || scale[2] != savedScale.z) && currOperation == ImGuizmo::SCALE)
                        {
                            Vector3 newValue = Vector3(scale[0], scale[1], scale[2]);
                            Vector3 oldValue = savedScale;
                            ActionInput<Vector3>* act = new ActionInput<Vector3>(oldValue, newValue, m_CurrentObject->GetName(), "Transform", "m_WorldScale", m_CurrentLayer, m_CurrentObject);
                            act->Execute();
                            m_Undo.push_back(std::move(act));
                            ClearRedo();
                        }
                        else if ((rot[0] != savedRot.x || rot[1] != savedRot.y || rot[2] != savedRot.z) && currOperation == ImGuizmo::ROTATE)
                        {
                            Vector3 newValue = Vector3(rot[0], rot[1], rot[2]);
                            Vector3 oldValue = savedRot;
                            ActionInput<Vector3>* act = new ActionInput<Vector3>(oldValue, newValue, m_CurrentObject->GetName(), "Transform", "m_WorldRotation", m_CurrentLayer, m_CurrentObject);
                            act->Execute();
                            m_Undo.push_back(std::move(act));
                            ClearRedo();
                        }
                    }
                    else if (m_SelectedObjects.size() > 1)
                    {
                        Vector3 tmpTrans { trans[0], trans[1], trans[2] };
                        Vector3 tmpRot { rot[0], rot[1], rot[2] };
                        Vector3 tmpScale { scale[0], scale[1], scale[2] };
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
                        if ((m_Trans[0] != 0.0f || m_Trans[1] != 0.0f || m_Trans[2] != 0.0f) && currOperation == ImGuizmo::TRANSLATE)
                        {
                            ActionMultiTransform* act = new ActionMultiTransform(m_Trans, ids, "Transform", "m_WorldPosition", m_CurrentLayer);
                            act->Execute();
                            m_Undo.push_back(std::move(act));
                            ClearRedo();
                        }
                        else if ((m_Scale[0] != 0.0f || m_Scale[1] != 0.0f || m_Scale[2] != 0.0f) && currOperation == ImGuizmo::SCALE)
                        {
                            ActionMultiTransform* act = new ActionMultiTransform(m_Scale, ids, "Transform", "m_WorldScale", m_CurrentLayer);
                            act->Execute();
                            m_Undo.push_back(std::move(act));
                            ClearRedo();
                        }
                        else if ((m_Rot[0] != 0.0f || m_Rot[1] != 0.0f || m_Rot[2] != 0.0f) && currOperation == ImGuizmo::ROTATE)
                        {
                            ActionMultiTransform* act = new ActionMultiTransform(m_Rot, ids, "Transform", "m_WorldRotation", m_CurrentLayer);
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

void Editor::DrawArchetype()
{
    static int selected = -1;
    m_Global_Spaces += " ";
    std::vector<std::string> archetypes;

    for (auto a : m_Archetypes)
    {
        archetypes.push_back(a.first);
    }

    ImGui::PushItemWidth(200.0f);
    ImGui::ListBox(m_Global_Spaces.c_str(), &selected, vector_getter, static_cast<void*>(&archetypes), (int)archetypes.size(), 5);
    ImGui::PopItemWidth();
    static char string[MAX_STRING_LENGTH];
    // SecureZeroMemory(string, MAX_STRING_LENGTH);
    if (ImGui::Button("Create Base Archetype")) ImGui::OpenPopup("ArchetypeName");

    ImGui::Separator();

    if (selected > -1 && m_Archetypes.size())
    {
        GameObject* currentArchetype = m_UpdatedArchetypes[archetypes[selected]];
        ImGui::Text("Archetype Name : %s", currentArchetype->GetName().c_str());
        // ImGui::Text("Archetype ID : %d", currentArchetype->GetID());
        // Add Archetype Inspector here
        // Idea is to have the similar thing to the inspector
        // only save changes and will affect all archetypes, and need clone

        // Add Tags here
        ImGui::Text("Tag:");
        ImGui::SameLine();
        if (ImGui::Selectable(currentArchetype->GetTag() == std::string {} ? "Click me to add Tag" : currentArchetype->GetTag().c_str())) ImGui::OpenPopup("ArcheTagsPopUp");
        if (ImGui::BeginPopup("ArcheTagsPopUp"))
        {
            ImGui::Text("Tags");
            ImGui::Separator();
            for (auto& tag : m_Tags)
            {
                if (ImGui::Selectable(tag.c_str()))
                {
                    currentArchetype->SetTag(tag.c_str());
                }
            }
            if (ImGui::Selectable(""))
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
        ImGui::Text("Num of Components : %d", list.size());
        ImGui::Separator();
        if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponentPopup");
        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            ImGui::Text("Components");
            ImGui::Separator();
            for (int i = 0; i < AddComp.size(); ++i)
                if (ImGui::Selectable(AddComp[i].c_str())) Factory::m_Factories->at(AddComp[i])->create(currentArchetype);
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove Component")) ImGui::OpenPopup("RemoveComponentPopup");
        if (ImGui::BeginPopup("RemoveComponentPopup"))
        {
            ImGui::Text("Components");
            ImGui::Separator();
            for (int i = 0; i < DelComp.size(); ++i)
            {
                if (DelComp[i] == "LuaScript")
                {
                    auto scripts = currentArchetype->GetScripts();
                    for (size_t j = 0; j < scripts.size(); ++j)
                    {
                        if (ImGui::Selectable((std::string("Script:") + scripts[j]).c_str())) currentArchetype->RemoveScript(scripts[j]);
                    }
                }
                else if (ImGui::Selectable(DelComp[i].c_str()))
                    Factory::m_Factories->at(DelComp[i])->remove(currentArchetype);
            }
            ImGui::EndPopup();
        }
        if (ImGui::Button("Revert"))
        {
            GameObject* tmp = new GameObject(nullptr, 0);
            tmp->SetName(currentArchetype->GetName());
            m_Archetypes[archetypes[selected]]->Clone(tmp);
            ActionRevertArchetype* act = new ActionRevertArchetype(tmp);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            return;
        }
        ImGui::SameLine();
        if (ImGui::Button("Make Changes"))
        {
            GameObject* tmp = new GameObject(nullptr, 0);
            tmp->SetName(currentArchetype->GetName());
            currentArchetype->Clone(tmp);
            ActionMakeChanges* act = new ActionMakeChanges(tmp);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            return;
        }
        ImGui::SameLine();
        if (ImGui::Button("Create Archetype"))
        {
            Layer* ly = m_CurrentLayer;
            ActionCreateObjectArchetype* act = new ActionCreateObjectArchetype(ly, currentArchetype->GetName(), currentArchetype->GetName());
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            return;
        }
        if (ImGui::Button("Delete Archetype"))
        {
            ActionDeleteArchetype* act = new ActionDeleteArchetype(m_Archetypes[archetypes[selected]], m_UpdatedArchetypes[archetypes[selected]]);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            selected = -1;
            return;
        }
        ImGui::Separator();
        int i = 0;
        for (auto& comp : list)
        {
            if (ImGui::TreeNode((std::string("Comp ") + std::to_string(i) + std::string(" : ") + comp->GetName()).c_str()))
            {
                char* address = reinterpret_cast<char*>(comp);
                // ImGui::Text("Component Num : %d", i);
                // ImGui::Text("Component Name : %s", comp->GetName().c_str());
                try
                {
                    if (!Factory::m_Reflection->at(comp->GetName())->getParents().empty())
                        ParentArchetypeInspector(address, Factory::m_Reflection->at(comp->GetName())->getParents().back().key, currentArchetype);
                    auto component = (*Factory::m_Reflection).at(comp->GetName().c_str());
                    auto properties = component->getProperties();
                    ImGui::Indent(5.0f);
                    for (int i = 0; i < properties.size(); ++i)
                    {
                        ImGui::Text("%s : ", properties[i].name.c_str());
                        // if else for each type
                        if (properties[i].type == typeid(std::string).name())
                        {
                            char tmp[MAX_STRING_LENGTH];
                            SecureZeroMemory(tmp, MAX_STRING_LENGTH);
                            ImGui::SameLine();
                            m_Global_Spaces += " ";
                            std::string* string = reinterpret_cast<std::string*>(address + properties[i].offset);
                            std::copy(string->begin(), string->end(), tmp);
                            // here for textrenderer
                            if (comp->GetName() == "TextRenderer" && properties[i].name == "m_Text")
                            {
                                ImGui::InputTextMultiline(m_Global_Spaces.c_str(), tmp, MAX_STRING_LENGTH);
                                if (!ImGui::IsItemActive() && *string != tmp) *string = std::string(tmp);
                            }
                            else
                            {
                                ImGui::InputText(m_Global_Spaces.c_str(), tmp, MAX_STRING_LENGTH);
                                if (ImGui::IsKeyPressed(KEY_RETURN) && *string != tmp) *string = std::string(tmp);
                            }
                        }
                        else if (properties[i].type == typeid(iVector2).name())
                        {
                            iVector2* vec3 = reinterpret_cast<iVector2*>(address + properties[i].offset);
                            int tmp[2];
                            tmp[0] = vec3->x;
                            tmp[1] = vec3->y;
                            m_Global_Spaces += " ";
                            ImGui::InputInt2(m_Global_Spaces.c_str(), tmp);
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
                            ImGui::InputFloat2(m_Global_Spaces.c_str(), tmp);
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
                            ImGui::InputFloat3(m_Global_Spaces.c_str(), tmp);
                            vec3->x = tmp[0];
                            vec3->y = tmp[1];
                            vec3->z = tmp[2];
                        }
                        else if (properties[i].type == typeid(float).name())
                        {
                            ImGui::SameLine();
                            float* tmp = reinterpret_cast<float*>(address + properties[i].offset);
                            m_Global_Spaces += " ";
                            ImGui::InputFloat(m_Global_Spaces.c_str(), tmp);
                        }
                        else if (properties[i].type == typeid(int).name())
                        {
                            ImGui::SameLine();
                            int* tmp = reinterpret_cast<int*>(address + properties[i].offset);
                            m_Global_Spaces += " ";
                            ImGui::InputInt(m_Global_Spaces.c_str(), tmp);
                        }
                        else if (properties[i].type == typeid(bool).name())
                        {
                            ImGui::SameLine();
                            bool* tmp = reinterpret_cast<bool*>(address + properties[i].offset);
                            m_Global_Spaces += " ";
                            ImGui::Checkbox(m_Global_Spaces.c_str(), tmp);
                        }
                        else if (properties[i].type == typeid(Vector4).name())
                        {
                            static std::string hashes { "##" };
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
                            ImGui::InputFloat4(m_Global_Spaces.c_str(), tmp);
                            if (ImGui::IsKeyPressed(KEY_RETURN))
                            {
                                if (tmp[0] != clr->x || tmp[1] != clr->y || tmp[2] != clr->z || tmp[3] != clr->w)
                                {
                                    Color4 newValue(tmp[0], tmp[1], tmp[2], tmp[3]);
                                    *clr = newValue;
                                    ImGui::TreePop();
                                    return;
                                }
                            }
                            // ImGui::SameLine();
                            bool open_popup = ImGui::ColorButton(("Current Color" + hashes + properties[i].name).c_str(), color);
                            if (open_popup)
                            {
                                ImGui::OpenPopup(("ColorPicker" + hashes + properties[i].name).c_str());
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
                                        ImGui::CloseCurrentPopup();
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
                                ImGui::Combo(m_Global_Spaces.c_str(), selected, enums_getter, &enums, (int)enums.size());
                            }
                            catch (...)
                            {
                                ImGui::Text("Enum not in Enum Reflection");
                            }
                        }
                        else if (properties[i].type == typeid(HTexture).name())
                        {
                            HTexture texture = *reinterpret_cast<HTexture*>(address + properties[i].offset);
                            std::string name;
                            int selected = -1;
                            ResourceGUID currId = 0;
                            if (texture.Validate()) currId = (texture)->GetGUID();
                            for (int i = 0; i < m_TextureIDs.size(); ++i)
                                if (currId == m_TextureIDs[i])
                                {
                                    selected = i;
                                    name = m_TextureNames[i];
                                }
                            int tmp = selected;
                            ImGui::SameLine();
                            ImGui::Text("%s", name.c_str());
                            m_Global_Spaces += " ";
                            ImGui::Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_TextureNames, (int)m_TextureNames.size());
                            if (selected != tmp)
                            {
                                *reinterpret_cast<HTexture*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<Texture>(m_TextureIDs[selected]);
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
                            for (int i = 0; i < m_MeshIDs.size(); ++i)
                                if (currId == m_MeshIDs[i])
                                {
                                    selected = i;
                                    name = m_MeshNames[i];
                                }
                            int tmp = selected;
                            ImGui::SameLine();
                            ImGui::Text("%s", name.c_str());
                            m_Global_Spaces += " ";
                            ImGui::Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_MeshNames, (int)m_MeshNames.size());
                            if (selected != tmp)
                            {
                                *reinterpret_cast<HMesh*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<Mesh>(m_MeshIDs[selected]);

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
                            for (int i = 0; i < m_AnimationIDs.size(); ++i)
                                if (currId == m_AnimationIDs[i])
                                {
                                    selected = i;
                                    name = m_AnimationNames[i];
                                }
                            int tmp = selected;
                            ImGui::SameLine();
                            ImGui::Text("%s", name.c_str());
                            m_Global_Spaces += " ";
                            ImGui::Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_AnimationNames, (int)m_AnimationNames.size());
                            if (selected != tmp)
                            {
                                *reinterpret_cast<HAnimationSet*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<AnimationSet>(m_AnimationIDs[selected]);
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
                            for (int i = 0; i < m_FontIDs.size(); ++i)
                                if (currId == m_FontIDs[i])
                                {
                                    selected = i;
                                    name = m_FontNames[i];
                                }
                            int tmp = selected;
                            ImGui::SameLine();
                            ImGui::Text("%s", name.c_str());
                            m_Global_Spaces += " ";
                            ImGui::Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_FontNames, (int)m_FontNames.size());
                            if (selected != tmp) *reinterpret_cast<HFont*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<Font>(m_FontIDs[selected]);
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
                        ImGui::ListBox("SoundTracks", &currentSound, vector_getter, &SoundTracks, (int)SoundTracks.size(), 4);
                        if (currentSound != s)
                        {
                            std::string newValue = SoundTracks[currentSound];
                            sound->SetAudioClip(newValue);
                        }
                    }
                    ImGui::Unindent(5.0f);
                }
                catch (...)
                {
                    ImGui::Text("Component %s not in reflection factory!", comp->GetName().c_str());
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            ++i;
        }
    }

    if (ImGui::BeginPopupModal("ArchetypeName", NULL, ImGuiWindowFlags_NoResize))
    {
        m_Global_Spaces += " ";
        ImGui::Text("Archetype Name : ");
        ImGui::SameLine();
        ImGui::InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
        ImGui::NewLine();
        ImGui::InvisibleButton("OKCANCELSPACING1", ImVec2(60, 1));
        ImGui::SameLine();
        if (ImGui::Button("OK", ImVec2(120, 0)) || ImGui::IsKeyPressed(KEY_RETURN))
        {
            if (std::string {} != string)
            {
                auto go = CreateArchetype(string);
                ActionCreateArchetype* act = new ActionCreateArchetype(go, m_UpdatedArchetypes[string]);
                m_Undo.push_back(std::move(act));
                ClearRedo();
                SecureZeroMemory(string, MAX_STRING_LENGTH);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        ImGui::InvisibleButton("OKCANCELSPACING2", ImVec2(120, 1));
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            SecureZeroMemory(string, MAX_STRING_LENGTH);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        ImGui::InvisibleButton("OKCANCELSPACING3", ImVec2(60, 1));
        ImGui::EndPopup();
    }
}

void Editor::ParentArchetypeInspector(char* address, std::string parent, GameObject* currentArchetype)
{
    if (parent == "IComponent" || parent == "") return;
    if (!Factory::m_Reflection->at(parent)->getParents().empty()) ParentArchetypeInspector(address, Factory::m_Reflection->at(parent)->getParents().back().key, currentArchetype);
    auto properties = Factory::m_Reflection->at(parent)->getProperties();
    ImGui::Indent(5.0f);
    if (ImGui::TreeNode((std::string("Parent ") + std::string(" : ") + parent).c_str()))
    {
        for (int i = 0; i < properties.size(); ++i)
        {
            ImGui::Text("%s : ", properties[i].name.c_str());
            // if else for each type
            if (properties[i].type == typeid(std::string).name())
            {
                char tmp[MAX_STRING_LENGTH];
                SecureZeroMemory(tmp, MAX_STRING_LENGTH);
                ImGui::SameLine();
                m_Global_Spaces += " ";
                std::string* string = reinterpret_cast<std::string*>(address + properties[i].offset);
                std::copy(string->begin(), string->end(), tmp);
                ImGui::InputText(m_Global_Spaces.c_str(), tmp, MAX_STRING_LENGTH);
                if (ImGui::IsKeyPressed(KEY_RETURN) && *string != tmp) *string = std::string(tmp);
            }
            else if (properties[i].type == typeid(iVector2).name())
            {
                iVector2* vec3 = reinterpret_cast<iVector2*>(address + properties[i].offset);
                int tmp[2];
                tmp[0] = vec3->x;
                tmp[1] = vec3->y;
                m_Global_Spaces += " ";
                ImGui::InputInt2(m_Global_Spaces.c_str(), tmp);
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
                ImGui::InputFloat2(m_Global_Spaces.c_str(), tmp);
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
                ImGui::InputFloat3(m_Global_Spaces.c_str(), tmp);
                vec3->x = tmp[0];
                vec3->y = tmp[1];
                vec3->z = tmp[2];
            }
            else if (properties[i].type == typeid(float).name())
            {
                ImGui::SameLine();
                float* tmp = reinterpret_cast<float*>(address + properties[i].offset);
                m_Global_Spaces += " ";
                ImGui::InputFloat(m_Global_Spaces.c_str(), tmp);
            }
            else if (properties[i].type == typeid(int).name())
            {
                ImGui::SameLine();
                int* tmp = reinterpret_cast<int*>(address + properties[i].offset);
                m_Global_Spaces += " ";
                ImGui::InputInt(m_Global_Spaces.c_str(), tmp);
            }
            else if (properties[i].type == typeid(bool).name())
            {
                ImGui::SameLine();
                bool* tmp = reinterpret_cast<bool*>(address + properties[i].offset);
                m_Global_Spaces += " ";
                ImGui::Checkbox(m_Global_Spaces.c_str(), tmp);
            }
            else if (properties[i].type == typeid(Vector4).name())
            {
                static std::string hashes { "##" };
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
                ImGui::InputFloat4(m_Global_Spaces.c_str(), tmp);
                if (ImGui::IsKeyPressed(KEY_RETURN))
                {
                    if (tmp[0] != clr->x || tmp[1] != clr->y || tmp[2] != clr->z || tmp[3] != clr->w)
                    {
                        Color4 newValue(tmp[0], tmp[1], tmp[2], tmp[3]);
                        *clr = newValue;
                        ImGui::TreePop();
                        return;
                    }
                }
                // ImGui::SameLine();
                bool open_popup = ImGui::ColorButton(("Current Color" + hashes + properties[i].name).c_str(), color);
                if (open_popup)
                {
                    ImGui::OpenPopup(("ColorPicker" + hashes + properties[i].name).c_str());
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
                            ImGui::CloseCurrentPopup();
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
                    ImGui::Combo(m_Global_Spaces.c_str(), selected, enums_getter, &enums, (int)enums.size());
                }
                catch (...)
                {
                    ImGui::Text("Enum not in Enum Reflection");
                }
            }
            else if (properties[i].type == typeid(HTexture).name())
            {
                HTexture texture = *reinterpret_cast<HTexture*>(address + properties[i].offset);
                std::string name;
                int selected = -1;
                ResourceGUID currId = 0;
                if (texture.Validate()) currId = (texture)->GetGUID();
                for (int i = 0; i < m_TextureIDs.size(); ++i)
                    if (currId == m_TextureIDs[i])
                    {
                        selected = i;
                        name = m_TextureNames[i];
                    }
                int tmp = selected;
                ImGui::SameLine();
                ImGui::Text("%s", name.c_str());
                m_Global_Spaces += " ";
                ImGui::Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_TextureNames, (int)m_TextureNames.size());
                if (selected != tmp)
                {
                    *reinterpret_cast<HTexture*>(address + properties[i].offset) = ResourceManager::Instance().GetResource<Texture>(m_TextureIDs[selected]);
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
        ImGui::TreePop();
    }
    ImGui::Unindent(5.0f);
}

void Editor::ParentInspector(char* address, std::string comp, std::string parent)
{
    if (parent == "IComponent" || parent == "") return;
    if (!Factory::m_Reflection->at(parent)->getParents().empty()) ParentInspector(address, comp, Factory::m_Reflection->at(parent)->getParents().back().key);
    auto properties = Factory::m_Reflection->at(parent)->getProperties();
    // ImGui::Indent(5.0f);
    // if (ImGui::TreeNode((std::string("Parent ") + std::string(" : ") + parent).c_str()))
    // {
    for (auto& prop : properties)
    {
        ImGui::Text("%s : ", prop.name.c_str());
        if (prop.type == typeid(std::string).name())
        {
            ImGui::SameLine();
            ParentStructOptions(reinterpret_cast<std::string*>(address + prop.offset), m_CurrentObject, comp);
        }
        else if (prop.type == typeid(float).name())
        {
            ImGui::SameLine();
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
            ImGui::SameLine();
            ParentStructOptions(reinterpret_cast<float*>(address + prop.offset), m_CurrentObject, comp);
        }
        else if (prop.type == typeid(int).name())
        {
            ImGui::SameLine();
            ParentStructOptions(reinterpret_cast<int*>(address + prop.offset), m_CurrentObject, comp);
        }
        else if (prop.type == typeid(bool).name())
        {
            ImGui::SameLine();
            ParentStructOptions(reinterpret_cast<bool*>(address + prop.offset), m_CurrentObject, comp);
        }
        else if (prop.type == typeid(Color4).name())
        {
            ImGui::SameLine();
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
                    ImGui::Text("m_attractors : ");
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
                    if (ImGui::Button("Add Attractor")) ImGui::OpenPopup("AddAttractorPopUp");
                    if (ImGui::BeginPopup("AddAttractorPopUp"))
                    {
                        ImGui::Text("Attractors");
                        ImGui::Separator();
                        for (int i = 0; i < attractor_name.size(); ++i)
                            if (ImGui::Selectable(attractor_name[i].c_str()))
                            {
                                ActionAddAttractor* act = new ActionAddAttractor(m_CurrentObject, m_attractor, attractor_id[i]);
                                act->Execute();
                                m_Undo.push_back(std::move(act));
                                ClearRedo();
                            }
                        ImGui::EndPopup();
                    }
                    for (int i = 0; i < m_attractor.size(); ++i)
                    {
                        ImGui::Bullet();
                        ImGui::Text("%s", m_CurrentLayer->GetObjectById(m_attractor[i]));
                        ImGui::SameLine();
                        if (ImGui::SmallButton((std::string("Delete") + "##" + std::to_string(i)).c_str()))
                        {
                            ActionDeleteAttractor* act = new ActionDeleteAttractor(m_CurrentObject, m_attractor, m_attractor[i]);
                            act->Execute();
                            m_Undo.push_back(std::move(act));
                            ClearRedo();
                        }
                    }
                }
            }
        }
    }
    // ImGui::TreePop();
    //}
    // ImGui::Unindent(5.0f);
}

void Editor::StructInspector(char* address, std::string comp)
{
    size_t pos = comp.find(" ");
    std::string type = comp.substr(pos + 1);
    try
    {
        if (ImGui::TreeNode((std::string("Struct ") + std::string(" : ") + type).c_str()))
        {
            if (!Factory::m_Reflection->at(type)->getParents().empty()) ParentInspector(address, type, Factory::m_Reflection->at(type.c_str())->getParents().back().key);
            auto component = (*Factory::m_Reflection).at(type);
            auto properties = component->getProperties();
            ImGui::Indent(5.0f);
            for (int i = 0; i < properties.size(); ++i)
            {
                ImGui::Text("%s : ", properties[i].name.c_str());
                // if else for each type
                if (properties[i].type == typeid(std::string).name())
                {
                    ImGui::SameLine();
                    ParentStructOptions(reinterpret_cast<std::string*>(address + properties[i].offset), m_CurrentObject, type);
                }
                else if (properties[i].type == typeid(Vector3).name())
                {
                    ParentStructOptions(reinterpret_cast<Vector3*>(address + properties[i].offset), m_CurrentObject, type);
                }
                else if (properties[i].type == typeid(Vector2).name())
                {
                    ParentStructOptions(reinterpret_cast<Vector2*>(address + properties[i].offset), m_CurrentObject, comp);
                }
                else if (properties[i].type == typeid(TVector2<int>).name())
                {
                    ParentStructOptions(reinterpret_cast<TVector2<int>*>(address + properties[i].offset), m_CurrentObject, comp);
                }
                else if (properties[i].type == typeid(float).name())
                {
                    ImGui::SameLine();
                    ParentStructOptions(reinterpret_cast<float*>(address + properties[i].offset), m_CurrentObject, type);
                }
                else if (properties[i].type == typeid(int).name())
                {
                    ImGui::SameLine();
                    ParentStructOptions(reinterpret_cast<int*>(address + properties[i].offset), m_CurrentObject, type);
                }
                else if (properties[i].type == typeid(bool).name())
                {
                    ImGui::SameLine();
                    ParentStructOptions(reinterpret_cast<bool*>(address + properties[i].offset), m_CurrentObject, type);
                }
                else if (properties[i].type == typeid(Color4).name())
                {
                    ImGui::SameLine();
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
            ImGui::Unindent(5.0f);
            ImGui::TreePop();
        }
    }
    catch (...)
    {
        ImGui::TreePop();
    }
}

void Editor::ClearRedo()
{
    while (!m_Redo.empty())
    {
        Action* tmp = m_Redo.front();
        m_Redo.pop_front();
        delete tmp;
    }
}

void Editor::DrawTextEditor()
{
    // if (IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) m_editorFocus = true;
    // else m_editorFocus = false;
    // if (!GetCurrentDock()->active) m_editorFocus = false;
    if (ImGui::Button("New File"))
    {
        ZeroMemory(&m_EditorFile, sizeof(m_EditorFile));
        SecureZeroMemory(m_EditorStringBuffer, 2 << 23);
        m_EditorFileName.clear();
    }
    ImGui::SameLine();
    if (ImGui::Button("Open File"))
    {
        OPENFILENAME ofn;
        ZeroMemory(&m_EditorFile, sizeof(m_EditorFile));
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = Application::Instance().GetWindowHandle();
        ofn.lpstrFilter = "Lua Files\0*.lua\0All Files\0*.*\0";
        ofn.lpstrFile = m_EditorFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFileTitle = (LPSTR) "Load File";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameA(&ofn))
        {
            std::ifstream input(m_EditorFile);
            if (input.is_open())
            {
                SecureZeroMemory(m_EditorStringBuffer, 2 << 23);
                std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
                std::copy(contents.begin(), contents.end(), m_EditorStringBuffer);
                input.close();
                m_EditorFileName = m_EditorFile;
                auto pos = m_EditorFileName.find_last_of("\\");
                m_EditorFileName.erase(0, pos + 1);
            }
        }
    }
    ImGui::SameLine();
    if (!m_EditorFileName.empty())
    {
        if (ImGui::Button("Save"))
        {
            std::fstream pFile;
            pFile.open(m_EditorFile, std::fstream::out);
            pFile << m_EditorStringBuffer;
            pFile.close();
            std::cout << "File Saved" << std::endl;
        }
        ImGui::SameLine();
    }
    if (ImGui::Button("Save File"))
    {
        OPENFILENAME ofn;
        ZeroMemory(&m_EditorFile, sizeof(m_EditorFile));
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = Application::Instance().GetWindowHandle();
        ofn.lpstrFilter = "Lua Files\0*.lua\0All Files\0*.*\0";
        ofn.lpstrFile = m_EditorFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFileTitle = (LPSTR) "Save File";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetSaveFileNameA(&ofn))
        {
            std::string file(m_EditorFile);
            auto dir = file.find_last_of('\\');
            auto ext = file.find_first_of('.', dir);
            if (ext == file.npos)
            {
                file += ".lua";
                std::copy(file.begin(), file.end(), m_EditorFile);
            }
            std::fstream pFile;
            pFile.open(file, std::fstream::out);
            pFile << m_EditorStringBuffer;
            pFile.close();
            m_EditorFileName = m_EditorFile;
            auto pos = m_EditorFileName.find_last_of("\\");
            m_EditorFileName.erase(0, pos + 1);
        }
    }
    ImGui::SameLine();
    ImVec2 sz = ImGui::GetWindowSize();
    ImVec2 winSz = ImVec2((float)Application::Instance().GetWindowWidth(), (float)Application::Instance().GetWindowHeight());
    ImGui::Text("Filename : %s", m_EditorFileName.c_str());
    ImGui::BeginChild("EditorText", ImVec2(-1, -1), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    ImGui::InputTextMultiline("##source", m_EditorStringBuffer, IM_ARRAYSIZE(m_EditorStringBuffer), ImVec2(winSz.x, -1), ImGuiInputTextFlags_AllowTabInput);
    ImGui::EndChild();
}

void Editor::DrawLayerEditor()
{
    if (ImGui::Button("New Layer"))
    {
        ImGui::OpenPopup("AddLayer");
    }
    if (ImGui::BeginPopupModal("AddLayer"))
    {
        ImGui::SetItemDefaultFocus();
        static char AddLayerName[MAX_STRING_LENGTH];
        ImGui::Text("New Layer Name : ");
        m_Global_Spaces += " ";
        ImGui::SameLine();
        ImGui::InputText(m_Global_Spaces.c_str(), AddLayerName, MAX_STRING_LENGTH);
        std::string name(AddLayerName);
        ImGui::NewLine();
        ImGui::InvisibleButton("OKCANCELSPACING1", ImVec2(60, 1));
        ImGui::SameLine();
        if (ImGui::Button("OK", ImVec2(120, 0)) && !name.empty())
        {
            ActionCreateLayer* act = new ActionCreateLayer(name, m_CurrentLayer);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            SecureZeroMemory(AddLayerName, MAX_STRING_LENGTH);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        ImGui::InvisibleButton("OKCANCELSPACING2", ImVec2(120, 1));
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            SecureZeroMemory(AddLayerName, MAX_STRING_LENGTH);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        ImGui::InvisibleButton("OKCANCELSPACING3", ImVec2(60, 1));
        ImGui::EndPopup();
    }
    if (Application::Instance().GetCurrentScene()->GetLayers().size() > 1)
    {
        ImGui::SameLine();
        if (ImGui::Button("Delete Layer"))
        {
            ActionDeleteLayer* act = new ActionDeleteLayer(m_CurrentLayer);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            m_CurrentObject = nullptr;
            m_SelectedObjects.clear();
            Renderer::Instance().SetSelectedObjects({ 0 });
            m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayers().back();
            Renderer::Instance().SetCurrentLayer(m_CurrentLayer);
        }
    }
    auto lys = Application::Instance().GetCurrentScene()->GetLayers();
    for (auto& ly : lys)
    {
        if (ly == m_CurrentLayer)
            ImGui::Selectable(ly->GetName().c_str(), true);
        else if (ImGui::Selectable(ly->GetName().c_str()))
        {
            m_CurrentLayer = ly;
            Renderer::Instance().SetCurrentLayer(ly);
            m_CurrentObject = nullptr;
            m_SelectedObjects.clear();
            Renderer::Instance().SetSelectedObjects({ 0 });
        }
    }
}

void Editor::DrawProfiler()
{
    float real = 1.0f / Application::Instance().GetGameTimer().GetActualFrameTime();
    float rdt = Application::Instance().GetGameTimer().GetActualFrameTime();
    ImGui::Text("Current Frame Time : %f", real);
    // input sys
    ImGui::Text("Input System : %f", m_Timings[0]);
    static float input_t[60] = { 0 };
    if (m_Timer < 60)
        input_t[m_Timer] = (m_Timings[0] / rdt) * 100.0f;
    else
    {
        memcpy(input_t, input_t + 1, 59 * sizeof(float));
        input_t[59] = (m_Timings[0] / rdt) * 100.0f;
    }
    m_Global_Spaces += " ";
    ImGui::PlotLines(m_Global_Spaces.c_str(), input_t, IM_ARRAYSIZE(input_t), 0, "Input", 0, 100, ImVec2(0, 50));
    // physics sys
    ImGui::Text("Physics System : %f", m_Timings[1]);
    static float physics_t[60] = { 0 };
    if (m_Timer < 60)
        physics_t[m_Timer] = (m_Timings[1] / rdt) * 100.0f;
    else
    {
        memcpy(physics_t, physics_t + 1, 59 * sizeof(float));
        physics_t[59] = (m_Timings[1] / rdt) * 100.0f;
    }
    m_Global_Spaces += " ";
    ImGui::PlotLines(m_Global_Spaces.c_str(), physics_t, IM_ARRAYSIZE(input_t), 0, "Physics", 0, 100, ImVec2(0, 50));
    // scene sys
    ImGui::Text("Scene System : %f", m_Timings[2]);
    static float Logic_t[60] = { 0 };
    if (m_Timer < 60)
        Logic_t[m_Timer] = (m_Timings[2] / rdt) * 100.0f;
    else
    {
        memcpy(Logic_t, Logic_t + 1, 59 * sizeof(float));
        Logic_t[59] = (m_Timings[2] / rdt) * 100.0f;
    }
    m_Global_Spaces += " ";
    ImGui::PlotLines(m_Global_Spaces.c_str(), Logic_t, IM_ARRAYSIZE(input_t), 0, "Logic", 0, 100, ImVec2(0, 50));
    // render sys
    ImGui::Text("Renderer System : %f", m_Timings[3]);
    static float Render_t[60] = { 0 };
    if (m_Timer < 60)
        Render_t[m_Timer] = (m_Timings[3] / rdt) * 100.0f;
    else
    {
        memcpy(Render_t, Render_t + 1, 59 * sizeof(float));
        Render_t[59] = (m_Timings[3] / rdt) * 100.0f;
    }
    m_Global_Spaces += " ";
    ImGui::PlotLines(m_Global_Spaces.c_str(), Render_t, IM_ARRAYSIZE(input_t), 0, "Renderer", 0, 100, ImVec2(0, 50));
    // editor sys
    ImGui::Text("Editor System : %f", m_Timings[4]);
    static float Editor_t[60] = { 0 };
    if (m_Timer < 60)
        Editor_t[m_Timer] = (m_Timings[4] / rdt) * 100.0f;
    else
    {
        memcpy(Editor_t, Editor_t + 1, 59 * sizeof(float));
        Editor_t[59] = (m_Timings[4] / rdt) * 100.0f;
    }
    m_Global_Spaces += " ";
    ImGui::PlotLines(m_Global_Spaces.c_str(), Editor_t, IM_ARRAYSIZE(input_t), 0, "Editor", 0, 100, ImVec2(0, 50));
    // audio sys
    ImGui::Text("Audio System : %f", m_Timings[5]);
    static float Audio_t[60] = { 0 };
    if (m_Timer < 60)
        Audio_t[m_Timer] = (m_Timings[5] / rdt) * 100.0f;
    else
    {
        memcpy(Audio_t, Audio_t + 1, 59 * sizeof(float));
        Audio_t[59] = (m_Timings[5] / rdt) * 100.0f;
    }
    m_Global_Spaces += " ";
    ImGui::PlotLines(m_Global_Spaces.c_str(), Audio_t, IM_ARRAYSIZE(input_t), 0, "Audio", 0, 100, ImVec2(0, 50));
    if (m_Timer < 60) ++m_Timer;
    // animation sys
    ImGui::Text("Animation System : %f", m_Timings[6]);
    static float Anim_t[60] = { 0 };
    if (m_Timer < 60)
        Anim_t[m_Timer] = (m_Timings[6] / rdt) * 100.0f;
    else
    {
        memcpy(Anim_t, Anim_t + 1, 59 * sizeof(float));
        Anim_t[59] = (m_Timings[6] / rdt) * 100.0f;
    }
    m_Global_Spaces += " ";
    ImGui::PlotLines(m_Global_Spaces.c_str(), Anim_t, IM_ARRAYSIZE(input_t), 0, "Anim", 0, 100, ImVec2(0, 50));
    if (m_Timer < 60) ++m_Timer;

    // Particle System
    ImGui::Text("Particle System : %f", m_Timings[7]);
    static float Part_t[60] = { 0 };
    if (m_Timer < 60)
        Part_t[m_Timer] = (m_Timings[7] / rdt) * 100.0f;
    else
    {
        memcpy(Part_t, Part_t + 1, 59 * sizeof(float));
        Part_t[59] = (m_Timings[7] / rdt) * 100.0f;
    }
    m_Global_Spaces += " ";
    ImGui::PlotLines(m_Global_Spaces.c_str(), Part_t, IM_ARRAYSIZE(input_t), 0, "Particle", 0, 100, ImVec2(0, 50));
    if (m_Timer < 60) ++m_Timer;

    // AI System
    ImGui::Text("AI System : %f", m_Timings[8]);
    static float AI_t[60] = { 0 };
    if (m_Timer < 60)
        AI_t[m_Timer] = (m_Timings[8] / rdt) * 100.0f;
    else
    {
        memcpy(AI_t, AI_t + 1, 59 * sizeof(float));
        AI_t[59] = (m_Timings[8] / rdt) * 100.0f;
    }
    m_Global_Spaces += " ";
    ImGui::PlotLines(m_Global_Spaces.c_str(), AI_t, IM_ARRAYSIZE(input_t), 0, "AI", 0, 100, ImVec2(0, 50));
    if (m_Timer < 60) ++m_Timer;
}

void Editor::DrawFullScreenViewport()
{
    if (Input::Instance().GetKeyPressed(KEY_F8))
    {
        m_IsViewportFullScreen = !m_IsViewportFullScreen;
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)Application::Instance().GetWindowWidth(), (float)Application::Instance().GetWindowHeight()));

    if (ImGui::Begin("FullScreen ViewPort", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
    {
        if (Application::Instance().GetGameTimer().IsPlayModePaused())
        {
            if (ImGui::Button("Play"))
            {
                ms_IsGameRunning = true;
                while (ShowCursor(false) > 0)
                    ;
                AudioSystem::Instance().UnPauseAudio();
                Application::Instance().GetGameTimer().SetPlayModePaused(false);
            }
        }
        else
        {
            if (ImGui::Button("Pause") || Input::Instance().GetKeyPressed(KEY_F6))
            {
                while (ShowCursor(true) < 0)
                    ;
                AudioSystem::Instance().PauseAudio();
                Application::Instance().GetGameTimer().SetPlayModePaused(true);
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Stop") || Input::Instance().GetKeyPressed(KEY_F5))
        {
            Application::Instance().GetGameTimer().SetPlayModePaused(false);
            while (ShowCursor(true) < 0)
                ;
            // Show cursor
            // ShowCursor(true);
            ms_IsGameRunning = false;

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
            Renderer::Instance().SetSelectedObjects({ 0 });
            Renderer::Instance().ChangeCamera(false);
            ParticleSystem::Instance().Reset();
            AISystem::Instance().RevertBase();
        }

        if (ImGui::IsWindowHovered() && (Application::Instance().GetGameTimer().IsEditorPaused() || Application::Instance().GetGameTimer().IsPlayModePaused()))
        {
            static Vector3 tmp;
            static Vector3 current;

            if (ImGui::IsKeyPressed(70) && !Input::Instance().GetKeyDown(KEY_LALT) && m_CurrentObject) m_CurrentLayer->GetEditorCamera()->LookAt(m_CurrentObject);
            m_CurrentLayer->GetEditorCamera()->SetUpdate(true);
            m_CurrentLayer->GetEditorCamera()->OnUpdate(1 / 60.0f);
            m_CurrentLayer->GetEditorCamera()->SetUpdate(false);
        }
        else
        {
            m_CurrentLayer->GetEditorCamera()->Cancel();
        }

        ImVec2 windowSize = ImGui::GetWindowSize();
        windowSize.y -= ImGuiStyleVar_FramePadding * 2 + ImGui::GetFontSize() + 13.0f;

        Renderer::Instance().SetWindowSize(iVector2((int)windowSize.x, (int)windowSize.y));
        Renderer::Instance().GetCurrentEditorLayer()->GetEditorCamera()->SetViewportSize(iVector2((int)windowSize.x, (int)windowSize.y));

        ImGui::Image((ImTextureID)((__int64)Renderer::Instance().GetRenderTexture()), windowSize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
    }
}

void Editor::DrawTagsEditor()
{
    static int selected_tag = -1;
    m_Global_Spaces += " ";
    std::vector<std::string> tags;
    for (auto& s : m_Tags)
        tags.push_back(s);
    int height = (int)tags.size() > 33 ? 33 : (int)tags.size();
    ImGui::PushItemWidth(300.0f);
    ImGui::ListBox(m_Global_Spaces.c_str(), &selected_tag, vector_getter, static_cast<void*>(&tags), (int)tags.size(), height);
    ImGui::PopItemWidth();
    ImGui::Separator();
    if (selected_tag >= 0)
    {
        ImGui::Text("Selected Tag : %s", tags[selected_tag].c_str());
        ImGui::NewLine();
        ImGui::Text("New Name : ");
        ImGui::SameLine();
        m_Global_Spaces += " ";
        char newString[MAX_STRING_LENGTH];
        SecureZeroMemory(newString, MAX_STRING_LENGTH);
        ImGui::InputText(m_Global_Spaces.c_str(), newString, MAX_STRING_LENGTH);
        if (ImGui::IsKeyPressed(KEY_RETURN))
        {
            std::string s = newString;
            if (!s.empty() && std::find(tags.begin(), tags.end(), s) == tags.end())
            {
                ActionChangeTag* act = new ActionChangeTag(tags[selected_tag], s);
                act->Execute();
                m_Undo.push_back(std::move(act));
                ClearRedo();
                selected_tag = -1;
            }
        }
        if (ImGui::Button("Delete Tag"))
        {
            ActionDeleteTag* act = new ActionDeleteTag(tags[selected_tag]);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            selected_tag = -1;
        }
        ImGui::Separator();
    }
    // Add tags here

    ImGui::Text("Tag Name : ");
    ImGui::SameLine();
    m_Global_Spaces += " ";
    static char string[MAX_STRING_LENGTH];
    ImGui::InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
    if (ImGui::Button("Add Tag") || ImGui::IsKeyPressed(KEY_RETURN))
    {
        std::string s = string;
        SecureZeroMemory(string, MAX_STRING_LENGTH);
        if (!s.empty() && std::find(tags.begin(), tags.end(), s) == tags.end())
        {
            ActionAddTag* act = new ActionAddTag(s);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            selected_tag = -1;
        }
    }
}

void Editor::DrawPhysicsEditor()
{
#define PhysicsWorld PhysicsSystem::Instance().s_PhyWorldSettings
    if (ImGui::Button("Save Physics World"))
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
    ImGui::SameLine();
    if (ImGui::Button("Load Physics World"))
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
    ImGui::Separator();
    ImGui::Text("World Name : %s", PhysicsWorld.m_WorldName.c_str());
    ImGui::Text("Persistant Contact Distance Threshold");
    ImGui::SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_PersistantContactDistThreshold);

    ImGui::Text("Friction Coefficient");
    ImGui::SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultFrictCoefficient);

    ImGui::Text("Restitution");
    ImGui::SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultRestitution);

    ImGui::Text("Restitution Velocity Threshold");
    ImGui::SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_RestitutionVelThreshold);

    ImGui::Text("Rolling Resistance");
    ImGui::SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultRollResis);

    ImGui::Text("Sleeping Enabled");
    ImGui::SameLine();
    PhysicsOptionsBool(&PhysicsWorld.m_IsSleepingEnabled);

    ImGui::Text("Time Before Sleep");
    ImGui::SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultTimeBeforeSleep);

    ImGui::Text("Sleep Linear Velocity");
    ImGui::SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultSleepLinearVel);

    ImGui::Text("Sleep Angular Velocity");
    ImGui::SameLine();
    PhysicsOptionsFloat(&PhysicsWorld.m_DefaultSleepAngularVel);

    ImGui::Text("Gravity");
    PhysicsOptionsVector3(&PhysicsWorld.m_Gravity);

    ImGui::Text("Velocity Solver Iterations");
    ImGui::SameLine();
    PhysicsOptionsUint(&PhysicsWorld.m_DefaultVelSolverIteration);

    ImGui::Text("Position Solver Iterations");
    ImGui::SameLine();
    PhysicsOptionsUint(&PhysicsWorld.m_DefaultPosSolverIteration);

    ImGui::Text("Max Convex Contact Manifold");
    PhysicsOptionsUint(&PhysicsWorld.m_MaxConvexContactManifoldCount);

    ImGui::Text("Max Concave Contact Manifold");
    PhysicsOptionsUint(&PhysicsWorld.m_MaxConcaveContactManifoldCount);

    ImGui::Text("Contact Manifold Similar Angle");
    PhysicsOptionsFloat(&PhysicsWorld.m_ContactManifoldSimilarAngle);
}

void Editor::DrawResourceManager()
{
    static bool b = false;
    ImGui::Checkbox("Debugging Resources", &b);
    ImGui::Button("Add Resource");
    if (b)
    {
        ImGui::SameLine();
        if (ImGui::Button("Clean Resources XML"))
        {
            // Mesh
            std::string path("Resources\\Models");
            unsigned i = 1;

            for (i = 1; i < m_MeshIDs.size(); ++i)
            {
                bool b = false;
                auto resource = ResourceManager::Instance().GetResource<Mesh>(m_MeshIDs[i]);
                if (resource.Validate())
                {
                    std::wstring res = resource->GetFilePath().stem().c_str();
                    for (auto const& entry : std::filesystem::directory_iterator(path))
                    {
                        auto path = std::filesystem::path(entry.path());
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
                        std::cout << m_MeshNames[i] << std::endl;
                    }
                }
            }

            // Animation Set
            path = "Resources\\Models";

            for (i = 1; i < m_AnimationIDs.size(); ++i)
            {
                bool b = false;
                auto resource = ResourceManager::Instance().GetResource<AnimationSet>(m_AnimationIDs[i]);
                if (resource.Validate())
                {
                    std::wstring res = resource->GetFilePath().stem().c_str();
                    for (auto const& entry : std::filesystem::directory_iterator(path))
                    {
                        auto path = std::filesystem::path(entry.path());
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
                        std::cout << m_AnimationNames[i] << std::endl;
                    }
                }
            }

            // texture
            path = "Resources\\Texture";

            for (i = 1; i < m_TextureIDs.size(); ++i)
            {
                bool b = false;
                auto resource = ResourceManager::Instance().GetResource<Texture>(m_TextureIDs[i]);
                if (resource.Validate())
                {
                    std::wstring res = resource->GetFilePath().stem().c_str();
                    for (auto const& entry : std::filesystem::directory_iterator(path))
                    {
                        auto path = std::filesystem::path(entry.path());
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
                        std::cout << m_TextureNames[i] << std::endl;
                    }
                }
            }

            // Font
            path = "Resources\\Fonts";

            for (i = 1; i < m_FontIDs.size(); ++i)
            {
                bool b = false;
                auto resource = ResourceManager::Instance().GetResource<Font>(m_FontIDs[i]);
                if (resource.Validate())
                {
                    std::wstring res = resource->GetFilePath().stem().c_str();
                    for (auto const& entry : std::filesystem::directory_iterator(path))
                    {
                        auto path = std::filesystem::path(entry.path());
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
                        std::cout << m_FontNames[i] << std::endl;
                    }
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clean Resources Source"))
        {
            HWND handle = Application::Instance().GetWindowHandle();
            std::string message("YOU ARE DELETING ALL THE SOURCE FILES, EVANGELION 3.5 YOU CAN(NOT) UNDO, P.S. SHINJI IS A BITCH");
            int result = MessageBox(handle, LPCSTR(message.c_str()), "WARNING", MB_OKCANCEL);
            if (result == IDOK)
            {
                std::vector<std::filesystem::path> filesToBeDeleted;
                std::cout << "SELECTED OK" << std::endl;
                // Mesh
                std::string path("Resources\\Models");
                for (auto const& entry : std::filesystem::directory_iterator(path))
                {
                    auto path = std::filesystem::path(entry.path());
                    std::wstring file = path.stem().c_str();

                    unsigned i = 1;

                    for (i = 1; i < m_MeshIDs.size(); ++i)
                    {
                        auto resource = ResourceManager::Instance().GetResource<Mesh>(m_MeshIDs[i]);
                        if (resource.Validate())
                        {
                            std::wstring res = resource->GetFilePath().stem().c_str();
                            if (res == file) break;
                        }
                    }

                    if (i != m_MeshIDs.size()) continue;

                    for (i = 1; i < m_AnimationIDs.size(); ++i)
                    {
                        auto resource = ResourceManager::Instance().GetResource<AnimationSet>(m_AnimationIDs[i]);
                        if (resource.Validate())
                        {
                            std::wstring res = resource->GetFilePath().stem().c_str();
                            if (res == file) break;
                        }
                    }

                    if (i != m_AnimationIDs.size()) continue;

                    filesToBeDeleted.push_back(entry.path());
                }

                // Texture
                path = "Resources\\Texture";
                for (auto const& entry : std::filesystem::directory_iterator(path))
                {
                    auto path = std::filesystem::path(entry.path());
                    std::wstring file = path.stem().c_str();

                    unsigned i = 1;

                    for (i = 1; i < m_TextureIDs.size(); ++i)
                    {
                        auto resource = ResourceManager::Instance().GetResource<Texture>(m_TextureIDs[i]);
                        if (resource.Validate())
                        {
                            std::wstring res = resource->GetFilePath().stem().c_str();
                            if (res == file) break;
                        }
                    }

                    if (i != m_TextureIDs.size()) continue;

                    filesToBeDeleted.push_back(entry.path());
                }

                // Font
                path = "Resources\\Fonts";
                for (auto const& entry : std::filesystem::directory_iterator(path))
                {
                    auto path = std::filesystem::path(entry.path());
                    std::wstring file = path.stem().c_str();

                    unsigned i = 1;

                    for (i = 1; i < m_FontIDs.size(); ++i)
                    {
                        auto resource = ResourceManager::Instance().GetResource<Font>(m_FontIDs[i]);
                        if (resource.Validate())
                        {
                            std::wstring res = resource->GetFilePath().stem().c_str();
                            if (res == file) break;
                        }
                    }

                    if (i != m_FontIDs.size()) continue;

                    filesToBeDeleted.push_back(entry.path());
                }

                for (const auto& entry : filesToBeDeleted)
                {
                    std::cout << entry << std::endl;
                    if (!std::filesystem::is_directory(entry))
                    {
                        std::filesystem::remove(entry);
                    }
                    else
                    {
                        std::filesystem::remove_all(entry);
                    }
                }
            }
        }
    }
    ImGui::Separator();
    ImGui::PushFont(m_ImGuiFontBold);
    ImGui::Text("Meshes");
    ImGui::PopFont();
    static int mesh_selected = -1;
    static int mesh_height = static_cast<int>(m_MeshNames.size()) > 6 ? 6 : static_cast<int>(m_MeshNames.size());
    if (ImGui::Button("Delete Mesh"))
    {
        if (!m_MeshNames[mesh_selected].empty())
        {
            HMesh tmp = ResourceManager::Instance().GetResource<Mesh>(m_MeshIDs[mesh_selected]);
            ResourceManager::Instance().DestroyResource<Mesh>(tmp);
        }
    }
    ImGui::SameLine();
    ImGui::Text("Mesh List Box Height : ");
    ImGui::PushItemWidth(100);
    ImGui::SameLine();
    ImGui::InputInt("##meshHeight", &mesh_height);
    ImGui::PopItemWidth();
    // int mesh_tmp = mesh_selected;
    ImGui::PushItemWidth(300);
    ImGui::ListBox("##Mesh", &mesh_selected, vector_getter, &m_MeshNames, (int)m_MeshNames.size(), mesh_height);
    ImGui::PopItemWidth();
    ImGui::Separator();

    ImGui::PushFont(m_ImGuiFontBold);
    ImGui::Text("Animations");
    ImGui::PopFont();
    static int anim_selected = -1;
    static int anim_height = static_cast<int>(m_AnimationNames.size()) > 6 ? 6 : static_cast<int>(m_AnimationNames.size());
    if (ImGui::Button("Delete Animation"))
    {
        if (!m_AnimationNames[anim_selected].empty())
        {
            HAnimationSet tmp = ResourceManager::Instance().GetResource<AnimationSet>(m_AnimationIDs[anim_selected]);
            ResourceManager::Instance().DestroyResource<AnimationSet>(tmp);
        }
    }
    ImGui::SameLine();
    ImGui::Text("Animation List Box Height : ");
    ImGui::PushItemWidth(100);
    ImGui::SameLine();
    ImGui::InputInt("##animHeight", &anim_height);
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(300);
    ImGui::ListBox("##Animation", &anim_selected, vector_getter, &m_AnimationNames, (int)m_AnimationNames.size(), anim_height);
    ImGui::PopItemWidth();
    ImGui::Separator();

    ImGui::PushFont(m_ImGuiFontBold);
    ImGui::Text("Textures");
    ImGui::PopFont();
    static int tex_selected = -1;
    static int tex_height = static_cast<int>(m_TextureNames.size()) > 6 ? 6 : static_cast<int>(m_TextureNames.size());
    if (ImGui::Button("Delete Texture"))
    {
        if (!m_TextureNames[tex_selected].empty())
        {
            HTexture tmp = ResourceManager::Instance().GetResource<Texture>(m_TextureIDs[tex_selected]);
            ResourceManager::Instance().DestroyResource<Texture>(tmp);
        }
    }
    ImGui::SameLine();
    ImGui::Text("Texture List Box Height : ");
    ImGui::PushItemWidth(100);
    ImGui::SameLine();
    ImGui::InputInt("##textHeight", &tex_height);
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(300);
    ImGui::ListBox("##Texture", &tex_selected, vector_getter, &m_TextureNames, (int)m_TextureNames.size(), tex_height);
    ImGui::PopItemWidth();
    ImGui::Separator();

    ImGui::PushFont(m_ImGuiFontBold);
    ImGui::Text("Fonts");
    ImGui::PopFont();
    static int font_selected = -1;
    static int font_height = static_cast<int>(m_FontNames.size()) > 6 ? 6 : static_cast<int>(m_FontNames.size());
    if (ImGui::Button("Delete Font"))
    {
        if (!m_FontNames[font_selected].empty())
        {
            HFont tmp = ResourceManager::Instance().GetResource<Font>(m_FontIDs[font_selected]);
            ResourceManager::Instance().DestroyResource<Font>(tmp);
        }
    }
    ImGui::SameLine();
    ImGui::Text("Font List Box Height : ");
    ImGui::PushItemWidth(100);
    ImGui::SameLine();
    ImGui::InputInt("##fontHeight", &font_height);
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(300);
    ImGui::ListBox("##Font", &font_selected, vector_getter, &m_FontNames, (int)m_FontNames.size(), font_height);
    ImGui::PopItemWidth();
}

void Editor::PhysicsOptionsFloat(float* f)
{
    float tmp = *f;
    m_Global_Spaces += " ";
    ImGui::InputFloat(m_Global_Spaces.c_str(), &tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN) && tmp != *f)
    {
        // Physics Action here
        float oldValue = *f;
        float newValue = tmp;
        ActionInputPhysics<float>* act = new ActionInputPhysics<float>(oldValue, newValue, f);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::PhysicsOptionsBool(bool* b)
{
    bool tmp = *b;
    m_Global_Spaces += " ";
    ImGui::Checkbox(m_Global_Spaces.c_str(), &tmp);
    if (tmp != *b)
    {
        bool oldValue = *b;
        bool newValue = tmp;
        ActionInputPhysics<bool>* act = new ActionInputPhysics<bool>(oldValue, newValue, b);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::PhysicsOptionsVector3(Vector3* v)
{
    Vector3 tmp = *v;
    m_Global_Spaces += " ";
    ImGui::InputFloat3(m_Global_Spaces.c_str(), &tmp.x);
    if (ImGui::IsKeyPressed(KEY_RETURN))
    {
        if (tmp.x != v->x || tmp.y != v->y || tmp.z != v->z)
        {
            Vector3 newValue = tmp;
            Vector3 oldValue = *v;
            ActionInputPhysics<Vector3>* act = new ActionInputPhysics<Vector3>(oldValue, newValue, v);
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
    ImGui::InputInt(m_Global_Spaces.c_str(), &tmp, 0);
    if (ImGui::IsKeyPressed(KEY_RETURN) && tmp != (int)*u && tmp >= 0.0f)
    {
        uint newValue = (uint)tmp;
        uint oldValue = *u;
        ActionInputPhysics<uint>* act = new ActionInputPhysics<uint>(oldValue, newValue, u);
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
        ImGui::InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
    else
        ImGui::InputTextMultiline(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
    if (c != "TextRenderer" && p != "m_Text")
    {
        if (ImGui::IsKeyPressed(KEY_RETURN) && *s != string)
        {
            std::string oldValue = *s;
            std::string newValue = string;
            ActionInput<std::string>* act = new ActionInput<std::string>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
    else
    {
        if (!ImGui::IsItemActive() && *s != string)
        {
            std::string oldValue = *s;
            std::string newValue = string;
            ActionInput<std::string>* act = new ActionInput<std::string>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
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
    ImGui::InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
    if (ImGui::IsKeyPressed(KEY_RETURN) && *s != string)
    {
        std::string oldValue = *s;
        std::string newValue = string;
        ActionChangeName* act = new ActionChangeName(oldValue, newValue, m_CurrentLayer, m_CurrentObject);
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
    ImGui::InputFloat2(m_Global_Spaces.c_str(), tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec2->x || tmp[1] != vec2->y)
        {
            Vector2 newValue(tmp[0], tmp[1]);
            Vector2 oldValue = *vec2;
            ActionInput<Vector2>* act = new ActionInput<Vector2>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
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
    ImGui::InputFloat3(m_Global_Spaces.c_str(), tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec3->x || tmp[1] != vec3->y || tmp[2] != vec3->z)
        {
            Vector3 newValue(tmp[0], tmp[1], tmp[2]);
            Vector3 oldValue = *vec3;
            ActionInput<Vector3>* act = new ActionInput<Vector3>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
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
    ImGui::InputFloat(m_Global_Spaces.c_str(), &tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN) && tmp != *f)
    {
        float oldValue = *f;
        float newValue = tmp;
        ActionInput<float>* act = new ActionInput<float>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::Options(int* i, GameObject* go, std::string c, std::string p)
{
    int tmp = *i;
    m_Global_Spaces += " ";
    ImGui::InputInt(m_Global_Spaces.c_str(), &tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN) && tmp != *i)
    {
        int oldValue = *i;
        int newValue = tmp;
        ActionInput<int>* act = new ActionInput<int>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
        act->Execute();
        m_Undo.push_back(std::move(act));
        ClearRedo();
    }
}

void Editor::Options(bool* b, GameObject* go, std::string c, std::string p)
{
    bool tmp = *b;
    m_Global_Spaces += " ";
    ImGui::Checkbox(m_Global_Spaces.c_str(), &tmp);
    if (tmp != *b)
    {
        int oldValue = *b;
        int newValue = tmp;
        ActionInput<bool>* act = new ActionInput<bool>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
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
    ImGui::InputFloat4(m_Global_Spaces.c_str(), tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != clr->x || tmp[1] != clr->y || tmp[2] != clr->z || tmp[3] != clr->w)
        {
            Color4 newValue(tmp[0], tmp[1], tmp[2], tmp[3]);
            Color4 oldValue = *clr;
            ActionInput<Color4>* act = new ActionInput<Color4>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
            return;
        }
    }
    // ImGui::SameLine();
    bool open_popup = ImGui::ColorButton(("Current Color" + hashes + p).c_str(), color);
    if (open_popup)
    {
        ImGui::OpenPopup(("ColorPicker" + hashes + p).c_str());
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
                ActionInput<Color4>* act = new ActionInput<Color4>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
                act->Execute();
                m_Undo.push_back(std::move(act));
                ClearRedo();
                ImGui::CloseCurrentPopup();
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
    for (int i = 0; i < m_MeshIDs.size(); ++i)
        if (currId == m_MeshIDs[i])
        {
            selected = i;
            name = m_MeshNames[i];
        }
    int tmp = selected;
    ImGui::SameLine();
    ImGui::Text("%s", name.c_str());
    m_Global_Spaces += " ";
    ImGui::Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_MeshNames, (int)m_MeshNames.size());
    if (selected != tmp)
    {
        ResourceGUID oldIndex = tmp == -1 ? 0 : m_MeshIDs[tmp];
        ResourceGUID newIndex = m_MeshIDs[selected];
        ActionMeshInput* act = new ActionMeshInput(m_CurrentObject, c, m_CurrentLayer, oldIndex, newIndex);
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
    for (int i = 0; i < m_TextureIDs.size(); ++i)
        if (currId == m_TextureIDs[i])
        {
            selected = i;
            name = m_TextureNames[i];
        }
    int tmp = selected;
    ImGui::SameLine();
    ImGui::Text("%s", name.c_str());
    m_Global_Spaces += " ";
    ImGui::Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_TextureNames, (int)m_TextureNames.size());
    if (selected != tmp)
    {
        ResourceGUID oldIndex = tmp == -1 ? 0 : m_TextureIDs[tmp];
        ResourceGUID newIndex = m_TextureIDs[selected];
        ActionInputTexture* act = new ActionInputTexture(m_CurrentObject, c, m_CurrentLayer, oldIndex, newIndex, p);
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
    for (int i = 0; i < m_AnimationIDs.size(); ++i)
        if (currId == m_AnimationIDs[i])
        {
            selected = i;
            name = m_AnimationNames[i];
        }
    int tmp = selected;
    ImGui::SameLine();
    ImGui::Text("%s", name.c_str());
    m_Global_Spaces += " ";
    ImGui::Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_AnimationNames, (int)m_AnimationNames.size());
    if (selected != tmp)
    {
        ResourceGUID oldIndex = tmp == -1 ? 0 : m_AnimationIDs[tmp];
        ResourceGUID newIndex = m_AnimationIDs[selected];
        ActionInputAnimation* act = new ActionInputAnimation(m_CurrentObject, c, m_CurrentLayer, oldIndex, newIndex, p);
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
    ImGui::InputInt2(m_Global_Spaces.c_str(), tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec2->x || tmp[1] != vec2->y)
        {
            TVector2<int> newValue(tmp[0], tmp[1]);
            TVector2<int> oldValue = *vec2;
            ActionInput<TVector2<int>>* act = new ActionInput<TVector2<int>>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
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
    for (int i = 0; i < m_FontIDs.size(); ++i)
        if (currId == m_FontIDs[i])
        {
            selected = i;
            name = m_FontNames[i];
        }
    int tmp = selected;
    ImGui::SameLine();
    ImGui::Text("%s", name.c_str());
    m_Global_Spaces += " ";
    ImGui::Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_FontNames, (int)m_FontNames.size());
    if (selected != tmp)
    {
        ResourceGUID oldIndex = tmp == -1 ? 0 : m_FontIDs[tmp];
        ResourceGUID newIndex = m_FontIDs[selected];
        ActionInputFont* act = new ActionInputFont(m_CurrentObject, c, m_CurrentLayer, oldIndex, newIndex);
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
        ImGui::Combo(m_Global_Spaces.c_str(), &tmp, enums_getter, &enums, (int)enums.size());
        if (tmp != selected)
        {
            int oldValue = selected;
            int newValue = tmp;
            ActionInput<int>* act = new ActionInput<int>(oldValue, newValue, go->GetName(), c, p, m_CurrentLayer, m_CurrentObject);
            act->Execute();
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
    catch (...)
    {
        ImGui::Text("Enum not in Enum Reflection");
    }
}

void Editor::ParentStructOptions(std::string* s, GameObject* go, std::string c)
{
    char string[MAX_STRING_LENGTH];
    SecureZeroMemory(string, MAX_STRING_LENGTH);
    std::copy(s->c_str(), s->c_str() + s->size(), string);
    m_Global_Spaces += " ";
    ImGui::InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
    if (ImGui::IsKeyPressed(KEY_RETURN) && *s != string)
    {
        ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
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
    ImGui::InputFloat(m_Global_Spaces.c_str(), &tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN) && tmp != *f)
    {
        ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
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
    ImGui::InputInt(m_Global_Spaces.c_str(), &tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN) && tmp != *i)
    {
        ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
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
    ImGui::Checkbox(m_Global_Spaces.c_str(), &tmp);
    if (tmp != *b)
    {
        ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
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
    ImGui::InputFloat2(m_Global_Spaces.c_str(), tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec2->x || tmp[1] != vec2->y)
        {
            Vector2 newValue(tmp[0], tmp[1]);
            ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
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
    ImGui::InputFloat3(m_Global_Spaces.c_str(), tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec3->x || tmp[1] != vec3->y || tmp[2] != vec3->z)
        {
            Vector3 newValue(tmp[0], tmp[1], tmp[2]);
            ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
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
    ImGui::InputInt2(m_Global_Spaces.c_str(), tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != vec2->x || tmp[1] != vec2->y)
        {
            TVector2<int> newValue(tmp[0], tmp[1]);
            ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
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
    static std::string hashes { "##" };
    ImVec4 backup_color;
    static ImVec4 tmp_color;

    float tmp[4];
    tmp[0] = clr->x;
    tmp[1] = clr->y;
    tmp[2] = clr->z;
    tmp[3] = clr->w;
    ImVec4 color = ImVec4(tmp[0], tmp[1], tmp[2], tmp[3]);
    m_Global_Spaces += " ";
    ImGui::InputFloat4(m_Global_Spaces.c_str(), tmp);
    if (ImGui::IsKeyPressed(KEY_RETURN))
    {
        if (tmp[0] != clr->x || tmp[1] != clr->y || tmp[2] != clr->z || tmp[3] != clr->w)
        {
            ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
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
    bool open_popup = ImGui::ColorButton(("Current Color" + hashes + p).c_str(), color);
    if (open_popup)
    {
        ImGui::OpenPopup(("ColorPicker" + hashes + p).c_str());
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
                ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
                Color4 newValue(tmp_color.x, tmp_color.y, tmp_color.z, tmp_color.w);
                *clr = newValue;
                go->UpdateComponents();
                act->SetNew(go);
                m_Undo.push_back(std::move(act));
                ClearRedo();
                ImGui::CloseCurrentPopup();
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
    for (int i = 0; i < m_TextureIDs.size(); ++i)
        if (currId == m_TextureIDs[i])
        {
            selected = i;
            name = m_TextureNames[i];
        }
    int tmp = selected;
    ImGui::SameLine();
    ImGui::Text("%s", name.c_str());
    m_Global_Spaces += " ";
    ImGui::Combo(m_Global_Spaces.c_str(), &selected, vector_getter, &m_TextureNames, (int)m_TextureNames.size());
    if (selected != tmp)
    {
        ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
        *texture = ResourceManager::Instance().GetResource<Texture>(m_TextureIDs[selected]);
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
        ImGui::Combo(m_Global_Spaces.c_str(), &tmp, enums_getter, &enums, (int)enums.size());
        if (tmp != selected)
        {
            ActionParentStructInput* act = new ActionParentStructInput(go, c, m_CurrentLayer);
            *i = tmp;
            go->UpdateComponents();
            act->SetNew(go);
            m_Undo.push_back(std::move(act));
            ClearRedo();
        }
    }
    catch (...)
    {
        ImGui::Text("Enum not in Enum Reflection");
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
        Serializer { file }.SaveScene(Application::Instance().GetCurrentScene());
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
        Renderer::Instance().SetSelectedObjects({ 0 });
        ClearRedoUndo();
        Serializer { filename }.LoadScene();
    }
    m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayers().back();
    Renderer::Instance().SetCurrentLayer(m_CurrentLayer);
    Renderer::Instance().ChangeCamera(false);
    Application::Instance().GetGameTimer().SetEditorPaused(true);
}

void Editor::CreateNewSceneTab()
{
    if (ImGui::BeginPopupModal("NewScene", 0, ImGuiWindowFlags_NoResize))
    {
        ImGui::SetItemDefaultFocus();
        static char LayerName[MAX_STRING_LENGTH];
        ImGui::Text("New Layer Name : ");
        m_Global_Spaces += " ";
        ImGui::SameLine();
        ImGui::InputText(m_Global_Spaces.c_str(), LayerName, MAX_STRING_LENGTH);
        std::string name(LayerName);
        ImGui::NewLine();
        ImGui::InvisibleButton("OKCANCELSPACING1", ImVec2(60, 1));
        ImGui::SameLine();
        if (ImGui::Button("OK", ImVec2(120, 0)) && !name.empty())
        {
            m_CurrentObject = nullptr;
            m_SelectedObjects.clear();
            Renderer::Instance().SetSelectedObjects({ 0 });
            ClearRedoUndo();
            Application::Instance().NewScene("Scene");
            Application::Instance().GetCurrentScene()->CreateLayer(name);
            m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayers().back();
            Renderer::Instance().SetCurrentLayer(m_CurrentLayer);
            SecureZeroMemory(LayerName, MAX_STRING_LENGTH);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        ImGui::InvisibleButton("OKCANCELSPACING2", ImVec2(120, 1));
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            SecureZeroMemory(LayerName, MAX_STRING_LENGTH);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        ImGui::InvisibleButton("OKCANCELSPACING3", ImVec2(60, 1));
        ImGui::EndPopup();
    }
}

void Editor::Duplicate()
{
    if (!m_CurrentLayer) return;
    if (m_SelectedObjects.empty()) return;
    ActionDuplicate* act = new ActionDuplicate(m_CurrentLayer, m_SelectedObjects);
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
    tinyxml2::XMLDocument document;

    if (document.LoadFile((std::string(m_CurrentWorkingDirectory) + "\\imgui_editor_windows.xml").c_str()) == tinyxml2::XMLError::XML_SUCCESS)
    {
        tinyxml2::XMLNode* root = document.FirstChild();
        tinyxml2::XMLElement* Data = root->FirstChildElement();
        std::string boolean = Data->Attribute("AppConsole");

        m_Console.m_IsActiveWindow = (bool)std::stoi(boolean);

        for (int i = 0; i < NUM_OF_WINDOWS; ++i)
        {
            if (!Data) return;
            if (!Data->Attribute(("Window" + std::to_string(i)).c_str())) break;
            std::string boolean = Data->Attribute(("Window" + std::to_string(i)).c_str());
            m_WindowStates[i] = (bool)std::stoi(boolean);
        }
    }
}

void Editor::SaveData()
{
    tinyxml2::XMLDocument document;

    tinyxml2::XMLNode* rootNode = document.NewElement("Activeness");
    document.InsertEndChild(rootNode);

    tinyxml2::XMLElement* dataElement = document.NewElement("Windows");
    dataElement->SetAttribute("AppConsole", (int)m_Console.m_IsActiveWindow);

    rootNode->InsertEndChild(dataElement);

    for (int i = 0; i < NUM_OF_WINDOWS; ++i)
    {
        dataElement->SetAttribute(("Window" + std::to_string(i)).c_str(), (int)m_WindowStates[i]);
    }

    tinyxml2::XMLError result = document.SaveFile((std::string(m_CurrentWorkingDirectory) + "\\imgui_editor_windows.xml").c_str());
}

void Editor::SetUndoRedoSize(const unsigned& count)
{
    m_RedoUndoCount = count;
}

AppConsole& Editor::GetConsole()
{
    return m_Console;
}

void Editor::KeyDown(WPARAM key, bool pressed)
{
    ImGui::GetIO().KeysDown[key] = pressed;
}

void Editor::CharInput(char c)
{
    ImGui::GetIO().AddInputCharacter(c);
}

void Editor::MouseScroll(short s)
{
    ImGui::GetIO().MouseWheel = s;
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

void Editor::RecursionSaveArchetypeParent(const char* name, unsigned char* base, tinyxml2::XMLElement* pComponent, tinyxml2::XMLDocument& doc)
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
                tinyxml2::XMLElement* pElem = doc.NewElement(comp.name.c_str());
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
                tinyxml2::XMLElement* pElem = doc.NewElement(comp.name.c_str());
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

void Editor::RecursionSaveArchetypeStruct(registration::variant prop, unsigned char* base, tinyxml2::XMLElement* pComponent, tinyxml2::XMLDocument& doc)
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
                tinyxml2::XMLElement* pElem = doc.NewElement(rProp.name.c_str());
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
                tinyxml2::XMLElement* pElem = doc.NewElement(rProp.name.c_str());
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
    tinyxml2::XMLDocument doc;

    tinyxml2::XMLNode* pRoot = doc.NewElement("Archetypes");
    doc.InsertEndChild(pRoot);

    for (auto& pair : m_Archetypes)
    {
        GameObject* go = pair.second;
        if (go->GetName() == "EditorCamera") continue;
        tinyxml2::XMLElement* pElement = doc.NewElement(go->GetName().c_str());
        pElement->SetAttribute("Tag", go->GetTag().c_str());
        pRoot->InsertEndChild(pElement);
        for (IComponent* comp : go->GetComponentList())
        {
            tinyxml2::XMLElement* pComponent = doc.NewElement(comp->GetName().c_str());
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
                    tinyxml2::XMLElement* pElem = doc.NewElement(prop.name.c_str());
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
                    tinyxml2::XMLElement* pElem = doc.NewElement(prop.name.c_str());
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
    tinyxml2::XMLError result = doc.SaveFile(filename.c_str());
    if (result != tinyxml2::XMLError::XML_SUCCESS) throw("ERROR");  // Replace with proper throw and assert
    std::cout << "Saving " << filename << " completed!" << std::endl;
}

void Editor::RecursionLoadArchetypeStruct(tinyxml2::XMLElement* attribute, unsigned char* base)
{
    std::string aType = attribute->Attribute("Type");
    size_t pos = aType.find(" ");
    std::string type = aType.substr(pos + 1);
    auto sProperties = (*Factory::m_Reflection).at(type)->getProperties();
    tinyxml2::XMLElement* sAttribute = attribute->FirstChildElement();
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
                std::string attributeValue { sAttribute->Attribute("Value") };
                attribute = attributeValue;
                // std::cout << attribute << std::endl;
            }
        }
    }
}

void Editor::RecursionLoadArchetypeParent(tinyxml2::XMLElement* attribute, unsigned char* base, const char* comp)
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
                    std::string attributeValue { attribute->Attribute("Value") };
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
    tinyxml2::XMLDocument file;
    if (file.LoadFile(filename.c_str()) == tinyxml2::XMLError::XML_SUCCESS)
    {
        tinyxml2::XMLNode* root = file.FirstChild();

        tinyxml2::XMLElement* pGo = root->FirstChildElement();
        for (; pGo; pGo = pGo->NextSiblingElement())
        {
            GameObject* Go = new GameObject(nullptr, 0);
            if (pGo->Attribute("Tag"))
            {
                Go->SetTag(pGo->Attribute("Tag"));
            }
            Go->SetName(pGo->Value());
            tinyxml2::XMLElement* pComp = pGo->FirstChildElement();
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
                tinyxml2::XMLElement* pAttribute = pComp->FirstChildElement();
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
                            std::string attributeValue { pAttribute->Attribute("Value") };
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
    m_TextureIDs.clear();
    m_TextureNames.clear();
    m_MeshIDs.clear();
    m_MeshNames.clear();
    m_AnimationIDs.clear();
    m_AnimationNames.clear();
    m_FontIDs.clear();
    m_FontNames.clear();

    m_FontIDs.push_back(0);
    m_FontNames.push_back(std::string {});
    m_TextureIDs.push_back(0);
    m_TextureNames.push_back(std::string {});
    m_MeshIDs.push_back(0);
    m_MeshNames.push_back(std::string {});
    m_AnimationIDs.push_back(0);
    m_AnimationNames.push_back(std::string {});

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
        m_TextureIDs.push_back(r->GetGUID());
        m_TextureNames.push_back(r->m_Name);
    }

    for (auto& r : meshes)
    {
        m_MeshIDs.push_back(r->GetGUID());
        m_MeshNames.push_back(r->m_Name);
    }

    for (auto& r : anims)
    {
        m_AnimationIDs.push_back(r->GetGUID());
        m_AnimationNames.push_back(r->m_Name);
    }

    for (auto& f : fonts)
    {
        m_FontIDs.push_back(f->GetGUID());
        m_FontNames.push_back(f->m_Name);
    }
}
void Editor::RecursiveParentAndChildObject(GameObject* go, std::string s, int& selected)
{
    unsigned parent = m_CurrentObject ? m_CurrentObject->GetParentObject() : 0;
    while (parent)
    {
        if (parent == go->GetID())
        {
            ImGui::SetNextItemOpen(true);
            break;
        }
        parent = m_CurrentLayer->GetObjectById(parent)->GetParentObject();
    }

    if (ImGui::TreeNode(s.c_str()))
    {
        if (ImGui::IsItemClicked())
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
                        Renderer::Instance().SetSelectedObjects({ 0 });
                        m_CurrentObject = nullptr;
                    }
                }
            }
            else
            {
                m_SelectedObjects.clear();
                m_SelectedObjects.push_back(m_CurrentObject);
                Renderer::Instance().SetSelectedObjects({ m_CurrentObject->GetID() });
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
                if (ImGui::Selectable((gameObject->GetName() + "##" + std::to_string(*begin)).c_str(), selected == *begin))
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
                        Renderer::Instance().SetSelectedObjects({ m_CurrentObject->GetID() });
                    }
                    selected = *begin;
                }
                if (ImGui::IsItemHovered() && Input::Instance().GetMousePressed(MOUSE_MID) && !m_OutlinerSelectedObject)
                {
                    m_OutlinerSelectedObject = gameObject;
                }
                if (Input::Instance().GetKeyUp(MOUSE_MID) && m_OutlinerSelectedObject && ImGui::IsItemHovered())
                {
                    if (m_OutlinerSelectedObject != gameObject)
                    {
                        bool tmp = false;
                        if (m_OutlinerSelectedObject && m_OutlinerSelectedObject->GetIsChildren())
                        {
                            unsigned parent = m_OutlinerSelectedObject->GetParentObject();
                            if (parent == gameObject->GetID()) tmp = true;
                            m_CurrentLayer->GetObjectById(parent)->RemoveChild(m_OutlinerSelectedObject->GetID());
                            m_OutlinerSelectedObject->SetIsChildren(false);
                            m_OutlinerSelectedObject->SetParentObject(0);
                        }
                        if (!tmp)
                        {
                            gameObject->AttachChild(m_OutlinerSelectedObject->GetID());
                            m_OutlinerSelectedObject->SetIsChildren(true);
                            m_OutlinerSelectedObject->SetParentObject(gameObject->GetID());
                            m_OutlinerSelectedObject = nullptr;
                        }
                    }
                }
            }
            else
            {
                RecursiveParentAndChildObject(gameObject, (gameObject->GetName() + "##" + std::to_string(*begin)), selected);
            }
        }
        ImGui::TreePop();
    }
    if (ImGui::IsItemHovered() && Input::Instance().GetMousePressed(MOUSE_MID) && !m_OutlinerSelectedObject) m_OutlinerSelectedObject = go;
    if (Input::Instance().GetKeyUp(MOUSE_MID) && m_OutlinerSelectedObject && ImGui::IsItemHovered())
    {
        if (m_OutlinerSelectedObject != go)
        {
            bool tmp = false;
            if (m_OutlinerSelectedObject && m_OutlinerSelectedObject->GetIsChildren())
            {
                unsigned parent = m_OutlinerSelectedObject->GetParentObject();
                if (go->GetID() == parent) tmp = true;
                m_CurrentLayer->GetObjectById(parent)->RemoveChild(m_OutlinerSelectedObject->GetID());
                m_OutlinerSelectedObject->SetIsChildren(false);
                m_OutlinerSelectedObject->SetParentObject(0);
            }
            if (!tmp)
            {
                go->AttachChild(m_OutlinerSelectedObject->GetID());
                m_OutlinerSelectedObject->SetIsChildren(true);
                m_OutlinerSelectedObject->SetParentObject(go->GetID());
                m_OutlinerSelectedObject = nullptr;
            }
        }
    }
}

void Editor::LoadTags()
{
    tinyxml2::XMLDocument file;
    if (file.LoadFile((std::string(m_CurrentWorkingDirectory) + "\\tags.xml").c_str()) == tinyxml2::XMLError::XML_SUCCESS)
    {
        tinyxml2::XMLNode* root = file.FirstChild();
        tinyxml2::XMLElement* Data = root->FirstChildElement();
        auto a = Data->FirstAttribute();
        while (a)
        {
            m_Tags.insert(a->Name());
            a = a->Next();
        }
    }
}

void Editor::SaveTags()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLNode* pRoot = doc.NewElement("Tags");
    doc.InsertEndChild(pRoot);
    tinyxml2::XMLElement* Data = doc.NewElement("Name");
    pRoot->InsertEndChild(Data);
    for (auto& s : m_Tags)
        Data->SetAttribute(s.c_str(), 1);
    tinyxml2::XMLError result = doc.SaveFile((std::string(m_CurrentWorkingDirectory) + "\\tags.xml").c_str());
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
    m_Timings[0] = input;
    m_Timings[1] = phy;
    m_Timings[2] = Comps;
    m_Timings[3] = renderer;
    m_Timings[4] = editor;
    m_Timings[5] = audio;
    m_Timings[6] = anim;
    m_Timings[7] = particle;
    m_Timings[8] = ai;
}

void Editor::AddTag(std::string t)
{
    m_Tags.insert(t);
}

void Editor::RemoveTag(std::string t)
{
    m_Tags.erase(t);
}

void Editor::DrawOutliner()
{
    // if (m_selectedOutliner) std::cout << m_selectedOutliner->GetName() << std::endl;
    // if (m_selectedOutliner) ImGui::Text("Selected Outliner : %s", m_selectedOutliner->GetName());
    // LayerList ly = Application::Instance().GetCurrentScene()->GetLayers();
    static int selected = -1;
    if (m_CurrentObject)
    {
        selected = m_CurrentObject->GetID();
        ImGui::SetNextItemOpen(true);
    }
    int i = 0;
    auto lyStart = m_CurrentLayer;
    if (ImGui::TreeNode(((lyStart)->GetName() + "##" + std::to_string(i)).c_str()))
    {
        for (auto& go : (lyStart)->GetObjectsList())
        {
            if (go->GetIsChildren()) continue;
            if (!go->GetChildrenObjects().size())
            {
                if (ImGui::Selectable((go->GetName() + "##" + std::to_string(i)).c_str(), selected == go->GetID()))
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
                                Renderer::Instance().SetSelectedObjects({ 0 });
                            }
                        }
                    }
                    else
                    {
                        m_SelectedObjects.clear();
                        m_SelectedObjects.push_back(go);
                        Renderer::Instance().SetSelectedObjects({ go->GetID() });
                    }
                    selected = go->GetID();
                }
                if (Input::Instance().GetKeyUp(MOUSE_MID) && m_OutlinerSelectedObject && ImGui::IsItemHovered())
                {
                    if (m_OutlinerSelectedObject != go)
                    {
                        bool tmp = false;
                        if (m_OutlinerSelectedObject && m_OutlinerSelectedObject->GetIsChildren())
                        {
                            unsigned parent = m_OutlinerSelectedObject->GetParentObject();
                            if (go->GetID() == parent) tmp = true;
                            m_CurrentLayer->GetObjectById(parent)->RemoveChild(m_OutlinerSelectedObject->GetID());
                            m_OutlinerSelectedObject->SetIsChildren(false);
                            m_OutlinerSelectedObject->SetParentObject(0);
                        }
                        if (!tmp)
                        {
                            go->AttachChild(m_OutlinerSelectedObject->GetID());
                            m_OutlinerSelectedObject->SetIsChildren(true);
                            m_OutlinerSelectedObject->SetParentObject(go->GetID());
                            m_OutlinerSelectedObject = nullptr;
                        }
                    }
                }
                if (ImGui::IsItemHovered() && Input::Instance().GetMousePressed(MOUSE_MID) && !m_OutlinerSelectedObject) m_OutlinerSelectedObject = go;
            }
            else
            {
                RecursiveParentAndChildObject(go, (go->GetName() + "##" + std::to_string(i)), selected);
            }

            ++i;
        }
        ImGui::TreePop();
    }
    if (Input::Instance().GetKeyUp(MOUSE_MID))
    {
        if (m_OutlinerSelectedObject && m_OutlinerSelectedObject->GetIsChildren())
        {
            unsigned parent = m_OutlinerSelectedObject->GetParentObject();
            m_CurrentLayer->GetObjectById(parent)->RemoveChild(m_OutlinerSelectedObject->GetID());
            m_OutlinerSelectedObject->SetIsChildren(false);
            m_OutlinerSelectedObject->SetParentObject(0);
        }
        m_OutlinerSelectedObject = nullptr;
    }
}

void Editor::DrawInspector()
{
    if (m_CurrentObject)
    {
        // TODO: Reflection of Components and editable and also the pushing into the
        // deque for undo
        ImGui::Text("Chinese");
        // ImGui::Text("Name of Malay : %s", m_CurrentObject->GetName().c_str());
        ImGui::Text("Name : ");
        // Need to change when we implement the destroy and undo
        Options(const_cast<std::string*>(&m_CurrentObject->GetName()));
        ImGui::Text("GuID : %d", m_CurrentObject->GetID());
        if (!m_CurrentObject->GetArchetype().empty()) ImGui::Text("Archetype : %s", m_CurrentObject->GetArchetype().c_str());
        ImGui::Text("Tag : ");
        ImGui::SameLine();
        if (ImGui::Selectable(m_CurrentObject->GetTag() == std::string {} ? "Click me to add Tag" : m_CurrentObject->GetTag().c_str())) ImGui::OpenPopup("TagsPopUp");
        if (ImGui::BeginPopup("TagsPopUp"))
        {
            ImGui::Text("Tags");
            ImGui::Separator();
            for (auto& tag : m_Tags)
            {
                if (ImGui::Selectable(tag.c_str()))
                {
                    ActionTagObject* act = new ActionTagObject(m_CurrentObject->GetTag(), tag, m_CurrentObject);
                    act->Execute();
                    m_Undo.push_back(std::move(act));
                    ClearRedo();
                }
            }
            if (ImGui::Selectable(""))
            {
                if (m_CurrentObject->GetTag() != "")
                {
                    ActionTagObject* act = new ActionTagObject(m_CurrentObject->GetTag(), "", m_CurrentObject);
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
        ImGui::Text("Num of Components : %d", list.size());
        ImGui::Separator();
        if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponentPopup");
        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            ImGui::Text("Components");
            ImGui::Separator();
            for (int i = 0; i < AddComp.size(); ++i)
                if (ImGui::Selectable(AddComp[i].c_str()))
                {
                    ActionAddComponent* act = new ActionAddComponent(m_CurrentObject, AddComp[i], m_CurrentLayer);
                    act->Execute();
                    m_Undo.push_back(std::move(act));
                    ClearRedo();
                }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove Component")) ImGui::OpenPopup("RemoveComponentPopup");
        if (ImGui::BeginPopup("RemoveComponentPopup"))
        {
            ImGui::Text("Components");
            ImGui::Separator();
            for (int i = 0; i < DelComp.size(); ++i)
            {
                if (DelComp[i] == "LuaScript")
                {
                    auto scripts = m_CurrentObject->GetScripts();
                    for (size_t j = 0; j < scripts.size(); ++j)
                    {
                        if (ImGui::Selectable((std::string("Script:") + scripts[j]).c_str()))
                        {
                            ActionRevertScript* act = new ActionRevertScript(m_CurrentObject, scripts[j], m_CurrentLayer);
                            act->Execute();
                            m_Undo.push_back(std::move(act));
                            ClearRedo();
                        }
                    }
                }
                else if (ImGui::Selectable(DelComp[i].c_str()))
                {
                    ActionRemoveComponent* act = new ActionRemoveComponent(m_CurrentObject, DelComp[i], m_CurrentLayer);
                    act->Execute();
                    m_Undo.push_back(std::move(act));
                    ClearRedo();
                }
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) m_CurrentObject->UpdateComponents();
        ImGui::Separator();
        int i = 0;
        for (auto& comp : list)
        {
            if (ImGui::TreeNode((std::string("Comp ") + std::to_string(i) + std::string(" : ") + comp->GetName()).c_str()))
            {
                char* address = reinterpret_cast<char*>(comp);
                // ImGui::Text("Component Num : %d", i);
                // ImGui::Text("Component Name : %s", comp->GetName().c_str());
                try
                {
                    if (!Factory::m_Reflection->at(comp->GetName().c_str())->getParents().empty())
                        ParentInspector(address, comp->GetName(), Factory::m_Reflection->at(comp->GetName().c_str())->getParents().back().key);
                    auto component = (*Factory::m_Reflection).at(comp->GetName().c_str());
                    auto properties = component->getProperties();
                    ImGui::Indent(5.0f);
                    for (int i = 0; i < properties.size(); ++i)
                    {
                        ImGui::Text("%s : ", properties[i].name.c_str());
                        // if else for each type
                        if (properties[i].type == typeid(std::string).name())
                        {
                            ImGui::SameLine();
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
                            ImGui::SameLine();
                            Options(reinterpret_cast<float*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if (properties[i].type == typeid(int).name())
                        {
                            ImGui::SameLine();
                            Options(reinterpret_cast<int*>(address + properties[i].offset), m_CurrentObject, comp->GetName(), properties[i].name);
                        }
                        else if (properties[i].type == typeid(bool).name())
                        {
                            ImGui::SameLine();
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
                        ImGui::ListBox("SoundTracks", &currentSound, vector_getter, &SoundTracks, (int)SoundTracks.size(), 4);
                        if (currentSound != s)
                        {
                            if (!sound->IsPlaying())
                                sound->SetAudioClip(SoundTracks[currentSound]);
                            else
                                sound->SetAndPlayAudioClip(SoundTracks[currentSound]);
                            std::string oldValue = s < 0 ? std::string {} : SoundTracks[s];
                            std::string newValue = SoundTracks[currentSound];
                            ActionInput<std::string>* act =
                                new ActionInput<std::string>(oldValue, newValue, m_CurrentObject->GetName(), "AudioEmitter", "audioClipName_", m_CurrentLayer, m_CurrentObject);
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
                        if (ImGui::Button(p.c_str())) sound->Play();
                        ImGui::SameLine();
                        if (ImGui::Button(stop.c_str())) sound->Stop();
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
                                ImGui::Text("%s : ", var.first.c_str());
                                m_Global_Spaces += " ";
                                ImGui::SameLine();
                                ImGui::Checkbox(m_Global_Spaces.c_str(), &tmp);
                                if (b != tmp)
                                {
                                    int oldValue = b;
                                    int newValue = tmp;
                                    ActionInputScript<bool>* act = new ActionInputScript<bool>(m_CurrentObject, filename, oldValue, newValue, var.first);
                                    act->Execute();
                                    m_Undo.push_back(std::move(act));
                                    ClearRedo();
                                }
                            }
                            else if (var.second == "float")
                            {
                                float f = script->get<float>(var.first);
                                float tmp = f;
                                ImGui::Text("%s : ", var.first.c_str());
                                m_Global_Spaces += " ";
                                ImGui::SameLine();
                                ImGui::InputFloat(m_Global_Spaces.c_str(), &tmp);
                                if (ImGui::IsKeyPressed(KEY_RETURN) && f != tmp)
                                {
                                    float oldValue = f;
                                    float newValue = tmp;
                                    ActionInputScript<float>* act = new ActionInputScript<float>(m_CurrentObject, filename, oldValue, newValue, var.first);
                                    act->Execute();
                                    m_Undo.push_back(std::move(act));
                                    ClearRedo();
                                }
                            }
                            else if (var.second == "int")
                            {
                                int i = script->get<int>(var.first);
                                int tmp = i;
                                ImGui::Text("%s : ", var.first.c_str());
                                m_Global_Spaces += " ";
                                ImGui::SameLine();
                                ImGui::InputInt(m_Global_Spaces.c_str(), &tmp);
                                if (ImGui::IsKeyPressed(KEY_RETURN) && i != tmp)
                                {
                                    int oldValue = i;
                                    int newValue = tmp;
                                    ActionInputScript<int>* act = new ActionInputScript<int>(m_CurrentObject, filename, oldValue, newValue, var.first);
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
                                ImGui::Text("%s : ", var.first.c_str());
                                m_Global_Spaces += " ";
                                ImGui::SameLine();
                                ImGui::InputText(m_Global_Spaces.c_str(), string, MAX_STRING_LENGTH);
                                if (ImGui::IsKeyPressed(KEY_RETURN) && s != string)
                                {
                                    std::string oldValue = s;
                                    std::string newValue = string;
                                    ActionInputScript<std::string>* act = new ActionInputScript<std::string>(m_CurrentObject, filename, oldValue, newValue, var.first);
                                    act->Execute();
                                    m_Undo.push_back(std::move(act));
                                    ClearRedo();
                                }
                            }
                        }
                    }
                    ImGui::Unindent(5.0f);
                }
                catch (...)
                {
                    ImGui::Text("Component %s not in reflection factory!", comp->GetName().c_str());
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            ++i;
        }
    }
}

void Editor::SetEditorMouse()
{
    ImGuiIO& io = ImGui::GetIO();
    // set up the imgui io
    Vector2 mousePos = GetInstance(Input).GetMousePosition();
    io.MousePos = ImVec2(mousePos.x, io.DisplaySize.y - mousePos.y);
    io.MouseDown[0] = GetInstance(Input).GetKeyDown(MOUSE_LEFT);
    io.MouseDown[1] = GetInstance(Input).GetKeyDown(MOUSE_RIGHT);
    io.MouseClickedPos[0] = ImVec2(mousePos.x, io.DisplaySize.y - mousePos.y);
    io.MouseClickedPos[1] = ImVec2(mousePos.x, io.DisplaySize.y - mousePos.y);
}

Editor::Editor(HWND hwnd, float x, float y)
    : m_Console {}
    , m_RedoUndoCount { 20 }
    , m_CurrentObject { nullptr }
    , m_Global_Spaces {}
    , m_GlobalIDCounter { 0 }
    , m_CurrentLayer(nullptr)
    , m_IsEditorInFocus(false)
    , m_CurrentWorkingDirectory(_getcwd(0, 0))
    , m_WindowStates()
    , m_Timer(0)
    , m_OutlinerSelectedObject(nullptr)
    , m_IsViewportFullScreen(false)
    , m_IsTransformInLocalSpace(false)
    , m_IsGameCameraAcitve(false)
{
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplWin32_Init(static_cast<void*>(hwnd));
    ImGui_ImplOpenGL3_Init();

    unsigned char* pixels;
    int width, height;

    for (int i = 0; i < NUM_OF_WINDOWS; ++i)
    {
        m_WindowStates[i] = false;
    }

    LoadData();

    for (int n = 0; n < IM_ARRAYSIZE(m_SavedPalettes); n++)
    {
        ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f, m_SavedPalettes[n].x, m_SavedPalettes[n].y, m_SavedPalettes[n].z);
        m_SavedPalettes[n].w = 1.0f;  // Alpha
    }

    SecureZeroMemory(m_EditorStringBuffer, 2 << 23);
    DeSerializeArchetypes();
    LoadTags();

    m_ImGuiFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\calibri.ttf", 13.0f);
    m_ImGuiFontBold = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\calibrib.ttf", 23.0f);
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
}

Editor::~Editor()
{
    SerializeArchetypes();
    SaveData();
    SaveTags();
    ClearRedoUndo();
    for (auto& a : m_Archetypes)
    {
        delete a.second;
    }
    for (auto& a : m_UpdatedArchetypes)
    {
        delete a.second;
    }
    // Need to save the archetype

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();
    free(m_CurrentWorkingDirectory);
}

void Editor::Update(float dt)
{
    ImGui::GetIO().DeltaTime = dt;

    UpdateMeshArray();

    if (m_CurrentLayer == nullptr)
    {
        m_CurrentLayer = Application::Instance().GetCurrentScene()->GetLayers().back();
    }

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    SetEditorMouse();
    ShortcutButtons();
    CreateNewSceneTab();
    // DrawStyleEditor();
    // DrawHelp();

    DrawMenuBar();

    ImGui::SetNextWindowPos(ImVec2 { 0.0f, 0.0f });
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("main_window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar);
    ImGui::DockSpace(ImGui::GetID("main_dock_space"));

    if (Application::Instance().GetGameTimer().IsEditorPaused() || !m_IsViewportFullScreen)
    {
        ImGuizmo::BeginFrame();
    }

    if (Application::Instance().GetGameTimer().IsEditorPaused() || !m_IsViewportFullScreen)
    {
        if (m_WindowStates[(int)EditorWindowType::Viewport])
        {
            ImGui::Begin("ViewPort");
            DrawViewport();
            ImGui::End();
        }
    }
    else
    {
        // FullScreen Viewport here
        DrawFullScreenViewport();
    }

    if (m_WindowStates[(int)EditorWindowType::Physics])
    {
        ImGui::Begin("Physics");
        DrawPhysicsEditor();
        ImGui::End();
    }

    if (m_WindowStates[(int)EditorWindowType::Outliner])
    {
        ImGui::Begin("Outliner");
        DrawOutliner();
        ImGui::End();
    }

    if (m_WindowStates[(int)EditorWindowType::Inspector])
    {
        ImGui::Begin("Inspector");
        DrawInspector();
        ImGui::End();
    }

    if (m_WindowStates[(int)EditorWindowType::Archetype])
    {
        ImGui::Begin("Archetype");
        DrawArchetype();
        ImGui::End();
    }

    if (m_WindowStates[(int)EditorWindowType::LayerEditor])
    {
        ImGui::Begin("LayerEditor");
        DrawLayerEditor();
        ImGui::End();
    }

    if (m_WindowStates[(int)EditorWindowType::Profiler])
    {
        ImGui::Begin("Profiler");
        DrawProfiler();
        ImGui::End();
    }

    if (m_WindowStates[(int)EditorWindowType::Tags])
    {
        ImGui::Begin("Tags");
        DrawTagsEditor();
        ImGui::End();
    }

    if (m_WindowStates[(int)EditorWindowType::TextEditor])
    {
        ImGui::Begin("TextEditor");
        DrawTextEditor();
        ImGui::End();
    }

    if (m_WindowStates[(int)EditorWindowType::Resource])
    {
        ImGui::Begin("ResourceManager");
        DrawResourceManager();
        ImGui::End();
    }

    m_Console.Draw();

    UpdateRedoUndo();
    m_Global_Spaces.clear();

    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    m_DeltaMousePos = ImGui::GetMousePos();
}
