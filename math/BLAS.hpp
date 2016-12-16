/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BLAS.hpp
 * Author: richard
 *
 * Created on April 13, 2016, 5:20 PM
 */

#ifndef PULSAR_GUARD_MATH__BLAS_HPP_
#define PULSAR_GUARD_MATH__BLAS_HPP_

#include <string>
#include <array>
#include <complex>
#include <tuple>
#include <vector>
#include "pulsar/exception/PulsarException.hpp"


extern "C" {
    void dsyev(char*,char*,int*,double*,int*,double*,double*,int*,int*);
    void dgeev(char*,char*,int*,double*,int*,double*,double*,double*,int*,
               double*,int*,double*,int*,int*);
    void dgesvd(char*, char*, int*,int*,double*,int*,double*,double*,int*,
                double*,int*,double*,int*,int*);
}

///The return type of the non-symmetric diagonalizer
typedef std::tuple<std::vector<std::complex<double>>,
                   std::vector<double>,
                   std::vector<double>> NonSymmDiagReturn_t;

typedef std::tuple<std::vector<double>,std::vector<double>,std::vector<double>>
    SVDReturn_t;

namespace pulsar{
namespace math{


/** \brief A C++-ified call to BLAS's symmetric matrix diagonalizer
 * 
 *  This function assumes that you are using some container like
 *  an std::vector for your flattened matrix.
 * 
 *  \param[in,out] Matrix The nxn matrix we are diagonalizing.  If EVecs==true
 *                        the rows of this matrix upon return will be the 
 *                        eigenvectors.  Must support the data() function
 *  \param[in,out] EVals  A PRE-ALLOCATED container of length n, must support
 *                        the size() function as well as the data() function
 *  \param[in] Stride     The number of elements in a row.  0 is the default
 *                        and a special value that means the stride is the
 *                        same as the length of the row
 *  \param[in] EVecs      True means you want the eigenvectors, False means you
 *                        don't
 *  \param[in] Upper      In theory you only have to store the upper or the
 *                        lower half of the matrix since it's symmetric.  By
 *                        default we assume you store the upper half, but if
 *                        you have the lower set this to false.  Note if you
 *                        have both this flag is irrelevant.
 */

template<typename Mat_t,typename EVal_t>
void SymmetricDiagonalize(Mat_t& Matrix,EVal_t& EVals,
         int Stride=0,bool EVecs=true,bool Upper=true){
    int n=EVals.size(),info,lwork=-1;
    if(Stride==0)Stride=n;
    double wkopt;
    //It's Fortran ultimately so upper is actually lower...stupid Fortran
    char v=EVecs?'V':'N',u=Upper?'L':'U';
    dsyev(&v,&u,&n,Matrix.data(),&Stride,EVals.data(),&wkopt,&lwork,&info);
    lwork=(int)wkopt;
    double* work=new double[lwork];
    dsyev(&v,&u,&n,&Matrix[0],&Stride,&EVals[0],work,&lwork,&info);
    delete [] work;
}

/** \brief Returns the eigenvalues and optionally the eigenvectors (left and/or
 *         right) of a non-symmetric real square matrix
 *   
 *  \param[in] Matrix The matrix to diagonalize
 *  \param[in] n The dimension of the matrix
 *  \param[in] Stride The stride of the matrix, 0 means there's no gap between
 *                    rows
 *  \param[in] RVecs True if you want the right eigenvectors
 *  \param[in] LVecs True if you want the left eigenvectors
 * 
 *  \return A tuple whose first element is the eigenvalues (which in general
 *  are complex), the second element is the right eigenvectors, and the third
 *  is the left eigenvectors.  If you requested that a certain eigenvector 
 *  not be computed than you should not access that element.
 *
 */
template<typename Mat_t>
NonSymmDiagReturn_t NonSymmetricDiagonalize(Mat_t Matrix,int n,
                             int Stride=0,
                              bool RVecs=true,bool LVecs=true){
    int info,lwork=-1;
    if(Stride==0)Stride=n;
    double wkopt;
    char rv=RVecs?'V':'N',lv=LVecs?'V':'N';
    std::vector<double> evalReal(n),evalImag(n);
    std::vector<double> vl(LVecs?n*Stride:1),vr(RVecs?n*Stride:1);
    dgeev(&rv,&lv,&n,Matrix.data(),&Stride,
          evalReal.data(),evalImag.data(),vl.data(),&n,vr.data(),&n,&wkopt,&lwork,&info);

    lwork=(int)wkopt;
    double* work=new double[lwork];
    dgeev(&rv,&lv,&n,Matrix.data(),&Stride,
            evalReal.data(),evalImag.data(),vl.data(),&n,vr.data(),&n,work,&lwork,&info);
    if(info!=0){
        throw PulsarException("There was a problem diagonalizing"
                "your matrix.","info code:",info);
    }
    delete [] work;
    std::vector<std::complex<double>> Evals(n);
    for(size_t i=0;i<(size_t)n;++i)
        Evals[i]=std::complex<double>(evalReal[i],evalImag[i]);
    return std::make_tuple(Evals,vr,vl);
    
}

template<typename Mat_t>
SVDReturn_t SVD(Mat_t M, size_t m, size_t n,size_t LDA=0,size_t LDU=0,
        size_t LDVT=0){
    LDA=std::max(LDA,m);
    LDU=std::max(LDU,m);
    LDVT=std::max(LDVT,n);
    int info,lwork=-1;
    double wkopt;
    std::vector<double> SVals(n),LVecs(LDU*m),RVecs(LDVT*n);
    dgesvd("S","S",(int*)(&m),(int*)(&n),M.data(),(int*)(&LDA),SVals.data(),
           LVecs.data(),(int*)(&LDU),RVecs.data(),(int*)(&LDVT),&wkopt,&lwork,
            &info);
    lwork=(int)wkopt;
    std::vector<double> work(lwork);
    dgesvd("S","S",(int*)(&m),(int*)(&n),M.data(),(int*)(&LDA),SVals.data(),
           LVecs.data(),(int*)(&LDU),RVecs.data(),(int*)(&LDVT),work.data(),
           &lwork,&info);
    if(info > 0)
        throw PulsarException("SVD failed to converge");
    
    return std::make_tuple(LVecs,SVals,RVecs);    
}


///Returns the cross product of two vectors
///\todo write in terms of wedge product
template<typename T1,typename T2>
std::array<double,3> Cross(const T1& LHS,const T2& RHS){
    return{LHS[1]*RHS[2]-LHS[2]*RHS[1],
           LHS[2]*RHS[0]-LHS[0]*RHS[2],
           LHS[0]*RHS[1]-LHS[1]*RHS[0]
    };
}
        

///Returns the dot product of two vectors
template<typename T1,typename T2>
double Dot(const T1& LHS,const T2& RHS){
    double sum=0.0;
    for(size_t i=0;i<LHS.size();++i)sum+=LHS[i]*RHS[i];
    return sum;
}

template<typename T1>
std::array<double,3> Normalize(const T1& Vec){
    double mag=sqrt(Dot(Vec,Vec));
    return {Vec[0]/mag,Vec[1]/mag,Vec[2]/mag};
}

}}//End namespace
#endif /* BLAS_HPP */

