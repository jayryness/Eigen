#pragma once

#include "Win/EngineWin.h"
#include "CommonDx11.h"
#include "DisplayDx11.h"
#include "TextureDx11.h"
#include "RenderDx11.h"

namespace Eigen
{

struct EngineCore
{
public:

    static EngineCore&          s_instance;

    ID3D11Device*               device;
    ID3D11DeviceContext*        immContext;     // Device's immediate context
    IDXGIFactory*               dxgiFactory;
    TextureSystem::Details      textureSystem;
    DisplaySystem::Details      displaySystem;
    RenderSystem::Details       renderSystem;

    EngineCore(const char** error, const Engine::Win::Params& params);
    ~EngineCore();

};

}
