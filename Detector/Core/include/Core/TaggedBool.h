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

// Copyright (C) 2016 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt)
//
// copied from https://github.com/akrzemi1/explicit/tree/master/include/ak_toolkit
// last commit: bd572fe05a700cc6766b3d09f8916c9975ccbb45

namespace LHCb::Detector {
  namespace tagged_bool_ns { // artificial namespace to prevent ADL lookups in namespace Gaudi

    template <typename Tag>
    class tagged_bool {
      bool value;
      template <typename /*OtherTag*/>
      friend class tagged_bool;

    public:
      constexpr explicit tagged_bool( bool v ) : value{v} {}

      constexpr explicit tagged_bool( int )    = delete;
      constexpr explicit tagged_bool( double ) = delete;
      constexpr explicit tagged_bool( void* )  = delete;

      template <typename OtherTag>
      constexpr explicit tagged_bool( tagged_bool<OtherTag> b ) : value{b.value} {}

      constexpr explicit    operator bool() const { return value; }
      constexpr tagged_bool operator!() const { return tagged_bool{!value}; }

      friend constexpr bool operator==( tagged_bool l, tagged_bool r ) { return l.value == r.value; }
      friend constexpr bool operator!=( tagged_bool l, tagged_bool r ) { return l.value != r.value; }
    };
  } // namespace tagged_bool_ns

  using tagged_bool_ns::tagged_bool; // with this tagged_bool is in namespace xplicit but with disabled ADL
} // namespace LHCb::Detector
