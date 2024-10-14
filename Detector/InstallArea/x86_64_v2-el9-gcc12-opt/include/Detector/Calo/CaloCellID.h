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

#include "CaloCellCode.h"

#include <map>
#include <sstream>
#include <string>

namespace LHCb::Detector::Calo {

  /**
   * Simple class to represent cell identifier for calorimeter devices * * * The
   * class represents the unique cell identifier for all calorimeter devices * *
   *
   * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *
   */

  static const unsigned int CLID_CellID = 2008;

  class CellID final {
  public:
    /// Default/empty constructor
    constexpr CellID() = default;

    static const unsigned int& classID() { return CLID_CellID; }

    /// Explicit constructor from Calo, Area, Row and Column
    constexpr CellID( CellCode::Index Calo, unsigned int Area, unsigned int Row, unsigned int Column );

    /// Explicit constructor from 32 bit representation
    explicit constexpr CellID( const unsigned int& all ) : m_all{all} {}

    /// Decoding function to extract index
    [[nodiscard]] constexpr unsigned int index() const {
      return ( m_all & CellCode::MaskIndex ) >> CellCode::ShiftIndex;
    }

    /// Decoding function to extract calorimeter identifier
    [[nodiscard]] constexpr CellCode::Index calo() const {
      return static_cast<CellCode::Index>( ( m_all & CellCode::MaskCalo ) >> CellCode::ShiftCalo );
    }

    /// Decoding function to extract calorimeter area identifier
    [[nodiscard]] constexpr unsigned int area() const { return ( m_all & CellCode::MaskArea ) >> CellCode::ShiftArea; }

    /// Decoding function to extract calorimeter row identifier
    [[nodiscard]] constexpr unsigned int row() const { return ( m_all & CellCode::MaskRow ) >> CellCode::ShiftRow; }

    /// Decoding function to extract calorimeter column identifier
    [[nodiscard]] constexpr unsigned int col() const { return ( m_all & CellCode::MaskCol ) >> CellCode::ShiftCol; }

    /// Comparison operator using 'index()' representation
    friend constexpr bool operator<( const CellID& lhs, const CellID& rhs ) { return lhs.index() < rhs.index(); }

    /// Equality operator using 'contents()' representation
    friend constexpr bool operator==( const CellID& lhs, const CellID& rhs ) { return lhs.m_all == rhs.m_all; }
    //
    /// InEquality operator in terms of not equality
    friend constexpr bool operator!=( const CellID& lhs, const CellID& rhs ) { return !( lhs == rhs ); }

    /// conversion to bool
    explicit operator bool() const { return *this != CellID{}; };

    /// return true if the cellID corresponds to a PIN-diode readout channel
    [[nodiscard]] bool isPin() const { return CellCode::isPinArea( calo(), area() ); }

    /// printOut to standard STD/STL output streams
    std::ostream& fillStream( std::ostream& os ) const {
      return os << "("
                << "'" << CellCode::caloName( calo() ) << "',"
                << "'" << CellCode::caloArea( calo(), area() ) << "'," << std::setw( 2 ) << row() << ","
                << std::setw( 2 ) << col() << ")";
    }

    /// Conversion to string representation
    explicit operator std::string() const {
      std::stringstream sID;
      sID << *this;
      return sID.str();
    }

    // as string method
    auto asString() const { return static_cast<std::string>( *this ); }

    /// Update Calorimeter identifier
    CellID& setCalo( CellCode::Index Calo ) {
      m_all = CellCode::setField( m_all, Calo, CellCode::ShiftCalo, CellCode::MaskCalo );
      return *this;
    }

    /// hash-function for python
    [[nodiscard]] constexpr std::size_t __hash__() const { return hash(); }

    /// hash-function
    [[nodiscard]] constexpr std::size_t hash() const { return index(); }

    /// get the name of Calorimeter
    [[nodiscard]] const std::string& caloName() const { return CellCode::caloName( calo() ); }

    /// get the name of Calorimeteter detector area
    [[nodiscard]] const std::string& areaName() const { return CellCode::caloArea( calo(), area() ); }

    /// Retrieve const  all significant bits representation of CellID (32bits)
    [[nodiscard]] constexpr const unsigned int& all() const { return m_all; }

    /// Update  all significant bits representation of CellID (32bits)
    void setAll( const unsigned int& value ) { m_all = value; }

    friend std::ostream& operator<<( std::ostream& str, const CellID& obj ) { return obj.fillStream( str ); }

  private:
    unsigned int m_all{0}; ///< all significant bits representation of CellID (32bits)

  }; // class CellID

  using Map = std::map<CellID, double>;

  constexpr CellID::CellID( CellCode::Index Calo, unsigned int Area, unsigned int Row, unsigned int Column ) {
    m_all = CellCode::setField( m_all, Calo, CellCode::ShiftCalo, CellCode::MaskCalo );
    m_all = CellCode::setField( m_all, Area, CellCode::ShiftArea, CellCode::MaskArea );
    m_all = CellCode::setField( m_all, Row, CellCode::ShiftRow, CellCode::MaskRow );
    m_all = CellCode::setField( m_all, Column, CellCode::ShiftCol, CellCode::MaskCol );
  }

  namespace DenseIndex::details {

    // cellIDs
    //   sub         row         columns
    // Ecal,Outer:  [ 6,21] : [0,15][16,47][48,63]     ( 16 ) * ( 16 | 32 | 16 )       [ 4 x 16 ]
    // Ecal,Outer:  [22,41] : [0,15]       [48,63]     ( 20 ) * ( 16 |    | 16 )
    // Ecal,Outer:  [42,57] : [0,15][16,47][48,63]     ( 16 ) * ( 16 | 32 | 16 )
    //
    // Ecal,Middle: [12,19] : [0,15][16,47][48,63]     (  8 ) * ( 16 | 32 | 16 )       [ 8 x 16 ]
    // Ecal,Middle: [20,43] : [0,15]       [48,63]     ( 24 ) * ( 16 |    | 16 )
    // Ecal,Middle: [44,51] : [0,15][16,47][48,63]     (  8 ) * ( 16 | 32 | 16 )
    //
    // Ecal,Inner:  [14,25] : [8,23][24,39][40,55]     ( 12 ) * ( 16 | 16 | 16 )      [ 12 x 16 ]
    // Ecal,Inner:  [26,37] : [8,23]       [40,55]     ( 12 ) * ( 16 |    | 16 )
    // Ecal,Inner:  [38,49] : [8,23][24,39][40,55]     ( 12 ) * ( 16 | 16 | 16 )
    //
    // Hcal,Outer   [ 3, 8] : [0, 7][ 8,23][24,31]     (  6 ) * (  8 | 16 |  8 )       [ 6 x 8 ]
    // Hcal,Outer   [ 9,22] : [0, 7]       [24,31]     ( 14 ) * (  8 |    |  8 )
    // Hcal,Outer   [23,28] : [0, 7][ 8,23][23,31]     (  6 ) * (  8 | 16 |  8 )
    //
    // Hcal,Inner   [ 2,13] : [0,13][14,17][18,31]     ( 12 ) * ( 14 |  4 | 14 )       [ 4 x 2 ]
    // Hcal,Inner   [14,17] : [0,13]       [18,31]     (  4 ) * ( 14 |    | 14 )
    // Hcal,Inner   [18,29] : [0,13][14,17][18,31]     ( 12 ) * ( 14 |  4 | 14 )
    //

    using Area = CellCode::CaloArea;

    template <typename C>
    constexpr auto nCells() {
      if ( C::area == Area::PinArea ) {
        return C::rows[0] * C::cols[0] + C::rows[2] * C::cols[2];
      } else {
        return C::rows[0] * ( C::cols[0] + C::cols[1] + C::cols[2] ) + C::rows[1] * ( C::cols[0] + C::cols[2] ) +
               C::rows[2] * ( C::cols[0] + C::cols[1] + C::cols[2] );
      }
    }
    template <typename C>
    constexpr auto offsetAfter() {
      return C::global_offset + nCells<C>();
    }

    template <CellCode::Index, Area>
    struct Constants;

    template <>
    struct Constants<CellCode::Index::EcalCalo, Area::Outer> {
      static constexpr auto global_offset = 0;
      static constexpr auto id0           = std::array{6, 0};
      static constexpr auto cols          = std::array{16, 32, 16};
      static constexpr auto rows          = std::array{16, 20, 16};
      static constexpr auto area          = Area::Outer;
    };
    template <>
    struct Constants<CellCode::Index::EcalCalo, Area::Middle> {
      static constexpr auto global_offset = offsetAfter<Constants<CellCode::Index::EcalCalo, Area::Outer>>();
      static constexpr auto id0           = std::array{12, 0};
      static constexpr auto cols          = std::array{16, 32, 16};
      static constexpr auto rows          = std::array{8, 24, 8};
      static constexpr auto area          = Area::Middle;
    };
    template <>
    struct Constants<CellCode::Index::EcalCalo, Area::Inner> {
      static constexpr auto global_offset = offsetAfter<Constants<CellCode::Index::EcalCalo, Area::Middle>>();
      static constexpr auto id0           = std::array{14, 8};
      static constexpr auto cols          = std::array{16, 16, 16};
      static constexpr auto rows          = std::array{12, 12, 12};
      static constexpr auto area          = Area::Inner;
    };
    template <>
    struct Constants<CellCode::Index::EcalCalo, Area::PinArea> {
      static constexpr auto global_offset = offsetAfter<Constants<CellCode::Index::EcalCalo, Area::Inner>>();
      static constexpr auto id0           = std::array{0, 0};
      static constexpr auto cols          = std::array{16, 32, 16};
      static constexpr auto rows          = std::array{4, 0, 4};
      static constexpr auto area          = Area::PinArea;
    };
    template <>
    struct Constants<CellCode::Index::HcalCalo, Area::Outer> {
      static constexpr auto global_offset = offsetAfter<Constants<CellCode::Index::EcalCalo, Area::PinArea>>();
      static constexpr auto id0           = std::array{3, 0};
      static constexpr auto cols          = std::array{8, 16, 8};
      static constexpr auto rows          = std::array{6, 14, 6};
      static constexpr auto area          = Area::Outer;
    };

    template <>
    struct Constants<CellCode::Index::HcalCalo, Area::Inner> {
      static constexpr auto global_offset = offsetAfter<Constants<CellCode::Index::HcalCalo, Area::Outer>>();
      static constexpr auto id0           = std::array{2, 0};
      static constexpr auto cols          = std::array{14, 4, 14};
      static constexpr auto rows          = std::array{12, 4, 12};
      static constexpr auto area          = Area::Inner;
    };
    template <>
    struct Constants<CellCode::Index::HcalCalo, Area::PinArea> {
      static constexpr auto global_offset = offsetAfter<Constants<CellCode::Index::HcalCalo, Area::Inner>>();
      static constexpr auto id0           = std::array{0, 0};
      static constexpr auto cols          = std::array{16, 32, 16};
      static constexpr auto rows          = std::array{4, 0, 4};
      static constexpr auto area          = Area::PinArea;
    };

    template <CellCode::Index calo, Area area>
    constexpr inline bool valid( int row, int col ) {
      using C = Constants<calo, area>;
      constexpr auto bound =
          std::array{std::array{C::id0[0], C::id0[0] + C::rows[0], C::id0[0] + C::rows[0] + C::rows[1],
                                C::id0[0] + C::rows[0] + C::rows[1] + C::rows[2]},
                     std::array{C::id0[1], C::id0[1] + C::cols[0], C::id0[1] + C::cols[0] + C::cols[1],
                                C::id0[1] + C::cols[0] + C::cols[1] + C::cols[2]}};
      int r = ( row < bound[0][0] ? -1 : row < bound[0][1] ? 0 : row < bound[0][2] ? 1 : row < bound[0][3] ? 2 : 3 );
      int c = ( col < bound[1][0] ? -1 : col < bound[1][1] ? 0 : col < bound[1][2] ? 1 : col < bound[1][3] ? 2 : 3 );
      return r != -1 && r != 3 && c != -1 && c != 3 && !( r == 1 && c == 1 );
    }

    template <CellCode::Index calo, Area area>
    constexpr inline int index( int row, int col ) {
      using C = Constants<calo, area>;

      constexpr auto strides = std::array{C::cols[0] + C::cols[1] + C::cols[2], C::cols[0] + C::cols[2],
                                          C::cols[0] + C::cols[1] + C::cols[2]};
      constexpr auto offsets =
          std::array{C::global_offset - C::id0[0] * strides[0] - C::id0[1],
                     C::global_offset - C::id0[0] * strides[1] - C::id0[1] + C::rows[0] * C::cols[1],
                     C::global_offset - C::id0[0] * strides[2] - C::id0[1] - C::rows[1] * C::cols[1]};
      constexpr auto bound = std::array{std::array{C::id0[0] + C::rows[0], C::id0[0] + C::rows[0] + C::rows[1]},
                                        std::array{C::id0[1] + C::cols[0], C::id0[1] + C::cols[0] + C::cols[1]}};

      if ( area == Area::PinArea ) {
        int off = ( row >= C::rows[0] && col >= ( C::cols[0] + C::cols[1] ) ) ? ( C::cols[0] + C::cols[1] ) : 0;
        return C::global_offset + row * C::cols[0] + col - off;
      } else {
        int r = ( ( row < bound[0][0] ) ? 0 : ( ( row < bound[0][1] ) ? 1 : 2 ) );
        return offsets[r] + row * strides[r] + col - ( r == 1 && col >= bound[1][1] ) * C::cols[1];
      }
    }

    template <CellCode::Index calo, Area area>
    constexpr inline CellID cellID( int id ) {
      using C = std::conditional_t<calo == CellCode::Index::HcalCalo && area == Area::Middle,
                                   Constants<calo, Area::Inner>, Constants<calo, area>>;
      id -= C::global_offset;
      assert( id < nCells<C>() );

      if ( area == Area::PinArea ) {
        if ( id < C::cols[0] + C::cols[1] + C::cols[2] ) {
          int c = id % C::cols[0];
          int r = id / C::cols[0];
          return CellID{calo, static_cast<unsigned int>( area ), static_cast<unsigned int>( r ),
                        static_cast<unsigned int>( c )};
        } else {
          int c = id % C::cols[0] + C::cols[0] + C::cols[1];
          int r = id / C::cols[0];
          return CellID{calo, static_cast<unsigned int>( area ), static_cast<unsigned int>( r ),
                        static_cast<unsigned int>( c )};
        }
      } else {
        constexpr auto strides = std::array{C::cols[0] + C::cols[1] + C::cols[2], C::cols[0] + C::cols[2],
                                            C::cols[0] + C::cols[1] + C::cols[2]};
        constexpr auto rows =
            std::array{C::rows[0] * ( C::cols[0] + C::cols[1] + C::cols[2] ), C::rows[1] * ( C::cols[0] + C::cols[2] ),
                       C::rows[2] * ( C::cols[0] + C::cols[1] + C::cols[2] )};
        if ( id < rows[0] ) {
          int r = C::id0[0] + id / strides[0];
          int c = C::id0[1] + id % strides[0];
          return CellID{calo, static_cast<unsigned int>( area ), static_cast<unsigned int>( r ),
                        static_cast<unsigned int>( c )};
        }
        id -= rows[0];
        if ( id < rows[1] ) {
          int r = C::id0[0] + C::rows[0] + id / strides[1];
          int c = C::id0[1] + id % strides[1];
          if ( c >= C::id0[1] + C::cols[0] ) c += C::cols[1];
          return CellID{calo, static_cast<unsigned int>( area ), static_cast<unsigned int>( r ),
                        static_cast<unsigned int>( c )};
        }
        id -= rows[1];
        assert( id < rows[2] );
        int r = C::id0[0] + C::rows[0] + C::rows[1] + id / strides[2];
        int c = C::id0[1] + id % strides[2];
        return CellID{calo, static_cast<unsigned int>( area ), static_cast<unsigned int>( r ),
                      static_cast<unsigned int>( c )};
      }
    }

    constexpr CellID toCellID( int id ) {
      if ( id < 0 ) return CellID{};
      if ( id < Constants<CellCode::Index::EcalCalo, Area::Middle>::global_offset ) {
        return cellID<CellCode::Index::EcalCalo, Area::Outer>( id );
      }
      if ( id < Constants<CellCode::Index::EcalCalo, Area::Inner>::global_offset ) {
        return cellID<CellCode::Index::EcalCalo, Area::Middle>( id );
      }
      if ( id < Constants<CellCode::Index::EcalCalo, Area::PinArea>::global_offset ) {
        return cellID<CellCode::Index::EcalCalo, Area::Inner>( id );
      }
      if ( id < Constants<CellCode::Index::HcalCalo, Area::Outer>::global_offset ) {
        return cellID<CellCode::Index::EcalCalo, Area::PinArea>( id );
      }
      if ( id < Constants<CellCode::Index::HcalCalo, Area::Inner>::global_offset ) {
        return cellID<CellCode::Index::HcalCalo, Area::Outer>( id );
      }
      if ( id < offsetAfter<Constants<CellCode::Index::HcalCalo, Area::Inner>>() ) {
        return cellID<CellCode::Index::HcalCalo, Area::Middle>( id );
      }
      if ( id < offsetAfter<Constants<CellCode::Index::HcalCalo, Area::PinArea>>() ) {
        return cellID<CellCode::Index::HcalCalo, Area::PinArea>( id );
      }
      return CellID{};
    }
  } // namespace DenseIndex::details

  constexpr inline int denseIndex( CellID id ) {
    using namespace DenseIndex::details;
    switch ( id.calo() ) {
    case CellCode::Index::EcalCalo:
      switch ( static_cast<Area>( id.area() ) ) {
      case Area::Outer:
        return index<CellCode::Index::EcalCalo, Area::Outer>( id.row(), id.col() );
      case Area::Middle:
        return index<CellCode::Index::EcalCalo, Area::Middle>( id.row(), id.col() );
      case Area::Inner:
        return index<CellCode::Index::EcalCalo, Area::Inner>( id.row(), id.col() );
      case Area::PinArea:
        return index<CellCode::Index::EcalCalo, Area::PinArea>( id.row(), id.col() );
      default:
        return -1;
      }
    case CellCode::Index::HcalCalo:
      switch ( static_cast<Area>( id.area() ) ) {
      case Area::Outer:
        return index<CellCode::Index::HcalCalo, Area::Outer>( id.row(), id.col() );
      case Area::Middle: // empty on purpose...
      case Area::Inner:
        return index<CellCode::Index::HcalCalo, Area::Inner>( id.row(), id.col() );
      case Area::PinArea:
        return index<CellCode::Index::HcalCalo, Area::PinArea>( id.row(), id.col() );
      default:
        return -1;
      }

    default:
      return -1;
    }
  }

  constexpr auto isValid( CellID id ) {
    using namespace DenseIndex::details;
    switch ( id.calo() ) {
    case CellCode::Index::EcalCalo:
      switch ( static_cast<Area>( id.area() ) ) {
      case Area::Outer:
        return valid<CellCode::Index::EcalCalo, Area::Outer>( id.row(), id.col() );
      case Area::Middle:
        return valid<CellCode::Index::EcalCalo, Area::Middle>( id.row(), id.col() );
      case Area::Inner:
        return valid<CellCode::Index::EcalCalo, Area::Inner>( id.row(), id.col() );
      default:
        return false;
      }
    case CellCode::Index::HcalCalo:
      switch ( static_cast<Area>( id.area() ) ) {
      case Area::Outer:
        return valid<CellCode::Index::HcalCalo, Area::Outer>( id.row(), id.col() );
      case Area::Middle: // empty on purpose...
      case Area::Inner:
        return valid<CellCode::Index::HcalCalo, Area::Inner>( id.row(), id.col() );
      default:
        return false;
      }
    default:
      return false;
    }
  }

  class Index {
    int m_idx;

  public:
    constexpr Index( CellID id ) : m_idx{denseIndex( id )} {}
    constexpr explicit operator bool() const { return m_idx >= 0; }
    constexpr          operator unsigned int() const {
      if ( m_idx < 0 ) throw std::out_of_range{"bad Calo::CellID"};
      return static_cast<unsigned int>( m_idx );
    }
    constexpr            operator CellID() const { return DenseIndex::details::toCellID( m_idx ); }
    static constexpr int max() {
      return DenseIndex::details::offsetAfter<
          DenseIndex::details::Constants<CellCode::Index::HcalCalo, DenseIndex::details::Area::PinArea>>();
    }
  };

} // namespace LHCb::Detector::Calo
