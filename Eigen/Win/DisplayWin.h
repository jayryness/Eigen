#pragma once

#include "../Display.h"
#include <Windows.h>

namespace Eigen
{

class Display::Win : public Display
{
public:

    static Win*     SafeCast(Display*);

    bool            Bind(HWND hwnd);

protected:

    Win()   {}
    ~Win()  {}
};

}
