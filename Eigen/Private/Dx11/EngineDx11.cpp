#include "../Placeholder.h"
#include "../Memory.h"
#include "EngineDx11.h"

namespace Eigen
{

Placeholder<EngineCore> g_coreMem;
EngineCore& EngineCore::s_instance = (EngineCore&) g_coreMem.storage;

Engine::Engine()
: m_renderSystem(EngineCore::s_instance.renderSystem)
, m_displaySystem(EngineCore::s_instance.displaySystem)
, m_textureSystem(EngineCore::s_instance.textureSystem)
{
}

void Engine::SubmitFrame()
{
    EngineCore::s_instance.renderSystem.SubmitFrame();
    EngineCore::s_instance.textureSystem.CollectGarbage(); // TODO - this actually needs to happen at the end of the submission thread if SubmitFrame is asynchronous
}

void Engine::Sync()
{

}

Engine::Win::Win(const Params& params, const char** error)
{
    Memory::g_allocator = params.allocator;

    new(g_coreMem.Init()) EngineCore(error, params);

    D3D_FEATURE_LEVEL featureLevels [] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

    HRESULT hr = D3D11CreateDevice(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        params.enableDebug ? D3D11_CREATE_DEVICE_DEBUG : 0,
        featureLevels,
        sizeof(featureLevels) / sizeof(*featureLevels),
        D3D11_SDK_VERSION,
        &EngineCore::s_instance.device,
        NULL,
        &EngineCore::s_instance.immContext);

    if (FAILED(hr))
    {
        *error = "D3D11 device creation failed.";
        return;
    }

    // Retrieve dxgi factory
    {
        ComPtr<IDXGIDevice> dxgiDevice;
        ComPtr<IDXGIAdapter> dxgiAdapter;
        hr = EngineCore::s_instance.device->QueryInterface(__uuidof(dxgiDevice), &dxgiDevice);
        hr = dxgiDevice.Get()->GetParent(__uuidof(dxgiAdapter), &dxgiAdapter);
        hr = dxgiAdapter.Get()->GetParent(__uuidof(EngineCore::s_instance.dxgiFactory), (void**) &EngineCore::s_instance.dxgiFactory);
    }

    if (FAILED(hr))
    {
        *error = "DXGI factory acquisition failed.";
        return;
    }

    for (unsigned i = 0; i < params.hwndCount; i++)
    {
        Display::Win* display = Display::Win::SafeCast(EngineCore::s_instance.displaySystem.Create());
        if (!display->Bind(params.hwnds[i]))
        {
            *error = "Display window bind failed.";
            return;
        }
    }
}

Engine::Win::~Win()
{
    EngineCore::s_instance.~EngineCore();
}

EngineCore::EngineCore(const char** error, const Engine::Win::Params& params)
: device(nullptr)
, immContext(nullptr)
, dxgiFactory(nullptr)
, displaySystem(error)
, textureSystem(error)
, renderSystem(error, params.scratchMemSize)
{
}

EngineCore::~EngineCore()
{
    if (immContext != nullptr)
        immContext->Release();

    if (device != nullptr)
        device->Release();

    if (dxgiFactory != nullptr)
        dxgiFactory->Release();
}

}
