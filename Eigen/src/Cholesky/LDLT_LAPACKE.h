/*
 Copyright (c) 2025, Swift Navigation. All rights reversed.
*/

#ifndef EIGEN_LDLT_LAPACKE_H
#define EIGEN_LDLT_LAPACKE_H

#include <iostream>

namespace Eigen { 

namespace internal {

  template <typename TranspositionsType>
void transpositions_from_ipiv(Index n, bool uplo_upper, const Array<lapack_int, Dynamic, 1> &ipiv, TranspositionsType &transpositions) {
  eigen_assert((ipiv > 0).all() && "Matrix was indefinite!");
  // std::cout << "ipiv(" << ipiv.size() << "): " << ipiv.transpose() << std::endl;
  using IdxArr = Array<Index, Dynamic, 1>;
  IdxArr permutations{IdxArr::LinSpaced(n, 0, n - 1)};

  if (uplo_upper) {
    for (Index k = n - 1; k >= 0; --k) {
      Index p = ipiv[k] - 1;
      if (p != k) {
        std::swap(permutations[k], permutations[p]);
      }
    }
  } else {
    for (Index k = 0; k < n; ++k) {
      Index p = ipiv[k] - 1;
      if (p != k) {
        std::swap(permutations[k], permutations[p]);
      }
    }
  }

  transpositions.resize(n);
  transpositions.setIdentity();

  IdxArr inverse(n);
  for (Index i = 0; i < n; ++i) {
    inverse[permutations[i]] = i;
  }

  for (Index i = 0; i < n; ++i) {
    const Index k = inverse[i];
    transpositions.indices()(i) = k;
    if (k != i) {
      Index vi = permutations[i];
      Index vk = permutations[k];
      std::swap(permutations[i], permutations[k]);
      inverse[vi] = k;
      inverse[vk] = vi;
    }
  }

  // std::cout << "transpositions.indices() (" << transpositions.rows()
  //           << "): " << transpositions.indices().transpose() << std::endl;
}
  
template<typename Scalar> struct lapacke_ldlt;

#define EIGEN_LAPACKE_LDLT(EIGTYPE, BLASTYPE, LAPACKE_PREFIX) \
template<> struct lapacke_ldlt<EIGTYPE> \
{ \
  template<typename MatrixType, typename TranspositionsType> \
  static inline ComputationInfo sytrf(MatrixType& m, char uplo, TranspositionsType &transpositions, SignMatrix &sign) \
  { \
    lapack_int matrix_order; \
    lapack_int size, lda, info, StorageOrder; \
    EIGTYPE* a; \
    eigen_assert(m.rows()==m.cols()); \
    /* Set up parameters for ?sytrf */ \
    size = convert_index<lapack_int>(m.rows()); \
    StorageOrder = MatrixType::Flags&RowMajorBit?RowMajor:ColMajor; \
    matrix_order = StorageOrder==RowMajor ? LAPACK_ROW_MAJOR : LAPACK_COL_MAJOR; \
    a = &(m.coeffRef(0,0)); \
  lda = convert_index<lapack_int>(m.outerStride()); \
  Array<lapack_int, Dynamic, 1> ipiv(m.rows()); \
\
info = LAPACKE_##LAPACKE_PREFIX##sytrf( matrix_order, uplo, size, (BLASTYPE*)a, lda, ipiv.data() ); \
  if (info < 0 || (ipiv <= 0).any()) { \
  sign = Indefinite; \
  std::cerr << "sytrf: info = " << info << " (ipiv <= 0).any() = " << (ipiv <= 0).any() << std::endl;\
  std::cerr << "sytrf: returning Invalid" << std::endl; \
  return InvalidInput; \
  } \
  if (info > 0) { \
  std::cerr << "sytrf: element " << info << " was exactly 0; input was PSD" << std::endl; \
  } \
  if (numext::real(m.coeff(0, 0)) > static_cast<typename MatrixType::RealScalar>(0)) { \
  sign = PositiveSemiDef; \
  } else if (numext::real(m.coeff(0, 0)) < static_cast<typename MatrixType::RealScalar>(0)) { \
  sign = NegativeSemiDef; \
  } \
  transpositions_from_ipiv(m.rows(), uplo == '*', ipiv, transpositions); \
  return Success; \
  } \
}; \
template<> struct ldlt_inplace<EIGTYPE, Lower> \
{ \
  template<typename MatrixType, typename TranspositionsType, typename Workspace> \
  static ComputationInfo unblocked(MatrixType& m, TranspositionsType &transpositions, Workspace &, SignMatrix &sign) \
  { \
    return lapacke_ldlt<EIGTYPE>::sytrf(m, 'L', transpositions, sign); \
  } \
  template<typename MatrixType, typename TranspositionsType, typename Workspace> \
  static ComputationInfo blocked(MatrixType& m, TranspositionsType &transpositions, Workspace &, SignMatrix &sign) \
  { \
    return lapacke_ldlt<EIGTYPE>::sytrf(m, 'L', transpositions, sign); \
  } \
  template<typename MatrixType, typename TranspositionType, typename Workspace, typename WType> \
  static bool update(MatrixType& mat, const TranspositionType& transpositions, Workspace& tmp, const WType& w, const typename MatrixType::RealScalar& sigma=1) \
  { tmp = transpositions * w; return Eigen::internal::ldlt_rank_update_lower(mat, tmp, sigma); } \
}; \
template<> struct ldlt_inplace<EIGTYPE, Upper> \
{ \
  template<typename MatrixType, typename TranspositionsType, typename Workspace> \
  static ComputationInfo unblocked(MatrixType& m, TranspositionsType &transpositions, Workspace &, SignMatrix &sign) \
  { \
  std::cout << "upper unblocked" << std::endl; \
    return lapacke_ldlt<EIGTYPE>::sytrf(m, 'U', transpositions, sign); \
  } \
  template<typename MatrixType, typename TranspositionType, typename Workspace, typename WType> \
  static bool update(MatrixType& mat, const TranspositionType& transpositions, Workspace& tmp, const WType& w, const typename MatrixType::RealScalar& sigma=1) \
  { \
    Transpose<MatrixType> matt(mat); \
  return ldlt_inplace<EIGTYPE, Lower>::update(matt, transpositions, tmp, w.conjugate(), sigma); \
  } \
};

EIGEN_LAPACKE_LDLT(double, double, d)
EIGEN_LAPACKE_LDLT(float, float, s)
EIGEN_LAPACKE_LDLT(dcomplex, lapack_complex_double, z)
EIGEN_LAPACKE_LDLT(scomplex, lapack_complex_float, c)

} // end namespace internal

} // end namespace Eigen

#endif // EIGEN_LDLT_LAPACKE_H
