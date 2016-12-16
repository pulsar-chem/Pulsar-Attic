/*! \file
 *
 * \brief Convenience functions for testing the core library
 * \author Benjamin Pritchard (ben@bennyp.org)
 */ 


#ifndef PULSAR_GUARD_TESTING__TESTINGBASE_HPP_
#define PULSAR_GUARD_TESTING__TESTINGBASE_HPP_

#include <functional>

#include "pulsar/output/GlobalOutput.hpp"

namespace pulsar{

/*! \brief Test a function call
 *
 * \tparam T A callable object type
 * \tparam Targs Types of the object's arguments
 *
 * \param [in] func A callable object
 * \param [in] Fargs Arguments for that callable object
 *
 * \return False If the function fails (throws), True if it succeeds
 */
template<typename T, typename... Targs>
bool TestFunc(T func, Targs... Fargs)
{
    try {
       func(Fargs...); 
    }
    catch(std::exception & ex)
    {
        print_global_debug(ex.what());
        print_global_debug("\n");
        return false;
    }   
    catch(...)
    {
        print_global_debug("Caught unknown exception\n");
        return false;
    }   

    // here if no exceptions are thrown
    return true;
}



/*! \brief Test a function call (that returns a bool)
 *
 * \tparam T A callable object type that returns a bool
 * \tparam Targs Types of the object's arguments
 *
 * \param [in] func A callable object
 * \param [in] Fargs Arguments for that callable object
 *
 * \return False If the function fails (returns false or throws), True if it succeeds
 */
template<typename T, typename... Targs>
bool TestBoolFunc(T func, Targs... Fargs)
{
    try {
        return func(Fargs...);
    }
    catch(std::exception & ex)
    {
        print_global_debug(ex.what());
        print_global_debug("\n");
        return false;
    }   
    catch(...)
    {
        print_global_debug("Caught unknown exception\n");
        return false;
    }   
}




/*! \brief Test construction of an object
 *
 * \tparam T An object to construct
 * \tparam Targs Types of the object's constructor's arguments
 *
 * \param [in] Fargs Arguments for the object's constructor
 *
 * \return True If the construction fails, False if it succeeds
 */
template<typename T, typename... Targs>
bool TestConstruct(Targs... Fargs)
{
    try {
       T obj(Fargs...); 
    }
    catch(std::exception & ex)
    {
        print_global_debug(ex.what());
        print_global_debug("\n");
        return false;
    }   
    catch(...)
    {
        print_global_debug("Caught unknown exception\n");
        return false;
    }   

    // here if no exceptions are thrown
    return true;
}

} // close namespace pulsar

#endif
