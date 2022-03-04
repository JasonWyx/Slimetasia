#pragma once
#include <assert.h>

#include "Utility.h"

namespace PE
{
    template <typename T, typename U> class pair
    {
    public:
        pair(const T& first, const U& second)
            : m_First(first)
            , m_Second(second)
        {
        }
        ~pair() = default;

        pair(const pair& rhs) = default;
        pair(pair&& rhs) = default;

        // getters
        T& First() { return m_First; }
        T First() const { return m_First; }
        U& Second() { return m_Second; }
        U Second() const { return m_Second; }

        // operators
        pair& operator=(const pair& rhs);
        pair& operator=(pair&& rhs);
        bool operator<(const pair& rhs);
        bool operator<=(const pair& rhs);
        bool operator>(const pair& rhs);
        bool operator>=(const pair& rhs);
        bool operator==(const pair& rhs);
        bool operator!=(const pair& rhs);

        // funcs
        void Swap(pair& rhs);

    private:
        T m_First;
        U m_Second;
    };

    template <typename T, typename U> pair<T, U>& Makepair(const T& first, const U& second) { return pair(first, second); }

    /*template<size_t I, typename T, typename U>
    typename tuple_elem<I, PE::pair<T, U>>::type& get(PE::pair<T, U>& inp)
    {
        std::cout << "An invalid index was passed in, a pair only contains 2 elements; 0 and 1." << std::endl;
        assert(false);
        return inp.First;
    }

    template<typename T, typename U>
    typename tuple_elem<0, PE::pair<T, U>>::type& get<T, U>(PE::pair<T, U>& inp)
    {
        return inp.First;
    }

    template<typename T, typename U>
    typename tuple_elem<1, PE::pair<T, U>>::type& get<1, T, U>(PE::pair<T, U>& inp)
    {
        return inp.Second;
    }

    template<size_t I, typename T, typename U>
    typename tuple_elem<I, PE::pair<T, U>>::type&& get(PE::pair<T, U>& inp)
    {
        std::cout << "An invalid index was passed in, a pair only contains 2 elements; 0 and 1." << std::endl;
        assert(false);
    }

    template<typename T, typename U>
    typename tuple_elem<I, PE::pair<T, U>>::type&& get<0, T, U>(PE::pair<T, U>& inp)
    {
        return inp.First;
    }

    template<typename T, typename U>
    typename tuple_elem<I, PE::pair<T, U>>::type&& get<1, T, U>(PE::pair<T, U>& inp)
    {
        return inp.Second;
    }

    template<size_t I, typename T, typename U>
    const typename tuple_elem<I, PE::pair<T, U>>::type& get(PE::pair<T, U>& inp)
    {
        std::cout << "An invalid index was passed in, a pair only contains 2 elements; 0 and 1." << std::endl;
        assert(false);
    }

    template<typename T, typename U>
    const typename tuple_elem<I, PE::pair<T, U>>::type& get<0, T, U>(PE::pair<T, U>& inp)
    {
        return inp.First;
    }

    template<typename T, typename U>
    const typename tuple_elem<I, PE::pair<T, U>>::type& get<1, T, U>(PE::pair<T, U>& inp)
    {
        return inp.Second;
    }*/

    template <typename T, typename U> void pair<T, U>::Swap(pair& rhs)
    {
        std::swap(m_First, rhs.m_First);
        std::swap(m_Second, rhs.m_Second);
    }

    template <typename T, typename U> pair<T, U>& pair<T, U>::operator=(const pair& rhs)
    {
        if (this != &rhs)
        {
            m_First = rhs.m_First;
            m_Second = rhs.m_Second;
        }
        return *this;
    }

    template <typename T, typename U> pair<T, U>& pair<T, U>::operator=(pair&& rhs)
    {
        if (this != &rhs) *this = std::move(rhs);

        return *this;
    }

    template <typename T, typename U> bool pair<T, U>::operator<(const pair& rhs) { return m_First < rhs.m_First; }

    template <typename T, typename U> bool pair<T, U>::operator<=(const pair& rhs) { return m_First <= rhs.m_First; }

    template <typename T, typename U> bool pair<T, U>::operator>(const pair& rhs) { return m_First > rhs.m_First; }

    template <typename T, typename U> bool pair<T, U>::operator>=(const pair& rhs) { return m_First >= rhs.m_First; }

    template <typename T, typename U> bool pair<T, U>::operator==(const pair& rhs) { return m_First == rhs.m_First; }

    template <typename T, typename U> bool pair<T, U>::operator!=(const pair& rhs) { return m_First != rhs.m_First; }
}  // namespace PE
