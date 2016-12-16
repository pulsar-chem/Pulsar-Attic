/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Geometry.hpp
 * Author: richard
 *
 * Created on April 15, 2016, 3:43 PM
 */

#ifndef PULSAR_GUARD_MATH__GEOMETRY_HPP_
#define PULSAR_GUARD_MATH__GEOMETRY_HPP_

#include<array>
#include<cmath>
#include "pulsar/constants.h"//For Pi
#include "pulsar/math/BLAS.hpp"
namespace pulsar{
namespace math{

///Given three points returns the norm of the plane they lie in
template<typename T>
std::array<double,3> get_plane(const T& p1,const T& p2, const T& p3){
    typedef std::array<double,3> V_t;
    return Cross(V_t({p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]}),
                 V_t({p3[0]-p1[0],p3[1]-p1[1],p3[2]-p1[2]}));
}


///Returns rotation matrix about the unit vector by \p n degrees
template<typename T>
std::array<double,9> rotation(const T& Axis, double ndegrees){
    const double angle=ndegrees*PI/180.0,x=Axis[0],y=Axis[1],z=Axis[2];
    const double c=cos(angle),s=sin(angle);
    double c1=1-c;
    return {c+x*x*c1, x*y*c1-z*s,x*z*c1+y*s,
                y*x*c1+z*s,c+y*y*c1,y*z*c1-x*s,
                z*x*c1-y*s,z*y*c1+x*s,c+z*z*c1};
}

///Reflection through plane normal to unit vector \p Norm
template<typename T>
std::array<double,9> reflection(const T& Norm){
        const double x=Norm[0],y=Norm[1],z=Norm[2];
        return {1-2*x*x, -2*x*y, -2*x*z,
                 -2*x*y,1-2*y*y, -2*y*z,
                 -2*x*z, -2*y*z,1-2*z*z};
}

///Rotation about \p Axis by \p n degrees followed by reflection
///perpendicular to it
template<typename T>
std::array<double,9> roto_reflection(const T& P,double n){
        std::array<double,9> Rot=rotation(P,n),Ref=reflection(P),Result={};
        for(size_t i=0;i<3;++i)
            for(size_t j=0;j<3;++j)
                for(size_t k=0;k<3;++k)
                    Result[i*3+j]+=Ref[i*3+k]*Rot[k*3+j];
        return Result;
}



}}//End namespaces


#endif /* GEOMETRY_HPP */

