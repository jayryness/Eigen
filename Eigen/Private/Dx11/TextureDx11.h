#pragma once

#include "Texture.h"
#include "CommonDx11.h"
#include <type_traits>

namespace Eigen
{

    class Texture::Details : public Texture
    {
    public:

        explicit Details(const Params& params)
        : Texture(params)
        , m_next(nullptr)
        {}

        Details*                m_next;
        ComPtr<ID3D11Resource>  m_d3dResource;
    };

    class TargetGroup::Details : public TargetGroup
    {
    public:

        explicit Details(const TargetGroup::Params& params);
        ~Details();

        unsigned                        m_viewCount;
        ID3D11RenderTargetView*         m_d3dTargetViews[cMaxTargets];
        ComPtr<ID3D11DepthStencilView>  m_d3dDepthStencilView;
        ComPtr<ID3D11DepthStencilView>  m_d3dReadOnlyDepthStencilView;
    };


    class TextureSystem::Details : public TextureSystem
    {
    public:

        Details(const char** error);
        ~Details();

        void CollectGarbage();

        TextureTargetPtr CreateTargetWithResource(const TextureTarget::Params& params, ID3D11Resource* d3dResource);

    private:

        friend void DestroyRefCounted(Texture*);
        friend void DestroyRefCounted(TextureTarget*);
        friend void DestroyRefCounted(TargetGroup*);
    };


}
