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
#include "Core/DeAlignmentCall.h"
#include "Core/Keys.h"

#include "DD4hep/AlignmentsCalculator.h"

#include "TTimeStamp.h"

/// Interface to client Callback in order to update the condition
dd4hep::Condition LHCb::Detector::DeAlignmentCall::operator()( const dd4hep::ConditionKey& /* key */,
                                                               dd4hep::cond::ConditionUpdateContext& ctxt ) {
  using Calculator = dd4hep::align::AlignmentsCalculator;
  using Deltas     = Calculator::OrderedDeltas;
  Calculator                calc;
  dd4hep::ConditionsHashMap slice;
  TTimeStamp                start;
  dd4hep::Condition         cond      = dd4hep::Condition( Keys::alignmentsComputedKeyName, "Calculator" );
  Deltas&                   deltas    = cond.bind<Deltas>();
  size_t                    num_delta = calc.extract_deltas( ctxt, deltas );
  Calculator::Result        ares      = calc.compute( deltas, slice );
  if ( ctxt.registerMapping( *ctxt.iov, slice.data ) == slice.data.size() ) {
    TTimeStamp stop;
    dd4hep::printout( dd4hep::INFO, "Align", "Alignments:(D:%ld,C:%ld,M:%ld,*:%ld) Effective IOV:%s  [%7.5f seconds]",
                      num_delta, ares.computed, ares.missing, ares.multiply, ctxt.iov->str().c_str(),
                      stop.AsDouble() - start.AsDouble() );
    return cond;
  }
  dd4hep::except( "DeAlignmentCall", "Failed to register all alignment conditions!" );
  return nullptr;
}
