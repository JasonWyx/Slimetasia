// ===========================================================================|
// INCLUDES                                                                   |
// ===========================================================================|
#include "Application.h"

#include <GL/wglew.h>

#include <array>
#include <iostream>
#include <limits>
#include <sstream>

#include "AISystem.h"
#include "AnimationSystem.h"
#include "AudioSystem.h"
#include "CollisionMesh_3D.h"
#include "Editor.h"
#include "Factory.h"
#include "Input.h"
#include "MeshRenderer.h"
#include "ParticleSystem.h"
#include "PhysicsSystem.h"
#include "Reflection.h"
#include "Renderer.h"
#include "RendererVk.h"
#include "ResourceImporter.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "Timer.h"
#include "luascript.h"

std::streambuf* Application::oss = std::cout.rdbuf();
std::ostringstream Application::os;
bool Application::s_IsGameRunning = false;

#define MAXCOUNT 1000000

#define DESIRED_FRAME_RATE 1.f / 60.f

const DWORD Application::s_WindowStyles[] = {
    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,  // windowed
    WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,                        // borderless
    WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE                         // fullscreen
};
const DWORD Application::s_WindowStylesEx[] = {
    WS_EX_APPWINDOW | WS_EX_ACCEPTFILES,  // windowed
    WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,   // borderless
    WS_EX_APPWINDOW | WS_EX_WINDOWEDGE    // fullscreen
};

LRESULT CALLBACK Application::WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    Application* app = Application::InstancePtr();

    switch (message)
    {
        case WM_CREATE:
        {
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            Input::Instance().SetMousePosition(LOWORD(lParam), app->m_WindowHeight - HIWORD(lParam));
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        {
            return 0;
        }
        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            GetInstance(Editor).KeyDown(wParam, !(lParam & 0x80000000));
            return 0;
        }
        /// Droping files into the window
        case WM_DROPFILES:
        {
            HDROP dropInfo = (HDROP)wParam;
            UINT amount = DragQueryFile(dropInfo, 0xFFFFFFFF, NULL, NULL);

            std::vector<std::filesystem::path> failedFiles;
            std::vector<std::filesystem::path> successFiles;

            /// Check for each files
            for (unsigned i = 0; i < amount; ++i)
            {
                TCHAR buff[MAX_PATH] = { 0 };
                DragQueryFile(dropInfo, i, buff, MAX_PATH);

                std::filesystem::path filePath = buff;

                /// Check if file type is what we accept
                if (RESOURCEMANAGER.CopyNewResource(filePath, hwnd))
                    successFiles.push_back(filePath);
                else
                    failedFiles.push_back(filePath);
            }

            // Handle special case for skybox
            if (successFiles.size() == 6)
            {
                bool right = false;
                bool left = false;
                bool top = false;
                bool bottom = false;
                bool front = false;
                bool back = false;

                for (const std::filesystem::path& s : successFiles)
                {
                    if (s.filename().string().rfind("_ft") != std::string::npos) front = true;
                    if (s.filename().string().rfind("_bk") != std::string::npos) back = true;
                    if (s.filename().string().rfind("_rt") != std::string::npos) right = true;
                    if (s.filename().string().rfind("_lf") != std::string::npos) left = true;
                    if (s.filename().string().rfind("_up") != std::string::npos) top = true;
                    if (s.filename().string().rfind("_dn") != std::string::npos) bottom = true;
                }

                if (right && left && top && bottom && front && back)
                {
                    ResourceImporter::ImportTexture(successFiles);
                }
                else  // 6 resources that are not skybox
                {
                    for (const auto& filePath : successFiles)
                    {
                        switch (RESOURCEMANAGER.GetResourceType(filePath.string()))
                        {
                            case ResourceType::Audio: RESOURCEMANAGER.CreateResource<AudioResource>(filePath.filename().replace_extension().string(), filePath)->Load(); break;
                            case ResourceType::Texture: ResourceImporter::ImportTexture({ filePath.c_str() }); break;
                            case ResourceType::Model: ResourceImporter::ImportModel(filePath.c_str()); break;
                            case ResourceType::Font: RESOURCEMANAGER.CreateResource<Font>(filePath.filename().replace_extension().string(), filePath)->Load(); break;
                        }
                    }
                }
            }
            else
            {
                /// Load in new file as a resource
                for (const auto& filePath : successFiles)
                {
                    switch (RESOURCEMANAGER.GetResourceType(filePath.string()))
                    {
                        case ResourceType::Audio: RESOURCEMANAGER.CreateResource<AudioResource>(filePath.filename().replace_extension().string(), filePath)->Load(); break;
                        case ResourceType::Texture: ResourceImporter::ImportTexture({ filePath.c_str() }); break;
                        case ResourceType::Model: ResourceImporter::ImportModel(filePath.c_str()); break;
                        case ResourceType::Font: RESOURCEMANAGER.CreateResource<Font>(filePath.filename().replace_extension().string(), filePath)->Load(); break;
                    }
                }
            }

            // Display failed/Success files
            if (failedFiles.size() > 0)
            {
                std::string errorMessage { "The following files failed to be added : \n" };

                for (int i = 0; i < failedFiles.size(); ++i)
                {
                    errorMessage += failedFiles[i].string() + " \n";
                }
                errorMessage += "Legal file types : \n";

                // Fetch legal file extensions
                const auto& types = RESOURCEMANAGER.GetAllAcceptableFileTypes();

                errorMessage += "Audio: ";
                for (const auto& type : types[static_cast<int>(ResourceType::Audio)])
                {
                    errorMessage += type + " ";
                }
                errorMessage += "\n";

                errorMessage += "Texture: ";
                for (const auto& type : types[static_cast<int>(ResourceType::Texture)])
                {
                    errorMessage += type + " ";
                }
                errorMessage += "\n";

                errorMessage += "Model: ";
                for (const auto& type : types[static_cast<int>(ResourceType::Model)])
                {
                    errorMessage += type + " ";
                }
                errorMessage += "\n";

                MessageBox(hwnd, LPCSTR(errorMessage.c_str()), NULL, MB_OK);
            }
            if (successFiles.size() > 0)
            {
                std::string errorMessage { "The following files have been added : \n" };

                for (int i = 0; i < successFiles.size(); ++i)
                    errorMessage += successFiles[i].filename().replace_extension().string() + " \n";

                MessageBox(hwnd, LPCSTR(errorMessage.c_str()), "Success", MB_OK);
            }

            return 0;
        }
        case WM_CHAR:
        {
            GetInstance(Editor).CharInput(static_cast<char>(wParam));
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            short offset = ((short)wParam - delta) / 120;  // scroll is in multiples of 120
            GetInstance(Editor).MouseScroll(-offset);
            return 0;
        }
        case WM_SETFOCUS:
        {
            if (Application::InstancePtr()) Application::Instance().m_focus = false;
            return 0;
        }
        case WM_KILLFOCUS:
        {
            if (Application::InstancePtr()) Application::Instance().m_focus = true;
            return 0;
        }
        case WM_SIZE:
        {
            hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            if (Application::InstancePtr())
            {
                Application::Instance().SetWindowWidth(LOWORD(lParam));
                Application::Instance().SetWindowHeight(HIWORD(lParam));
                Renderer::Instance().SetWindowSize(iVector2(LOWORD(lParam), HIWORD(lParam)));
            }
            return 0;
        }
        case WM_PAINT:
        {
            hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            if (app != nullptr)
            {
                app->QuitProgram();
            }
            return 0;
        }

    }  // switch(message)

    return DefWindowProc(hwnd, message, wParam, lParam);
}

Application::Application(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
    : ISystem<Application>()
    , m_Instance(hInstance)
    , m_Window()
    , m_DeviceContext()
    , m_GLRenderContext()
    , m_WindowMessage()
    , m_WindowMode(WindowMode::Windowed)
    , m_WindowWidth(0)
    , m_WindowHeight(0)
    , m_IsFocused()
    , m_CurrentScene(nullptr)
    , m_CurrentSceneCopy(nullptr)
    , m_IsQuitting(false)
    , m_IsFullscreen(false)
    , m_IsLoadingNewScene(false)
    , m_IsReloadingScene(false)
    , m_Pool()
    , m_focus(false)
{
    char const* appName = "Slimetasia";

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wcex.lpfnWndProc = WindowProcedure;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = appName;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, TEXT("Program requires Windows NT!"), appName, MB_ICONERROR);
    }

// #define LEGACY_GL_INIT
#ifndef LEGACY_GL_INIT

    HWND dummyWND = CreateWindowEx(s_WindowStylesEx[(int)WindowMode::Windowed], appName, "Slimetasia", s_WindowStyles[(int)WindowMode::Windowed], 0, 0, 1, 1, nullptr, nullptr, hInstance, nullptr);

    HDC dummyDC = GetDC(dummyWND);

    PIXELFORMATDESCRIPTOR dummyPFD = { sizeof(PIXELFORMATDESCRIPTOR),
                                       1,
                                       PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,  // Flags
                                       PFD_TYPE_RGBA,                                               // The kind of framebuffer. RGBA or palette.
                                       32,                                                          // Colordepth of the framebuffer.
                                       0,
                                       0,
                                       0,
                                       0,
                                       0,
                                       0,
                                       0,
                                       0,
                                       0,
                                       0,
                                       0,
                                       0,
                                       0,
                                       24,  // Number of bits for the depthbuffer
                                       8,   // Number of bits for the stencilbuffer
                                       0,   // Number of Aux buffers in the framebuffer.
                                       PFD_MAIN_PLANE,
                                       0,
                                       0,
                                       0,
                                       0 };

    int dummyPF = ChoosePixelFormat(dummyDC, &dummyPFD);

    if (!dummyPF)
    {
        printf_s("ERROR - Failed to get PixelFormat\n");
        ReleaseDC(dummyWND, dummyDC);
        return;
    }

    if (!SetPixelFormat(dummyDC, dummyPF, &dummyPFD))
    {
        std::cout << "ERROR: Failed to set PixelFormat" << std::endl;
        ReleaseDC(dummyWND, dummyDC);
        return;
    }

    HGLRC dummyRC = wglCreateContext(dummyDC);

    if (!dummyRC)
    {
        printf_s("ERROR - Failed to create GL context\n");
        ReleaseDC(dummyWND, dummyDC);
        return;
    }

    if (!wglMakeCurrent(dummyDC, dummyRC))
    {
        printf_s("ERROR - Failed to make GL context current\n");
        wglDeleteContext(dummyRC);
        ReleaseDC(dummyWND, dummyDC);
        return;
    }

    if (glewInit())
    {
        std::cout << "ERROR: GLEW failed to initialize!" << std::endl;
    }
    if (wglewInit())
    {
        std::cout << "ERROR: WGLEW failed to initialize!" << std::endl;
    }

#ifndef EDITOR

    // Get screen resolution
    m_MaxWindowWidth = GetSystemMetrics(SM_CXSCREEN);
    m_MaxWindowHeight = GetSystemMetrics(SM_CYSCREEN);

    RECT wndRect = { 0, 0, m_MaxWindowWidth, m_MaxWindowHeight };

    // Get actual usable window size
    AdjustWindowRectEx(&wndRect, s_WindowStyles[(int)WindowMode::Windowed], TRUE, s_WindowStylesEx[(int)WindowMode::Windowed]);

#else
    RECT wndRect;

    SystemParametersInfoA(SPI_GETWORKAREA, 0, &wndRect, 0);

    // Minus task bar
    wndRect.bottom -= GetSystemMetrics(SM_CYCAPTION);

#endif

    m_WindowWidth = wndRect.right - wndRect.left;
    m_WindowHeight = wndRect.bottom - wndRect.top;

    m_Window = CreateWindowEx(s_WindowStylesEx[(int)WindowMode::Windowed], appName, "Slimetasia", s_WindowStyles[(int)WindowMode::Windowed], wndRect.top, wndRect.left, m_WindowWidth,
                              m_WindowHeight,  // Let AdjustWindowRectEx calculate window size for us
                              nullptr, nullptr, hInstance, nullptr);

    m_DeviceContext = GetDC(m_Window);
    // Set window size and show
    ShowWindow(m_Window, SW_SHOWMAXIMIZED);
    UpdateWindow(m_Window);

    DEVMODE devMode;
    devMode.dmSize = sizeof(DEVMODE);

    // Retrieve current device settings into devMode
    if (!EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &devMode)) return;

    const int pixelAttribs[] = { WGL_DRAW_TO_WINDOW_ARB,
                                 GL_TRUE,
                                 WGL_SUPPORT_OPENGL_ARB,
                                 GL_TRUE,
                                 WGL_DOUBLE_BUFFER_ARB,
                                 GL_TRUE,
                                 WGL_PIXEL_TYPE_ARB,
                                 WGL_TYPE_RGBA_ARB,
                                 WGL_ACCELERATION_ARB,
                                 WGL_FULL_ACCELERATION_ARB,
                                 WGL_COLOR_BITS_ARB,
                                 32,
                                 WGL_ALPHA_BITS_ARB,
                                 8,
                                 WGL_DEPTH_BITS_ARB,
                                 24,
                                 WGL_STENCIL_BITS_ARB,
                                 8,
                                 0 };

    int pfdIndex = 0;
    unsigned numFormats = 0;
    bool status = wglChoosePixelFormatARB(m_DeviceContext, pixelAttribs, nullptr, 1, &pfdIndex, &numFormats);

    if (status == false || numFormats == 0)
    {
        printf_s("ERROR - Failed to get PixelFormat\n");
        ReleaseDC(m_Window, m_DeviceContext);
        return;
    }

    PIXELFORMATDESCRIPTOR pfd = {};
    DescribePixelFormat(m_DeviceContext, pfdIndex, sizeof(pfd), &pfd);
    SetPixelFormat(m_DeviceContext, pfdIndex, &pfd);

    const int majorMin = 4;
    const int minorMin = 5;
    int contextAttribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB,
                             majorMin,
                             WGL_CONTEXT_MINOR_VERSION_ARB,
                             minorMin,
                             WGL_CONTEXT_FLAGS_ARB,
#ifdef EDITOR
                             WGL_CONTEXT_DEBUG_BIT_ARB,
#else
                             WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#endif
                             WGL_CONTEXT_PROFILE_MASK_ARB,
                             WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                             0 };

    m_GLRenderContext = wglCreateContextAttribsARB(m_DeviceContext, 0, contextAttribs);
    if (m_GLRenderContext == NULL)
    {
        printf_s("ERROR - Failed to create GL context\n");
        ReleaseDC(m_Window, m_DeviceContext);
        return;
    }

    // Creating a dummy context so that we can load the wrangled functions
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(dummyRC);
    ReleaseDC(dummyWND, dummyDC);
    DestroyWindow(dummyWND);

    if (!wglMakeCurrent(m_DeviceContext, m_GLRenderContext))
    {
        printf_s("ERROR - Failed to make GL context current\n");
        wglDeleteContext(m_GLRenderContext);
        ReleaseDC(m_Window, m_DeviceContext);
        return;
    }

    if (glewInit())
    {
        std::cout << "ERROR: GLEW failed to initialize!" << std::endl;
    }
    if (wglewInit())
    {
        std::cout << "ERROR: WGLEW failed to initialize!" << std::endl;
    }

#else  // #ifndef MODERN_GL_INIT

    // Get screen resolution
    m_MaxWindowWidth = GetSystemMetrics(SM_CXSCREEN);
    m_MaxWindowHeight = GetSystemMetrics(SM_CYSCREEN);

    RECT rect;
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &rect, 0);

    uint y = GetSystemMetrics(SM_CYCAPTION);

    rect.bottom -= y;

    RECT wndRect = rect;

    // Get actual usable window size
    // AdjustWindowRectEx(&wndRect, dwStyle, FALSE, 0);

    m_WindowWidth = wndRect.right - wndRect.left;
    m_WindowHeight = wndRect.bottom - wndRect.top;

    m_Window = CreateWindowEx(s_WindowStylesEx[(int)WindowMode::Windowed], appName, "Slimetasia", s_WindowStyles[(int)WindowMode::Windowed], wndRect.top, wndRect.left, m_WindowWidth,
                              m_WindowHeight,  // Let AdjustWindowRectEx calculate window size for us
                              nullptr, nullptr, hInstance, nullptr);

    // Set window size and show
    ShowWindow(m_Window, SW_SHOWMAXIMIZED);

    UpdateWindow(m_Window);

    m_DeviceContext = GetDC(m_Window);

    DEVMODE devMode;
    devMode.dmSize = sizeof(DEVMODE);

    // Retrieve current device settings into devMode
    if (!EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &devMode)) return;

    PIXELFORMATDESCRIPTOR pfd;
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = (BYTE)devMode.dmBitsPerPel;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

    int pf = ChoosePixelFormat(m_DeviceContext, &pfd);

    if (!pf)
    {
        printf_s("ERROR - Failed to get PixelFormat\n");
        ReleaseDC(m_Window, m_DeviceContext);
        return;
    }

    if (!SetPixelFormat(m_DeviceContext, pf, &pfd))
    {
        std::cout << "ERROR: Failed to set PixelFormat" << std::endl;
        ReleaseDC(m_Window, m_DeviceContext);
        return;
    }

    if (!(m_GLRenderContext = wglCreateContext(m_DeviceContext)))
    {
        printf_s("ERROR - Failed to create GL context\n");
        ReleaseDC(m_Window, m_DeviceContext);
        return;
    }

    if (!(wglMakeCurrent(m_DeviceContext, m_GLRenderContext)))
    {
        printf_s("ERROR - Failed to make GL context current\n");
        wglDeleteContext(m_GLRenderContext);
        ReleaseDC(m_Window, m_DeviceContext);
        return;
    }
    if (glewInit())
    {
        std::cout << "ERROR: GLEW failed to initialize!" << std::endl;
    }
    if (wglewInit())
    {
        std::cout << "ERROR: WGLEW failed to initialize!" << std::endl;
    }

#endif  // #ifdef MODERN_GL_INIT

#ifdef EDITOR
    wglSwapIntervalEXT(1);
    DragAcceptFiles(m_Window, true);
    // std::cout.rdbuf(os.rdbuf());
#else
    wglSwapIntervalEXT(1);
#endif

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    m_GameTimer = Timer(1.f / 60.0f);
    m_ComputeTimer = Timer();

    Factory::Initialize();
    AudioSystem::Initialize();
    PhysicsSystem::Initialize();
    AnimationSystem::Initialize();
    ResourceManager::Initialize();
    ResourceManager::Instance().RefreshResources();
    AISystem::Initialize();

#if defined(USE_VULKAN_RENDERER)
    RendererVk::Initialize(hInstance, m_Window, m_WindowWidth, m_WindowHeight);
#else
    Renderer::Initialize(iVector2(m_WindowWidth, m_WindowHeight));
#endif  // #if defined(USE_VULKAN_RENDERER)

    Input::Initialize();
    Editor::Initialize(m_Window, static_cast<float>(m_WindowWidth), static_cast<float>(m_WindowHeight));
    ParticleSystem ::Initialize(MAXCOUNT);
}

Application::~Application()
{
    if (m_CurrentScene)
    {
        delete m_CurrentScene;
    }
    if (m_CurrentSceneCopy)
    {
        delete m_CurrentSceneCopy;
    }

    Editor::Shutdown();
    Input::Shutdown();
#if defined(USE_VULKAN_RENDERER)
    RendererVk::Shutdown();
#else
    Renderer::Shutdown();
#endif  // #if defined(USE_VULKAN_RENDERER)
    AISystem::Shutdown();
    ResourceManager::Shutdown();
    AnimationSystem::Shutdown();
    PhysicsSystem::Shutdown();
    AudioSystem::Shutdown();
    Factory::Shutdown();
    ParticleSystem ::Shutdown();

    wglDeleteContext(m_GLRenderContext);
    ReleaseDC(m_Window, m_DeviceContext);
    DestroyWindow(m_Window);
    UnregisterClass("Slimetasia", m_Instance);
}

void Application::ProcessWindowMessages()
{
    // Process all messages
    while (PeekMessage(&m_WindowMessage, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&m_WindowMessage);
        DispatchMessage(&m_WindowMessage);

        if (m_WindowMessage.message == WM_QUIT) return;
    }
}

void Application::ToggleFullScreen(bool isFullscreen)
{
    m_IsFullscreen = isFullscreen;

    if (m_IsFullscreen)
    {
        RECT wndRect = { 0, 0, m_MaxWindowWidth, m_MaxWindowHeight };

        // Set window styles
        SetWindowLongPtr(m_Window, GWL_STYLE, s_WindowStyles[(int)WindowMode::Borderless]);
        SetWindowLongPtr(m_Window, GWL_EXSTYLE, s_WindowStylesEx[(int)WindowMode::Borderless]);

        // Get actual usable window size
        AdjustWindowRectEx(&wndRect, s_WindowStyles[(int)WindowMode::Borderless], FALSE, s_WindowStylesEx[(int)WindowMode::Borderless]);

        m_WindowWidth = wndRect.right - wndRect.left;
        m_WindowHeight = wndRect.bottom - wndRect.top;

        // Set window size and show
        SetWindowPos(m_Window, 0, 0, 0, m_WindowWidth, m_WindowHeight, SWP_SHOWWINDOW);
        ShowWindow(m_Window, SW_SHOWMAXIMIZED);
    }
    else
    {
        RECT wndRect = { 0, 0, m_MaxWindowWidth, m_MaxWindowHeight };

        // Set window styles
        SetWindowLongPtr(m_Window, GWL_STYLE, s_WindowStyles[(int)WindowMode::Windowed]);
        SetWindowLongPtr(m_Window, GWL_EXSTYLE, s_WindowStylesEx[(int)WindowMode::Windowed]);

        // Get actual usable window size
        AdjustWindowRectEx(&wndRect, s_WindowStyles[(int)WindowMode::Windowed], TRUE, s_WindowStylesEx[(int)WindowMode::Windowed]);

        m_WindowWidth = wndRect.right - wndRect.left;
        m_WindowHeight = wndRect.bottom - wndRect.top;

        // Set window size and show
        SetWindowPos(m_Window, 0, 0, 0, m_WindowWidth, m_WindowHeight, SWP_SHOWWINDOW);
        ShowWindow(m_Window, SW_SHOWMAXIMIZED);
    }
}

void Application::RunMainLoop()
{
#ifdef EDITOR
    NewScene("NewScene");
    Layer* layer = m_CurrentScene->CreateLayer("NewLayer");
    // Renderer::Instance().SetCurrentLayer(layer);
    m_GameTimer.SetEditorPaused(true);
#else
    ToggleFullScreen(true);
    // LoadScene("Resources/Level_Changi.xml");
    // LoadScene("Resources/mainMenu_test.xml");
    LoadScene("Resources/Level_DigipenLogo.xml");

    m_GameTimer.SetEditorPaused(false);
#endif
    float inputTime = 0.f, physicsTime = 0.f, gameobjTime = 0.f, animationTime = 0.f, ParticleTime = 0.f, Aitime = 0.f, audioTime = 0.f, renderTime = 0.f, editorTime = 0.f;

    while (!m_IsQuitting)
    {
        ProcessWindowMessages();

        float accumulator = 0.f;

        if (m_IsLoadingNewScene)
        {
            Serializer(m_NewSceneFileName).LoadScene();
            Renderer::Instance().SetCurrentLayer(m_CurrentScene->GetLayers().front());
            Editor::Instance().SetLayer(m_CurrentScene->GetLayers().front());
            m_IsLoadingNewScene = false;
        }

        if (Input::Instance().GetKeyPressed(KEY_F11))
        {
            ToggleFullScreen(!m_IsFullscreen);
        }

        if (m_GameTimer.GetScaledFrameTime())
            Input::Instance().ToggleMouseWrap(true);
        else
            Input::Instance().ToggleMouseWrap(false);

        m_GameTimer.EndTimer();
        float scaledFrameTime = m_GameTimer.GetScaledFrameTime();
        float actualFrameTime = m_GameTimer.GetActualFrameTime();
        float physicsFrameTime = scaledFrameTime > DESIRED_FRAME_RATE ? DESIRED_FRAME_RATE : scaledFrameTime;
        m_GameTimer.StartTimer();  // Sets current to start, previous loop

        m_ComputeTimer.StartTimer();
        Input::Instance().Update();
        inputTime = m_ComputeTimer.GetTimePassed();

        m_ComputeTimer.StartTimer();
        PhysicsSystem::Instance().Update(physicsFrameTime);
        physicsTime = m_ComputeTimer.GetTimePassed();

        m_ComputeTimer.StartTimer();
        m_CurrentScene->Update(scaledFrameTime);
        gameobjTime = m_ComputeTimer.GetTimePassed();

        m_ComputeTimer.StartTimer();
        AnimationSystem::Instance().Update(scaledFrameTime);
        animationTime = m_ComputeTimer.GetTimePassed();

        m_ComputeTimer.StartTimer();
        ParticleSystem::Instance().Update(scaledFrameTime);
        ParticleTime = m_ComputeTimer.GetTimePassed();

        m_ComputeTimer.StartTimer();
        AISystem::Instance().Update(scaledFrameTime);
        Aitime = m_ComputeTimer.GetTimePassed();

        m_ComputeTimer.StartTimer();
        AUDIOSYSTEM.Update(scaledFrameTime);
        audioTime = m_ComputeTimer.GetTimePassed();

        m_ComputeTimer.StartTimer();
#if defined(USE_VULKAN_RENDERER)
        RendererVk::Instance().Update(scaledFrameTime);
#else
        Renderer::Instance().Update(scaledFrameTime);
#endif  // #if defined(USE_VULKAN_RENDERER)
        renderTime = m_ComputeTimer.GetTimePassed();

#ifdef EDITOR
        m_ComputeTimer.StartTimer();
        Editor::Instance().Update(actualFrameTime);
        editorTime = m_ComputeTimer.GetTimePassed();
#endif

        scaledFrameTime -= physicsFrameTime;

        while (scaledFrameTime > physicsFrameTime)
        {
            PhysicsSystem::Instance().Update(physicsFrameTime);
            scaledFrameTime -= physicsFrameTime;
        }

#ifdef EDITOR
        Editor::Instance().SystemTimer(inputTime, physicsTime, gameobjTime, renderTime, editorTime, audioTime, animationTime, ParticleTime, Aitime);

        GetInstance(Editor).GetConsole().AddLog(os.str().c_str());
        os.str("");
        os.clear();
#endif

        SwapBuffers(m_DeviceContext);
    }
}

void Application::QuitProgram()
{
    m_IsQuitting = true;
}

void Application::NewScene(char const* sceneName)
{
    Scene* newScene = new Scene(sceneName);

    if (m_CurrentScene)
    {
        delete m_CurrentScene;
    }

    m_CurrentScene = newScene;
}

void Application::LoadScene(char const* fileName)
{
    m_IsLoadingNewScene = true;
    m_NewSceneFileName = fileName;
}

void Application::ReloadScene()
{
    m_IsReloadingScene = true;
}

Scene* Application::GetCurrentScene() const
{
    return m_CurrentScene;
}

bool Application::IsFocused() const
{
    return m_IsFocused;
}

WindowMode Application::GetWindowMode() const
{
    return m_WindowMode;
}

int Application::GetWindowWidth() const
{
    return m_WindowWidth;
}

int Application::GetWindowHeight() const
{
    return m_WindowHeight;
}

HWND Application::GetWindowHandle() const
{
    return m_Window;
}

Timer& Application::GetGameTimer()
{
    return m_GameTimer;
}

void Application::SetWindowWidth(int width)
{
    m_WindowWidth = width;
}

void Application::SetWindowHeight(int height)
{
    m_WindowHeight = height;
}

int Application::GetMaxWindowWidth() const
{
    return m_MaxWindowWidth;
}

int Application::GetMaxWindowHeight() const
{
    return m_MaxWindowHeight;
}

ThreadPool& Application::GetThreadPool()
{
    return m_Pool;
}
