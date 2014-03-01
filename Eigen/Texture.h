#pragma once

#include "Util.h"
#include <cstdint>

namespace Eigen
{


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Texture
    //

    class Texture : public RefCounted<Texture>
    {
    public:

        enum Format
        {
            cFormat_Unspecified     = 0,
            cFormat_BC1,
            cFormat_BC2,
            cFormat_BC3,
            cFormat_BC4,
            cFormat_BC5,
            cFormat_BC6,
            cFormat_BC7,
            cFormat_RGBA8,
            cFormat_RGB10_A2,
            cFormat_RGBA16,
            cFormat_RGBA16f,
            cFormat_RGBA32f,
            cFormat_Count
        };

        enum Flags
        {
            cFlag_Dynamic           = (1 << 0),
            cFlag_CubeMap           = (1 << 1),
        };

        struct Params
        {
            uint8_t     format      = cFormat_Unspecified;
            uint8_t     reserved    = 0;
            uint8_t     flags       = 0;
            uint8_t     lastMip     = 0;
            uint16_t    width       = 0;
            uint16_t    height      = 0;
            uint16_t    depth       = 0;
            uint16_t    arrayLength = 0;
        };

        const Params&   GetParams() const;

        class           Details;

    protected:

        Params          m_params;

                        Texture(const Params& params) : m_params(params) {}
                       ~Texture() {}
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // TextureTarget
    //
    // Note: TextureTarget is bit-for-bit with Texture, but doesn't inherit. Behind the scenes,
    // they can be backed by the same object type, though that may vary by platform.

    class TextureTarget : public RefCounted<TextureTarget>
    {
    public:

        enum Format
        {
            cFormat_Unspecified     = 0,
            cFormat_RGBA8           = Texture::cFormat_RGBA8,
            cFormat_RGB10_A2        = Texture::cFormat_RGB10_A2,
            cFormat_RGBA16          = Texture::cFormat_RGBA16,
            cFormat_RGBA16f         = Texture::cFormat_RGBA16f,
            cFormat_RGBA32f         = Texture::cFormat_RGBA32f,
            cFormat_D24_S8          = Texture::cFormat_Count,
            cFormat_D32f,
            cFormat_D32f_S8,
        };

        enum Multisampling
        {
            cMultisampling_1x       = 0,
            cMultisampling_2x,
            cMultisampling_4x,
            cMultisampling_8x,
        };

        enum Flags
        {
            cFlag_CpuReadAccess     = (1 << 2),
        };

        struct Params
        {
            uint8_t     format      = cFormat_Unspecified;
            uint8_t     multisampling = cMultisampling_1x;
            uint8_t     flags       = 0;
            uint8_t     lastMip     = 0;
            uint16_t    width       = 0;
            uint16_t    height      = 0;
            uint16_t    depth       = 0;
            uint16_t    arrayLength = 0;
        };

        struct Slice
        {
            uint16_t    mip         = 0;
            uint16_t    planeOffset = 0;
        };

        const Params&   GetParams() const;
        operator        Texture*();

    protected:

        Params          m_params;

                        TextureTarget() {}
                       ~TextureTarget() {}
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // TargetGroup
    //

    class TargetGroup : public RefCounted<TargetGroup>
    {
    public:

        enum { cMaxTargets = 8 };

        struct Params
        {
            TextureTarget*          targets     [cMaxTargets];
            TextureTarget::Slice    targetSlices[cMaxTargets];
            TextureTarget*          zbuffer                     = nullptr;
            TextureTarget::Slice    zbufferSlice;
            uint32_t                planes                      = 0;
            Params();
        };

        const Params&               GetParams() const;

        class                       Details;

    private:

        Params                      m_params;

                                    TargetGroup() {}
                                   ~TargetGroup() {}
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // TextureSystem
    //

    typedef RefPtr<Texture>         TexturePtr;
    typedef RefPtr<TextureTarget>   TextureTargetPtr;
    typedef RefPtr<TargetGroup>     TargetGroupPtr;

    class TextureSystem
    {
    public:

        TexturePtr        CreateTexture(const Texture::Params& params);
        TextureTargetPtr  CreateTarget(const TextureTarget::Params& params);
        TargetGroupPtr    CreateTargetGroup(const TargetGroup::Params& params);

        class Details;

    protected:

        TextureSystem() {}
    };


    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////


    inline const Texture::Params& Texture::GetParams() const
    {
        return m_params;
    }

    inline const TextureTarget::Params& TextureTarget::GetParams() const
    {
        return m_params;
    }

    inline TextureTarget::operator Texture*()
    {
        return reinterpret_cast<Texture*>(this);
    }

    inline const TargetGroup::Params& TargetGroup::GetParams() const
    {
        return m_params;
    }

    inline TargetGroup::Params::Params()
    {
        memset(targets, 0, sizeof(targets));
    }

}
