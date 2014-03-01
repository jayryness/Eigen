#include "EngineDx11.h"
#include "RenderDx11.h"
#include "TextureDx11.h"

namespace Eigen
{

    inline D3D11_CLEAR_FLAG TranslateClearFlags(unsigned stageClearFlags)
    {
        unsigned result = (stageClearFlags & Stage::cClearFlag_Depth) * D3D11_CLEAR_DEPTH / Stage::cClearFlag_Depth;
        result |= (stageClearFlags & Stage::cClearFlag_Stencil) * D3D11_CLEAR_STENCIL / Stage::cClearFlag_Stencil;
        return (D3D11_CLEAR_FLAG)result;
    }

    RenderSystem::Details::Details(const char** error, unsigned scratchMemSize)
    : Core(scratchMemSize)
    {
    }

    void RenderSystem::Core::SubmitContext(RenderContext::Details* ctxt, Stage* stages, int stageCount)
    {
        const EngineCore& engine = EngineCore::s_instance;

        for (int i = 0; i < stageCount; i++)
        {
            const Stage::Params& stageParams = stages[i].GetParams();

            // Set targets

            const TargetGroup::Details* targets = (TargetGroup::Details*)stageParams.targetGroup;

            switch (stageParams.type)
            {
            case Stage::cType_Clear:
                if (stageParams.clearParams.flags & (Stage::cClearFlag_Depth | Stage::cClearFlag_Stencil))
                    engine.immContext->ClearDepthStencilView(targets->m_d3dDepthStencilView.Get(), TranslateClearFlags(stageParams.clearParams.flags), stageParams.clearParams.depth, stageParams.clearParams.stencil);
                for (unsigned i = 0; i < targets->m_viewCount; i++)
                    engine.immContext->ClearRenderTargetView(targets->m_d3dTargetViews[i], stageParams.clearParams.colors[i]);
                continue;

            case Stage::cType_Shader:
                engine.immContext->OMSetRenderTargets(targets->m_viewCount, targets->m_d3dTargetViews, targets->m_d3dDepthStencilView.Get());
                // TODO
                continue;

            case Stage::cType_Batches:
                {
                    engine.immContext->OMSetRenderTargets(targets->m_viewCount, targets->m_d3dTargetViews, targets->m_d3dDepthStencilView.Get());
                    const Stage::RenderParams& renderParams = stageParams.renderParams;
                    BatchList& batchList = ctxt->batchLists[renderParams.renderPort->index];
                    if (batchList.sorted[renderParams.sortType] == nullptr)
                    {
                        // TODO commit sort job
                        //batchList.sorted[renderParams.sortType] = 
                    }
                    //CommitBatches(*(TargetGroup::Params*)stageParams.targetGroup, batchList.sorted[renderParams.sortType], batchList.count);
                }
                continue;
            }
        }
    }

}
