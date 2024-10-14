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

// Framework include files
#include "DD4hep/AlignmentsCalculator.h"
#include "DD4hep/ConditionDerived.h"
#include "DD4hep/Conditions.h"

namespace LHCb::Detector {

  /// Callback, which triggers the alignment computation once the delta-parameters are loaded
  /**
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  class DeAlignmentCall : public dd4hep::cond::ConditionUpdateCall {
  public:
    dd4hep::DetElement                                  top;
    dd4hep::align::AlignmentsCalculator::ExtractContext extract_context;
    /// Initializing constructor
    DeAlignmentCall( dd4hep::DetElement t ) : top( t ) {}
    /// Default destructor
    virtual ~DeAlignmentCall() = default;
    /// Interface to client Callback in order to update the condition
    virtual dd4hep::Condition operator()( const dd4hep::ConditionKey& /* key */,
                                          dd4hep::cond::ConditionUpdateContext& ctxt ) override final;
  };

} // End namespace LHCb::Detector
