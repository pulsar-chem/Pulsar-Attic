/*! \file
 *
 * \brief Functions for testing serialization (header)
 * \author Benjamin Pritchard (ben@bennyp.org)
 */ 


#ifndef PULSAR_GUARD_TESTING__TESTSERIALIZATION_HPP_
#define PULSAR_GUARD_TESTING__TESTSERIALIZATION_HPP_

#include <string>
#include "pulsar/util/PythonHelper.hpp"
#include "pulsar/testing/TestingBase.hpp"
#include "pulsar/util/Serialization.hpp"
#include "bphash/Hasher.hpp"

namespace pulsar{

/*! \brief serialize an object, then unserialize
 *
 * \tparam T C++ type to use
 *
 * \warning Object must be registered to python
 */
template<typename T, typename Equality = std::equal_to<T>>
bool RoundTripSerialization(pybind11::object obj)
{
    using namespace bphash;
    

    T & cppobj = pulsar::convert_to_cpp<T &>(obj);

    // Initial hash
    HashValue hash1 = make_hash(HashType::Hash128, cppobj);

    MemoryArchive mar;
    mar.begin_serialization();
    mar.serialize(cppobj);
    mar.end_serialization();

    mar.begin_unserialization();
    T newobj(std::move(mar.unserialize_single<T>()));
    mar.end_unserialization();

    // Hash after unserialization
    HashValue hash2 = make_hash(HashType::Hash128, newobj);

    // Round trip a byte array
    ByteArray ba = to_byte_array(cppobj);
    T newobj2 = from_byte_array<T>(ba);
    HashValue hash3 = make_hash(HashType::Hash128, newobj2);

    // Round trip a byte array, but using pointers
    std::unique_ptr<T> newobj3 = new_from_byte_array<T>(ba);
    HashValue hash4 = make_hash(HashType::Hash128, *newobj3);


    Equality eq;


    print_global_debug("Hash1: %s \n", hash_to_string(hash1));
    print_global_debug("Hash2: %s \n", hash_to_string(hash2));
    print_global_debug("Hash3: %s \n", hash_to_string(hash3));
    print_global_debug("Hash4: %s \n", hash_to_string(hash4));
    print_global_debug("Hash Eq 1: %? \n", hash1 == hash2);
    print_global_debug("Hash Eq 2: %? \n", hash2 == hash3);
    print_global_debug("Hash Eq 3: %? \n", hash3 == hash4);
    print_global_debug("Equality 1: %s \n",  eq(cppobj, newobj) ? "True" : "False");
    print_global_debug("Equality 2: %s \n",  eq(cppobj, newobj2) ? "True" : "False");
    print_global_debug("Equality 3: %s \n",  eq(cppobj, *newobj3) ? "True" : "False");


    // All hashes should match, and the two objects should
    // be equal
    return hash1 == hash2 &&
           hash2 == hash3 &&
           hash3 == hash4 &&
           eq(cppobj, newobj) &&
           eq(cppobj, newobj2) &&
           eq(cppobj, *newobj3);

}



/*! \brief Test serialization and unserialization via round trip conversion
 *
 * \tparam T C++ type to use
 *
 * \param [in] obj Object to convert
 * \return False if the conversion fails, True if it succeeds
 */
template<typename T, typename Equality = std::equal_to<T>>
int TestSerialization(pybind11::object obj)
{
    return TestBoolFunc(RoundTripSerialization<T, Equality>, obj);
}

} // close namespace pulsar



#endif
