/*****************************************************************************\
* (c) Copyright 2000-2021 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "boost/container/static_vector.hpp"

namespace LHCb::Detector::Calo {

  struct Tell1Param final {

    // To be dropped once everything is implemented
    struct NotImplemented : std::exception {
      const char* what() const noexcept override { return "not implemented"; };
    };

    struct FECards {
      using Container = boost::container::static_vector<unsigned char, 24>;
      Container m_data;

      auto size() const { return m_data.size(); }
      auto operator[]( size_t i ) const { return m_data[i]; }
      auto begin() const { return m_data.begin(); }
      auto end() const { return m_data.end(); }
      void push_back( unsigned char c ) { m_data.push_back( c ); }
      auto erase( Container::const_iterator i ) { return m_data.erase( i ); }
      // friend inline std::ostream& operator<<( std::ostream& os, FECards const& v ) {
      //  return GaudiUtils::details::ostream_joiner(
      //  	      os << '[', v.m_data, ", ",
      //  	      []( std::ostream& s, unsigned char c ) -> decltype( auto ) { return s << static_cast<int>( c ); }
      //  )
      //    << ']';
      //}
      friend inline std::ostream& operator<<( std::ostream&, FECards const& ) { throw NotImplemented(); }
    };

    Tell1Param( int num ) : m_number( num ) {}

    // getters
    int     number() const { return m_number; }
    FECards feCards() const { return m_feCards; }
    bool    readPin() const { return m_isPin; }

    // setters
    void addFeCard( int num ) {
      assert( num < 256 );
      m_feCards.push_back( static_cast<unsigned char>( num ) );
    }
    void setReadPin( bool pin ) { m_isPin = pin; }

  private:
    int     m_number{};
    FECards m_feCards;
    bool    m_isPin = false;
  };

} // namespace LHCb::Detector::Calo
