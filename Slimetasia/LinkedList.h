#pragma once
#include <assert.h>

#include "MemoryAllocator.h"
#include "Utility.h"

namespace PE
{
    template <typename T>
    struct ListElem
    {
        ListElem(const T& data, ListElem* next = nullptr)
            : m_data(data)
            , m_next(next)
        {
        }
        T m_data;
        ListElem* m_next;
    };

    // singly-linked link list.
    template <typename T>
    class ForwardList
    {
    public:

        ForwardList(MemoryAllocator& memallocator)
            : m_Datahead(nullptr)
            , m_Memallocator(memallocator)
            , m_Size(0u)
        {
        }
        virtual ~ForwardList() { Clear(); }
        virtual ListElem<T>* GetHead() const { return m_Datahead; }
        uint Size() const { return m_Size; }
        void InsertData(const T& data);
        void Clear();
        void PopFront();
        void Swap(ForwardList<T>& rhs);
        /*
        T operator()(const uint& index)const;
        T& operator()(const uint& index);*/

    private:

        ListElem<T>* m_Datahead;
        MemoryAllocator& m_Memallocator;
        uint m_Size;
    };

    template <typename T>
    void ForwardList<T>::InsertData(const T& data)
    {
        // allocates the memory required for the input data.
        auto elem = new (m_Memallocator.Allocate(sizeof(ListElem<T>))) ListElem<T>(data, m_Datahead);
        // asserts when the returned pointer is a nullptr.
        assert(elem);
        // inserts the element at the front of the list.
        m_Datahead = elem;
        ++m_Size;
    }

    template <typename T>
    void ForwardList<T>::Clear()
    {
        ListElem<T>* tmp = nullptr;
        while (m_Datahead)
        {
            tmp = m_Datahead->m_next;
            m_Memallocator.ReleaseMemory(m_Datahead, sizeof(ListElem<T>));
            m_Datahead = tmp;
        }

        m_Datahead = nullptr;
    }

    template <typename T>
    void ForwardList<T>::PopFront()
    {
        auto tmp = m_Datahead->m_next;
        m_Memallocator.ReleaseMemory(m_Datahead, sizeof(ListElem<T>));
        m_Datahead = tmp;
        --m_Size;
    }

    template <typename T>
    void ForwardList<T>::Swap(ForwardList<T>& rhs)
    {
        // shallow copying
        auto tmp = rhs;
        rhs = this;
        this = tmp;
    }

    template <typename T>
    struct DoubleListElem
    {
        DoubleListElem(const T& data, DoubleListElem* next = nullptr, DoubleListElem* prev = nullptr)
            : m_data(data)
            , m_Next(next)
            , m_Prev(prev)
        {
        }
        T m_data;
        DoubleListElem* m_Next;
        DoubleListElem* m_Prev;
    };

    // doubly-linked link list.
    template <typename T>
    class DoublyLinkedList
    {
    public:

        DoublyLinkedList(MemoryAllocator& memallocator)
            : m_DataHead(nullptr)
            , m_DataTail(nullptr)
            , m_Memallocator(memallocator)
            , m_Size(0u)
        {
        }

        ~DoublyLinkedList() { clear(); }

        // getters
        DoubleListElem<T>* GetHead() const { return m_DataHead; }
        DoubleListElem<T>* GetBack() const { return m_DataTail; }

        void InsertData(const T& data);
        void InsertBack(const T& data);
        void PopBack();
        void PopFront();
        void Swap(DoublyLinkedList<T>& rhs);

        // void EmplaceFront(T&&... Args);
        void clear();

    private:

        DoubleListElem<T>* m_DataHead;
        DoubleListElem<T>* m_DataTail;
        MemoryAllocator& m_Memallocator;
        uint m_Size;
    };

    template <typename T>
    void DoublyLinkedList<T>::InsertData(const T& data)
    {
        // allocates the memory required for the input data.
        auto elem = new (m_Memallocator.Allocate(sizeof(DoubleListElem<T>))) DoubleListElem<T>(data, m_DataHead);
        // asserts when the returned pointer is a nullptr.
        assert(elem);
        // inserts the element at the front of the list.
        m_DataHead = elem;
    }

    template <typename T>
    void DoublyLinkedList<T>::InsertBack(const T& data)
    {
        // allocates the memory required for the input data.
        auto elem = new (m_Memallocator.Allocate(sizeof(DoubleListElem<T>))) DoubleListElem<T>(data, nullptr);
        // asserts when the returned pointer is a nullptr.
        assert(elem);
        // inserts the element at the front of the list.
        m_DataTail->m_Next = elem;
        m_DataTail = elem;
    }

    template <typename T>
    void DoublyLinkedList<T>::PopBack()
    {
        if (m_Size)
        {
            auto tmp = m_DataTail;
            m_DataTail = m_DataTail->m_Prev;
            m_Memallocator.ReleaseMemory(tmp, sizeof(DoubleListElem<T>));
            --m_Size;
        }
    }

    template <typename T>
    void DoublyLinkedList<T>::PopFront()
    {
        if (m_Size)
        {
            auto tmp = m_DataHead;
            m_DataHead = m_DataHead->m_Next;
            m_Memallocator.ReleaseMemory(tmp, sizeof(DoubleListElem<T>));
            --m_Size;
        }
    }

    /*template <typename T>
    void DoublyLinkedList<T>::EmplaceFront(T&&... args)
    {
    }*/

    template <typename T>
    void DoublyLinkedList<T>::clear()
    {
        DoubleListElem<T>* tmp = nullptr;
        while (m_DataHead)
        {
            tmp = m_DataHead->m_Next;
            m_Memallocator.ReleaseMemory(m_DataHead, sizeof(DoubleListElem<T>));
            m_DataHead = tmp;
        }

        m_DataHead = nullptr;
        m_DataTail = nullptr;
    }

    template <typename T>
    void DoublyLinkedList<T>::Swap(DoublyLinkedList& rhs)
    {
        auto tmp = rhs;
        rhs = this;
        this = tmp;
    }
}  // namespace PE
