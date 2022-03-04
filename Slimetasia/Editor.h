#pragma once
#include <Windows.h>

#include <set>

#include "Actions.h"
#include "AppConsole.h"
#include "Application.h"
#include "CorePrerequisites.h"
#include "Dock.h"
#include "External Libraries\imgui\imgui.h"
#include "ISystem.h"
#include "Input.h"
#include "Serializer.h"
#include "Texture.h"

#define NUM_OF_WINDOWS 10
#define MAX_ARCHETYPES 100

using namespace ImGui;
namespace TX = tinyxml2;

class Editor : public ISystem<Editor>
{
    friend class ISystem<Editor>;

    enum WindowStates
    {
        Outliner_State = 0,
        Inspector_State = 1,
        Viewport_State = 2,
        Archetype_State = 3,
        TextEditor_State = 4,
        LayerEditor_State = 5,
        Profiler_State = 6,
        Tags_State = 7,
        Physics_State = 8,
        Resource_State = 9
    };

    unsigned m_GlobalIDCounter;
    std::string m_Global_Spaces;
    std::list<Actions*> m_Redo;
    std::list<Actions*> m_Undo;
    unsigned m_Redo_Undo_Size;
    AppConsole m_Console;
    GameObject* m_CurrentObject;
    std::vector<GameObject*> m_SelectedObjects;
    bool m_ActiveWindow[NUM_OF_WINDOWS];
    std::map<std::string, GameObject*> m_Archetypes;
    std::map<std::string, GameObject*> m_UpdatedArchetypes;
    Layer* m_CurrentLayer;
    ImVec4 m_saved_palette[32];
    ImVec4 m_mesh_drag_color;
    char m_text[2 << 23];
    char m_editorFile[MAX_PATH];
    std::string m_editorFileName;
    bool m_editorFocus;
    char* m_currwdir;
    float m_timings[9];
    std::list<Layer*> m_SavedLayers;
    std::vector<std::string> m_texture_names;
    std::vector<ResourceGUID> m_texture_ids;
    std::vector<std::string> m_mesh_names;
    std::vector<ResourceGUID> m_mesh_ids;
    std::vector<std::string> m_anim_names;
    std::vector<ResourceGUID> m_anim_ids;
    std::vector<std::string> m_font_names;
    std::vector<ResourceGUID> m_font_ids;
    int m_timer;
    GameObject* m_selectedOutliner;
    ImVec2 m_DeltaMousePos;
    bool m_viewportFullScreen;
    std::set<std::string> m_tags;
    bool m_Local;
    bool m_GameCameraInVP;
    ImFont* m_font;
    ImFont* m_fontBold;

    void Outliner();
    void Inspector();
    void SetEditorMouse();
    void Update_Redo_Undo();
    void Clear_Redo_Undo();
    void MainMenu();
    void ShortcutButtons();
    void StyleEditor();
    void Help();
    void Viewport();
    void Archetype();
    void ParentArchetypeInspector(char* address, std::string parent, GameObject* currentArchetype);
    void ParentInspector(char* address, std::string comp, std::string parent);
    void StructInspector(char* address, std::string comp);
    void ClearRedo();
    void TextEditor();
    void LayerEditor();
    void Profiler();
    void FullScreenVP();
    void TagsEditor();
    void PhysicsEditor();
    void ResourceManager();

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
    void Options(TVector2<int>* vec2, GameObject* go, std::string c, std::string p);
    void Options(HFont font, GameObject* go, std::string c, std::string p);
    void EnumOptions(int* i, std::string t, GameObject* go, std::string c, std::string p);

    // ParentStruct Options still need abit more options
    void ParentStructOptions(std::string* s, GameObject* go, std::string c);
    void ParentStructOptions(float* f, GameObject*& go, std::string c);
    void ParentStructOptions(int* i, GameObject*& go, std::string c);
    void ParentStructOptions(bool* b, GameObject*& go, std::string c);
    void ParentStructOptions(Vector3* vec3, GameObject*& go, std::string c);
    void ParentStructOptions(TVector2<int>* vec2, GameObject*& go, std::string c);
    void ParentStructOptions(Vector4* clr, GameObject*& go, std::string c, std::string p);
    void ParentStructOptions(HTexture* texture, GameObject*& go, std::string c);
    void ParentStructOptions(Vector2* vec2, GameObject*& go, std::string c);
    void ParentStructEnumOptions(int* i, std::string t, GameObject*& go, std::string c);
    // Need Color4 Options

    void Save();
    void Load();
    void New();
    void Duplicate();
    void ChildrenDuplicate(GameObject* original, GameObject* Clone);

    void LoadData();
    void SaveData();
    GameObject* CreateArchetype(const std::string& n);

    void RecursionSaveArchetypeParent(const char* name, unsigned char* base, TX::XMLElement* pComponent, TX::XMLDocument& doc);
    void RecursionSaveArchetypeStruct(registration::variant prop, unsigned char* base, TX::XMLElement* pComponent, TX::XMLDocument& doc);
    void SerializeArchetypes();
    void RecursionLoadArchetypeStruct(TX::XMLElement* attribute, unsigned char* base);
    void RecursionLoadArchetypeParent(TX::XMLElement* attribute, unsigned char* base, const char* comp);
    void DeSerializeArchetypes();

    void UpdateMeshArray();
    void RecursiveParentAndChildObject(GameObject* go, std::string s, int& selected);

    void LoadTags();
    void SaveTags();

public:
    // x and y are the display size
    Editor(float x, float y);
    ~Editor();

    void Update(float dt);
    void Draw();
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
    void SetCurrentObject(GameObject* go)
    {
        m_CurrentObject = go;
        if (!m_CurrentObject) m_SelectedObjects.clear();
    }
    GameObject* GetCurrentObject() { return m_CurrentObject; }
    std::set<std::string> GetTags() { return m_tags; }
    void AddTag(std::string t);
    void RemoveTag(std::string t);

    static bool s_ShowDebug;
    static bool s_ShowBV;
    static bool s_lockMousePosition;
    static bool s_isPlaying;
};
