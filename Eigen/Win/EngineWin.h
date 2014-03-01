#pragma once

#include "../Engine.h"
#include <Windows.h>

namespace Eigen
{

class Engine::Win : public Engine
{
public:

    struct Params : public Engine::Params
    {
        HWND*       hwnds     = nullptr;
        unsigned    hwndCount = 0;
    };

    Win(const Params& params, const char** error);
    ~Win();
};

}
