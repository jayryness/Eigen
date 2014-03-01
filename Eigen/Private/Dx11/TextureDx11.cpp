#include "TextureDx11.h"
#include "EngineDx11.h"
#include "../Placeholder.h"
#include "../Pool.h"

namespace Eigen
{
    static Placeholder<Pool<Texture::Details>>      g_texturePoolPlaceholder;
    static Placeholder<Pool<TargetGroup::Details>>  g_targetGroupPoolPlaceholder;
    static Pool<Texture::Details>&                  g_texturePool = (Pool<Texture::Details>&)g_texturePoolPlaceholder.storage;
    static Pool<TargetGroup::Details>&              g_targetGroupPool = (Pool<TargetGroup::Details>&)g_targetGroupPoolPlaceholder.storage;

    static std::atomic<Texture::Details*>           g_textureGarbage;

    static DXGI_FORMAT TranslateFormat(Texture::Format format)
    {
        switch (format)
        {
        case Texture::cFormat_BC1:              return DXGI_FORMAT_BC1_TYPELESS;
        case Texture::cFormat_BC2:              return DXGI_FORMAT_BC2_TYPELESS;
        case Texture::cFormat_BC3:              return DXGI_FORMAT_BC3_TYPELESS;
        case Texture::cFormat_BC4:              return DXGI_FORMAT_BC4_TYPELESS;
        case Texture::cFormat_BC5:              return DXGI_FORMAT_BC5_TYPELESS;
        case Texture::cFormat_BC6:              return DXGI_FORMAT_BC6H_TYPELESS;
        case Texture::cFormat_BC7:              return DXGI_FORMAT_BC7_TYPELESS;
        case Texture::cFormat_RGBA8:            return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case Texture::cFormat_RGB10_A2:         return DXGI_FORMAT_R10G10B10A2_TYPELESS;
        case Texture::cFormat_RGBA16:           return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        case Texture::cFormat_RGBA16f:          return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case Texture::cFormat_RGBA32f:          return DXGI_FORMAT_R32G32B32A32_FLOAT;
        default:                                return DXGI_FORMAT_UNKNOWN;
        }
    }

    static DXGI_FORMAT TranslateFormat(TextureTarget::Format format)
    {
        switch (format)
        {
        case TextureTarget::cFormat_RGBA8:      return DXGI_FORMAT_R8G8B8A8_UNORM;
        case TextureTarget::cFormat_RGB10_A2:   return DXGI_FORMAT_R10G10B10A2_UNORM;
        case TextureTarget::cFormat_RGBA16:     return DXGI_FORMAT_R16G16B16A16_UNORM;
        case TextureTarget::cFormat_RGBA16f:    return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case TextureTarget::cFormat_RGBA32f:    return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case TextureTarget::cFormat_D24_S8:     return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case TextureTarget::cFormat_D32f:       return DXGI_FORMAT_D32_FLOAT;
        case TextureTarget::cFormat_D32f_S8:    return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:                                return DXGI_FORMAT_UNKNOWN;
        }
    }

    void DescFromTargetParams(D3D11_RENDER_TARGET_VIEW_DESC& desc, const TextureTarget::Params& params, const TextureTarget::Slice& slice, unsigned planes)
    {
        desc.Format = DXGI_FORMAT_UNKNOWN;
        if (params.depth > 0)
        {
            assert(params.arrayLength == 0);
            assert(params.depth >= slice.planeOffset + planes);
            assert(params.multisampling == TextureTarget::cMultisampling_1x);
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
            desc.Texture3D.MipSlice = slice.mip;
            desc.Texture3D.FirstWSlice = slice.planeOffset;
            desc.Texture3D.WSize = planes;
        }
        else if (params.arrayLength > 0)
        {
            assert(params.arrayLength >= slice.planeOffset + planes);

            if (params.multisampling == TextureTarget::cMultisampling_1x)
            {
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                desc.Texture2DArray.MipSlice = slice.mip;
                desc.Texture2DArray.FirstArraySlice = slice.planeOffset;
                desc.Texture2DArray.ArraySize = planes;
            }
            else
            {
                assert(slice.mip == 0);
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                desc.Texture2DMSArray.FirstArraySlice = slice.planeOffset;
                desc.Texture2DMSArray.ArraySize = planes;
            }
        }
        else
        {
            if (params.multisampling == TextureTarget::cMultisampling_1x)
            {
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = slice.mip;
            }
            else
            {
                assert(slice.mip == 0);
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            }
        }
    }

    void DescFromTargetParams(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, const TextureTarget::Params& params, const TextureTarget::Slice& slice, unsigned planes)
    {
        desc.Format = DXGI_FORMAT_UNKNOWN;
        if (params.arrayLength > 0 || params.depth > 0)
        {
            assert((params.arrayLength > 0) ^ (params.depth > 0));
            assert(params.arrayLength == 0 || params.arrayLength >= slice.planeOffset + planes);
            assert(params.depth == 0 || params.depth >= slice.planeOffset + planes);

            if (params.multisampling == TextureTarget::cMultisampling_1x)
            {
                desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                desc.Texture2DArray.MipSlice = slice.mip;
                desc.Texture2DArray.FirstArraySlice = slice.planeOffset;
                desc.Texture2DArray.ArraySize = planes;
            }
            else
            {
                assert(slice.mip == 0);
                desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
                desc.Texture2DMSArray.FirstArraySlice = slice.planeOffset;
                desc.Texture2DMSArray.ArraySize = planes;
            }
        }
        else
        {
            if (params.multisampling == TextureTarget::cMultisampling_1x)
            {
                desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = slice.mip;
            }
            else
            {
                assert(slice.mip == 0);
                desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
            }
        }
        desc.Flags = 0;
    }

    static bool IsDepthFormat(TextureTarget::Format format)
    {
        return format == TextureTarget::cFormat_D24_S8 || format == TextureTarget::cFormat_D32f || format == TextureTarget::cFormat_D32f_S8;
    }

    TexturePtr TextureSystem::CreateTexture(const Texture::Params& params)
    {
        ComPtr<ID3D11Resource> d3dResource;

        if (params.depth > 0)
        {
            D3D11_TEXTURE3D_DESC desc;
            desc.Width = params.width;
            desc.Height = params.height;
            desc.Depth = params.depth;
            desc.MipLevels = params.lastMip+1;
            desc.Format = TranslateFormat((Texture::Format)params.format);
            desc.Usage = (params.flags & Texture::cFlag_Dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = (params.flags & Texture::cFlag_Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
            desc.MiscFlags = 0;

            HRESULT hr = EngineCore::s_instance.device->CreateTexture3D(&desc, nullptr, (ID3D11Texture3D**) d3dResource.GetAddressOf());
            if (FAILED(hr))
                return nullptr;
        }
        else if (params.height > 0)
        {
            D3D11_TEXTURE2D_DESC desc;
            desc.Width = params.width;
            desc.Height = params.height;
            desc.MipLevels = params.lastMip+1;
            desc.ArraySize = params.arrayLength > 0 ? params.arrayLength : 1;
            desc.Format = TranslateFormat((Texture::Format)params.format);
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = (params.flags & Texture::cFlag_Dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = (params.flags & Texture::cFlag_Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
            desc.MiscFlags = (params.flags & Texture::cFlag_CubeMap) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

            HRESULT hr = EngineCore::s_instance.device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**) d3dResource.GetAddressOf());
            if (FAILED(hr))
                return nullptr;
        }
        else
        {
            D3D11_TEXTURE1D_DESC desc;
            desc.Width = params.width;
            desc.MipLevels = params.lastMip+1;
            desc.ArraySize = params.arrayLength > 0 ? params.arrayLength : 1;
            desc.Format = TranslateFormat((Texture::Format)params.format);
            desc.Usage = (params.flags & Texture::cFlag_Dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = (params.flags & Texture::cFlag_Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
            desc.MiscFlags = 0;

            HRESULT hr = EngineCore::s_instance.device->CreateTexture1D(&desc, nullptr, (ID3D11Texture1D**) d3dResource.GetAddressOf());
            if (FAILED(hr))
                return nullptr;
        }

        Texture::Details* details = new(g_texturePool.Allocate()) Texture::Details(params);
        assert(details != nullptr);
        details->m_d3dResource.Swap(d3dResource);
        return details;
    }

    TextureTargetPtr TextureSystem::CreateTarget(const TextureTarget::Params& params)
    {
        ComPtr<ID3D11Resource> d3dResource;

        if (params.depth > 0)
        {
            D3D11_TEXTURE3D_DESC desc;
            desc.Width = params.width;
            desc.Height = params.height;
            desc.Depth = params.depth;
            desc.MipLevels = params.lastMip+1;
            desc.Format = TranslateFormat((TextureTarget::Format)params.format);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            desc.BindFlags |= IsDepthFormat((TextureTarget::Format)params.format) ? D3D11_BIND_DEPTH_STENCIL : 0;
            desc.CPUAccessFlags = (params.flags & TextureTarget::cFlag_CpuReadAccess) ? D3D11_CPU_ACCESS_READ : 0;
            desc.MiscFlags = 0;

            HRESULT hr = EngineCore::s_instance.device->CreateTexture3D(&desc, nullptr, (ID3D11Texture3D**) d3dResource.GetAddressOf());
            if (FAILED(hr))
                return nullptr;
        }
        else
        {
            D3D11_TEXTURE2D_DESC desc;
            desc.Width = params.width;
            desc.Height = params.height;
            desc.MipLevels = params.lastMip+1;
            desc.ArraySize = params.arrayLength > 0 ? params.arrayLength : 1;
            desc.Format = TranslateFormat((TextureTarget::Format)params.format);
            desc.SampleDesc.Count = 1 << params.multisampling;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            desc.BindFlags |= IsDepthFormat((TextureTarget::Format)params.format) ? D3D11_BIND_DEPTH_STENCIL : 0;
            desc.CPUAccessFlags = (params.flags & TextureTarget::cFlag_CpuReadAccess) ? D3D11_CPU_ACCESS_READ : 0;
            desc.MiscFlags = 0;

            HRESULT hr = EngineCore::s_instance.device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**) d3dResource.GetAddressOf());
            if (FAILED(hr))
                return nullptr;
        }

        Texture::Details* details = new(g_texturePool.Allocate()) Texture::Details((Texture::Params&)params);
        assert(details != nullptr);
        details->m_d3dResource.Swap(d3dResource);
        return (TextureTarget*)details;
    }

    TextureTargetPtr TextureSystem::Details::CreateTargetWithResource(const TextureTarget::Params& params, ID3D11Resource* d3dResource)
    {
        Texture::Details* details = new(g_texturePool.Allocate()) Texture::Details((Texture::Params&)params);
        assert(details != nullptr);
        details->m_d3dResource.Swap((ComPtr<ID3D11Resource>&)d3dResource);
        return (TextureTarget*) details;
    }

    TargetGroupPtr TextureSystem::CreateTargetGroup(const TargetGroup::Params& params)
    {
        TargetGroup::Details* targetGroup = new(g_targetGroupPool.Allocate()) TargetGroup::Details(params);
        assert(targetGroup != nullptr);
        TargetGroupPtr targetGroupPtr = targetGroup;

        const EngineCore& engine = EngineCore::s_instance;

        if (params.zbuffer != nullptr)
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC desc;
            DescFromTargetParams(desc, params.zbuffer->GetParams(), params.zbufferSlice, params.planes);

            HRESULT hr = engine.device->CreateDepthStencilView(((Texture::Details*)params.zbuffer)->m_d3dResource.Get(), &desc, targetGroup->m_d3dDepthStencilView.GetAddressOf());
            if (FAILED(hr))
                return nullptr;

            desc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
            hr = engine.device->CreateDepthStencilView(((Texture::Details*)params.zbuffer)->m_d3dResource.Get(), &desc, targetGroup->m_d3dReadOnlyDepthStencilView.GetAddressOf());
            if (FAILED(hr))
                return nullptr;
        }
        for (int i = 0; i < TargetGroup::cMaxTargets; i++)
        {
            if (params.targets[i] != nullptr)
            {
                D3D11_RENDER_TARGET_VIEW_DESC desc;
                DescFromTargetParams(desc, params.targets[i]->GetParams(), params.targetSlices[i], params.planes);
                HRESULT hr = engine.device->CreateRenderTargetView(((Texture::Details*)params.targets[i])->m_d3dResource.Get(), &desc, targetGroup->m_d3dTargetViews+i);
                if (FAILED(hr))
                    return nullptr;
            }
        }

        return targetGroup;
    }

    TargetGroup::Details::Details(const TargetGroup::Params& params)
    {
        AssignRef(m_params.zbuffer, params.zbuffer);
        for (m_viewCount = 0; m_viewCount < cMaxTargets && params.targets[m_viewCount] != nullptr; m_viewCount++)
            AssignRef(m_params.targets[m_viewCount], params.targets[m_viewCount]);
    }

    TargetGroup::Details::~Details()
    {
        AssignRef(m_params.zbuffer, nullptr);
        for (unsigned i = 0; i < m_viewCount; i++)
        {
            AssignRef(m_params.targets[i], nullptr);
            m_d3dTargetViews[i]->Release();
        }
    }

    void DestroyRefCounted(Texture* ptr)
    {
        // Defer destruction of textures so engine can ensure that they are all submitted first
        ((Texture::Details*)ptr)->m_next = g_textureGarbage.exchange((Texture::Details*)ptr, std::memory_order_relaxed);
    }

    void DestroyRefCounted(TextureTarget* ptr)
    {
        DestroyRefCounted((Texture*) ptr);
    }

    void DestroyRefCounted(TargetGroup* ptr)
    {
        g_targetGroupPool.Destroy((TargetGroup::Details*)ptr);
    }

    TextureSystem::Details::Details(const char** error)
    {
        new (g_texturePoolPlaceholder.Init()) Pool<Texture::Details>(256);
        new (g_targetGroupPoolPlaceholder.Init()) Pool<TargetGroup::Details>(64);
    }

    TextureSystem::Details::~Details()
    {
        CollectGarbage();
        g_texturePool.~Pool();
        g_targetGroupPool.~Pool();
    }

    void TextureSystem::Details::CollectGarbage()
    {
        Texture::Details* garbage = g_textureGarbage.exchange(nullptr, std::memory_order_relaxed);

        while (garbage != nullptr)
        {
            Texture::Details* next = garbage->m_next;
            g_texturePool.Destroy(garbage);
            garbage = next;
        }
    }

}
