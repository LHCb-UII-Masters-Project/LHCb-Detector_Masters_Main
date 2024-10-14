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

#include "Core/ConditionsRepository.h"
#include "Core/Keys.h"

#include "DD4hep/ConditionDerived.h"
#include "DD4hep/DetElement.h"

#include <memory>
#include <unordered_map>

namespace LHCb::Detector {

  /// Selector to determine which conditions are mandatory for event processing
  /**
   *
   *  \author  Markus Frank
   *  \date    2018-11-30
   *  \version  1.0
   */
  struct RequestSelector {
    const ConditionsRepository& existing;
    ConditionsRepository&       required;

    /// Initializing constructor
    RequestSelector( const ConditionsRepository& e, ConditionsRepository& r ) : existing( e ), required( r ) {}
    /// Inhibit assignment
    RequestSelector& operator=( const RequestSelector& copy ) = delete;
    /// Selector call for single condition
    void select( dd4hep::Condition::key_type key );
    /// Selector call for condition groups
    void select( std::set<dd4hep::Condition::key_type>& needed, bool clr );
    /// Selector call for condition groups
    void select( std::vector<dd4hep::Condition::key_type>& needed, bool clr );
  };

  /// Base class for detector element conditions callbacks
  /**
   *
   *  \author  Markus Frank
   *  \date    2018-11-30
   *  \version  1.0
   */
  struct DeConditionCall : dd4hep::cond::ConditionUpdateCall {
    /* Do not use shared pointers here: There is a circular dependency between the
     * repository and the callback and the net result is that nothing at all is deleted.
     */
    /// Reference to the inventory of subdetector conditions
    ConditionsRepository* repository = 0;

    /// Default constructor
    DeConditionCall() = delete;
    /// Copy constructor
    DeConditionCall( const DeConditionCall& copy ) = delete;
    /// Initializing constructor
    DeConditionCall( std::shared_ptr<ConditionsRepository>& rep );
    /// Default destructor
    virtual ~DeConditionCall();
    /// Access the conditions derivation given the condiitons key using the repository
    const dd4hep::cond::ConditionDependency* derivationInfo( dd4hep::Condition::key_type key ) const;
    /// Access the detector element given the condiitons key using the repository
    // dd4hep::DetElement detectorElement( dd4hep::Condition::key_type key ) const;
  };

} // End namespace LHCb::Detector
