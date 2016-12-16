/*! \file
 *
 * \brief Matrices and vectors blocked by irrep and spin
 * \author Benjamin Pritchard (ben@bennyp.org)
 */


#include "pulsar/math/IrrepSpinMatrix.hpp"


namespace pulsar{
namespace math{


// Explicit instantiations
template class BlockByIrrepSpin<SimpleMatrixF>;
template class BlockByIrrepSpin<SimpleMatrixD>;
template class BlockByIrrepSpin<SimpleMatrixCF>;
template class BlockByIrrepSpin<SimpleMatrixCD>;

template class BlockByIrrepSpin<SimpleVectorF>;
template class BlockByIrrepSpin<SimpleVectorD>;
template class BlockByIrrepSpin<SimpleVectorCF>;
template class BlockByIrrepSpin<SimpleVectorCD>;

template class BlockByIrrepSpin<float>;
template class BlockByIrrepSpin<double>;
template class BlockByIrrepSpin<std::complex<float>>;
template class BlockByIrrepSpin<std::complex<double>>;

} // close namespace math
} // close namespace pulsar

