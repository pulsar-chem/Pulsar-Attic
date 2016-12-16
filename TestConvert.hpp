/*! \file
 *
 * \brief Various test for the core (header)
 * \author Benjamin Pritchard (ben@bennyp.org)
 */ 


#ifndef PULSAR_GUARD_TESTING__TESTCONVERT_HPP_
#define PULSAR_GUARD_TESTING__TESTCONVERT_HPP_

#include <string>

#include "pulsar/util/PythonHelper.hpp"
#include "pulsar/testing/TestingBase.hpp"


namespace pulsar{


/*! \brief An class not exported to python
 *
 * Used to test failure of convert_to_py
 */
template<typename T>
struct FailObject
{
    T t;
};



/*! \brief Testing of python-to-C++ conversions
 *
 * \tparam T C++ type to use
 *
 * \param [in] obj Object to convert
 * \return 1 if the conversion fails, 0 if it succeeds
 */
template<typename T>
int Testconvert_to_cpp(pybind11::object obj)
{
    return TestFunc(pulsar::convert_to_cpp<T>, obj);
}


/*! \brief Testing of C++-to-python conversions
 *
 * \tparam T C++ type to use
 *
 * \param [in] obj Object to convert
 * \return 1 if the conversion fails, 0 if it succeeds
 */
template<typename T>
int Testconvert_to_py(const T & obj)
{
    return TestFunc(pulsar::convert_to_py<T>, obj);
}


/*! \brief A single Python-to-C++-to-python round trip conversion
 *
 * \tparam T C++ type to use
 *
 * \param [in] obj Object to convert
 * \return 1 if the conversion fails, 0 if it succeeds
 */
template<typename T>
void PyCppPy(pybind11::object obj)
{
    T t = convert_to_cpp<T>(obj);
    pybind11::object obj2 = convert_to_py<T>(t);
}



/*! \brief A single Python-to-C++-to-python round trip conversion that fails
 *
 * This should fail due to the object being passwd to convert_to_py not
 * being exported to python
 *
 * \tparam T C++ type to use
 *
 * \param [in] obj Object to convert
 * \return 1 if the conversion fails, 0 if it succeeds
 */
template<typename T>
void PyCppPy_Fail(pybind11::object obj)
{
    T t = convert_to_cpp<T>(obj);
    FailObject<T> fo{t};
    pybind11::object obj2 = convert_to_py<FailObject<T>>(fo);
}


/*! \brief Test Python-to-C++-to-python round trip conversion
 *
 * \tparam T C++ type to use
 *
 * \param [in] obj Object to convert
 * \return 1 if the conversion fails, 0 if it succeeds
 */
template<typename T>
int TestPyCppPy(pybind11::object obj)
{
    return TestFunc(PyCppPy<T>, obj);
}



/*! \brief Test Python-to-C++-to-python round trip conversion that fails
 *
 * \tparam T C++ type to use
 *
 * \param [in] obj Object to convert
 * \return 1 if the conversion fails, 0 if it succeeds
 */
template<typename T>
int TestPyCppPy_Fail(pybind11::object obj)
{
    return TestFunc(PyCppPy_Fail<T>, obj);
}

} // close namespace pulsar



#endif
