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
#include "Core/DeConditionCall.h"
#include "DD4hep/Printout.h"

/// Selector call for single condition
void LHCb::Detector::RequestSelector::select( dd4hep::Condition::key_type key ) {
  // There are more derived conditions than normal numbers. Check these always first.
  auto rd = required.derived().find( key );
  if ( rd != required.derived().end() ) { return; }
  auto rc = required.conditions().find( key );
  if ( rc != required.conditions().end() ) { return; }
  auto ed = existing.derived().find( key );
  if ( ed != existing.derived().end() ) {
    dd4hep::cond::ConditionDependency* dep = ( *ed ).second;
    required.addDependency( dep );
    for ( const auto& e : dep->dependencies ) select( e.hash );
    return;
  }
  auto ec = existing.conditions().find( key );
  if ( ec != existing.conditions().end() ) {
    required.addLocationInfo( ec->first, ec->second );
    return;
  }
  // Serious error ..... We should never end up here
  dd4hep::except( "DeRequestSelector", "No conditions recipe found for conditions key: %s",
                  dd4hep::ConditionKey( key ).toString().c_str() );
}

/// Selector call for condition groups
void LHCb::Detector::RequestSelector::select( std::set<dd4hep::Condition::key_type>& needed, bool clr ) {
  if ( clr ) required.clear();
  for ( const auto key : needed ) { select( key ); }
}

/// Selector call for condition groups
void LHCb::Detector::RequestSelector::select( std::vector<dd4hep::Condition::key_type>& needed, bool clr ) {
  if ( clr ) required.clear();
  for ( const auto key : needed ) { select( key ); }
}

/// Initializing constructor
LHCb::Detector::DeConditionCall::DeConditionCall( std::shared_ptr<ConditionsRepository>& rep )
    : repository( rep.get() ) {}

/// Default destructor
LHCb::Detector::DeConditionCall::~DeConditionCall() {}

/// Access the conditions derivation given the condiitons key using the repository
const dd4hep::cond::ConditionDependency*
LHCb::Detector::DeConditionCall::derivationInfo( dd4hep::Condition::key_type key ) const {
  const auto itr_info = repository->derived().find( key );
  if ( itr_info == repository->derived().end() ) {
    dd4hep::except( "Conditions", "++ No detector element was present of conditions key:%016X", key );
  }
  return ( *itr_info ).second;
}

/// Access the detector element given the condiitons key using the repository
// dd4hep::DetElement LHCb::Detector::DeConditionCall::detectorElement( dd4hep::Condition::key_type key ) const {
//   return derivationInfo( key )->detector;

// }
