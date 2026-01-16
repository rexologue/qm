#ifndef MIV_CONTAINERS_ARRAY_H
#define MIV_CONTAINERS_ARRAY_H

#include <string>
#include <cstddef>
#include <algorithm>
#include <stdexcept>
#include <initializer_list>

namespace miv
{
    template <typename type>
    class array
    {
    public:
        // basic constructors
        array();
        array(std::size_t size);
        array(const array<type> &obj);
        array(array<type> &&obj) noexcept;

        // extra constructors
        array(std::initializer_list<type> init);
        array(const type *data, std::size_t size);
        array(type *data, std::size_t size); // copy-only, no stealing

        // methods
        type* data();
        const type* data() const;
        std::size_t size() const;

        bool empty() const;

        void clear();
        void fill(const type &value);
        void resize(std::size_t new_size);

        type &front();
        const type &front() const;

        type &back();
        const type &back() const;

        // === Search helpers ===
        //
        // npos ("no position") is a special constant meaning: "value was not found".
        // It is used as a return value of find().
        //
        // Why size_t(-1)?
        // std::size_t is an unsigned type. Converting -1 to size_t produces the maximum
        // representable value (SIZE_MAX). Such index can never be valid because any valid
        // index must satisfy: 0 <= idx < m_size.
        //
        // This is the same idea as std::string::npos.
        static constexpr std::size_t npos = static_cast<std::size_t>(-1);

        // find(value) returns the index of the FIRST occurrence of the value.
        // If the value is not found, it returns npos.
        //
        // Usage:
        //   auto idx = arr.find(x);
        //   if (idx == array<T>::npos) { /* not found */ }
        //   else { /* found at idx */ }
        std::size_t find(const type &value) const;

        // contains(value) returns true if the value exists in the array, otherwise false.
        // Internally it uses find().
        bool contains(const type &value) const;

        // iterators (pointers)
        type* begin();
        type* end();

        const type* begin() const;
        const type* end() const;

        const type* cbegin() const;
        const type* cend() const;

        // operators
        array<type> &operator=(const array<type> &obj);
        array<type> &operator=(array<type> &&obj) noexcept;

        type &operator[](std::size_t idx);
        const type &operator[](std::size_t idx) const;

        bool operator==(const array<type> &other) const;
        bool operator!=(const array<type> &other) const;

        ~array();
    
    private:
        type* m_data;
        std::size_t m_size;
    };

    // construsctors
    template <typename type>
    inline array<type>::array() : m_data(nullptr), m_size(0) {}

    template <typename type>
    inline array<type>::array(std::size_t size) : m_data(new type[size]), m_size(size) {}

    template <typename type>
    inline array<type>::array(std::initializer_list<type> init) : m_data(nullptr), m_size(0)
    {
        if (init.size() == 0)
        {
            return;
        }

        type *buffer = new type[init.size()];

        try
        {
            std::copy(init.begin(), init.end(), buffer);
        }
        catch (...)
        {
            delete[] buffer;
            throw;
        }

        m_data = buffer;
        m_size = init.size();
    }

    template <typename type>
    inline array<type>::array(const type *data, std::size_t size) : m_data(nullptr), m_size(0)
    {
        if (size == 0)
        {
            return;
        }

        if (data == nullptr)
        {
            throw std::invalid_argument("Input pointer is nullptr but size is not zero");
        }

        type *buffer = new type[size];

        try
        {
            std::copy(data, data + size, buffer);
        }
        catch (...)
        {
            delete[] buffer;
            throw;
        }

        m_data = buffer;
        m_size = size;
    }

    template <typename type>
    inline array<type>::array(type *data, std::size_t size) : array(static_cast<const type*>(data), size)
    {
        // copy-only by design
    }

    template <typename type>
    inline array<type>::~array()
    {
        delete[] m_data;
        m_size = 0;
        m_data = nullptr;
    }

    template <typename type>
    inline array<type>::array(const array<type> &obj)
    {
        type *buffer = new type[obj.m_size];

        try
        {
            std::copy(obj.m_data, obj.m_data + obj.m_size, buffer);
        }
        catch (...)
        {
            delete[] buffer;
            throw;
        }

        m_data = buffer;
        m_size = obj.m_size;
    }

    template <typename type>
    inline array<type>::array(array<type> &&obj) noexcept
    {
        m_data = obj.m_data;
        m_size = obj.m_size;

        obj.m_data = nullptr;
        obj.m_size = 0;
    }

    // methods
    template <typename type>
    type *array<type>::data()
    {
        return m_data;
    }

    template <typename type>
    const type *array<type>::data() const
    {
        return m_data;
    }

    template <typename type>
    std::size_t array<type>::size() const
    {
        return m_size;
    }

    template <typename type>
    bool array<type>::empty() const
    {
        return m_size == 0;
    }

    template <typename type>
    void array<type>::clear()
    {
        delete[] m_data;
        m_data = nullptr;
        m_size = 0;
    }

    template <typename type>
    void array<type>::fill(const type &value)
    {
        if (m_size == 0 || m_data == nullptr)
        {
            return;
        }

        std::fill(m_data, m_data + m_size, value);
    }

    template <typename type>
    void array<type>::resize(std::size_t new_size)
    {
        if (new_size == m_size)
        {
            return;
        }

        if (new_size == 0)
        {
            clear();
            return;
        }

        type *buffer = new type[new_size];

        std::size_t copy_size = (new_size < m_size) ? new_size : m_size;

        try
        {
            if (copy_size > 0 && m_data != nullptr)
            {
                std::copy(m_data, m_data + copy_size, buffer);
            }
        }
        catch (...)
        {
            delete[] buffer;
            throw;
        }

        delete[] m_data;
        m_data = buffer;
        m_size = new_size;
    }

    template <typename type>
    type &array<type>::front()
    {
        if (m_size == 0)
        {
            throw std::out_of_range("front() called on empty array");
        }

        return m_data[0];
    }

    template <typename type>
    const type &array<type>::front() const
    {
        if (m_size == 0)
        {
            throw std::out_of_range("front() called on empty array");
        }

        return m_data[0];
    }

    template <typename type>
    type &array<type>::back()
    {
        if (m_size == 0)
        {
            throw std::out_of_range("back() called on empty array");
        }

        return m_data[m_size - 1];
    }

    template <typename type>
    const type &array<type>::back() const
    {
        if (m_size == 0)
        {
            throw std::out_of_range("back() called on empty array");
        }

        return m_data[m_size - 1];
    }

    template <typename type>
    std::size_t array<type>::find(const type &value) const
    {
        // Empty array: nothing can be found.
        if (m_size == 0 || m_data == nullptr)
        {
            return npos;
        }

        // std::find returns iterator (here it is a pointer) to the first matching element.
        const type *it = std::find(m_data, m_data + m_size, value);

        // If iterator points to end, then nothing was found.
        if (it == m_data + m_size)
        {
            return npos;
        }

        // Convert pointer into index by pointer arithmetic.
        return static_cast<std::size_t>(it - m_data);
    }

    template <typename type>
    bool array<type>::contains(const type &value) const
    {
        // contains is just a convenient wrapper around find().
        return find(value) != npos;
    }

    // iterators
    template <typename type>
    type *array<type>::begin()
    {
        return m_data;
    }

    template <typename type>
    type *array<type>::end()
    {
        if (m_data == nullptr)
        {
            return nullptr;
        }

        return m_data + m_size;
    }

    template <typename type>
    const type *array<type>::begin() const
    {
        return m_data;
    }

    template <typename type>
    const type *array<type>::end() const
    {
        if (m_data == nullptr)
        {
            return nullptr;
        }

        return m_data + m_size;
    }

    template <typename type>
    const type *array<type>::cbegin() const
    {
        return begin();
    }

    template <typename type>
    const type *array<type>::cend() const
    {
        return end();
    }

    // operators
    template <typename type>
    array<type> &array<type>::operator=(const array<type> &obj)
    {
        type *buffer = new type[obj.m_size];
        
        try
        {
            std::copy(obj.m_data, obj.m_data + obj.m_size, buffer);
        }
        catch (...)
        {
            delete[] buffer;
            throw;
        }

        delete[] m_data;
        m_data = buffer;
        m_size = obj.m_size;

        return *this;
    }

    template <typename type>
    array<type> &array<type>::operator=(array<type> &&obj) noexcept
    {
        if (this != &obj) 
        {
            delete[] m_data;

            m_data = obj.m_data;
            m_size = obj.m_size;

            obj.m_data = nullptr;
            obj.m_size = 0;
        }

        return *this;
    }

    template <typename type>
    type &array<type>::operator[](std::size_t idx)
    {
        // In case if negative idx is given size_t convert it to SIZE_MAX and error will be thrown
        if (idx >= m_size)
        {
            throw std::out_of_range("Index " + std::to_string(idx) + " is out of range");
        }

        return m_data[idx];
    }

    template <typename type>
    const type &array<type>::operator[](std::size_t idx) const
    {
        // In case if negative idx is given size_t convert it to SIZE_MAX and error will be thrown
        if (idx >= m_size)
        {
            throw std::out_of_range("Index " + std::to_string(idx) + " is out of range");
        }

        return m_data[idx];
    }

    template <typename type>
    bool array<type>::operator==(const array<type> &other) const
    {
        if (m_size != other.m_size)
        {
            return false;
        }

        if (m_size == 0)
        {
            return true;
        }

        return std::equal(m_data, m_data + m_size, other.m_data);
    }

    template <typename type>
    bool array<type>::operator!=(const array<type> &other) const
    {
        return !(*this == other);
    }
}

#endif // MIV_CONTAINERS_ARRAY_H
