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

#include <algorithm>
#include <limits>
#include <type_traits>

namespace LHCb {

  /**
   * Generic method to compare two floating point values
   * ADL is used to use standard library by default but switch to custom
   * methods for abs and max in case of custom types, e.g. vector types of SIMDWrapper
   * Note that is should not be used to compare to 0, for this essentiallyZero
   * should be used
   *
   * CAVEAT: please note that `essentiallyEqual` is not transitive, i.e. even if
   *         `essentiallyEqual(a,b)` and `essentiallyEqual(b,c)` evaluate to `true`
   *         that does _not_ guarantee that `essentiallyEqual(a,c)` will evaluate to `true`.
   *         Hence do _NOT_ rely on `essentiallyEqual` in a comparison used (implicitly)
   *         for sorting -- when sorting, write comparisons which avoid checking for
   *         equality in the first place.
   */
  template <typename T, typename = std::enable_if_t<std::numeric_limits<T>::is_specialized>>
  constexpr auto essentiallyEqual( T const a, T const b ) {
    using std::abs, std::max;
    if constexpr ( std::numeric_limits<T>::is_exact ) {
      return a == b; // TODO: should we perhaps warn that in this case, `essentiallyEqual should not be used?
    } else {
      return abs( a - b ) <= max( abs( a ), abs( b ) ) * std::numeric_limits<T>::epsilon();
    }
  }

  /**
   * Generic method to compare a floating point value to 0
   * ADL is used to use standard library by default but switch to custom
   * methods for abs in case of custom types, e.g. vector types of SIMDWrapper
   */
  template <typename T, typename = std::enable_if_t<std::numeric_limits<T>::is_specialized>>
  constexpr auto essentiallyZero( T const a ) {
    using std::abs;
    if constexpr ( std::numeric_limits<T>::is_exact ) {
      return a == 0; // TODO: should we perhaps warn to not use essentiallyZero in this case?
    } else {
      return abs( a ) <= std::numeric_limits<T>::min();
    }
  }

} // namespace LHCb
