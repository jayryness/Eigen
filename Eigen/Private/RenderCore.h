#pragma once

#include "../Render.h"
#include "../Stage.h"
#include <atomic>
#include <cstdint>

namespace Eigen
{
    enum {                          cRenderPortKeyMask  = 0x1ff };
    enum {                          cMaxRenderPorts     = (cRenderPortKeyMask + 1) * 3 / 8 };
    enum {                          cMaxRenderContexts  = 8 };
    typedef unsigned char           RenderPortIndex;    // must accommodate cMaxRenderPorts-1


    struct CommittedBatch
    {
        //Shader*                   shader;
        //Geometry*                 geometry;
        //ParamData*
        CommittedBatch*             next;
        float                       sortDepth;
    };

    struct BatchList
    {
        std::atomic<CommittedBatch*> head;
        int                         count;
        CommittedBatch*             sorted[Stage::cSortType_Count];
    };


    struct RenderContext::Details : public RenderContext
    {
        uint64_t                    openedFrame;
        uint64_t                    closedFrame;
        BatchList                   batchLists[cMaxRenderPorts];
    };


    class RenderSystem::Core :      public RenderSystem
    {
    public:
                                    Core(unsigned scratchMemSize);
                                   ~Core();

        void                        SubmitFrame();

        template<class T> static T* ScratchAlloc(unsigned count = 1);

    protected:

        const RenderPort*           GetPort(const char* name);
        RenderContext::Details*     BeginContext();
        void                        EndContext(RenderContext::Details* context, Stage* stages, int stageCount);
        void                        SubmitContext(RenderContext::Details* context, Stage* stages, int stageCount);

        friend class                RenderSystem;
        struct RenderPort :         public Eigen::RenderPort {};

        unsigned                    m_portKeys    [cRenderPortKeyMask + 1];
        RenderPortIndex             m_portIndices [cRenderPortKeyMask + 1];
        RenderPort                  m_ports       [cMaxRenderPorts];
        unsigned                    m_portsCount; 

        RenderContext::Details      m_contexts    [cMaxRenderContexts];

        uint64_t                    m_frameId;

        char*                       m_strings;
        unsigned                    m_stringsEnd;
        unsigned                    m_stringsCapacity;

        int8_t*                     m_scratchMem;
        unsigned                    m_scratchSize;

        static std::atomic<int8_t*> s_frameAllocPtr;
        static int8_t*              s_frameAllocEnd;
    };

    template<class T> T* RenderSystem::Core::ScratchAlloc(unsigned count)
    {
        int8_t* p;
        unsigned bytes = count*sizeof(T);
        p = s_frameAllocPtr.fetch_add((bytes + 7) & ~7);   // [p = s_frameAllocPtr; s_frameAllocPtr += ...]
        if (p + bytes > s_frameAllocEnd)
        {
            return nullptr;
        }
        return (T*) p;
    }

}
