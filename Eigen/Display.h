#pragma once

#include "Texture.h"

namespace Eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Display
    //

    class Display
    {
    public:

        TextureTarget*      GetTarget() const;

        void                Present();

        class               Win;
        class               Details;

    protected:

        Display()
        : m_next(nullptr)   {}
        ~Display()          {}

        Display*            m_next;
        TextureTargetPtr    m_target;

    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // DisplaySystem
    //

    class DisplaySystem
    {
    public:
        static DisplaySystem&   Get();

        Display*                Create();
        void                    Destroy(Display* display);

        void                    PresentAll();

        Display*                NextDisplay(Display* prev = nullptr);

        class                   Details;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

    inline TextureTarget* Display::GetTarget() const
    {
        return m_target;
    }
}
