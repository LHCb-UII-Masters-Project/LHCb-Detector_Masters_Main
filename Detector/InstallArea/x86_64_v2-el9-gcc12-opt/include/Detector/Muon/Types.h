/*****************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once
#include <cassert>

namespace LHCb::Detector::Muon {

  class StationId;
  class SideId;
  class RegionId;
  class ChamberId;
  class GapId;
  class QuadrantId;

  class StationN;
  class SideN;
  class RegionN;
  class ChamberN;
  class GapN;
  class QuadrantN;

  namespace details {
    template <typename ID, typename N, unsigned int MAX>
    class BaseId {
    public:
      explicit constexpr BaseId( unsigned int id = 0 ) : m_id{id} { assert( id <= MAX ); }
      constexpr BaseId( const ID& id ) { m_id = id.value(); }
      constexpr BaseId& operator=( const ID& id ) {
        m_id = id.value();
        return *this;
      }
      constexpr BaseId( const N& n ) { m_id = n.value() - N::min().value(); }
      constexpr BaseId& operator=( const N& n ) {
        m_id = n.value() - N::min().value();
        return *this;
      }

      // constexpr operator unsigned int() const { return m_id; }
      static constexpr ID    min() { return ID{0}; }
      static constexpr ID    max() { return ID{MAX}; }
      constexpr unsigned int value() const { return m_id; }

    private:
      unsigned int m_id;
    };
    template <typename N, typename ID, unsigned int MIN, unsigned int MAX>
    class BaseNumber {
    public:
      explicit constexpr BaseNumber( unsigned int n = 0 ) : m_n{n} { assert( n >= MIN && n <= MAX ); }
      constexpr BaseNumber( const N& n ) { m_n = n.value(); }
      constexpr BaseNumber& operator=( const N& n ) {
        m_n = n.value();
        return *this;
      }
      BaseNumber( const ID& id ) { m_n = id.value() + MIN; }
      BaseNumber& operator=( const ID& id ) {
        m_n = id.value() + MIN;
        return *this;
      }

      // constexpr operator unsigned int() const { return m_id; }
      static constexpr ID    min() { return ID{MIN}; }
      static constexpr ID    max() { return ID{MAX}; }
      constexpr unsigned int value() const { return m_n; }

    private:
      unsigned int m_n;
    };
  } // namespace details

  class StationId : public details::BaseId<StationId, StationN, 3> {
    using BaseId::BaseId;
  };
  class StationN : public details::BaseNumber<StationN, StationId, 2, 5> {
    using BaseNumber::BaseNumber;
  };
  class SideId : public details::BaseId<SideId, SideN, 1> {
    using BaseId::BaseId;
  };
  class SideN : public details::BaseNumber<SideN, SideId, 0, 1> {
    using BaseNumber::BaseNumber;
  };
  class RegionId : public details::BaseId<RegionId, RegionN, 3> {
    using BaseId::BaseId;
  };
  class RegionN : public details::BaseNumber<RegionN, RegionId, 1, 4> {
    using BaseNumber::BaseNumber;
  };
  class ChamberId : public details::BaseId<ChamberId, ChamberN, 255> {
    using BaseId::BaseId;
  };
  class ChamberN : public details::BaseNumber<ChamberN, ChamberId, 1, 256> {
    using BaseNumber::BaseNumber;
  };
  class GapId : public details::BaseId<GapId, GapN, 3> {
    using BaseId::BaseId;
  };
  class GapN : public details::BaseNumber<GapN, GapId, 1, 4> {
    using BaseNumber::BaseNumber;
  };
  class QuadrantId : public details::BaseId<QuadrantId, QuadrantN, 3> {
    using BaseId::BaseId;
  };
  class QuadrantN : public details::BaseNumber<QuadrantN, QuadrantId, 0, 3> {
    using BaseNumber::BaseNumber;
  };
} // namespace LHCb::Detector::Muon
