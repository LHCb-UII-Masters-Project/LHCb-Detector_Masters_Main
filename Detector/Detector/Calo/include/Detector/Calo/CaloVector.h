/*****************************************************************************\
* (c) Copyright 2000-2018 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "Detector/Calo/CaloCellID.h"
#include "boost/call_traits.hpp"
#include <vector>

/**
 *  A dedicated vector for Calo information accessed by CellID.
 *  Allows normal sequencial access, as this inherits form std::vector.
 *  But it allows at the same time direct access by CellID.
 *  Based on the original CaloCollection implementation by Vanya Belyaev
 *  This is a vector, plus some functions for direct access, and of course
 *  methods to load the internal vector.
 *
 *  @author Olivier Callot
 */

template <class CONTENT, class INDEX_ = LHCb::Detector::Calo::CellID>
class CaloVector final : private std::vector<CONTENT> {
public:
  /// type fo content
  using Content = CONTENT;
  /// own type
  using ContVector             = std::vector<CONTENT>;
  using Vector                 = ContVector;
  using iterator               = typename Vector::iterator;
  using const_iterator         = typename Vector::const_iterator;
  using reverse_iterator       = typename Vector::reverse_iterator;
  using const_reverse_iterator = typename Vector::const_reverse_iterator;
  using const_reference        = typename Vector::const_reference;
  using Indices                = std::vector<int>;

  /// type for the argument
  using INDEX = typename boost::call_traits<const INDEX_>::param_type;

  /** constructor:
   *  One specifies a default value,
   *  returned when the index is invalid or non existent.
   *  @param   def   default value to be returned for wrong/non-existing index
   */
  CaloVector( Content def = Content() ) ///< default value
      : ContVector( 0 ), m_cc_def( std::move( def ) ), m_index( 0 ) {}

  /// copying is not allowed!
  CaloVector( const CaloVector& ) = delete;
  /// assignment  is not allowed!
  CaloVector& operator=( const CaloVector& ) = delete;

  CaloVector( CaloVector&& ) = default;
  CaloVector& operator=( CaloVector&& ) = default;

  /// following lines from std::vector interface

  /// sequential access to content container (const version!)
  const_iterator begin() const { return Vector::begin(); }
  /// sequential access to content container (const version!)
  const_iterator end() const { return Vector::end(); }
  /// sequential access to content container (const version!)
  const_reverse_iterator rbegin() const { return Vector::rbegin(); }
  /// sequential access to content container (const version!)
  const_reverse_iterator rend() const { return Vector::rend(); }

  decltype( auto ) front() const { return Vector::front(); }
  decltype( auto ) back() const { return Vector::back(); }

  /// size of content container
  typename Vector::size_type size() const { return Vector::size(); }
  /// clear the container
  void clear() {
    Vector::clear();
    m_index.clear();
  }

  /** returns the position in the vector, -1 if not present
   *  @param  id index
   */
  int index( INDEX id ) const {
    const unsigned int indx = id.index();
    return ( indx < m_index.size() ) ? m_index[indx] : -1;
  }

  /** Access to the content itself using LHCb::Detector::Calo::CellID as index.
   *  Check the boundaries of the index array.
   *  @param id index
   */
  inline Content& operator[]( INDEX id ) {
    const int indx = index( id );
    return ( 0 > indx ) ? def() : *( begin() + indx );
  }

  /** Access to the content itself using LHCb::Detector::Calo::CellID as index.
   *  (const version)
   *  Check the boundaries of the index array.
   *  @param id index
   */
  inline const Content& operator[]( INDEX id ) const {
    const int indx = index( id );
    return ( 0 > indx ) ? def() : *( begin() + indx );
  }

  /** Access to the content itself using LHCb::Detector::Calo::CellID as index.
   *  Check the boundaries of the index array.
   *  @param id index
   */
  inline Content& operator()( INDEX id ) { return ( *this )[id]; }

  /** Access to the content itself using LHCb::Detector::Calo::CellID as index.
   *  (const version)
   *  Check the boundaries of the index array.
   *  @param id index
   */
  inline const Content& operator()( INDEX id ) const { return ( *this )[id]; }

  /** Add one entry. vector is extended, index vector is updated.
   *  @param content   value to be added into CaloVector
   *  @param id        index
   *  @return          status code
   */
  void addEntry( const Content& content, INDEX id ) {
    const unsigned int indx = id.index();
    if ( m_index.size() <= indx ) {
      const Indices::size_type  num = indx + 1 - m_index.size();
      const Indices::value_type val = -1;
      m_index.insert( m_index.end(), num, val );
    }
    m_index[indx] = this->size();
    this->push_back( content );
  }

  /// set size for intermediate container
  void setSize( const unsigned int Num ) { m_index.reserve( Num ); }

  /// extended "reserve"
  void reserve( const unsigned int Num ) {
    Vector::reserve( Num );
    m_index.reserve( Num );
  }

  /// sequential access to content container
  iterator begin() { return Vector::begin(); }
  /// sequential access to content container
  iterator end() { return Vector::end(); }
  /// sequential access to content container
  reverse_iterator rbegin() { return Vector::rbegin(); }
  /// sequential access to content container
  reverse_iterator rend() { return Vector::rend(); }

  /// access to container (useful in python for iteration)
  inline const Content& at( const size_t i ) const { return Vector::at( i ); }

private:
  /// get default value
  inline Content& def() { return m_cc_def; }
  /// get default value (const version)
  inline const Content& def() const { return m_cc_def; }

  Content m_cc_def; ///< default value
  Indices m_index;  ///< array of indices in the vector
};
