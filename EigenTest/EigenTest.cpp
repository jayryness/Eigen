#include <Windows.h>
#include <cstdio>
#include "Eigen/Win/EngineWin.h"
#include "Eigen/Display.h"
#include "Eigen/Texture.h"
#include "Eigen/Render.h"
#include "Eigen/Stage.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static bool g_quit = false;

HWND Init()
{
    WNDCLASS wc = { sizeof(WNDCLASSEX) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LONG_PTR);
    wc.hInstance = GetModuleHandle(NULL);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"Eigen";
    ATOM atom = RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, L"Eigen Test", WS_OVERLAPPEDWINDOW, 0, 0, 600, 600, NULL, NULL, wc.hInstance, NULL);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE || message == WM_NCCREATE)
    {
        return 1;
    }

    LRESULT result = 0;

    switch (message)
    {
    case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
        }
        return 0;

    case WM_DISPLAYCHANGE:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_PAINT:
        ValidateRect(hwnd, NULL);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        g_quit = true;
        return 1;

    case WM_MOUSEMOVE:
        {
            float x = 0.f;
            float y = 0.f;
            //x -= float(g_mx) / pDemoApp->m_pRenderTarget->GetSize().width;
            //y -= float(g_my) / pDemoApp->m_pRenderTarget->GetSize().height;

            //g_mx = LOWORD(lParam);
            //g_my = HIWORD(lParam);

            //x += float(g_mx) / pDemoApp->m_pRenderTarget->GetSize().width;
            //y += float(g_my) / pDemoApp->m_pRenderTarget->GetSize().height;

            //g_dx += x*100.f;
            //g_dy += y*100.f;

            //float norm = sqrt(g_dx*g_dx + g_dy*g_dy);
            //g_dx /= norm;
            //g_dy /= norm;

            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

int main(int argc, const char** argv)
{
    printf("Initializing...");

    HWND hwnd = Init();
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    const char* error = nullptr;
    Eigen::Engine::Win::Params engineParams;
    engineParams.hwnds = &hwnd;
    engineParams.hwndCount = 1;
    engineParams.enableDebug = true;
    Eigen::Engine::Win engine(engineParams, &error);

    if (error != nullptr)
    {
        printf("Engine initialization error:\n%s\n", error);
        getchar();
        return 0;
    }

    Eigen::Display* display = engine.GetDisplaySystem().NextDisplay();

    Eigen::RenderSystem& renderSystem = engine.GetRenderSystem();
    const Eigen::RenderPort* port = renderSystem.GetPort("One");
    renderSystem.GetPort("One");
    renderSystem.GetPort("Two");
    renderSystem.GetPort("Three");
    renderSystem.GetPort("Four");
    renderSystem.GetPort("Five");
    renderSystem.GetPort("Six");
    renderSystem.GetPort("Seven");
    renderSystem.GetPort("Eight");

    Eigen::TextureSystem& textureSystem = engine.GetTextureSystem();
    Eigen::TextureTargetPtr target;
    Eigen::TextureTarget::Params targetParams;
    targetParams.width = uint16_t(clientRect.right - clientRect.left);
    targetParams.height = uint16_t(clientRect.bottom - clientRect.top);
    targetParams.format = Eigen::Texture::cFormat_RGB10_A2;
    target = display->GetTarget();// textureSystem.CreateTarget(targetParams);
    //if (!target)
    //{
    //    printf("Render target creation failed.\n");
    //    getchar();
    //    return 0;
    //}

    Eigen::TargetGroup::Params testGroupParams;
    Eigen::TargetGroupPtr testGroup;
    testGroupParams.targets[0] = target.ptr;
    testGroup = textureSystem.CreateTargetGroup(testGroupParams);

    printf(" done.\n");

    HWND consoleHwnd = GetConsoleWindow();
    ShowWindow(consoleHwnd, SW_MINIMIZE);

    printf("Running.\n");

    while (!g_quit)
    {
        MSG msg;
        while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
        {
            DispatchMessage(&msg);
        }

        Eigen::RenderSystem& renderSystem = engine.GetRenderSystem();
        Eigen::RenderContext* context = renderSystem.BeginContext();
        context->CommitBatch(port, Eigen::RenderBatch(), 0.f);
        Eigen::Stage::Params stageParams;
        stageParams.type = Eigen::Stage::cType_Clear;
        stageParams.clearParams.flags = Eigen::Stage::cClearFlag_Color;
        stageParams.clearParams.colors[0][0] = 0.7f;
        stageParams.clearParams.colors[0][1] = 0.8f;
        stageParams.clearParams.colors[0][2] = 0.9f;
        stageParams.clearParams.colors[0][3] = 1.f;
        stageParams.targetGroup = testGroup;
        Eigen::Stage stage(stageParams);
        renderSystem.EndContext(context, &stage, 1);
        engine.SubmitFrame();

        engine.GetDisplaySystem().PresentAll();
    }

    printf("Bye\n");
    Sleep(1000);

    return 0;
}
