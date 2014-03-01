#pragma once

#include "Win/DisplayWin.h"
#include "CommonDx11.h"

namespace Eigen
{

    class Display::Details : public Display::Win
    {
    public:

        Details** Next()
        {
            return (Details**) &m_next;
        }

        ComPtr<IDXGISwapChain> m_swapChain;
    };

    class DisplaySystem::Details : public DisplaySystem
    {
    public:

        Details(const char** error);
        ~Details();

        Display::Details* m_first;
    };

}