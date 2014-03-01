#pragma once

#include "../Allocator.h"
#include <type_traits>

namespace Eigen
{
namespace Memory
{

    extern Allocator* g_allocator;

    //class Tracker
    //{
    //public:
    //    Tracker(const char* tag) : m_tag(tag)
    //    {}
    //    const char* m_tag;
    //    unsigned    m_allocatedCount = 0;
    //    unsigned    m_freedCount = 0;
    //    size_t      m_bytesCurrent = 0;
    //    size_t      m_bytesPeak = 0;
    //};

    template<class T> T* Allocate(unsigned count=1)
    {
        void* mem = g_allocator->Alloc(count*sizeof(T));
        return (T*)mem;
    }

    inline void Free(void* ptr)
    {
        g_allocator->Free(ptr);
    }

    template<class T> void Destroy(T* ptr, unsigned count=1)
    {
        for (T* p = ptr; p < ptr + count; p++)
        {
            p->~T();
        }
        g_allocator->Free(ptr);
    }

}
}
