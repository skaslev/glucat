#ifndef _GLUCAT_GENERATION_IMP_H
#define _GLUCAT_GENERATION_IMP_H
/***************************************************************************
	  GluCat : Generic library of universal Clifford algebra templates
    generation_imp.h : Implement functions for generation of the matrix representation
                             -------------------
    begin                : Wed Jan 23 2002
    copyright            : (C) 2002 by Paul C. Leopardi
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

namespace glucat { namespace gen
{
  // References for algorithms:
  // [M]: Scott Meyers, "Effective C++" Second Edition, Addison-Wesley, 1998.
  // [P]: Ian R. Porteous, "Clifford algebras and the classical groups", Cambridge UP, 1995.
  // [L]: Pertti Lounesto, "Clifford algebras and spinors", Cambridge UP, 1997.

  /// Single instance of generator table
  // Reference: [M] Item 47
  template< class Matrix_T >
  generator_table<Matrix_T>&
  generator_table<Matrix_T>::
  generator()
  { static generator_table<Matrix_T> g; return g;}

  /// Pointer to generators for a specific signature
  // Reference: [P] Table 15.27, p 133
  template< class Matrix_T >
  const Matrix_T*
  generator_table<Matrix_T>::
  operator() (const index_t p, const index_t q)
  {
    const index_t bott = pos_mod(p-q, 8);
    switch(bott)
    {
    case 0:
    case 2:
      // Construct generators
      return &(gen_vector(p, q)[q]);
      break;
    default:
      // Select generators from the vector for a larger frame
      const index_t super_p = p + std::max(offset_to_super[bott],0);
      const index_t super_q = q - std::min(offset_to_super[bott],0);
      return &(gen_vector(super_p, super_q)[super_q]);
      break;
    }
  }

  /// Construct a vector of generators for a specific signature
  template< class Matrix_T >
  const std::vector<Matrix_T>&
  generator_table<Matrix_T>::
  gen_vector(const index_t p, const index_t q)
  {
    const index_t card = p + q;
    const index_t bias = p - q;
    const index_t bott = pos_mod(bias, 8);
    const signature_t sig(p, q);
    if (this->find(sig) == this->end())
      switch(bott)
      {
      case 0:
        if (bias < 0)
          // Construct generators for p,q given generators for p+4,q-4
          gen_from_pp4_qm4(gen_vector(p+4, q-4), sig);
        else if (bias > 0)
          // Construct generators for p,q given generators for p-4,q+4
          gen_from_pm4_qp4(gen_vector(p-4, q+4), sig);
        else if (card == 0)
        { // Base case. Save a generator vector containing one matrix, size 1.
          std::vector<Matrix_T> result(1);
          result[0].resize(1, 1);
          this->insert(make_pair(sig, result));
        }
        else
          // Construct generators for p,q given generators for p-1,q-1
          gen_from_pm1_qm1(gen_vector(p-1, q-1), sig);
        break;
      case 2:
        if (bias < 2)
          // Construct generators for p,q given generators for p+4,q-4
          gen_from_pp4_qm4(gen_vector(p+4, q-4), sig);
        else if (bias > 2)
          // Construct generators for p,q given generators for p-4,q+4
          gen_from_pm4_qp4(gen_vector(p-4, q+4), sig);
        else
          // Construct generators for p,q given generators for q+1,p-1
          gen_from_qp1_pm1(gen_vector(q+1, p-1), sig);
        break;
      default:
        break;
      }
    return (*this)[sig];
  }

  /// Construct generators for p,q given generators for p-1,q-1
  // Reference: [P] Proposition 15.17, p 131
  template< class Matrix_T >
  void
  generator_table<Matrix_T>::
  gen_from_pm1_qm1(const std::vector<Matrix_T>& old, const signature_t sig)
  {
    Matrix_T pos(2,2);
    pos(0,1) =     1;
    pos(1,0) = 1;

    Matrix_T dup(2,2);
    dup(0,0) = 1;
    dup(1,1) =    -1;

    Matrix_T neg(2,2);
    neg(0,1) =    -1;
    neg(1,0) = 1;

    const int new_size = old.size() + 2;
    const int old_dim = old[0].size1();
    const int new_dim = old_dim*2;
    const Matrix_T& eye = matrix::unit<Matrix_T>(old_dim);

    std::vector<Matrix_T> result(new_size);
    for (int 
        k = 0; 
        k != new_size; 
        ++k)
      result[k].resize(new_dim, new_dim);
    
    result[0] = matrix::kron(neg, eye);
    for (int 
        k = 1; 
        k != new_size-1; 
        ++k)
      result[k] = matrix::kron(dup, old[k-1]);
    result[new_size-1] = matrix::kron(pos, eye);

    // Save the resulting generator array.
    this->insert(make_pair(sig, result));
  }

  /// Construct generators for p,q given generators for p-4,q+4
  // Reference: [L] 16.4 Periodicity of 8, p216
  template< class Matrix_T >
  void
  generator_table<Matrix_T>::
  gen_from_pm4_qp4(const std::vector<Matrix_T>& old, const signature_t sig)
  {
    const int dim = old[0].size1();
    const int old_size = old.size();
    Matrix_T h(dim, dim);
    h = old[0];
    for (int 
        k = 1; 
        k != 4; 
        ++k)
      h = matrix::mono_prod(old[k], h);

    std::vector<Matrix_T> result(old_size);
    for (int 
        k = 0; 
        k != 4; 
        ++k)
    {
      result[k+old_size-4].resize( dim, dim );
      result[k+old_size-4] = matrix::mono_prod(old[k], h);
    }
    for (int 
        k = 4; 
        k != old_size; 
        ++k)
    {
      result[k-4].resize(dim, dim);
      result[k-4] = old[k];
    }
    // Save the resulting generator array.
    this->insert(make_pair(sig, result));
  }

  /// Construct generators for p,q given generators for p+4,q-4
  // Reference: [L] 16.4 Periodicity of 8, p216
  template< class Matrix_T >
  void
  generator_table<Matrix_T>::
  gen_from_pp4_qm4(const std::vector<Matrix_T>& old, const signature_t sig)
  {
    const int dim = old[0].size1();
    const int old_size = old.size();
    Matrix_T h(dim, dim);
    h = old[old_size-1];
    for (int 
        k = 1; 
        k != 4; 
        ++k)
      h = matrix::mono_prod(old[old_size-1-k], h);

    std::vector<Matrix_T> result(old_size);
    for (int 
        k = 0; 
        k != 4; 
        ++k)
    {
      result[k].resize(dim, dim);
      result[k] = matrix::mono_prod(old[k+old_size-4], h);
    }
    for (int 
        k = 4; 
        k != old_size; 
        ++k)
    {
      result[k].resize(dim, dim);
      result[k] = old[k-4];
    }
    // Save the resulting generator array.
    this->insert(make_pair(sig, result));
  }

  /// Construct generators for p,q given generators for q+1,p-1
  // Reference: [P] Proposition 15.20, p 131
  template< class Matrix_T >
  void
  generator_table<Matrix_T>::
  gen_from_qp1_pm1(const std::vector<Matrix_T>& old, const signature_t sig)
  {
    const int dim = old[0].size1();
    const int old_size = old.size();
    const Matrix_T& a = old[old_size-1];
    std::vector<Matrix_T> result(old_size);
    int m = 0;
    for (int 
        k = old_size-1; 
        k != 0; 
        --k, ++m)
    {
      result[m].resize(dim, dim);
      result[m] = matrix::mono_prod(old[k-1], a);
    }
    result[old_size-1].resize(dim, dim);
    result[old_size-1] = a;

    // Save the resulting generator array.
    this->insert(make_pair(sig, result));
  }

} }
#endif  // _GLUCAT_GENERATION_IMP_H
