/*! \file
 *
 * \brief Operator overloads for set operations
 * \author Benjamin Pritchard (ben@bennyp.org)
 */ 

#ifndef PULSAR_GUARD_MATH__SETOPERATIONS_HPP_
#define PULSAR_GUARD_MATH__SETOPERATIONS_HPP_

#include "pulsar/exception/Assert.hpp"
#include "pulsar/math/Universe.hpp"

#include <type_traits>


// operator +=  ->  union_assign
template<typename LHS, typename RHS>
auto operator+=(LHS & lhs, const RHS & rhs)
-> decltype(static_cast<LHS & (LHS::*)(const RHS &)>(&LHS::union_assign), lhs)
{
    return lhs.union_assign(rhs);
}

template<typename LHS, typename RHS>
auto operator+=(LHS & lhs, RHS && rhs)
  -> decltype(static_cast<LHS & (LHS::*)(RHS &&)>(&LHS::union_assign), lhs)
{
    return lhs.union_assign(std::move(rhs));
}


// operator +  -> set_union
template<typename LHS, typename RHS>
auto operator+(const LHS & lhs, const RHS & rhs)
-> decltype(static_cast<LHS & (LHS::*)(const RHS &)>(&LHS::set_union),
            std::remove_reference<typename std::remove_const<decltype(lhs)>::type>::type)
{
    return lhs.set_union(rhs);
}


// operator -= -> difference_assign
template<typename LHS, typename RHS>
auto operator-=(LHS & lhs, const RHS & rhs)
-> decltype(static_cast<LHS & (LHS::*)(const RHS &)>(&LHS::difference_assign), lhs)
{
    return lhs.difference_assign(rhs);
}

template<typename LHS, typename RHS>
auto operator-=(LHS & lhs, RHS && rhs)
  -> decltype(static_cast<LHS & (LHS::*)(RHS &&)>(&LHS::difference_assign), lhs)
{
    return lhs.difference_assign(std::move(rhs));
}


// operator - -> difference
template<typename LHS, typename RHS>
auto operator-(const LHS & lhs, const RHS & rhs)
-> decltype(static_cast<LHS & (LHS::*)(const RHS &)>(&LHS::difference),
            std::remove_reference<typename std::remove_const<decltype(lhs)>::type>::type)
{
    return lhs.difference(rhs);
}


// operator /= -> intersection_assign
template<typename LHS, typename RHS>
auto operator/=(LHS & lhs, const RHS & rhs)
-> decltype(static_cast<LHS & (LHS::*)(const RHS &)>(&LHS::intersection_assign), lhs)
{
    return lhs.intersection_assign(rhs);
}

template<typename LHS, typename RHS>
auto operator/=(LHS & lhs, RHS && rhs)
  -> decltype(static_cast<LHS & (LHS::*)(RHS &&)>(&LHS::intersection_assign), lhs)
{
    return lhs.intersection_assign(std::move(rhs));
}


// operator / -> intersection
template<typename LHS, typename RHS>
auto operator/(const LHS & lhs, const RHS & rhs)
-> decltype(static_cast<LHS & (LHS::*)(const RHS &)>(&LHS::intersection),
            std::remove_reference<typename std::remove_const<decltype(lhs)>::type>::type)
{
    return lhs.intersection(rhs);
}


// operator < -> is_proper_subset_of
template<typename LHS, typename RHS>
auto operator<(const LHS & lhs, const RHS & rhs)
-> decltype(static_cast<bool (LHS::*)(const RHS &) const>(&LHS::is_proper_subset_of), bool())
{
    return lhs.is_proper_subset_of(rhs);
}


// operator > -> is_proper_superset_of
template<typename LHS, typename RHS>
auto operator>(const LHS & lhs, const RHS & rhs)
-> decltype(static_cast<bool (LHS::*)(const RHS &) const>(&LHS::is_proper_superset_of), bool())
{
    return lhs.is_proper_superset_of(rhs);
}


// operator <= -> is_subset_of
template<typename LHS, typename RHS>
auto operator>(const LHS & lhs, const RHS & rhs)
-> decltype(static_cast<bool (LHS::*)(const RHS &) const>(&LHS::is_subset_of), bool())
{
    return lhs.is_subset_of(rhs);
}


// operator >= -> is_superset_of
template<typename LHS, typename RHS>
auto operator>(const LHS & lhs, const RHS & rhs)
-> decltype(static_cast<bool (LHS::*)(const RHS &) const>(&LHS::is_superset_of), bool())
{
    return lhs.is_superset_of(rhs);
}


#endif

