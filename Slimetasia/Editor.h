#pragma once
#pragma once
#include <Windows.h>

#include <set>

#include "Action.h"
#include "AppConsole.h"
#include "Application.h"
#include "CorePrerequisites.h"
#include "External Libraries\imgui\imgui.h"
#include "ISystem.h"
#include "Input.h"
#include "Serializer.h"
#include "Texture.h"

class Editor : public ISystem<Editor>
{
    friend class ISystem<Editor>;

    enum class WindowType
    {
        Outliner = 0,
        Inspector,
        Viewport,
        Archetype,
        TextEditor,
        LayerEditor,
        Profiler,
        Tags,
        Physics,
        Resource,

        Count
    };

    unsigned m_GlobalIDCounter;
    std::string m_Global_Spaces;
    std::list<Action*> m_Redo;
    std::list<Action*> m_Undo;
    unsigned m_RedoUndoCount;
    AppConsole m_Console;
    GameObject* m_CurrentObject;
    std::vector<GameObject*> m_SelectedObjects;
    bool m_WindowStates[static_cast<unsigned>(WindowType::Count)];
    std::map<std::string, GameObject*> m_Archetypes;
    std::map<std::string, GameObject*> m_UpdatedArchetypes;
    Layer* m_CurrentLayer;
    ImVec4 m_SavedPalettes[32];
    ImVec4 m_MeshDragColor;
    char m_EditorStringBuffer[2 << 23];
    char m_EditorFile[MAX_PATH];
    std::string m_EditorFileName;
    bool m_IsEditorInFocus;
    char* m_CurrentWorkingDirectory;
    float m_Timings[9];
    std::list<Layer*> m_SavedLayers;
    std::vector<std::string> m_TextureNames;
    std::vector<ResourceGUID> m_TextureIDs;
    std::vector<std::string> m_MeshNames;
    std::vector<ResourceGUID> m_MeshIDs;
    std::vector<std::string> m_AnimationNames;
    std::vector<ResourceGUID> m_AnimationIDs;
    std::vector<std::string> m_FontNames;
    std::vector<ResourceGUID> m_FontIDs;
    int m_Timer;
    GameObject* m_OutlinerSelectedObject;
    ImVec2 m_DeltaMousePos;
    bool m_IsViewportFullScreen;
    std::set<std::string> m_Tags;
    bool m_IsTransformInLocalSpace;
    bool m_IsGameCameraAcitve;

    void DrawOutliner();
    void DrawInspector();
    void SetEditorMouse();
    void UpdateRedoUndo();
    void ClearRedoUndo();

    void DrawMenuBar();
    void ShortcutButtons();

    void DrawStyleEditor();
    void DrawHelp();
    void DrawViewport();
    void DrawArchetype();
    void ParentArchetypeInspector(char* address, std::string parent, GameObject* currentArchetype);
    void ParentInspector(char* address, std::string comp, std::string parent);
    void StructInspector(char* address, std::string comp);
    void ClearRedo();

    void DrawTextEditor();
    void DrawLayerEditor();
    void DrawProfiler();
    void DrawFullScreenViewport();
    void DrawTagsEditor();
    void DrawPhysicsEditor();
    void DrawResourceManager();

    // Physics Options
    void PhysicsOptionsFloat(float* f);
    void PhysicsOptionsBool(bool* b);
    void PhysicsOptionsVector3(Vector3* v);
    void PhysicsOptionsUint(uint* u);

    // Options overloading
    void Options(std::string* s, GameObject* go, std::string c, std::string p);
    void Options(std::string* s);
    void Options(Vector3* vec3, GameObject* go, std::string c, std::string p);
    void Options(float* f, GameObject* go, std::string c, std::string p);
    void Options(int* i, GameObject* go, std::string c, std::string p);
    void Options(bool* b, GameObject* go, std::string c, std::string p);
    void Options(Color4* clr, GameObject* go, std::string c, std::string p);
    void Options(HMesh mesh, GameObject* go, std::string c, std::string p);
    void Options(HTexture texture, GameObject* go, std::string c, std::string p);
    void Options(HAnimationSet anim, GameObject* go, std::string c, std::string p);
    void Options(Vector2* vec2, GameObject* go, std::string c, std::string p);
    void Options(iVector2* vec2, GameObject* go, std::string c, std::string p);
    void Options(HFont font, GameObject* go, std::string c, std::string p);
    void EnumOptions(int* i, std::string t, GameObject* go, std::string c, std::string p);

    // ParentStruct Options still need abit more options
    void ParentStructOptions(std::string* s, GameObject* go, std::string c);
    void ParentStructOptions(float* f, GameObject*& go, std::string c);
    void ParentStructOptions(int* i, GameObject*& go, std::string c);
    void ParentStructOptions(bool* b, GameObject*& go, std::string c);
    void ParentStructOptions(Vector3* vec3, GameObject*& go, std::string c);
    void ParentStructOptions(iVector2* vec2, GameObject*& go, std::string c);
    void ParentStructOptions(Vector4* clr, GameObject*& go, std::string c, std::string p);
    void ParentStructOptions(HTexture* texture, GameObject*& go, std::string c);
    void ParentStructOptions(Vector2* vec2, GameObject*& go, std::string c);
    void ParentStructEnumOptions(int* i, std::string t, GameObject*& go, std::string c);
    // Need Color4 Options

    void Save();
    void Load();
    void CreateNewSceneTab();
    void Duplicate();
    void ChildrenDuplicate(GameObject* original, GameObject* Clone);

    void LoadData();
    void SaveData();
    GameObject* CreateArchetype(const std::string& n);

    void RecursionSaveArchetypeParent(const char* name, unsigned char* base, tinyxml2::XMLElement* pComponent, tinyxml2::XMLDocument& doc);
    void RecursionSaveArchetypeStruct(registration::variant prop, unsigned char* base, tinyxml2::XMLElement* pComponent, tinyxml2::XMLDocument& doc);
    void SerializeArchetypes();
    void RecursionLoadArchetypeStruct(tinyxml2::XMLElement* attribute, unsigned char* base);
    void RecursionLoadArchetypeParent(tinyxml2::XMLElement* attribute, unsigned char* base, const char* comp);
    void DeSerializeArchetypes();

    void UpdateMeshArray();
    void RecursiveParentAndChildObject(GameObject* go, std::string s, int& selected);

    void LoadTags();
    void SaveTags();

public:

    // x and y are the display size
    Editor(HWND hwnd, float x, float y);
    ~Editor();

    void Update(float dt);

    void Undo();
    void Redo();
    void SetUndoRedoSize(const unsigned& sz);
    AppConsole& GetConsole();
    void KeyDown(WPARAM key, bool pressed);
    void CharInput(char c);
    void MouseScroll(short s);
    void AddArchetype(GameObject* go, GameObject* ugo);
    void RemoveArchetype(GameObject* go, GameObject* ugo);
    GameObject* RevertArchetype(GameObject* go);
    GameObject* MakeChangesArchetype(GameObject* go);
    GameObject* GetArchetype(std::string n);
    GameObject* CreateArchetypeObject(std::string n);
    GameObject* LuaCreateArchetypeObject(std::string n, Layer* layer);
    void SetLayer(Layer* ly);
    void SystemTimer(float input, float phy, float Comps, float renderer, float editor, float audio, float anim, float particle, float ai);
    void SetCurrentObject(GameObject* go);
    GameObject* GetCurrentObject();
    std::set<std::string> GetTags();
    void AddTag(std::string t);
    void RemoveTag(std::string t);

    static bool ms_ShowDebug;
    static bool ms_ShowBoundingVolume;
    static bool ms_ShouldLockMousePosition;
    static bool ms_IsGameRunning;
};
