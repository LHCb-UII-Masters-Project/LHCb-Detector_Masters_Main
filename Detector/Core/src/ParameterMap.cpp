/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

// Framework include files
#include "Core/ParameterMap.h"
#include "DD4hep/Handle.h"

namespace LHCb::Detector {

  namespace {
    static ParameterMap::Parameter s_empty;
  }

  /// Type dependent accessor to a named parameter
  template <typename T>
  T ParameterMap::Parameter::get() const {
    if ( this == &s_empty ) return 0;
    return dd4hep::_toType<T>( value );
  }

  /// Add/update a parameter value
  bool ParameterMap::set( const std::string& nam, const std::string& val, const std::string& type ) {
    const auto i = m_params.find( nam );
    if ( i == m_params.end() ) { return m_params.insert( make_pair( nam, Parameter( val, type ) ) ).second; }
    ( *i ).second.value = val;
    ( *i ).second.type  = type;
    return false;
  }

  /// Access single parameter
  const ParameterMap::Parameter& ParameterMap::parameter( const std::string& nam, bool throw_if ) const {
    const auto i = m_params.find( nam );
    if ( i != m_params.end() )
      return ( *i ).second;
    else if ( throw_if ) {
      throw std::runtime_error( "ParameterMap: Attempt to access non-existng parameter: " + nam );
    }
    return s_empty;
  }

  /// Type dependent accessor to a named parameter
  template <typename T>
  T ParameterMap::param( const std::string& nam, bool throw_if ) const {
    return parameter( nam, throw_if ).template get<T>();
  }

#define INST( x )                                                                                                      \
  template x ParameterMap::Parameter::get() const;                                                                     \
  template x ParameterMap::param( const std::string& nam, bool throw_if_not_present ) const

  INST( bool );
  INST( int );
  INST( short );
  INST( long );
  INST( double );
  INST( float );
  INST( std::string );
} // namespace LHCb::Detector
