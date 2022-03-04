#pragma once
#include <Windows.h>

#include <string>

#include "ISystem.h"
#include "ThreadPool.h"
#include "Timer.h"

#define MAXNUMOFTHREADS 4

class Scene;

enum class WindowMode
{
    Windowed = 0,
    Borderless,
    Fullscreen
};

class Application : public ISystem<Application>
{
private:
    // Windows API variables
    HINSTANCE m_Instance;
    HWND m_Window;
    HDC m_DeviceContext;
    HGLRC m_GLRenderContext;
    MSG m_WindowMessage;

    // Window options
    WindowMode m_WindowMode;
    int m_WindowWidth;
    int m_WindowHeight;

    // Window status
    bool m_IsFocused;
    bool m_IsQuitting;

    // Scene data
    bool m_IsLoadingNewScene;
    bool m_IsReloadingScene;
    std::string m_NewSceneFileName;
    Scene* m_CurrentScene;
    Scene* m_CurrentSceneCopy;  // Used for scene reloading

    Timer m_GameTimer;
    Timer m_ComputeTimer;

    // Full Screen Flag
    bool m_IsFullscreen;
    int m_MaxWindowWidth;
    int m_MaxWindowHeight;

    ThreadPool m_Pool;

    static const DWORD s_WindowStyles[];
    static const DWORD s_WindowStylesEx[];

    static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void ProcessWindowMessages();

public:
    Application(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow);
    ~Application();

    void RunMainLoop();
    void QuitProgram();
    void ToggleFullScreen(bool isFullscreen);
    void NewScene(char const* sceneName);
    void LoadScene(char const* fileName);
    void ReloadScene();
    Scene* GetCurrentScene() const;
    bool IsFocused() const;
    WindowMode GetWindowMode() const;
    int GetWindowWidth() const;
    int GetWindowHeight() const;
    HWND GetWindowHandle() const;
    Timer& GetGameTimer();
    void SetWindowWidth(int width);
    void SetWindowHeight(int h);
    int GetMaxWindowWidth() const;
    int GetMaxWindowHeight() const;
    ThreadPool& GetThreadPool();
    bool m_focus;
    static std::streambuf* oss;
    static std::ostringstream os;
    static bool s_IsGameRunning;

    HINSTANCE& GetAppInstance() { return m_Instance; }
};
