/*****************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <Detector/Muon/ReadoutCond.h>

#include "ReadoutCondYaml.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <yaml-cpp/yaml.h>

using LHCb::Detector::Muon::ReadoutCond;

ReadoutCond::ReadoutCond( const YAML::Node& node ) {
  params = node.as<std::vector<Parameters>>();

  auto calc_cum_prob = []( const auto& cluster ) {
    std::vector<double> cum_prob(
        std::max_element( cluster.begin(), cluster.end(), []( auto a, auto b ) { return a.size < b.size; } )->size );
    // fill CumProbX in position i with value p from cluster
    for ( const auto& c : cluster ) cum_prob[c.size - 1] = c.prob;
    // actually compute the partial propability
    std::partial_sum( cum_prob.begin(), cum_prob.end(), cum_prob.begin() );
    // alright paranoia here, normalise the cumlative sum
    std::transform( cum_prob.begin(), cum_prob.end(), cum_prob.begin(),
                    [&cum_prob]( auto& i ) { return i / cum_prob.back(); } );
    return cum_prob;
  };

  // Set CumProbX and CumProbY from cluserX and clusterY
  for ( auto& param : params ) {
    assert( param.ClusterX.size() && param.ClusterY.size() );
    param.CumProbX = calc_cum_prob( param.ClusterX );
    param.CumProbY = calc_cum_prob( param.ClusterY );
  }
}

namespace {
  // helper to avoid FPEs
  double safe_exponential( double arg ) {
    if ( ( arg ) < -100. ) return 0.0;
    return std::exp( arg );
  }
} // namespace

int ReadoutCond::singleGapCluster( int xy, double randomNumber, double xpos, int i ) const {
  if ( randomNumber > 1. || randomNumber < 0. ) { return 1; }

  const auto& param   = params[i];
  const auto& cumProb = xy ? param.CumProbY : param.CumProbX;

  // now should have a cumlative sum in cumProb
  int icsize;
  if ( cumProb.size() >= 3 && randomNumber > cumProb[1] ) {
    // do _not_ do the correction for the pad edge effect
    // (cluster size at least 3 anyway)
    icsize = 1;
    while ( cumProb[icsize] < randomNumber ) { ++icsize; }
  } else {
    // must correct for pad edge effect
    auto corrected_cumProb = cumProb;

    const auto padEdgeSize  = xy ? param.PadEdgeSizeY : param.PadEdgeSizeX;
    const auto padEdgeSigma = xy ? param.PadEdgeSigmaY : param.PadEdgeSigmaX;

    if ( padEdgeSigma > 0. && padEdgeSize < 0. ) {
      // tested that there _is_ an effect
      const double edgeEffect =
          padEdgeSize * safe_exponential( -0.5 * ( xpos * xpos ) / ( padEdgeSigma * padEdgeSigma ) );
      corrected_cumProb[0] += edgeEffect;
      corrected_cumProb[1] -= edgeEffect;
    }
    icsize = 0;
    while ( corrected_cumProb[icsize] < randomNumber ) { ++icsize; }
  }
  return icsize + 1;
}
