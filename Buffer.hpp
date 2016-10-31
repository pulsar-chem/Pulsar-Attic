/*! \file
 *
 * \brief A simple class for a raw data buffer
 * \author Benjamin Pritchard (ben@bennyp.org)
 */

#ifndef BPMODULE_GUARD_MATH__BUFFER_HPP_
#define BPMODULE_GUARD_MATH__BUFFER_HPP_

#include <vector>
#include <algorithm>

#include "bpmodule/exception/Assert.hpp"
#include "bpmodule/exception/Exceptions.hpp"




namespace bpmodule {
namespace math {


/*! \brief A simple buffer for raw data
 *
 * \note Used instead of std::vector for a few
 *       reasons. In particular, this prevents
 *       automatic conversion to python lists.
 */
template<typename T>
class BufferT
{

    public:
        BufferT(size_t capacity)
            : buf_(new T[capacity]),
              capacity_(capacity_),
              size_(0)
        { }


        BufferT(size_t size, T const * const buf)
            : BufferT(size)
        {
            std::copy(buf, buf + size, buf_.get()); 
            size_ = size;
        }


        BufferT(void) 
            : BufferT(0)
        { }


        BufferT(BufferT &&) = default;
        BufferT & operator=(BufferT &&) = default;
        BufferT(const BufferT & rhs) = default;
        BufferT & operator=(const BufferT &) = default;


        T & operator[](size_t i) ASSERTIONS_ONLY
        {
            using namespace exception;
            Assert<MathException>("Value out of range in Buffer",
                                  "i", i, "size", size_);

            return buf_.get() + i;
        }


        const T & operator[](size_t i) const ASSERTIONS_ONLY
        {
            using namespace exception;
            Assert<MathException>("Value out of range in Buffer",
                                  "i", i, "size", size_);

            return buf_.get() + i;
        }


        T & At(size_t i)
        {
            if(i >= size_)
                throw exception::MathException("Value out of range in Buffer",
                                               "i", i, "size", size_);
            return buf_.get() + i;
        }

        const T & At(size_t i) const
        {
            if(i >= size_)
                throw exception::MathException("Value out of range in Buffer",
                                               "i", i, "size", size_);
            return buf_.get() + i;
        }
        

        size_t Size(void) const noexcept
        {
            return size_;
        }


        size_t Capacity(void) const noexcept
        {
            return capacity_;
        }



    private:
        std::vector<T> buf_;
        size_t size_;
        size_t capacity_;
};


typedef Buffer<double> DBuffer;
typedef Buffer<float> FBuffer;
typedef Buffer<std::complex<double>> CDBuffer;
typedef Buffer<std::complex<float>> CFBuffer;



} // close namespace math
} // close namespace bpmodule


#endif

