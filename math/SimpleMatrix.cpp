/*! \file
 *
 * \brief General-purpose dense matrix & vector classes
 * \author Benjamin Pritchard (ben@bennyp.org)
 */

#include "pulsar/math/SimpleMatrix.hpp"


namespace pulsar{
namespace math{

template class SimpleMatrix<float>;
template class SimpleMatrix<double>;
template class SimpleMatrix<std::complex<float>>;
template class SimpleMatrix<std::complex<double>>;
template class SimpleVector<float>;
template class SimpleVector<double>;
template class SimpleVector<std::complex<float>>;
template class SimpleVector<std::complex<double>>;


} // close namespace math
} // close namespace pulsar
