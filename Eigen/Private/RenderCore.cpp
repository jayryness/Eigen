#include "RenderCore.h"
#include "Stage.h"
#include "Memory.h"
#include "Hash.h"

namespace Eigen
{
    static int8_t*              g_scratchMem    = nullptr;
    static unsigned             g_scratchSize   = 0;
    std::atomic<int8_t*>        RenderSystem::Core::s_frameAllocPtr = nullptr;
    int8_t*                     RenderSystem::Core::s_frameAllocEnd = nullptr;

    void RenderContext::CommitBatch(const RenderPort* port, const RenderBatch& batch, float sortDepth)
    {
        Details* details = (Details*)this;

        // Ensure that the context is open

        if (details->openedFrame == details->closedFrame)
        {
            // error TODO
            return;
        }

        // Create committed batch in scratch memory

        CommittedBatch* cb = RenderSystem::Core::ScratchAlloc<CommittedBatch>();
        if (cb == nullptr)
        {
            // error TODO
            return;
        }
        cb->next = nullptr;
        cb->sortDepth = sortDepth;

        // Commit batch to list

        BatchList& batchList = details->batchLists[port->index];
        cb->next = batchList.head.exchange(cb, std::memory_order_relaxed);  // [cb->next = head; head = cb;]
        batchList.count++;
    }

    RenderSystem::Core::Core(unsigned scratchMemSize)
    : m_portsCount(0)
    , m_frameId(1)
    , m_strings(nullptr)
    , m_stringsEnd(0)
    , m_stringsCapacity(2048)
    , m_scratchMem(nullptr)
    , m_scratchSize(scratchMemSize)
    {
        memset(m_portKeys, 0, sizeof(m_portKeys));
        memset(m_portIndices, -1, sizeof(m_portIndices));
        memset(m_contexts, -1, sizeof(m_contexts));
        m_strings = Memory::Allocate<char>(m_stringsCapacity);
        m_scratchMem = Memory::Allocate<int8_t>(scratchMemSize);
        s_frameAllocPtr = m_scratchMem;
        s_frameAllocEnd = m_scratchMem + scratchMemSize / 2;
    }

    RenderSystem::Core::~Core()
    {
        Memory::Free(m_scratchMem);
        Memory::Free(m_strings);
    }

    inline const RenderPort* RenderSystem::Core::GetPort(const char* name)
    {
        int nameLength = (int) strlen(name);

        // Find the port

        unsigned key = Hash32(name, nameLength);
        unsigned slot = key & cRenderPortKeyMask;
        while (m_portKeys[slot] != 0)
        {
            if (m_portKeys[slot] == key)
            {
                RenderPort* renderPort = m_ports + m_portIndices[slot];
                return renderPort;
            }

            slot = (slot+1) & cRenderPortKeyMask;
        }

        // It's not in the table - can we add it?

        if (m_portsCount == cMaxRenderPorts)
            return nullptr;

        // Reserve space in string table and copy name

        if (m_stringsEnd + nameLength + 1 > m_stringsCapacity)
        {
            m_stringsCapacity = (m_stringsEnd + nameLength + 1) * 2;
            char* strings = Memory::Allocate<char>(m_stringsCapacity);
            memcpy(strings, m_strings, m_stringsEnd);
            Memory::Free(m_strings);
            m_strings = strings;
        }
        memcpy(m_strings + m_stringsEnd, name, nameLength + 1);

        // Populate the slot with the new port

        m_portKeys[slot] = key;
        m_portIndices[slot] = m_portsCount;

        RenderPort* renderPort = m_ports + m_portsCount;
        renderPort->name = m_strings + m_stringsEnd;
        renderPort->index = m_portsCount;

        m_stringsEnd += nameLength + 1;
        m_portsCount++;

        return renderPort;
    }

    const RenderPort* RenderSystem::GetPort(const char* name)
    {
        return ((Core*)this)->GetPort(name);
    }

    inline RenderContext::Details* RenderSystem::Core::BeginContext()
    {
        for (int i = 0; i < cMaxRenderContexts; i++)
        {
            if (m_contexts[i].openedFrame != m_frameId)
            {
                RenderContext::Details* context = m_contexts + i;
                context->openedFrame = m_frameId;
                memset(context->batchLists, 0, m_portsCount*sizeof(*context->batchLists));
                return context;
            }
        }
        return nullptr;
    }

    RenderContext* RenderSystem::BeginContext()
    {
        return ((Core*)this)->BeginContext();
    }

    inline void RenderSystem::Core::EndContext(RenderContext::Details* ctxt, Stage* stages, int stageCount)
    {
        if (ctxt->openedFrame != m_frameId || ctxt->closedFrame == m_frameId)
        {
            // error TODO
            return;
        }

        ctxt->closedFrame = m_frameId;

        SubmitContext(ctxt, stages, stageCount);
    }

    void RenderSystem::EndContext(RenderContext* context, Stage* stages, int stageCount)
    {
        return ((Core*)this)->EndContext((RenderContext::Details*)context, stages, stageCount);
    }

    void RenderSystem::Core::SubmitFrame()
    {
        // Ensure that no contexts were left open

        for (int i = 0; i < cMaxRenderContexts; i++)
        {
            if (m_contexts[i].openedFrame != m_contexts[i].closedFrame)
            {
                // error TODO
                return;
            }
        }

        // TODO

        // Toggle frame alloc segment of scratch mem

        if (s_frameAllocPtr == m_scratchMem)
        {
            s_frameAllocPtr = s_frameAllocEnd;
            s_frameAllocEnd = m_scratchMem + m_scratchSize;
        }
        else
        {
            s_frameAllocPtr = m_scratchMem;
            s_frameAllocEnd = m_scratchMem + m_scratchSize / 2;
        }
        m_frameId++;
    }

}
