#pragma once

#include <cassert>
#include <type_traits>

namespace Eigen
{

    // This is used to make space for objects in static memory without constructing them

    template<class T> struct Placeholder
    {
        typedef typename std::aligned_union<0, T>::type Storage;

        size_t  header;
        Storage storage;
        size_t  footer;

        void* Init()
        {
            header = size_t(0xBAADF00DBAADF00DLL);
            footer = size_t(0xBAADF00DBAADF00DLL);
            return &storage;
        }
        ~Placeholder()
        {
            // check for overrun
            assert(header == size_t(0xBAADF00DBAADF00DLL) && footer == size_t(0xBAADF00DBAADF00DLL));
        }
    };

}
