#pragma once

#include "Memory.h"
#include <algorithm>

namespace Eigen
{

    template<class T> class Pool
    {
    public:
        static Pool&    Instance();

                        Pool(int initialCapacity = 0);
                       ~Pool();

        int             Count() const;
        int             Capacity() const;

        void*           Allocate();
        void            Free(void* ptr);

        T*              Create();
        T*              Create(const T& a);
        void            Destroy(T* object);

    private:

        struct          Block;

        Block*          CreateBlock(unsigned capacity);
        void            DestroyBlock(Block* block);

        struct Node
        {
            T           payload;
            union
            {
                Block*  block;
                Node*   next;
            };
        };

        struct Block
        {
            int         capacity;
            int         count;
            Block*      next;
            Node*       free;
        };

        Block*          m_block;
        int             m_capacity;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T> Pool<T>& Pool<T>::Instance()
    {
        static Pool s_pool(1 + 1024/sizeof(T));
        return s_pool;
    }

    template<class T> Pool<T>::Pool(int initialCapacity)
    : m_capacity(0)
    {
        if (initialCapacity)
            CreateBlock(initialCapacity);
    }

    template<class T> Pool<T>::~Pool()
    {
        while (m_block)
            DestroyBlock(m_block);

        assert(m_capacity == 0);
    }

    template<class T> int Pool<T>::Count() const
    {
        int count = 0;
        for (Block* block = m_block; block != nullptr; block = block->next)
            count += block->count;

        return count;
    }

    template<class T> int Pool<T>::Capacity() const
    {
        return m_capacity;
    }

    template<class T> typename Pool<T>::Block* Pool<T>::CreateBlock(unsigned capacity)
    {
        // new blocks are prepended to the list, so that larger blocks are in front of smaller blocks
        Block* block = (Block*)Memory::Allocate<int8_t>(sizeof(Block) + sizeof(Node) * capacity);
        Node* nodes = (Node*)(block + 1);

        block->capacity = capacity;
        block->count = 0;
        block->next = m_block;
        block->free = nodes;

        for (unsigned i = 0; i < capacity-1; i++)
            nodes[i].next = nodes + i+1;

        nodes[capacity-1].next = nullptr;

        m_block = block;
        m_capacity += capacity;

        return block;
    }

    template<class T> void Pool<T>::DestroyBlock(Block* block)
    {
        assert(block != nullptr && block->count == 0);

        // walk the singly-linked list to find the preceeding block
        Block** prev;
        for (prev = &m_block; *prev != block; prev = &(*prev)->next)
            ;

        *prev = block->next;
        m_capacity -= block->capacity;

        Memory::Free(block);
    }

    template<class T> void* Pool<T>::Allocate()
    {
        Node* node = nullptr;

        Block* block = m_block;

        if (block == nullptr)
            block = CreateBlock(std::max(m_capacity, 1 + 256/(int)sizeof(T)));

        node = block->free;
        block->free = node->next;
        node->block = block;

        block->count++;

        return (void*)&node->payload;
    }

    template<class T> void Pool<T>::Free(void* ptr)
    {
        if (ptr == nullptr)
            return;

        static_assert(offsetof(Node, payload) == 0, "Bad struct packing");
        Node *node = (Node*) ptr;
        Block* block = node->block;
        node->next = block->free;
        block->free = node;

        block->count--;

        // if the block is empty, and not the first block, clean it up
        if (block->count == 0 && block != m_block)
            DestroyBlock(block);
    }

    template<class T> T* Pool<T>::Create()
    {
        T* ptr = new(Allocate()) T;
        return ptr;
    }

    template<class T> T* Pool<T>::Create(const T& a)
    {
        T* ptr = new(Allocate()) T(a);
        return ptr;
    }

    template<class T> void Pool<T>::Destroy(T* object)
    {
        if (object != nullptr)
        {
            object->~T();
            Free(object);
        }
    }

}
