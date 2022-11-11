#pragma once

#include <exception>
#include <memory>
#include <type_traits>
#include <typeinfo>

class Any;

template <class Type>
Type any_cast(Any&);
template <class Type>
Type any_cast(const Any&);
template <class Type>
Type* any_cast(Any*);
template <class Type>
const Type* any_cast(const Any*);

struct bad_any_cast : public std::bad_cast
{
};

class Any
{
public:

    template <class Type>
    friend Type any_cast(Any&);
    template <class Type>
    friend Type any_cast(const Any&);
    template <class Type>
    friend Type* any_cast(Any*);
    template <class Type>
    friend const Type* any_cast(const Any*);

    Any()
        : ptr(nullptr)
    {
    }

    Any(Any&& x) noexcept
        : ptr(std::move(x.ptr))
    {
    }

    Any(const Any& x)
    {
        if (x.ptr) ptr = x.ptr->clone();
    }

    template <class Type>
    Any(const Type& x)
        : ptr(new concrete<typename std::decay<const Type>::type>(x))
    {
    }

    Any& operator=(Any&& rhs) noexcept
    {
        ptr = std::move(rhs.ptr);
        return (*this);
    }

    Any& operator=(const Any& rhs)
    {
        ptr = std::move(Any(rhs).ptr);
        return (*this);
    }

    template <class T>
    Any& operator=(T&& x)
    {
        ptr.reset(new concrete<typename std::decay<T>::type>(typename std::decay<T>::type(x)));
        return (*this);
    }

    template <class T>
    Any& operator=(const T& x)
    {
        ptr.reset(new concrete<typename std::decay<T>::type>(typename std::decay<T>::type(x)));
        return (*this);
    }

    void clear() { ptr.reset(nullptr); }

    bool empty() const { return ptr == nullptr; }

    const std::type_info& type() const { return (!empty()) ? ptr->type() : typeid(void); }

private:

    struct placeholder
    {
        virtual std::unique_ptr<placeholder> clone() const = 0;
        virtual const std::type_info& type() const = 0;
        virtual ~placeholder() {}
    };

    template <class T>
    struct concrete : public placeholder
    {
        concrete(T&& x)
            : value(std::move(x))
        {
        }

        concrete(const T& x)
            : value(x)
        {
        }

        virtual std::unique_ptr<placeholder> clone() const override { return std::unique_ptr<placeholder>(new concrete<T>(value)); }

        virtual const std::type_info& type() const override { return typeid(T); }

        T value;
    };

    std::unique_ptr<placeholder> ptr;
};

template <class Type>
Type any_cast(Any& val)
{
    if (val.ptr->type() != typeid(Type)) throw bad_any_cast();
    return static_cast<Any::concrete<Type>*>(val.ptr.get())->value;
}

template <class Type>
Type any_cast(const Any& val)
{
    return any_cast<Type>(Any(val));
}
template <class Type>
Type* any_cast(Any* ptr)
{
    return dynamic_cast<Type*>(ptr->ptr.get());
}
template <class Type>
const Type* any_cast(const Any* ptr)
{
    return dynamic_cast<const Type*>(ptr->ptr.get());
}
