#ifndef _GLUCAT_MATRIX_IMP_H
#define _GLUCAT_MATRIX_IMP_H
/***************************************************************************
    GluCat : Generic library of universal Clifford algebra templates
    matrix_imp.h : Implement common matrix functions
                             -------------------
    begin                : Sun 2001-12-09
    copyright            : (C) 2001 by Paul C. Leopardi
    email                : leopardi@bigpond.net.au
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *   See http://www.fsf.org/copyleft/lesser.html for details               *
 ***************************************************************************
 This library is based on a prototype written by Arvind Raja and was
 licensed under the LGPL with permission of the author. See Arvind Raja,
 "Object-oriented implementations of Clifford algebras in C++: a prototype",
 in Ablamowicz, Lounesto and Parra (eds.)
 "Clifford algebras with numeric and symbolic computations", Birkhauser, 1996.
 ***************************************************************************
     See also Arvind Raja's original header comments in glucat.h
 ***************************************************************************/

namespace glucat
{
  /// Kronecker tensor product of matrices - as per Matlab kron
  template< class Matrix_T >
  void
  kron(const Matrix_T& x, const Matrix_T& y, Matrix_T& z)
  {
    const int rx = x.nrows();
    const int cx = x.ncols();
    const int ry = y.nrows();
    const int cy = y.ncols();
    z = Matrix_T(rx*ry, cx*cy);
    mtl::set(z, 0);
    for (      typename Matrix_T::const_iterator i = x.begin(); i != x.end(); ++i)
      for (    typename Matrix_T::Row::const_iterator j = (*i).begin(); j != (*i).end(); ++j)
        for (  typename Matrix_T::const_iterator k = y.begin(); k != y.end(); ++k)
          for (typename Matrix_T::Row::const_iterator l = (*k).begin(); l != (*k).end(); ++l)
              z(j.row()*ry + l.row(), j.column()*cy + l.column()) = (*j) * (*l);
  }

  /// Unit matrix - as per Matlab eye
  template< class Matrix_T >
  void
  unit(int n, Matrix_T& result)
  {
    mtl::set(result, 0);
    mtl::set_diagonal(result, 1);
  }

  /// Does a matrix have only one non-zero per row (or column) ?
  // Note: This should return false for dense matrices,
  // but this is OK for the current use of this function,
  // which is to help determine when a compressed multiply should have a dense result
  template< class Matrix_T >
  bool
  is_singlet(const Matrix_T& x)
  {
    for (typename Matrix_T::const_iterator i = x.begin(); i != x.end(); ++i)
      if ((*i).size() != 1)
        return false;
    return true;
  }

  /// Does a matrix have only one non-zero per row and column ?
  // Note: This should return false for dense matrices,
  // but this is OK for the current use of this function,
  // which is to help determine when a compressed multiply should have a dense result
  template< class Matrix_T >
  bool
  is_perm_shaped(const Matrix_T& x)
  {
    if (is_singlet(x))
    {
      Matrix_T xt(x.ncols(), x.nrows());
      mtl::transpose(x, xt);
      return is_singlet(xt);
    }
    return false;
  }

  /// Generate the next generation of generators, given the previous generation
  template< class Matrix_T >
  void
  gen(const vector< Matrix_T >& old, vector< vector< Matrix_T > >& generators)
  {
    Matrix_T z(2,2);
    mtl::set(z, 0);
    z(0,0) =   1;
    z(1,1) =      -1;

    Matrix_T pos(2,2);
    mtl::set(pos, 0);
    pos(0,1) =     1;
    pos(1,0) = 1;

    Matrix_T neg(2,2);
    mtl::set(neg, 0);
    neg(0,1) =    -1;
    neg(1,0) = 1;

    const int new_size = old.size() + 2;
    const int old_mat_shape = old[0].nrows();
		Matrix_T eye(old_mat_shape, old_mat_shape);
    unit<Matrix_T>(old_mat_shape, eye);

    vector< Matrix_T > result(new_size);
    kron(neg , eye, result[0]);
    kron(pos , eye, result[new_size-1]);
    for (int k = 1; k != new_size-1; ++k)
      kron(z , old[k-1], result[k]);
    // Save the resulting generator array.
    generators.push_back(result);
  }

  /// Generate a specific generation of generators, given the whole family tree
  template< class Matrix_T >
  const vector< Matrix_T >&
  gengen(const int n, vector< vector< Matrix_T > >& generators)
  {
    if (n >= int(generators.size()))
      if (n == 0)
      { // Base case. Generator array will contain one matrix, size 1.
        // Save this generator array.
        generators.resize(1);
        generators[0].resize(1);
        generators[0][0] = Matrix_T(1,1);
        mtl::set(generators[0][0], 0);
      }
      else
        // Use recursion to define generator array in terms of smaller generator array
        gen(gengen<Matrix_T>(n-1, generators), generators);
    return generators[n];
  }

  /// Inner product: sum(x(i,j)*y(i,j))/x.nrows()
  template< class Matrix_T, class Scalar_T >
  Scalar_T
  inner(const Matrix_T& x, const Matrix_T& y)
  {
    Scalar_T result = 0;
    for (typename Matrix_T::const_iterator i = x.begin(); i != x.end(); ++i)
      for (typename Matrix_T::Row::const_iterator j = (*i).begin(); j != (*i).end(); ++j)
        result += (*j) * y(j.row(),j.column());
    return result / x.nrows();
  }

}
#endif  // _GLUCAT_MATRIX_IMP_H
