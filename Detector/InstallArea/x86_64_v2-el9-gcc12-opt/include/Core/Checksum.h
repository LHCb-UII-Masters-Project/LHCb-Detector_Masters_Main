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

#pragma once

#include <Core/GeometryTools.h>
#include <Core/crc.h>

#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>

#include <TClass.h>
#include <TGeoBBox.h>
#include <TGeoBoolNode.h>
#include <TGeoCompositeShape.h>
#include <TGeoCone.h>
#include <TGeoNode.h>
#include <TGeoPcon.h>
#include <TGeoShape.h>
#include <TGeoSphere.h>
#include <TGeoTorus.h>
#include <TGeoTrd1.h>
#include <TGeoTrd2.h>
#include <TGeoTube.h>

using lhcb::geometrytools::discretize_angle;
using lhcb::geometrytools::discretize_length;
using lhcb::geometrytools::discretize_rotmatrix;
using lhcb::geometrytools::discretize_vec;

namespace lhcb::Detector::checksum {

  /*
   * Accumulators that can be used to gather information about the geometry
   */
  class AccumulatorCrc {

  public:
    using ret = long;
    void accumulate( long value ) { m_accumulator = lhcb::Detector::crc::checksum_long( m_accumulator, value ); }
    void accumulate( std::string value ) {
      auto str = value.c_str();
      for ( size_t i = 0; i < value.length(); i++ ) {
        m_accumulator = lhcb::Detector::crc::checksum_long( m_accumulator, (long)str[i] );
      }
    }
    long result() { return m_accumulator; }

  private:
    long m_accumulator = 0;
  };

  class AccumulatorStr {

  public:
    using ret = std::string;

    AccumulatorStr() {}

    void accumulate( long value ) { m_accumulator << value << "."; }
    void accumulate( std::string value ) { m_accumulator << value << "."; }

    std::string result() { return m_accumulator.str(); }

  private:
    std::stringstream m_accumulator;
  };

  /*
   * Checksum methos for the various shapes used in the LHCb description
   */

  template <typename T>
  void checksum_shape( T& accumulator, const TGeoShape* shape );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoCompositeShape* shape );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoBBox* shape );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoShape* shape );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoTubeSeg* sh );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoShapeAssembly* sh );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoConeSeg* sh );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoPcon* sh );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoTrd1* sh );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoTrd2* sh );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoTrap* sh );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoCtub* sh );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoSphere* sh );
  template <typename T>
  void checksum_shape( T& accumulator, TGeoTorus* sh );

  template <typename T>
  void checksum_matrix( T& accumulator, TGeoMatrix* m ) {
    const Double_t* tr  = m->GetTranslation();
    const Double_t* rot = m->GetRotationMatrix();
    for ( int i = 0; i < 9; i++ ) { accumulator.accumulate( discretize_rotmatrix( rot[i] ) ); }
    for ( int i = 0; i < 3; i++ ) { accumulator.accumulate( discretize_length( tr[i] ) ); }
  }

  template <typename T>
  void checksum_cstring( T& accumulator, const char* str ) {
    accumulator.accumulate( str );
  }

  template <typename T>
  void check_material( T& accumulator, TGeoMaterial* sh ) {
    accumulator.accumulate( discretize_length( sh->GetRadLen() ) );
    accumulator.accumulate( discretize_length( sh->GetIntLen() ) );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoCompositeShape* sh ) {

    const TGeoBoolNode*        boolean      = sh->GetBoolNode();
    TGeoBoolNode::EGeoBoolType oper         = boolean->GetBooleanOperator();
    TGeoMatrix*                left_matrix  = boolean->GetLeftMatrix();
    TGeoMatrix*                right_matrix = boolean->GetRightMatrix();
    TGeoShape*                 left_solid   = boolean->GetLeftShape();
    TGeoShape*                 right_solid  = boolean->GetRightShape();

    accumulator.accumulate( (long)oper );
    checksum_shape( accumulator, left_solid );
    checksum_shape( accumulator, right_solid );
    checksum_matrix( accumulator, left_matrix );
    checksum_matrix( accumulator, right_matrix );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoBBox* shape ) {
    accumulator.accumulate( discretize_length( shape->GetDX() ) );
    accumulator.accumulate( discretize_length( shape->GetDY() ) );
    accumulator.accumulate( discretize_length( shape->GetDZ() ) );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoTubeSeg* sh ) {

    std::array<long, 6> vals{discretize_length( sh->GetRmin() ), discretize_length( sh->GetRmax() ),
                             discretize_length( sh->GetDz() ), discretize_angle( sh->GetPhi1() ),
                             discretize_angle( sh->GetPhi2() )};

    std::for_each( vals.begin(), vals.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoShapeAssembly* sh ) {
    std::array<long, 3> vals{discretize_length( sh->GetDX() ), discretize_length( sh->GetDY() ),
                             discretize_length( sh->GetDZ() )};
    std::for_each( vals.begin(), vals.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoConeSeg* sh ) {
    std::array<long, 7> vals{discretize_length( sh->GetDz() ),    discretize_length( sh->GetRmin1() ),
                             discretize_length( sh->GetRmax1() ), discretize_length( sh->GetRmin2() ),
                             discretize_length( sh->GetRmax2() ), discretize_angle( sh->GetPhi1() ),
                             discretize_angle( sh->GetPhi2() )};
    std::for_each( vals.begin(), vals.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoPcon* sh ) {

    std::vector<long> pars{discretize_angle( sh->GetPhi1() ), discretize_angle( sh->GetDphi() ), long( sh->GetNz() )};
    pars.reserve( 3 + 3 * sh->GetNz() );
    for ( Int_t i = 0; i < sh->GetNz(); ++i ) {
      pars.emplace_back( discretize_length( sh->GetZ( i ) ) );
      pars.emplace_back( discretize_length( sh->GetRmin( i ) ) );
      pars.emplace_back( discretize_length( sh->GetRmax( i ) ) );
    }
    std::for_each( pars.begin(), pars.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoTrd1* sh ) {
    std::array<long, 4> vals{discretize_length( sh->GetDx1() ), discretize_length( sh->GetDx2() ),
                             discretize_length( sh->GetDy() ), discretize_length( sh->GetDz() )};
    std::for_each( vals.begin(), vals.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoTrd2* sh ) {
    std::array<long, 5> vals{discretize_length( sh->GetDx1() ), discretize_length( sh->GetDx2() ),
                             discretize_length( sh->GetDy1() ), discretize_length( sh->GetDy2() ),
                             discretize_length( sh->GetDz() )};
    std::for_each( vals.begin(), vals.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoTrap* sh ) {

    std::vector<long> vals{
        discretize_length( sh->GetDz() ),    discretize_angle( sh->GetTheta() ), discretize_angle( sh->GetPhi() ),
        discretize_length( sh->GetH1() ),    discretize_length( sh->GetBl1() ),  discretize_length( sh->GetTl1() ),
        discretize_angle( sh->GetAlpha1() ), discretize_length( sh->GetH2() ),   discretize_length( sh->GetBl2() ),
        discretize_length( sh->GetTl2() ),   discretize_angle( sh->GetAlpha2() )};

    std::for_each( vals.begin(), vals.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoCone* sh ) {

    std::vector<long> vals{discretize_length( sh->GetDz() ), discretize_length( sh->GetRmin1() ),
                           discretize_length( sh->GetRmax1() ), discretize_length( sh->GetRmin2() ),
                           discretize_length( sh->GetRmax2() )};

    std::for_each( vals.begin(), vals.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoSphere* sh ) {
    std::vector<long> vals{discretize_length( sh->GetRmin() ),  discretize_length( sh->GetRmax() ),
                           discretize_angle( sh->GetTheta1() ), discretize_angle( sh->GetTheta2() ),
                           discretize_angle( sh->GetPhi1() ),   discretize_angle( sh->GetPhi2() )};

    std::for_each( vals.begin(), vals.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoCtub* sh ) {
    const Double_t*   lo = sh->GetNlow();
    const Double_t*   hi = sh->GetNhigh();
    std::vector<long> vals{discretize_length( sh->GetRmin() ),
                           discretize_length( sh->GetRmax() ),
                           discretize_length( sh->GetDz() ),
                           discretize_angle( sh->GetPhi1() ),
                           discretize_angle( sh->GetPhi2() ),
                           discretize_vec( lo[0] ),
                           discretize_vec( lo[1] ),
                           discretize_vec( lo[2] ),
                           discretize_vec( hi[0] ),
                           discretize_vec( hi[1] ),
                           discretize_vec( hi[2] )};

    std::for_each( vals.begin(), vals.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  template <typename T>
  void checksum_shape( T& accumulator, TGeoTorus* sh ) {
    std::vector<long> vals{discretize_length( sh->GetR() ), discretize_length( sh->GetRmin() ),
                           discretize_length( sh->GetRmax() ), discretize_angle( sh->GetPhi1() ),
                           discretize_angle( sh->GetDphi() )};

    std::for_each( vals.begin(), vals.end(), [&accumulator]( long value ) { accumulator.accumulate( value ); } );
  }

  /*
   * Code to do the indirection from TGeoShape* to the detailed shape
   */

  template <typename T>
  void checksum_shape( T& accumulator, TGeoShape* shape ) {

    if ( !shape ) { throw std::invalid_argument( "null shape" ); }

    TClass* cl = shape->IsA();
    if ( cl == TGeoBBox::Class() )
      return checksum_shape( accumulator, static_cast<TGeoBBox*>( shape ) );
    else if ( cl == TGeoCompositeShape::Class() )
      return checksum_shape( accumulator, static_cast<TGeoCompositeShape*>( shape ) );
    else if ( cl == TGeoTubeSeg::Class() )
      return checksum_shape( accumulator, static_cast<TGeoTubeSeg*>( shape ) );
    else if ( cl == TGeoShapeAssembly::Class() )
      return checksum_shape( accumulator, static_cast<TGeoShapeAssembly*>( shape ) );
    else if ( cl == TGeoConeSeg::Class() )
      return checksum_shape( accumulator, static_cast<TGeoConeSeg*>( shape ) );
    else if ( cl == TGeoTrd1::Class() )
      return checksum_shape( accumulator, static_cast<TGeoTrd1*>( shape ) );
    else if ( cl == TGeoTrd2::Class() )
      return checksum_shape( accumulator, static_cast<TGeoTrd2*>( shape ) );
    else if ( cl == TGeoPcon::Class() )
      return checksum_shape( accumulator, static_cast<TGeoPcon*>( shape ) );
    else if ( cl == TGeoTrap::Class() )
      return checksum_shape( accumulator, static_cast<TGeoTrap*>( shape ) );
    else if ( cl == TGeoCone::Class() )
      return checksum_shape( accumulator, static_cast<TGeoCone*>( shape ) );
    else if ( cl == TGeoSphere::Class() )
      return checksum_shape( accumulator, static_cast<TGeoSphere*>( shape ) );
    else if ( cl == TGeoCtub::Class() )
      return checksum_shape( accumulator, static_cast<TGeoCtub*>( shape ) );
    else if ( cl == TGeoTorus::Class() )
      return checksum_shape( accumulator, static_cast<TGeoTorus*>( shape ) );
    else {
      throw std::runtime_error( "Failed to access dimensions for shape of type " + std::string( cl->GetName() ) );
    }
  }

  /*
   * Tools to iterate over the geometry tree
   */
  template <typename T>
  void checksum_node( T& accumulator, TGeoNode* node ) {

    // Checksum the position of the node
    checksum_matrix( accumulator, node->GetMatrix() );

    // Checksum of the medium and material
    TGeoMedium* medium = node->GetMedium();
    checksum_cstring( accumulator, medium->GetName() );
    TGeoMaterial* material = medium->GetMaterial();
    checksum_cstring( accumulator, material->GetName() );
    check_material( accumulator, material );

    // And now the shape
    checksum_shape( accumulator, node->GetVolume()->GetShape() );
  }

  template <typename ACC>
  std::map<std::string, typename ACC::ret> geometry_checksum( TGeoNode* node, int maxlevel ) {

    std::map<std::string, typename ACC::ret> checksums;

    // callback to compute the checkum of a node and its children checksums
    auto compute_checksum =
        [&checksums, maxlevel ]( TGeoNode * n, const lhcb::geometrytools::nodeMap<typename ACC::ret>& daughters,
                                 std::string path, int level ) -> typename ACC::ret {
      // Checksum of the node itself
      ACC accumulator;
      checksum_node( accumulator, n );
      for ( auto d : daughters ) {
        typename ACC::ret tcheck = d.second;
        accumulator.accumulate( tcheck );
      }

      if ( level < maxlevel ) { checksums.emplace( path, accumulator.result() ); }
      return accumulator.result();
    };

    lhcb::geometrytools::traverse( node, compute_checksum );
    return checksums;
  }

  /*
   * Methods usable by clients
   */
  auto geometry_crc = geometry_checksum<AccumulatorCrc>;

} // namespace lhcb::Detector::checksum
