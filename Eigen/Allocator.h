#pragma once

#include <new>

namespace Eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocator
    //

    class Allocator
    {
    public:
        virtual void* Alloc(unsigned bytes) = 0;
        virtual void  Free(void* ptr) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

    class Mallocator : public Allocator
    {
    public:
        static Mallocator* Get() throw()
        {
            static Mallocator s_mallocator;
            return &s_mallocator;
        }

        virtual void* Alloc(unsigned bytes)
        {
            return malloc(bytes);
        }

        virtual void Free(void* ptr)
        {
            free(ptr);
        }
    };

}
