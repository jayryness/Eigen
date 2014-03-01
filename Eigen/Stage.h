#pragma once

#include "Texture.h"
#include <cstring>

namespace Eigen
{

    struct RenderPort;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Stage
    //

    class Stage
    {
    public:

        enum Type
        {
            cType_Unspecified   = 0,
            cType_Clear,
            cType_Shader,
            //cType_Sort,
            cType_Batches,
        };

        enum ClearFlags
        {
            cClearFlag_Color    = (1 << 0),
            cClearFlag_Depth    = (1 << 1),
            cClearFlag_Stencil  = (1 << 2),
        };

        enum SortType
        {
            cSortType_Default               = 0,
            cSortType_StateOptimized        = cSortType_Default,
            cSortType_IncreasingDepth,
            cSortType_DecreasingDepth,
            cSortType_Count
        };

        struct ClearParams
        {
            unsigned            flags;
            float               colors[TargetGroup::cMaxTargets][4];
            float               depth;
            unsigned            stencil;
        };

        struct RenderParams
        {
            const RenderPort*   renderPort;
            SortType            sortType;
        };

        struct Params
        {
            Type                type;
            union
            {
                ClearParams     clearParams;
                RenderParams    renderParams;
                //ShaderParams  shaderParams;
            };
            TargetGroup*        targetGroup;

            Params();
        };

        explicit                Stage(const Params& params);
        const Params&           GetParams() const;

    private:

        Params                  m_params;
        TargetGroupPtr          m_targetGroup;
    
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Pipeline
    //

    class Pipeline
    {
    public:

        struct Params
        {
            Stage*      stages;
            int         stageCount;
        };

        explicit        Pipeline(const Params& params);
                       ~Pipeline();

        const Params&   GetParams() const;

    private:

        Params          m_params;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    inline Stage::Params::Params()
    {
        memset(this, 0, sizeof(*this));
    }

    inline Stage::Stage(const Params& params)
    : m_params(params)
    , m_targetGroup(params.targetGroup)
    {
        assert(params.targetGroup != nullptr);
        if (params.type == cType_Clear)
        {
            assert(params.clearParams.flags != 0);
            assert((params.clearParams.flags & (cClearFlag_Depth | cClearFlag_Stencil)) ? params.targetGroup->GetParams().zbuffer != nullptr : true );
        }
    }

    inline const Stage::Params& Stage::GetParams() const
    {
        return m_params;
    }

    inline const Pipeline::Params& Pipeline::GetParams() const
    {
        return m_params;
    }

}
