#ifndef MIV_CONTAINERS_MATRIX_H
#define MIV_CONTAINERS_MATRIX_H

#include <cstddef>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <initializer_list>

#include "array.hpp"

namespace miv
{
    template <typename type>
    class matrix
    {
    public:
        // basic constructors
        matrix();
        matrix(std::size_t rows, std::size_t cols);

        matrix(const matrix<type> &obj);
        matrix(matrix<type> &&obj) noexcept;

        // extra constructors
        matrix(std::initializer_list<std::initializer_list<type>> init);

        // copy-only from raw buffer
        matrix(const type *data, std::size_t rows, std::size_t cols);
        matrix(type *data, std::size_t rows, std::size_t cols); // copy-only, no stealing

        // construct 1 x N matrix from array
        matrix(const miv::array<type> &row);

        // construct matrix from rows (each miv::array is one row)
        // all rows must have the same size
        matrix(std::initializer_list<miv::array<type>> rows);

        // state methods
        bool empty() const;
        std::size_t rows() const;
        std::size_t cols() const;
        std::size_t size() const;

        // access methods
        type *data();
        const type *data() const;

        // row helpers
        type *row_ptr(std::size_t r);
        const type *row_ptr(std::size_t r) const;

        // extract row/column as a new miv::array (copy)
        miv::array<type> row(std::size_t r) const;
        miv::array<type> col(std::size_t c) const;

        // Extract row/column as a new matrix (copy)
        //
        // row_matrix(r)  -> shape: 1 x cols
        // col_matrix(c)  -> shape: rows x 1
        matrix<type> row_matrix(std::size_t r) const;
        matrix<type> col_matrix(std::size_t c) const;

        // action methods
        void clear();
        void fill(const type &value);

        // resize changes matrix shape.
        // Data is preserved in row-major order up to min(old_size, new_size).
        void resize(std::size_t new_rows, std::size_t new_cols);

        // Permute rows/columns in-place (copy-safe).
        // Both functions expect a REAL permutation:
        // - perm.size() must match rows/cols
        // - all indices must be in range
        // - indices must be unique (no duplicates)
        void row_permute(const miv::array<std::size_t> &perm);
        void col_permute(const miv::array<std::size_t> &perm);

        // iterators (pointers)
        type *begin();
        type *end();

        const type *begin() const;
        const type *end() const;

        const type *cbegin() const;
        const type *cend() const;

        // operators
        matrix<type> &operator=(const matrix<type> &obj);
        matrix<type> &operator=(matrix<type> &&obj) noexcept;

        // element access (checked, same philosophy as your array)
        type &operator()(std::size_t r, std::size_t c);
        const type &operator()(std::size_t r, std::size_t c) const;

        bool operator==(const matrix<type> &other) const;
        bool operator!=(const matrix<type> &other) const;

        ~matrix();

    private:
        // index helpers
        std::size_t linear_index(std::size_t r, std::size_t c) const;

        miv::array<type> m_data;
        std::size_t m_rows;
        std::size_t m_cols;
    };

    // constructors
    template <typename type>
    inline matrix<type>::matrix() : m_data(), m_rows(0), m_cols(0) {}

    template <typename type>
    inline matrix<type>::matrix(std::size_t rows, std::size_t cols)
        : m_data(rows * cols), m_rows(rows), m_cols(cols)
    {
        // rows * cols may overflow size_t in extreme cases;
        // user responsibility for sane sizes (like STL containers).
    }

    template <typename type>
    inline matrix<type>::matrix(const matrix<type> &obj)
        : m_data(obj.m_data), m_rows(obj.m_rows), m_cols(obj.m_cols) {}

    template <typename type>
    inline matrix<type>::matrix(matrix<type> &&obj) noexcept
        : m_data(std::move(obj.m_data)), m_rows(obj.m_rows), m_cols(obj.m_cols)
    {
        obj.m_rows = 0;
        obj.m_cols = 0;
    }

    template <typename type>
    inline matrix<type>::matrix(std::initializer_list<std::initializer_list<type>> init)
        : m_data(), m_rows(0), m_cols(0)
    {
        std::size_t r = init.size();
        std::size_t c = 0;

        // find max cols to allow "ragged" init -> we forbid ragged for safety
        for (const auto &row : init)
        {
            if (c == 0)
            {
                c = row.size();
            }
            else if (row.size() != c)
            {
                throw std::invalid_argument("matrix initializer_list must be rectangular (all rows same size)");
            }
        }

        if (r == 0 || c == 0)
        {
            return;
        }

        m_rows = r;
        m_cols = c;
        m_data.resize(m_rows * m_cols);

        std::size_t rr = 0;
        for (const auto &row : init)
        {
            std::size_t cc = 0;
            for (const auto &val : row)
            {
                (*this)(rr, cc) = val;
                ++cc;
            }
            ++rr;
        }
    }

    template <typename type>
    inline matrix<type>::matrix(const type *data, std::size_t rows, std::size_t cols)
        : m_data(), m_rows(0), m_cols(0)
    {
        if (rows == 0 || cols == 0)
        {
            return;
        }

        if (data == nullptr)
        {
            throw std::invalid_argument("Input pointer is nullptr but rows*cols is not zero");
        }

        m_rows = rows;
        m_cols = cols;
        m_data.resize(m_rows * m_cols);

        try
        {
            std::copy(data, data + (m_rows * m_cols), m_data.data());
        }
        catch (...)
        {
            // m_data will clean itself
            throw;
        }
    }

    template <typename type>
    inline matrix<type>::matrix(type *data, std::size_t rows, std::size_t cols)
        : matrix(static_cast<const type*>(data), rows, cols)
    {
        // copy-only by design
    }

        template <typename type>
    inline matrix<type>::matrix(const miv::array<type> &row)
        : m_data(row), m_rows(0), m_cols(0)
    {
        // Treat input array as one full row
        m_rows = (row.size() == 0) ? 0 : 1;
        m_cols = row.size();
    }

    template <typename type>
    inline matrix<type>::matrix(std::initializer_list<miv::array<type>> rows)
        : m_data(), m_rows(0), m_cols(0)
    {
        if (rows.size() == 0)
        {
            return;
        }

        // Determine columns from the first row
        m_cols = rows.begin()->size();
        m_rows = rows.size();

        // Allow empty matrix if cols==0
        if (m_cols == 0)
        {
            // If first row is empty, then all must be empty
            for (const auto &r : rows)
            {
                if (r.size() != 0)
                {
                    throw std::invalid_argument("matrix rows must have the same size");
                }
            }
            return;
        }

        // Validate all rows have the same size
        for (const auto &r : rows)
        {
            if (r.size() != m_cols)
            {
                throw std::invalid_argument("matrix rows must have the same size");
            }
        }

        // Allocate full storage
        m_data.resize(m_rows * m_cols);

        // Copy rows into contiguous buffer (row-major)
        std::size_t rr = 0;
        for (const auto &r : rows)
        {
            type *dst = m_data.data() + rr * m_cols;

            try
            {
                std::copy(r.data(), r.data() + m_cols, dst);
            }
            catch (...)
            {
                // m_data cleans itself
                throw;
            }

            ++rr;
        }
    }

    template <typename type>
    inline matrix<type>::~matrix()
    {
        // m_data destructor does the job
        m_rows = 0;
        m_cols = 0;
    }

    // state methods
    template <typename type>
    inline bool matrix<type>::empty() const
    {
        return m_rows == 0 || m_cols == 0;
    }

    template <typename type>
    inline std::size_t matrix<type>::rows() const
    {
        return m_rows;
    }

    template <typename type>
    inline std::size_t matrix<type>::cols() const
    {
        return m_cols;
    }

    template <typename type>
    inline std::size_t matrix<type>::size() const
    {
        return m_data.size();
    }

    // access methods
    template <typename type>
    inline type *matrix<type>::data()
    {
        return m_data.data();
    }

    template <typename type>
    inline const type *matrix<type>::data() const
    {
        return m_data.data();
    }

    template <typename type>
    inline type *matrix<type>::row_ptr(std::size_t r)
    {
        if (r >= m_rows)
        {
            throw std::out_of_range("Row " + std::to_string(r) + " is out of range");
        }

        return m_data.data() + (r * m_cols);
    }

    template <typename type>
    inline const type *matrix<type>::row_ptr(std::size_t r) const
    {
        if (r >= m_rows)
        {
            throw std::out_of_range("Row " + std::to_string(r) + " is out of range");
        }

        return m_data.data() + (r * m_cols);
    }

    template <typename type>
    inline miv::array<type> matrix<type>::row(std::size_t r) const
    {
        if (r >= m_rows)
        {
            throw std::out_of_range("Row " + std::to_string(r) + " is out of range");
        }

        if (m_cols == 0)
        {
            return miv::array<type>();
        }

        const type *src = m_data.data() + r * m_cols;

        // Copy row into a separate array
        return miv::array<type>(src, m_cols);
    }

    template <typename type>
    inline miv::array<type> matrix<type>::col(std::size_t c) const
    {
        if (c >= m_cols)
        {
            throw std::out_of_range("Column " + std::to_string(c) + " is out of range");
        }

        if (m_rows == 0)
        {
            return miv::array<type>();
        }

        miv::array<type> out(m_rows);

        for (std::size_t r = 0; r < m_rows; ++r)
        {
            out[r] = (*this)(r, c);
        }

        return out;
    }

    template <typename type>
    inline matrix<type> matrix<type>::row_matrix(std::size_t r) const
    {
        if (r >= m_rows)
        {
            throw std::out_of_range("Row " + std::to_string(r) + " is out of range");
        }

        // 1 x cols
        matrix<type> out(1, m_cols);

        if (m_cols == 0)
        {
            return out;
        }

        const type *src = m_data.data() + r * m_cols;

        try
        {
            std::copy(src, src + m_cols, out.data());
        }
        catch (...)
        {
            throw;
        }

        return out;
    }

    template <typename type>
    inline matrix<type> matrix<type>::col_matrix(std::size_t c) const
    {
        if (c >= m_cols)
        {
            throw std::out_of_range("Column " + std::to_string(c) + " is out of range");
        }

        // rows x 1
        matrix<type> out(m_rows, 1);

        if (m_rows == 0)
        {
            return out;
        }

        for (std::size_t r = 0; r < m_rows; ++r)
        {
            out(r, 0) = (*this)(r, c);
        }

        return out;
    }

    // action methods
    template <typename type>
    inline void matrix<type>::clear()
    {
        m_data.clear();
        m_rows = 0;
        m_cols = 0;
    }

    template <typename type>
    inline void matrix<type>::fill(const type &value)
    {
        m_data.fill(value);
    }

    template <typename type>
    inline void matrix<type>::resize(std::size_t new_rows, std::size_t new_cols)
    {
        if (new_rows == m_rows && new_cols == m_cols)
        {
            return;
        }

        if (new_rows == 0 || new_cols == 0)
        {
            clear();
            return;
        }

        // Keep old buffer, build new buffer, copy min region row-wise.
        miv::array<type> new_data(new_rows * new_cols);

        std::size_t min_r = (new_rows < m_rows) ? new_rows : m_rows;
        std::size_t min_c = (new_cols < m_cols) ? new_cols : m_cols;

        try
        {
            for (std::size_t r = 0; r < min_r; ++r)
            {
                const type *src = m_data.data() + (r * m_cols);
                type *dst = new_data.data() + (r * new_cols);

                std::copy(src, src + min_c, dst);
            }
        }
        catch (...)
        {
            // new_data cleans itself
            throw;
        }

        m_data = std::move(new_data);
        m_rows = new_rows;
        m_cols = new_cols;
    }

    template <typename type>
    inline void matrix<type>::row_permute(const miv::array<std::size_t> &perm)
    {
        if (perm.size() != m_rows)
        {
            throw std::invalid_argument("row_permute(): perm size must match number of rows");
        }

        // Validate indices are in range
        for (std::size_t i = 0; i < perm.size(); ++i)
        {
            if (perm[i] >= m_rows)
            {
                throw std::out_of_range("row_permute(): row index " + std::to_string(perm[i]) + " is out of range");
            }
        }

        // Validate uniqueness (true permutation)
        for (std::size_t i = 0; i < perm.size(); ++i)
        {
            for (std::size_t j = i + 1; j < perm.size(); ++j)
            {
                if (perm[i] == perm[j])
                {
                    throw std::invalid_argument("row_permute(): permutation contains duplicate indices");
                }
            }
        }

        if (m_rows == 0 || m_cols == 0)
        {
            return;
        }

        miv::array<type> new_data(m_rows * m_cols);

        try
        {
            for (std::size_t r = 0; r < m_rows; ++r)
            {
                const type *src = m_data.data() + perm[r] * m_cols;
                type *dst = new_data.data() + r * m_cols;

                std::copy(src, src + m_cols, dst);
            }
        }
        catch (...)
        {
            throw;
        }

        m_data = std::move(new_data);
    }

    template <typename type>
    inline void matrix<type>::col_permute(const miv::array<std::size_t> &perm)
    {
        if (perm.size() != m_cols)
        {
            throw std::invalid_argument("col_permute(): perm size must match number of cols");
        }

        // Validate indices are in range
        for (std::size_t i = 0; i < perm.size(); ++i)
        {
            if (perm[i] >= m_cols)
            {
                throw std::out_of_range("col_permute(): col index " + std::to_string(perm[i]) + " is out of range");
            }
        }

        // Validate uniqueness (true permutation)
        for (std::size_t i = 0; i < perm.size(); ++i)
        {
            for (std::size_t j = i + 1; j < perm.size(); ++j)
            {
                if (perm[i] == perm[j])
                {
                    throw std::invalid_argument("col_permute(): permutation contains duplicate indices");
                }
            }
        }

        if (m_rows == 0 || m_cols == 0)
        {
            return;
        }

        miv::array<type> new_data(m_rows * m_cols);

        try
        {
            for (std::size_t r = 0; r < m_rows; ++r)
            {
                for (std::size_t c = 0; c < m_cols; ++c)
                {
                    // new(r, c) = old(r, perm[c])
                    new_data[r * m_cols + c] = (*this)(r, perm[c]);
                }
            }
        }
        catch (...)
        {
            throw;
        }

        m_data = std::move(new_data);
    }

    // iterators
    template <typename type>
    inline type *matrix<type>::begin()
    {
        return m_data.begin();
    }

    template <typename type>
    inline type *matrix<type>::end()
    {
        return m_data.end();
    }

    template <typename type>
    inline const type *matrix<type>::begin() const
    {
        return m_data.begin();
    }

    template <typename type>
    inline const type *matrix<type>::end() const
    {
        return m_data.end();
    }

    template <typename type>
    inline const type *matrix<type>::cbegin() const
    {
        return m_data.cbegin();
    }

    template <typename type>
    inline const type *matrix<type>::cend() const
    {
        return m_data.cend();
    }

    // operators
    template <typename type>
    inline matrix<type> &matrix<type>::operator=(const matrix<type> &obj)
    {
        if (this != &obj)
        {
            m_data = obj.m_data;
            m_rows = obj.m_rows;
            m_cols = obj.m_cols;
        }

        return *this;
    }

    template <typename type>
    inline matrix<type> &matrix<type>::operator=(matrix<type> &&obj) noexcept
    {
        if (this != &obj)
        {
            m_data = std::move(obj.m_data);
            m_rows = obj.m_rows;
            m_cols = obj.m_cols;

            obj.m_rows = 0;
            obj.m_cols = 0;
        }

        return *this;
    }

    template <typename type>
    inline std::size_t matrix<type>::linear_index(std::size_t r, std::size_t c) const
    {
        // bounds check here allows operator() to stay clean
        if (r >= m_rows || c >= m_cols)
        {
            throw std::out_of_range("Index (" + std::to_string(r) + "," + std::to_string(c) + ") is out of range");
        }

        return r * m_cols + c;
    }

    template <typename type>
    inline type &matrix<type>::operator()(std::size_t r, std::size_t c)
    {
        return m_data[linear_index(r, c)];
    }

    template <typename type>
    inline const type &matrix<type>::operator()(std::size_t r, std::size_t c) const
    {
        return m_data[linear_index(r, c)];
    }

    template <typename type>
    inline bool matrix<type>::operator==(const matrix<type> &other) const
    {
        if (m_rows != other.m_rows || m_cols != other.m_cols)
        {
            return false;
        }

        return m_data == other.m_data;
    }

    template <typename type>
    inline bool matrix<type>::operator!=(const matrix<type> &other) const
    {
        return !(*this == other);
    }
}

#endif // MIV_CONTAINERS_MATRIX_H
