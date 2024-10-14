//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================

#define _USE_MATH_DEFINES

#include <Core/ShapeUtilities.h>

// Framework include files
#include <DD4hep/DD4hepUnits.h>
#include <DD4hep/MatrixHelpers.h>
#include <DD4hep/Printout.h>
#include <DD4hep/ShapeTags.h>
#include <DD4hep/Shapes.h>
#include <DD4hep/detail/ShapesInterna.h>
#include <TGeoArb8.h>
#include <TGeoCompositeShape.h>
#include <TGeoCone.h>
#include <TGeoEltu.h>
#include <TGeoHalfSpace.h>
#include <TGeoHype.h>
#include <TGeoParaboloid.h>
#include <TGeoPcon.h>
#include <TGeoPgon.h>
#include <TGeoScaledShape.h>
#include <TGeoShapeAssembly.h>
#include <TGeoSphere.h>
#include <TGeoTorus.h>
#include <TGeoTrd1.h>
#include <TGeoTrd2.h>
#include <TGeoTube.h>
#include <TGeoXtru.h>

// C/C++ include files
#include <iomanip>
#include <sstream>
#include <stdexcept>

// ROOT includes
#include <TClass.h>
#include <TGeoBoolNode.h>
#include <TGeoCompositeShape.h>
#include <TGeoMatrix.h>
#include <TGeoScaledShape.h>

using namespace std;

/// Namespace for the AIDA detector description toolkit
namespace lhcb::Detector::shapeutils {
  using dd4hep::BooleanSolid;
  using dd4hep::Box;
  using dd4hep::Cone;
  using dd4hep::ConeSegment;
  using dd4hep::CutTube;
  using dd4hep::DEBUG;
  using dd4hep::EightPointSolid;
  using dd4hep::EllipticalTube;
  using dd4hep::ERROR;
  using dd4hep::except;
  using dd4hep::ExtrudedPolygon;
  using dd4hep::HalfSpace;
  using dd4hep::Hyperboloid;
  using dd4hep::IntersectionSolid;
  using dd4hep::isA;
  using dd4hep::Paraboloid;
  using dd4hep::Polycone;
  using dd4hep::Polyhedra;
  using dd4hep::PolyhedraRegular;
  using dd4hep::PseudoTrap;
  using dd4hep::Scale;
  using dd4hep::ShapelessSolid;
  using dd4hep::Solid;
  using dd4hep::Sphere;
  using dd4hep::SubtractionSolid;
  using dd4hep::TessellatedSolid;
  using dd4hep::Torus;
  using dd4hep::Trap;
  using dd4hep::Trd1;
  using dd4hep::Trd2;
  using dd4hep::TruncatedTube;
  using dd4hep::Tube;
  using dd4hep::TwistedTube;
  using dd4hep::TwistedTubeObject;
  using dd4hep::typeName;
  using dd4hep::UnionSolid;
  using dd4hep::Volume;

  // name units differently
  namespace units = dd4hep;

  // Local, anonymous stuff
  namespace {
    template <typename T>
    inline T* get_ptr( const TGeoShape* shape ) {
      if ( shape && shape->IsA() == T::Class() ) return (T*)shape;
      except( "Dimension", "Invalid shape pointer!" );
      return 0;
    }
    template <typename T>
    inline bool check_shape_type( const Handle<TGeoShape>& solid ) {
      return solid.isValid() && solid->IsA() == T::Class();
    }
  } // namespace

  template <typename T>
  vector<double> dimensions( const TGeoShape* shape ) {
    stringstream str;
    if ( shape )
      str << "Shape: dimension(" << typeName( typeid( *shape ) ) << "): Invalid call!";
    else
      str << "Shape: dimension<TGeoShape): Invalid call && pointer!";
    throw runtime_error( str.str() );
  }
  template <>
  vector<double> dimensions<TGeoShapeAssembly>( const TGeoShape* shape ) {
    const auto* sh = get_ptr<TGeoShapeAssembly>( shape );
    return {discretize_length( sh->GetDX() ), discretize_length( sh->GetDY() ), discretize_length( sh->GetDZ() )};
  }
  template <>
  vector<double> dimensions<TGeoBBox>( const TGeoShape* shape ) {
    const auto* sh = get_ptr<TGeoBBox>( shape );
    return {discretize_length( sh->GetDX() ), discretize_length( sh->GetDY() ), discretize_length( sh->GetDZ() )};
  }
  template <>
  vector<double> dimensions<TGeoHalfSpace>( const TGeoShape* shape ) {
    auto* sh = get_ptr<TGeoHalfSpace>( shape );
    return {discretize_length( sh->GetPoint()[0] ), discretize_length( sh->GetPoint()[1] ),
            discretize_length( sh->GetPoint()[2] ), discretize_length( sh->GetNorm()[0] ),
            discretize_length( sh->GetNorm()[1] ),  discretize_length( sh->GetNorm()[2] )};
  }
  template <>
  vector<double> dimensions<TGeoPcon>( const TGeoShape* shape ) {
    const TGeoPcon* sh = get_ptr<TGeoPcon>( shape );
    vector<double>  pars{discretize_angle( sh->GetPhi1() ), discretize_angle( sh->GetDphi() ), double( sh->GetNz() )};
    pars.reserve( 3 + 3 * sh->GetNz() );
    for ( Int_t i = 0; i < sh->GetNz(); ++i ) {
      pars.emplace_back( discretize_length( sh->GetZ( i ) ) );
      pars.emplace_back( discretize_length( sh->GetRmin( i ) ) );
      pars.emplace_back( discretize_length( sh->GetRmax( i ) ) );
    }
    return pars;
  }
  template <>
  vector<double> dimensions<TGeoConeSeg>( const TGeoShape* shape ) {
    const TGeoConeSeg* sh = get_ptr<TGeoConeSeg>( shape );
    return {discretize_length( sh->GetDz() ),    discretize_length( sh->GetRmin1() ),
            discretize_length( sh->GetRmax1() ), discretize_length( sh->GetRmin2() ),
            discretize_length( sh->GetRmax2() ), discretize_angle( sh->GetPhi1() ),
            discretize_angle( sh->GetPhi2() )};
  }
  template <>
  vector<double> dimensions<TGeoCone>( const TGeoShape* shape ) {
    const TGeoCone* sh = get_ptr<TGeoCone>( shape );
    return {discretize_length( sh->GetDz() ), discretize_length( sh->GetRmin1() ), discretize_length( sh->GetRmax1() ),
            discretize_length( sh->GetRmin2() ), discretize_length( sh->GetRmax2() )};
  }
  template <>
  vector<double> dimensions<TGeoTube>( const TGeoShape* shape ) {
    const TGeoTube* sh = get_ptr<TGeoTube>( shape );
    return {discretize_length( sh->GetRmin() ), discretize_length( sh->GetRmax() ), discretize_length( sh->GetDz() )};
  }
  template <>
  vector<double> dimensions<TGeoTubeSeg>( const TGeoShape* shape ) {
    const TGeoTubeSeg* sh = get_ptr<TGeoTubeSeg>( shape );
    return {discretize_length( sh->GetRmin() ), discretize_length( sh->GetRmax() ), discretize_length( sh->GetDz() ),
            discretize_angle( sh->GetPhi1() ), discretize_angle( sh->GetPhi2() )};
  }
  template <>
  vector<double> dimensions<TwistedTubeObject>( const TGeoShape* shape ) {
    const TwistedTubeObject* sh = get_ptr<TwistedTubeObject>( shape );
    return {discretize_angle( sh->GetPhiTwist() ),      discretize_length( sh->GetRmin() ),
            discretize_length( sh->GetRmax() ),         discretize_length( sh->GetNegativeEndZ() ),
            discretize_length( sh->GetPositiveEndZ() ), double( sh->GetNsegments() ),
            discretize_angle( sh->GetPhi2() )};
  }
  template <>
  vector<double> dimensions<TGeoCtub>( const TGeoShape* shape ) {
    const TGeoCtub* sh = get_ptr<TGeoCtub>( shape );
    const Double_t* lo = sh->GetNlow();
    const Double_t* hi = sh->GetNhigh();
    return {discretize_length( sh->GetRmin() ),
            discretize_length( sh->GetRmax() ),
            discretize_length( sh->GetDz() ),
            discretize_angle( sh->GetPhi1() ),
            discretize_angle( sh->GetPhi2() ),
            lo[0],
            lo[1],
            lo[2],
            hi[0],
            hi[1],
            hi[2]};
  }
  template <>
  vector<double> dimensions<TGeoEltu>( const TGeoShape* shape ) {
    const TGeoEltu* sh = get_ptr<TGeoEltu>( shape );
    return {sh->GetA(), sh->GetB(), discretize_length( sh->GetDz() )};
  }
  template <>
  vector<double> dimensions<TGeoTrd1>( const TGeoShape* shape ) {
    const TGeoTrd1* sh = get_ptr<TGeoTrd1>( shape );
    return {discretize_length( sh->GetDx1() ), discretize_length( sh->GetDx2() ), discretize_length( sh->GetDy() ),
            discretize_length( sh->GetDz() )};
  }
  template <>
  vector<double> dimensions<TGeoTrd2>( const TGeoShape* shape ) {
    const TGeoTrd2* sh = get_ptr<TGeoTrd2>( shape );
    return {discretize_length( sh->GetDx1() ), discretize_length( sh->GetDx2() ), discretize_length( sh->GetDy1() ),
            discretize_length( sh->GetDy2() ), discretize_length( sh->GetDz() )};
  }
  template <>
  vector<double> dimensions<TGeoParaboloid>( const TGeoShape* shape ) {
    const TGeoParaboloid* sh = get_ptr<TGeoParaboloid>( shape );
    return {discretize_length( sh->GetRlo() ), discretize_length( sh->GetRhi() ), discretize_length( sh->GetDz() )};
  }
  template <>
  vector<double> dimensions<TGeoHype>( const TGeoShape* shape ) {
    const TGeoHype* sh = get_ptr<TGeoHype>( shape );
    return {discretize_length( sh->GetDz() ), discretize_length( sh->GetRmin() ), discretize_angle( sh->GetStIn() ),
            discretize_length( sh->GetRmax() ), discretize_angle( sh->GetStOut() )};
  }
  template <>
  vector<double> dimensions<TGeoSphere>( const TGeoShape* shape ) {
    const TGeoSphere* sh = get_ptr<TGeoSphere>( shape );
    return {discretize_length( sh->GetRmin() ),  discretize_length( sh->GetRmax() ),
            discretize_angle( sh->GetTheta1() ), discretize_angle( sh->GetTheta2() ),
            discretize_angle( sh->GetPhi1() ),   discretize_angle( sh->GetPhi2() )};
  }
  template <>
  vector<double> dimensions<TGeoTorus>( const TGeoShape* shape ) {
    const TGeoTorus* sh = get_ptr<TGeoTorus>( shape );
    return {discretize_length( sh->GetR() ), discretize_length( sh->GetRmin() ), discretize_length( sh->GetRmax() ),
            discretize_angle( sh->GetPhi1() ), discretize_angle( sh->GetDphi() )};
  }
  template <>
  vector<double> dimensions<TGeoPgon>( const TGeoShape* shape ) {
    const TGeoPgon* sh = get_ptr<TGeoPgon>( shape );
    vector<double> pars{discretize_angle( sh->GetPhi1() ), discretize_angle( sh->GetDphi() ), double( sh->GetNedges() ),
                        double( sh->GetNz() )};
    pars.reserve( 4 + 3 * sh->GetNz() );
    for ( Int_t i = 0; i < sh->GetNz(); ++i ) {
      pars.emplace_back( discretize_length( sh->GetZ( i ) ) );
      pars.emplace_back( discretize_length( sh->GetRmin( i ) ) );
      pars.emplace_back( discretize_length( sh->GetRmax( i ) ) );
    }
    return pars;
  }
  template <>
  vector<double> dimensions<TGeoXtru>( const TGeoShape* shape ) {
    const TGeoXtru* sh = get_ptr<TGeoXtru>( shape );
    Int_t           nz = sh->GetNz();
    vector<double>  pars{double( nz )};
    pars.reserve( 1 + 4 * nz );
    for ( Int_t i = 0; i < nz; ++i ) {
      pars.emplace_back( discretize_length( sh->GetZ( i ) ) );
      pars.emplace_back( discretize_length( sh->GetXOffset( i ) ) );
      pars.emplace_back( discretize_length( sh->GetYOffset( i ) ) );
      pars.emplace_back( sh->GetScale( i ) );
    }
    return pars;
  }
  template <>
  vector<double> dimensions<TGeoArb8>( const TGeoShape* shape ) {
    TGeoArb8* sh = get_ptr<TGeoArb8>( shape );
    struct _V {
      double xy[8][2];
    }* vtx = (_V*)sh->GetVertices();
    vector<double> pars{discretize_length( sh->GetDz() )};
    for ( size_t i = 0; i < 8; ++i ) {
      pars.emplace_back( discretize_length( vtx->xy[i][0] ) );
      pars.emplace_back( discretize_length( vtx->xy[i][1] ) );
    }
    return pars;
  }
  template <>
  vector<double> dimensions<TGeoTrap>( const TGeoShape* shape ) {
    const TGeoTrap* sh = get_ptr<TGeoTrap>( shape );
    return {discretize_length( sh->GetDz() ),    discretize_angle( sh->GetTheta() ), discretize_angle( sh->GetPhi() ),
            discretize_length( sh->GetH1() ),    discretize_length( sh->GetBl1() ),  discretize_length( sh->GetTl1() ),
            discretize_angle( sh->GetAlpha1() ), discretize_length( sh->GetH2() ),   discretize_length( sh->GetBl2() ),
            discretize_length( sh->GetTl2() ),   discretize_angle( sh->GetAlpha2() )};
  }
  template <>
  vector<double> dimensions<TGeoGtra>( const TGeoShape* shape ) {
    const TGeoGtra* sh = get_ptr<TGeoGtra>( shape );
    return {discretize_length( sh->GetDz() ),    discretize_angle( sh->GetTheta() ),
            discretize_angle( sh->GetPhi() ),    discretize_length( sh->GetH1() ),
            discretize_length( sh->GetBl1() ),   discretize_length( sh->GetTl1() ),
            discretize_angle( sh->GetAlpha1() ), discretize_length( sh->GetH2() ),
            discretize_length( sh->GetBl2() ),   discretize_length( sh->GetTl2() ),
            discretize_angle( sh->GetAlpha2() ), discretize_angle( sh->GetTwistAngle() )};
  }
  template <>
  vector<double> dimensions<TGeoScaledShape>( const TGeoShape* shape ) {
    TGeoScaledShape* sh    = get_ptr<TGeoScaledShape>( shape );
    TGeoShape*       s_sh  = sh->GetShape();
    const Double_t*  scale = sh->GetScale()->GetScale();
    vector<double>   pars{scale[0], scale[1], scale[2]};
    vector<double>   s_pars = get_shape_dimensions( s_sh );
    for ( auto p : s_pars ) pars.push_back( p );
    return pars;
  }

#if ROOT_VERSION_CODE > ROOT_VERSION( 6, 21, 0 )
  template <>
  vector<double> dimensions<TGeoTessellated>( const TGeoShape* shape ) {
    TGeoTessellated* sh        = get_ptr<TGeoTessellated>( shape );
    int              num_facet = sh->GetNfacets();
    int              num_vtx   = sh->GetNvertices();
    vector<double>   pars;

    printout( DEBUG, "TessellatedSolid", "+++ Saving %d vertices, %d facets", num_vtx, num_facet );
    pars.reserve( num_facet * 5 + num_vtx * 3 + 2 );
    pars.emplace_back( num_vtx );
    pars.emplace_back( num_facet );
    for ( int i = 0; i < num_vtx; ++i ) {
      const auto& v = sh->GetVertex( i );
      pars.emplace_back( discretize_length( v.x() ) );
      pars.emplace_back( discretize_length( v.y() ) );
      pars.emplace_back( discretize_length( v.z() ) );
    }
    for ( int i = 0; i < num_facet; ++i ) {
      const TGeoFacet& f = sh->GetFacet( i );
      pars.emplace_back( double( f.GetNvert() ) );
      for ( int j = 0, n = f.GetNvert(); j < n; ++j ) {
#  if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 1 )
        auto vertexIndex = f[j];
#  else
        auto vertexIndex = f.GetVertexIndex( j );
#  endif
        pars.emplace_back( double( vertexIndex ) );
      }
    }
    return pars;
  }
#endif

  template <>
  vector<double> dimensions<TGeoCompositeShape>( const TGeoShape* shape ) {
    const TGeoCompositeShape* sh           = get_ptr<TGeoCompositeShape>( shape );
    const TGeoBoolNode*       boolean      = sh->GetBoolNode();
    TGeoMatrix*               right_matrix = boolean->GetRightMatrix();
    TGeoShape*                left_solid   = boolean->GetLeftShape();
    TGeoShape*                right_solid  = boolean->GetRightShape();

    TGeoBoolNode::EGeoBoolType oper        = boolean->GetBooleanOperator();
    TGeoMatrix*                left_matrix = boolean->GetLeftMatrix();
    const Double_t*            left_tr     = left_matrix->GetTranslation();
    const Double_t*            left_rot    = left_matrix->GetRotationMatrix();
    const Double_t*            right_tr    = right_matrix->GetTranslation();
    const Double_t*            right_rot   = right_matrix->GetRotationMatrix();

    vector<double> pars{double( oper )};
    vector<double> left_par  = get_shape_dimensions( left_solid );
    vector<double> right_par = get_shape_dimensions( right_solid );

    pars.insert( pars.end(), left_par.begin(), left_par.end() );
    double left_rotfixed[9];
    for ( int i = 0; i < 9; i++ ) { left_rotfixed[i] = discretize_rotmatrix( left_rot[i] ); }
    pars.insert( pars.end(), left_rotfixed, left_rotfixed + 9 );
    double left_trcm[3];
    for ( int i = 0; i < 3; i++ ) { left_trcm[i] = discretize_length( left_tr[i] ); }
    pars.insert( pars.end(), left_trcm, left_trcm + 3 );
    pars.insert( pars.end(), right_par.begin(), right_par.end() );
    double right_rotfixed[9];
    for ( int i = 0; i < 9; i++ ) { right_rotfixed[i] = discretize_rotmatrix( right_rot[i] ); }
    pars.insert( pars.end(), right_rotfixed, right_rotfixed + 9 );
    double right_trcm[3];
    for ( int i = 0; i < 3; i++ ) { right_trcm[i] = discretize_length( right_tr[i] ); }
    pars.insert( pars.end(), right_trcm, right_trcm + 3 );
    return pars;
  }

  template <typename T>
  vector<double> dimensions( const Handle<TGeoShape>& shape ) {
    return dimensions<typename T::Object>( get_ptr<typename T::Object>( shape.ptr() ) );
  }
  template vector<double> dimensions<ShapelessSolid>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Box>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Scale>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<HalfSpace>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Polycone>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<ConeSegment>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Tube>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<CutTube>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<TwistedTube>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<EllipticalTube>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Cone>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Trap>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Trd1>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Trd2>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Torus>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Sphere>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Paraboloid>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Hyperboloid>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<PolyhedraRegular>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<Polyhedra>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<ExtrudedPolygon>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<EightPointSolid>( const Handle<TGeoShape>& shape );
#if ROOT_VERSION_CODE > ROOT_VERSION( 6, 21, 0 )
  template vector<double> dimensions<TessellatedSolid>( const Handle<TGeoShape>& shape );
#endif
  template vector<double> dimensions<BooleanSolid>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<SubtractionSolid>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<UnionSolid>( const Handle<TGeoShape>& shape );
  template vector<double> dimensions<IntersectionSolid>( const Handle<TGeoShape>& shape );

  template <>
  vector<double> dimensions<PseudoTrap>( const Handle<TGeoShape>& shape ) {
    const TGeoCompositeShape* sh           = get_ptr<TGeoCompositeShape>( shape.ptr() );
    TGeoMatrix*               right_matrix = sh->GetBoolNode()->GetRightMatrix();
    stringstream              params( right_matrix->GetTitle() );
    vector<double>            pars;
    pars.reserve( 7 );
#ifdef DIMENSION_DEBUG
    cout << "dimensions: [" << PSEUDOTRAP_TAG << "]" << endl << right_matrix->GetTitle() << endl;
#endif
    for ( size_t i = 0; i < 7; ++i ) {
      double val;
      params >> val;
      pars.emplace_back( val );
      if ( !params.good() ) { except( "PseudoTrap", "+++ dimensions: Invalid parameter stream." ); }
    }
    return pars;
  }

  template <>
  vector<double> dimensions<TruncatedTube>( const Handle<TGeoShape>& shape ) {
    const TGeoCompositeShape* sh           = get_ptr<TGeoCompositeShape>( shape.ptr() );
    TGeoMatrix*               right_matrix = sh->GetBoolNode()->GetRightMatrix();
    stringstream              params( right_matrix->GetTitle() );
    vector<double>            pars;
    pars.reserve( 8 );
    for ( size_t i = 0; i < 8; ++i ) {
      double val;
      params >> val;
      pars.emplace_back( val );
      if ( !params.good() ) { except( "TruncatedTube", "+++ dimensions: Invalid parameter stream." ); }
    }
    return pars;
  }

  template <>
  vector<double> dimensions<Solid>( const Handle<TGeoShape>& shape ) {
    if ( shape.ptr() ) {
      TClass* cl = shape->IsA();
      if ( cl == TGeoShapeAssembly::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoShapeAssembly>( shape.ptr() );
      else if ( cl == TGeoBBox::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoBBox>( shape.ptr() );
      else if ( cl == TGeoHalfSpace::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoHalfSpace>( shape.ptr() );
      else if ( cl == TGeoPgon::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoPgon>( shape.ptr() );
      else if ( cl == TGeoPcon::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoPcon>( shape.ptr() );
      else if ( cl == TGeoConeSeg::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoConeSeg>( shape.ptr() );
      else if ( cl == TGeoCone::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoCone>( shape.ptr() );
      else if ( cl == TGeoTube::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoTube>( shape.ptr() );
      else if ( cl == TGeoTubeSeg::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoTubeSeg>( shape.ptr() );
      else if ( cl == TGeoCtub::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoCtub>( shape.ptr() );
      else if ( cl == TGeoEltu::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoEltu>( shape.ptr() );
      else if ( cl == TwistedTubeObject::Class() )
        return lhcb::Detector::shapeutils::dimensions<TwistedTubeObject>( shape.ptr() );
      else if ( cl == TGeoTrd1::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoTrd1>( shape.ptr() );
      else if ( cl == TGeoTrd2::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoTrd2>( shape.ptr() );
      else if ( cl == TGeoParaboloid::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoParaboloid>( shape.ptr() );
      else if ( cl == TGeoHype::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoHype>( shape.ptr() );
      else if ( cl == TGeoGtra::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoGtra>( shape.ptr() );
      else if ( cl == TGeoTrap::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoTrap>( shape.ptr() );
      else if ( cl == TGeoArb8::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoArb8>( shape.ptr() );
      else if ( cl == TGeoSphere::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoSphere>( shape.ptr() );
      else if ( cl == TGeoTorus::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoTorus>( shape.ptr() );
      else if ( cl == TGeoXtru::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoXtru>( shape.ptr() );
      else if ( cl == TGeoScaledShape::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoScaledShape>( shape.ptr() );
#if ROOT_VERSION_CODE > ROOT_VERSION( 6, 21, 0 )
      else if ( cl == TGeoTessellated::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoTessellated>( shape.ptr() );
#endif
      else if ( isA<TruncatedTube>( shape.ptr() ) )
        return lhcb::Detector::shapeutils::dimensions<TruncatedTube>( shape );
      else if ( isA<PseudoTrap>( shape.ptr() ) )
        return lhcb::Detector::shapeutils::dimensions<PseudoTrap>( shape );
      else if ( cl == TGeoCompositeShape::Class() )
        return lhcb::Detector::shapeutils::dimensions<TGeoCompositeShape>( shape.ptr() );
      else {
        printout( ERROR, "Solid", "Failed to access dimensions for shape of type:%s.", cl->GetName() );
      }
      return {};
    }
    except( "Solid", "Failed to access dimensions [Invalid handle]." );
    return {};
  }

  /// Access Shape dimension parameters (As in TGeo, but angles in radians rather than degrees)
  vector<double> get_shape_dimensions( const Handle<TGeoShape>& shape ) {
    return lhcb::Detector::shapeutils::dimensions<Solid>( shape );
  }
  /// Access Shape dimension parameters (As in TGeo, but angles in radians rather than degrees)
  vector<double> get_shape_dimensions( TGeoShape* shape ) {
    return lhcb::Detector::shapeutils::dimensions<Solid>( Solid( shape ) );
  }

} // namespace lhcb::Detector::shapeutils
