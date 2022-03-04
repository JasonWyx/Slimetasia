#pragma once
#include "MemoryAllocator.h"
#include "Utility.h"

namespace PE
{
    template <typename T> class Vector
    {
    public:
        using difference_type = std::ptrdiff_t;
        // Vector Iterator
        class iterator
        {
        public:
            // Iterator traits
            using value_type = T;
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

            size_t Index() const { return m_CurrentIndex; }

            T* Ptr() const { return &(m_Buf[m_CurrentIndex]); }

        private:
            size_t m_CurrentIndex;
            T* m_Buf;
            size_t m_Size;
        };

        Vector(MemoryAllocator& memallocator)
            : m_Data(nullptr)
            , m_Memallocator(memallocator)
            , m_Size(0u)
            , m_Capacity(0)
        {
        }
        // Rule of 5
        Vector(const Vector<T>& rhs);
        Vector(Vector<T>&& rhs);
        Vector<T>& operator=(const Vector<T>& rhs);
        Vector<T>& operator=(Vector<T>&& rhs) noexcept;
        ~Vector();

        // getters
        size_t size() const noexcept { return m_Size; }
        size_t capacity() const noexcept { return m_Capacity; }
        iterator begin() const { return iterator(m_Data, 0, m_Size); }
        iterator end() const { return iterator(m_Data, m_Size, m_Size); }
        // operators
        T& operator[](const uint index);
        const T& operator[](const uint index) const;
        T& front();
        const T& front() const;
        T& back();
        const T& back() const;
        // funcs
        iterator insert(iterator pos, const T& val);
        void push_back(T&& val);
        void push_back(const T& val);
        void pop_back();
        void reserve(size_t capacity);
        void* Start() { return m_Data; }

    private:
        void* m_Data;
        MemoryAllocator& m_Memallocator;
        size_t m_Size;
        size_t m_Capacity;
    };
    // Rule of 5
    template <typename T>
    Vector<T>::Vector(const Vector<T>& rhs)
        : m_Data(nullptr)
        , m_Memallocator(rhs.m_Memallocator)
        , m_Size(rhs.m_Size)
        , m_Capacity(rhs.m_Capacity)
    {
        m_Data = m_Memallocator.Allocate(rhs.m_Capacity * sizeof(T));
        T* items = static_cast<T*>(rhs.m_Data);
        std::uninitialized_copy(items, items + rhs.m_Size, static_cast<T*>(m_Data));
    }

    template <typename T>
    Vector<T>::Vector(Vector<T>&& rhs)
        : m_Data(rhs.m_Data)
        , m_Memallocator(rhs.m_Memallocator)
        , m_Size(rhs.m_Size)
        , m_Capacity(rhs.m_Capacity)
    {
        rhs.m_Data = nullptr;
        rhs.m_Capacity = rhs.m_Size = 0;
    }

    template <typename T> Vector<T>& Vector<T>::operator=(const Vector<T>& rhs)
    {
        if (rhs.m_Size > 0)
        {
            // Allocate memory for the new array
            void* new_data = m_Memallocator.Allocate(rhs.m_Capacity * sizeof(T));
            T* dest = static_cast<T*>(new_data);
            T* items = static_cast<T*>(rhs.m_Data);
            T* olditems = static_cast<T*>(m_Data);
            std::uninitialized_copy(items, items + rhs.m_Size, dest);

            for (size_t i = 0; i < m_Size; i++)
                olditems[i].~T();

            // Release the previously allocated memory
            m_Memallocator.ReleaseMemory(m_Data, m_Capacity * sizeof(T));

            m_Data = new_data;
            assert(m_Data != nullptr);

            m_Capacity = rhs.m_Capacity;
            m_Size = rhs.m_Size;
        }
        return *this;
    }

    template <typename T> Vector<T>& Vector<T>::operator=(Vector<T>&& rhs) noexcept
    {
        std::swap(m_Data, rhs.m_Data);
        std::swap(m_Memallocator, rhs.m_Memallocator);
        std::swap(m_Size, rhs.m_Size);
        std::swap(m_Capacity, rhs.m_Capacity);
        return *this;
    }

    template <typename T> Vector<T>::~Vector()
    {
        T* items = static_cast<T*>(m_Data);
        for (size_t i = 0; i < m_Size; i++)
            items[i].~T();
        m_Memallocator.ReleaseMemory(m_Data, m_Capacity * sizeof(T));
        m_Data = nullptr;
        m_Size = 0;
        m_Capacity = 0;
    }

    // operators
    template <typename T> T& Vector<T>::operator[](const uint index)
    {
        assert(index < m_Size);
        return (static_cast<T*>(m_Data)[index]);
    }

    template <typename T> const T& Vector<T>::operator[](const uint index) const
    {
        assert(index < m_Size);
        return (static_cast<T*>(m_Data)[index]);
    }

    template <typename T> T& Vector<T>::front() { return (static_cast<T*>(m_Data)[0]); }

    template <typename T> const T& Vector<T>::front() const { return (static_cast<T*>(m_Data)[0]); }

    template <typename T> T& Vector<T>::back() { return (static_cast<T*>(m_Data)[m_Size - 1]); }

    template <typename T> const T& Vector<T>::back() const { return (static_cast<T*>(m_Data)[m_Size - 1]); }
    // funcs
    template <typename T> typename Vector<T>::iterator Vector<T>::insert(iterator pos, const T& val)
    {
        size_t offset = pos.Index();
        if (pos.Ptr() == &(static_cast<T*>(m_Data)[m_Size]))
        {
            push_back(val);
            return pos;
        }
        if (m_Size == m_Capacity)
        {
            m_Capacity = (m_Capacity == 0) ? 1 : (2 * m_Capacity);
        }
        if (m_Data != nullptr)
        {
            // Allocate memory for the new array
            void* new_data = m_Memallocator.Allocate(m_Capacity * sizeof(T));
            if (m_Size > 0)
            {
                T* dest = static_cast<T*>(new_data);
                T* items = static_cast<T*>(m_Data);
                std::uninitialized_copy(items, pos.Ptr(), dest);
                new (static_cast<char*>(new_data) + offset * sizeof(T)) T(val);
                std::uninitialized_copy(pos.Ptr(), &(items[m_Size]), &(dest[offset + 1]));
                for (size_t i = 0; i < m_Size; i++)
                    items[i].~T();
                m_Size++;
            }

            // Release the previously allocated memory
            m_Memallocator.ReleaseMemory(m_Data, m_Capacity * sizeof(T));
            m_Data = new_data;
        }
        else
        {
            push_back(val);
        }
        assert(m_Data != nullptr);
        iterator tmp(m_Data, offset, m_Size);
        return pos;
    }

    template <typename T> void Vector<T>::push_back(T&& val)
    {
        // Increase cap
        if (m_Size == m_Capacity)
        {
            size_t newCap = (m_Capacity == 0) ? 1 : (2 * m_Capacity);
            reserve(newCap);
            new (static_cast<char*>(m_Data) + m_Size * sizeof(T)) T(std::move(val));
            m_Size++;
        }
        else  // Insert normally
        {
            new (static_cast<char*>(m_Data) + m_Size * sizeof(T)) T(val);
            m_Size++;
        }
    }

    template <typename T> void Vector<T>::push_back(const T& val)
    {
        // Increase cap
        if (m_Size == m_Capacity)
        {
            size_t newCap = (m_Capacity == 0) ? 1 : (2 * m_Capacity);
            reserve(newCap);
            new (static_cast<char*>(m_Data) + m_Size * sizeof(T)) T(val);
            m_Size++;
        }
        else  // Insert Normally
        {
            new (static_cast<char*>(m_Data) + m_Size * sizeof(T)) T(val);
            m_Size++;
        }
    }

    template <typename T> void Vector<T>::pop_back()
    {
        T& lastObj = static_cast<T*>(m_Data)[m_Size - 1];
        (lastObj).~T();
        m_Size--;
    }

    template <typename T> void Vector<T>::reserve(size_t capacity)
    {
        if (capacity <= m_Capacity) return;

        // Allocate memory for the new array
        void* new_data = m_Memallocator.Allocate(capacity * sizeof(T));

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
            m_Memallocator.ReleaseMemory(m_Data, m_Capacity * sizeof(T));
        }

        m_Data = new_data;
        assert(m_Data != nullptr);

        m_Capacity = capacity;
    }
}  // namespace PE
