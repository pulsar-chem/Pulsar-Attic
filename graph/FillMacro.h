#ifndef PULSAR_GUARD_GRAPH__FILLMACRO_H_
#define PULSAR_GUARD_GRAPH__FILLMACRO_H_

#include <array>

#define COMMA ,
#define FILL_VIA_ARRAY(DATA_TYPE,DATA_SIZE,INPUT_,FXN_)\
   std::array<DATA_TYPE,DATA_SIZE> Temp(INPUT_);\
   FXN_(Temp.begin(),Temp.end());

/** \brief Auto defines a set of functions for filling a function
 *
 *
 *  Thanks to C++11 there are now a host of ways to initialize an
 *  object.  This macro represents a code factorization that introduces
 *  a fill fxn that takes an arbitrary, non-zero number of objects via a
 *  variadic template (technically the variadic template is only used
 *  for three or more so that we can distinguish between iterators and
 *  legit fills), an initializer list fill, and a fill from two iterators.
 *
 *  \param[in] DATA_TYPE The type of data that goes into your container
 *  \param[in] FXN_THAT_FILLS The name of the function that all of these
 *                            wrappers call.  Must take two iterators, one
 *                            the first of which is the begin iterator and
 *                            the second is the end iterator.
 *  \param[in] FILL_FXN_NAME The name of the resulting function, which will
 *                           be overloaded for all of the filling methods
 *                           described above
 *  \param[in] RETURN The return value of your overloaded function, leave blank if
 *                    this macro is making constructors
 *
 *
 */
#define DEFINE_FILL_FXNS(DATA_TYPE,FXN_THAT_FILLS,FILL_FXN_NAME,RETURN) \
RETURN FILL_FXN_NAME(DATA_TYPE arg1){\
   FILL_VIA_ARRAY(DATA_TYPE,1,{arg1},FXN_THAT_FILLS)\
}\
RETURN FILL_FXN_NAME(DATA_TYPE arg1,DATA_TYPE arg2){\
   FILL_VIA_ARRAY(DATA_TYPE,2,{arg1 COMMA arg2},FXN_THAT_FILLS)\
}\
template<typename...FILL_FXN_NAME##Args> \
RETURN FILL_FXN_NAME(DATA_TYPE a,\
                     DATA_TYPE b,\
                     DATA_TYPE c,\
                     FILL_FXN_NAME##Args...d){\
   FILL_VIA_ARRAY(DATA_TYPE,3+sizeof...(d),{a COMMA b COMMA c COMMA d...},FXN_THAT_FILLS)\
}\
RETURN FILL_FXN_NAME(const std::initializer_list<DATA_TYPE>& IL){\
   FXN_THAT_FILLS(IL.begin(),IL.end());\
}\
template<typename FILL_FXN_NAME##I1,typename FILL_FXN_NAME##I2>\
RETURN FILL_FXN_NAME(FILL_FXN_NAME##I1 I1,FILL_FXN_NAME##I2 I2){\
   FXN_THAT_FILLS(I1,I2);\
}




#endif /* GRAPH_FILLMACRO_H_ */
