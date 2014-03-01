#pragma once

#include "../RenderCore.h"
#include "CommonDx11.h"

namespace Eigen
{

    class RenderSystem::Details : public RenderSystem::Core
    {
    public:
        Details(const char** error, unsigned scratchMemSize);

    private:
        void CommitClear(const TargetGroup::Params& targets, const Stage::ClearParams& params);
        void CommitBatches(const TargetGroup::Params& targets, const CommittedBatch* batches, int batchCount);

    };

}
