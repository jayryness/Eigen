#include "DisplayDx11.h"
#include "EngineDx11.h"
#include "TextureDx11.h"
#include "../Memory.h"
#include "../Placeholder.h"

namespace Eigen
{

    DisplaySystem::Details::Details(const char** error)
    : m_first(nullptr)
    {
    }

    DisplaySystem::Details::~Details()
    {
        while (m_first != nullptr)
            Destroy(m_first);
    }

    Display::Win* Display::Win::SafeCast(Display* display)
    {
        return (Details*)display;
    }

    bool Display::Win::Bind(HWND hwnd)
    {
        Details* details = (Details*)this;
        ComPtr<IDXGISwapChain> swapChain;

        TextureTarget::Format format    = TextureTarget::cFormat_RGB10_A2;
        DXGI_FORMAT dxgiFormat          = DXGI_FORMAT_R10G10B10A2_UNORM;

        DXGI_SWAP_CHAIN_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.SampleDesc.Count = 1;
        desc.BufferDesc.Format = dxgiFormat;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 2;
        desc.OutputWindow = hwnd;
        desc.Windowed = true;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        HRESULT hr = EngineCore::s_instance.dxgiFactory->CreateSwapChain(EngineCore::s_instance.device, &desc, &swapChain);
        if (FAILED(hr))
        {
            return false;
        }

        ID3D11Texture2D* surface = nullptr;
        hr = swapChain->GetBuffer(0, __uuidof(surface), (void**) &surface);
        if (FAILED(hr))
        {
            return false;
        }

        TextureTarget::Params params;
        params.format = format;
        params.width = desc.BufferDesc.Width;
        params.height = desc.BufferDesc.Height;
        details->m_target = EngineCore::s_instance.textureSystem.CreateTargetWithResource(params, surface);
        details->m_swapChain.Swap(swapChain);

        return true;
    }

    void Display::Present()
    {
        Details* details = (Details*) this;
        details->m_swapChain.Get()->Present(0, 0);
    }

    Display* DisplaySystem::Create()
    {
        Display::Details** tail = &((Details*) this)->m_first;
        while (*tail != nullptr)
            tail = (*tail)->Next();

        *tail = new(Memory::Allocate<Display::Details>()) Display::Details();
        return *tail;
    }

    void DisplaySystem::Destroy(Display* display)
    {
        Display::Details** tail = &((Details*) this)->m_first;
        while (*tail != display)
            tail = (*tail)->Next();

        Display::Details* displayDetails = (Display::Details*) display;
        *tail = *(displayDetails->Next());

        Memory::Destroy<Display::Details>(displayDetails);
    }

    Display* DisplaySystem::NextDisplay(Display* prev)
    {
        if (prev == nullptr)
            return ((Details*) this)->m_first;
        Display::Details* display = (Display::Details*) prev;
        return *display->Next();
    }

    void DisplaySystem::PresentAll()
    {
        for (Display* display = NextDisplay(nullptr); display != nullptr; display = NextDisplay(display))
            display->Present();
    }

}
