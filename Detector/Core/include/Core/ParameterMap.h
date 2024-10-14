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
#pragma once

#include <map>
#include <string>

namespace LHCb::Detector {

  /// Generic detector element parameter map to specialize detector elements
  /**
   *  Concurrentcy notice:
   *  Except during filling, which is performed by the framework code,
   *  instances of this class are assumed to the read-only!
   *  Thread safety hence is no issue.
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  class ParameterMap final {
  public:
    /// Defintiion of a single parameter
    /**
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    class Parameter final {
    public:
      std::string value;
      std::string type;

    public:
      Parameter()                        = default;
      Parameter( Parameter&& copy )      = default;
      Parameter( const Parameter& copy ) = default;
      Parameter( const std::string& v, const std::string& t ) : value( v ), type( t ) {}
      Parameter& operator=( const Parameter& copy ) = default;
      bool       operator==( const Parameter& copy ) const { return value == copy.value && type == copy.type; }
      /// Type dependent accessor to a named parameter
      template <typename T>
      T get() const;
    };

    using Parameters = std::map<std::string, Parameter>;

  protected:
    /// The parameter map
    Parameters m_params;

    /// Access single parameter
    const Parameter& _param( const std::string& nam ) const;

  public:
    /// Defautl constructor
    ParameterMap() = default;
    /// Copy constructor
    ParameterMap( const ParameterMap& copy ) = default;
    /// Default destructor
    ~ParameterMap() = default;
    /// Assignment opererator
    ParameterMap& operator=( const ParameterMap& copy ) = default;
    /// Check the parameter existence
    bool exists( const std::string& nam ) const { return m_params.find( nam ) != m_params.end(); }
    /// Add/update a parameter value
    bool set( const std::string& nam, const std::string& val, const std::string& type );
    /// Access parameters set
    const Parameters& params() const { return m_params; }
    /// Access single parameter
    const Parameter& parameter( const std::string& nam, bool throw_if_not_present = true ) const;
    /// Type dependent accessor to a named parameter
    template <typename T>
    T param( const std::string& nam, bool throw_if_not_present = true ) const;
  };
} // End namespace LHCb::Detector
