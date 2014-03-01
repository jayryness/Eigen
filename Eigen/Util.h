#pragma once

#include <atomic>

namespace Eigen
{

    template<class T> struct RefPtr
    {
        RefPtr() : ptr(nullptr) {}
        RefPtr(T* ptr_) : ptr(ptr_)
        {
            if (ptr != nullptr)
                ChangeRef((RefCounted<T>*)ptr, +1);
        }

        RefPtr(const RefPtr& other) = delete;
        //RefPtr(const RefPtr& other) : ptr(other.ptr)
        //{
        //    if (ptr != nullptr)
        //        ChangeRef((RefCounted<T>*)ptr, +1);
        //}

        ~RefPtr()
        {
            if (ptr != nullptr)
                ChangeRef((RefCounted<T>*)ptr, -1);
        }

        operator T*() const
        {
            return ptr;
        }

        void operator=(RefPtr other)
        {
            Swap(other);
        }

        template<class T2> void Swap(RefPtr<T2>& other)
        {
            T* temp = ptr;
            ptr = static_cast<T*>(other.ptr);
            other.ptr = static_cast<T2*>(temp);
        }

        T* ptr;
    };

    template<class T> class RefCounted
    {
    public:

        friend void AssignRef(T*& dest, T*const src)
        {
            (RefPtr<T>&)dest = src;
        }

        friend void AddRef(T*& ptr)
        {
            if (ptr != nullptr)
                ChangeRef(ptr, +1);
        }

        friend void ReleaseRef(T*& ptr)
        {
            if (ptr != nullptr)
                ChangeRef(ptr, -1);
        }

    protected:

        RefCounted()
        {
            m_refCount.store(0, std::memory_order_relaxed);
        }

    private:

        friend void ChangeRef(RefCounted* ptr, int delta)
        {
            if (ptr->m_refCount.fetch_add(delta, std::memory_order_relaxed) <= -delta)
            {
                DestroyRefCounted((T*) ptr);
            }
        }

        friend void DestroyRefCounted(T*);
        std::atomic_int m_refCount;
    };

}
