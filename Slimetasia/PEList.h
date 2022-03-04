#pragma once
#include <assert.h>

#include <filesystem>
#include <iterator>

#include "CorePrerequisites.h"
#include "MemoryAllocator.h"
#include "Utility.h"

namespace PE
{
    template <typename T> class list
    {
    public:
        // List Iterator
        class iterator
        {
        public:
            // Iterator traits
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;
            using iterator_category = std::bidirectional_iterator_tag;

            iterator(void* buffer = nullptr, size_t index = 0u, size_t size = 0u)
                : m_CurrentIndex(index)
                , m_Buf(static_cast<T*>(buffer))
                , m_Size(size)
            {
            }

            iterator(const iterator& rhs)
                : m_CurrentIndex(rhs.m_CurrentIndex)
                , m_Buf(rhs.m_Buf)
                , m_Size(rhs.m_Size)
            {
            }

            ~iterator() = default;

            iterator& operator=(const iterator& rhs)
            {
                if (this != &rhs)
                {
                    m_CurrentIndex = rhs.m_CurrentIndex;
                    m_Buf = rhs.m_Buf;
                    m_Size = rhs.m_Size;
                }
                return *this;
            }

            reference operator*() const
            {
                assert(m_CurrentIndex >= 0 && m_CurrentIndex < m_Size);
                return m_Buf[m_CurrentIndex];
            }

            pointer operator->() const
            {
                assert(m_CurrentIndex >= 0 && m_CurrentIndex < m_Size);
                return &(m_Buf[m_CurrentIndex]);
            }

            iterator& operator++()
            {
                assert(m_CurrentIndex < m_Size);
                m_CurrentIndex++;
                return *this;
            }

            iterator operator++(int number)
            {
                assert(m_CurrentIndex < m_Size);
                iterator tmp = *this;
                m_CurrentIndex++;
                return tmp;
            }

            iterator& operator--()
            {
                assert(m_CurrentIndex > 0);
                m_CurrentIndex--;
                return *this;
            }

            iterator operator--(int number)
            {
                assert(m_CurrentIndex > 0);
                iterator tmp = *this;
                m_CurrentIndex--;
                return tmp;
            }

            bool operator==(const iterator& iterator) const
            {
                assert(m_CurrentIndex >= 0 && m_CurrentIndex <= m_Size);

                // If both iterators points to the end of the list
                if (m_CurrentIndex == m_Size && iterator.m_CurrentIndex == iterator.m_Size)
                {
                    return true;
                }

                return &(m_Buf[m_CurrentIndex]) == &(iterator.m_Buf[iterator.m_CurrentIndex]);
            }

            bool operator!=(const iterator& iterator) const { return !(*this == iterator); }

            friend class List;

        private:
            size_t m_CurrentIndex;
            T* m_Buf;
            size_t m_Size;
        };

        list(MemoryAllocator& allocator, size_t capacity = 0);
        // rule-of-5

        list(const list<T>& rhs);

        list(list<T>&& rhs) noexcept;

        ~list();

        list<T>& operator=(const list<T>& rhs);

        list<T>& operator=(list<T>&& rhs) noexcept;

        // getters
        size_t size() const noexcept { return m_Size; }
        size_t capacity() const noexcept { return m_Capacity; }
        iterator begin() const { return iterator(m_Data, 0, m_Size); }
        iterator end() const { return iterator(m_Data, m_Size, m_Size); }
        T& operator[](const uint index)
        {
            assert(index < m_Size);
            return (static_cast<T*>(m_Data)[index]);
        }

        const T& operator[](const uint index) const
        {
            assert(index < m_Size);
            return (static_cast<T*>(m_Data)[index]);
        }

        // funcs
        iterator find(const T& elem);
        iterator remove(const T& elem);
        iterator remove(const iterator& it);
        iterator removeAt(uint index);
        void reserve(size_t capacity);
        void add(const T& elem);
        void addRange(const list<T>& rhs);
        void clear();
        bool operator==(const list<T>& rhs) const;
        bool operator!=(const list<T>& rhs) const;

    private:
        void* m_Data;
        size_t m_Size;
        size_t m_Capacity;
        MemoryAllocator& m_MemAllocator;
    };

    template <typename T>
    list<T>::list(MemoryAllocator& allocator, size_t capacity)
        : m_Data(nullptr)
        , m_Size(0)
        , m_Capacity(0)
        , m_MemAllocator(allocator)
    {
        if (capacity) reserve(capacity);
    }

    template <typename T>
    list<T>::list(const list<T>& rhs)
        : m_Data(nullptr)
        , m_Size(0)
        , m_Capacity(0)
        , m_MemAllocator(rhs.m_MemAllocator)
    {
        addRange(rhs);
    }

    template <typename T>
    list<T>::list(list<T>&& rhs) noexcept
        : m_Data(std::move(rhs.m_Data))
        , m_Size(rhs.m_Size)
        , m_Capacity(rhs.m_Capacity)
        , m_MemAllocator(rhs.m_MemAllocator)
    {
    }

    template <typename T> list<T>::~list()
    {
        if (m_Capacity)
        {
            clear();
            m_MemAllocator.ReleaseMemory(m_Data, m_Capacity * sizeof(T));
        }
    }

    template <typename T> list<T>& list<T>::operator=(const list<T>& rhs)
    {
        if (this != &rhs)
        {
            clear();
            addRange(rhs);
        }

        return *this;
    }

    template <typename T> list<T>& list<T>::operator=(list<T>&& rhs) noexcept
    {
        if (this != &rhs)
        {
            clear();
            m_Data = std::move(rhs.m_Data);
            m_Size = rhs.m_Size;
            m_Capacity = rhs.m_Capacity;
            m_MemAllocator = std::move(rhs.m_MemAllocator);
        }

        return *this;
    }

    template <typename T> typename list<T>::iterator list<T>::find(const T& elem)
    {
        for (uint i = 0; i < m_Size; i++)
        {
            if (elem == static_cast<T*>(m_Data)[i]) return iterator(m_Data, i, m_Size);
        }

        return end();
    }

    template <typename T> typename list<T>::iterator list<T>::remove(const T& elem) { return remove(find(elem)); }

    template <typename T> typename list<T>::iterator list<T>::remove(const iterator& it)
    {
        assert(it.m_Buf == m_Data);
        return removeAt(it.m_CurrentIndex);
    }

    template <typename T> typename list<T>::iterator list<T>::removeAt(uint index)
    {
        assert(index >= 0 && index < m_Size);

        // Call the destructor
        (static_cast<T*>(m_Data)[index]).~T();

        m_Size--;

        if (index != m_Size)
        {
            char* dest = static_cast<char*>(m_Data) + index * sizeof(T);
            char* src = dest + sizeof(T);
            // Copies count characters from the object pointed to by src to the object pointed to by dest. Both objects are reinterpreted as arrays of unsigned char.
            std::memmove(static_cast<void*>(dest), static_cast<void*>(src), (m_Size - index) * sizeof(T));
        }

        // Return an iterator pointing to the element after the removed one
        return list<T>::iterator(m_Data, index, m_Size);
    }

    template <typename T> void list<T>::reserve(size_t capacity)
    {
        if (capacity <= m_Capacity) return;

        // Allocate memory for the new array
        void* new_data = m_MemAllocator.Allocate(capacity * sizeof(T));

        if (m_Data != nullptr)
        {
            if (m_Size > 0)
            {
                T* dest = static_cast<T*>(new_data);
                T* items = static_cast<T*>(m_Data);
                std::uninitialized_copy(items, items + m_Size, dest);

                for (size_t i = 0; i < m_Size; i++)
                    items[i].~T();
            }

            // Release the previously allocated memory
            m_MemAllocator.ReleaseMemory(m_Data, m_Capacity * sizeof(T));
        }

        m_Data = new_data;
        assert(m_Data != nullptr);

        m_Capacity = capacity;
    }

    template <typename T> void list<T>::add(const T& elem)
    {
        if (m_Size == m_Capacity)
        {
            reserve(m_Capacity == 0 ? 1 : m_Capacity * 2);
        }

        // Use the copy-constructor to construct the element
        new (static_cast<char*>(m_Data) + m_Size * sizeof(T)) T(elem);

        m_Size++;
    }

    template <typename T> void list<T>::addRange(const list<T>& rhs)
    {
        if (m_Size + rhs.m_Size > m_Capacity) reserve(m_Size + rhs.m_Size);

        // Add the elements of the list to the current one
        for (auto i = 0u; i < rhs.m_Size; i++)
        {
            new (static_cast<char*>(m_Data) + m_Size * sizeof(T)) T(rhs[i]);
            m_Size++;
        }
    }

    template <typename T> void list<T>::clear()
    {
        for (auto i = 0u; i < m_Size; i++)
            (static_cast<T*>(m_Data)[i]).~T();

        m_Size = 0;
    }

    template <typename T> bool list<T>::operator==(const list<T>& rhs) const
    {
        // comparing the size first.
        if (m_Size != rhs.m_Size) return false;

        // comparing the elements in the list.
        T* items = static_cast<T*>(m_Data);
        for (auto i = 0u; i < m_Size; i++)
        {
            if (items[i] != rhs[i]) return false;
        }

        return true;
    }

    template <typename T> bool list<T>::operator!=(const list<T>& rhs) const { return !((*this) == rhs); }

}  // namespace PE
