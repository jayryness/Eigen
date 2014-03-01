#pragma once

#include "Allocator.h"

namespace Eigen
{

    class DisplaySystem;
    class TextureSystem;
    class RenderSystem;


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Engine
    //

    class Engine
    {
    public:

        struct Params
        {
            Allocator*  allocator       = Mallocator::Get();
            unsigned    scratchMemSize  = 16*1024*1024;
            bool        enableDebug     = false;
        };

        DisplaySystem&  GetDisplaySystem();
        TextureSystem&  GetTextureSystem();
        RenderSystem&   GetRenderSystem();

        void            SubmitFrame();
        void            Sync();

        class           Win;

    protected:
        Engine();

        TextureSystem&  m_textureSystem;
        DisplaySystem&  m_displaySystem;
        RenderSystem&   m_renderSystem;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

    inline DisplaySystem& Engine::GetDisplaySystem()
    {
        return m_displaySystem;
    }

    inline TextureSystem& Engine::GetTextureSystem()
    {
        return m_textureSystem;
    }

    inline RenderSystem& Engine::GetRenderSystem()
    {
        return m_renderSystem;
    }
}
