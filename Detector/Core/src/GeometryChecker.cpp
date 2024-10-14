/*****************************************************************************\
* (c) Copyright 2020-2023 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"

#include "Math/Vector3D.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGeoManager.h"
#include "TGeoNavigator.h"
#include "TGraph2D.h"
#include "TH2F.h"
#include "TImage.h"
#include "TMath.h"
#include "TRandom3.h"

#include <map>
#include <string>

/**
 * Simple plugin that dumps the materials found between two points specified on the command line,
 * e.g. by doing:

 * geoPluginRun -input compact/run3/trunk/LHCb.xml -plugin find_intersections 0.0 0.0 0.0 1000.0 1000 1000
 *
 */
static long tgeo_find_intersections( dd4hep::Detector& /* description */, int argc, char** argv ) {

  const char* myname = "FindIntersections";

  if ( argc < 6 ) {
    dd4hep::printout( dd4hep::ERROR, myname, "Please specify the positions of the start and end points" );
    exit( 1 );
  }

  // Parse the start and end points
  ROOT::Math::XYZPoint p1{std::stod( argv[0] ), std::stod( argv[1] ), std::stod( argv[2] )};
  ROOT::Math::XYZPoint p2{std::stod( argv[3] ), std::stod( argv[4] ), std::stod( argv[5] )};

  // Set the initial status of the navigator
  auto nav = gGeoManager->GetCurrentNavigator();

  nav->SetCurrentPoint( p1.x(), p1.y(), p1.z() );
  auto direction = p2 - p1;
  direction /= direction.R();
  nav->SetCurrentDirection( direction.x(), direction.y(), direction.z() );
  bool nav_finished = false;

  // Set the navigator state, finding the node at the ciurrent position
  nav->FindNode();

  // Iterate until we're close enogh to the end point (1cm)
  while ( !nav_finished ) {

    const double*      ipos = nav->GetCurrentPoint();
    std::ostringstream os;
    os << ipos[0] << ", " << ipos[1] << ", " << ipos[2] << " " << nav->GetCurrentNode()->GetName() << " - "
       << nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetName();

    dd4hep::printout( dd4hep::INFO, myname, "%s", os.str().c_str() );

    ROOT::Math::XYZPoint i{ipos[0], ipos[1], ipos[2]};
    nav->FindNextBoundaryAndStep( sqrt( ( i - p2 ).Mag2() ) );
    const double*        pos = nav->GetCurrentPoint();
    ROOT::Math::XYZPoint p{pos[0], pos[1], pos[2]};
    nav_finished = ( ( p - p2 ).Mag2() < 1.0 );
  }

  return 1;
}
DECLARE_APPLY( find_intersections, tgeo_find_intersections )

/**
 * That scans the material in the z direction, for a zone defined in x and y
 *
 * geoPluginRun -input compact/run3/trunk/LHCb.xml -plugin scan_intersections -1000 1000 100 -1000 1000 100 12000 15000
 *
 */
static long tgeo_scan_intersections( dd4hep::Detector& /* description */, int argc, char** argv ) {

  const char* myname = "ScanIntersections";

  if ( argc < 8 ) {
    dd4hep::printout( dd4hep::ERROR, myname, "Please specify xmin xmax xstep ymin ymax ystep zmin zmax" );
    exit( 1 );
  }

  double xmin, xmax, xstep, ymin, ymax, ystep, zmin, zmax;
  xmin  = std::stod( argv[0] );
  xmax  = std::stod( argv[1] );
  xstep = std::stod( argv[2] );
  ymin  = std::stod( argv[3] );
  ymax  = std::stod( argv[4] );
  ystep = std::stod( argv[5] );
  zmin  = std::stod( argv[6] );
  zmax  = std::stod( argv[7] );

  dd4hep::printout( dd4hep::INFO, myname, "x: (%f, %f, %f) y:(%f, %f, %f) z:(%f, %f)", xmin, xmax, xstep, ymin, ymax,
                    ystep, zmin, zmax );

  for ( double x = xmin; x <= xmax; x += xstep ) {
    for ( double y = ymin; y <= ymax; y += ystep ) {

      // Set the initial status of the navigator
      double z   = zmin;
      auto   nav = gGeoManager->GetCurrentNavigator();
      nav->SetCurrentPoint( x, y, z );
      auto startpoint = ROOT::Math::XYZPoint( x, y, z );
      auto endpoint   = ROOT::Math::XYZPoint( x, y, zmax );
      auto direction  = endpoint - startpoint;
      direction /= direction.R();
      nav->SetCurrentDirection( direction.x(), direction.y(), direction.z() );

      // Set the navigator state, finding the node at the current position
      bool nav_finished = false;
      nav->FindNode();

      // Iterate until we're close enough to the end point (0.1mm)
      while ( !nav_finished ) {

        const double*      ipos = nav->GetCurrentPoint();
        std::ostringstream os;
        os << ipos[0] << " " << ipos[1] << " " << ipos[2] << " " << nav->GetCurrentNode()->GetName() << " "
           << nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetName();

        dd4hep::printout( dd4hep::INFO, myname, "%s", os.str().c_str() );

        ROOT::Math::XYZPoint i{ipos[0], ipos[1], ipos[2]};
        nav->FindNextBoundaryAndStep( sqrt( ( i - endpoint ).Mag2() ) );
        const double*        pos = nav->GetCurrentPoint();
        ROOT::Math::XYZPoint p{pos[0], pos[1], pos[2]};
        nav_finished = ( ( p - endpoint ).Mag2() < 0.1 );
        if ( nav_finished ) {
          std::ostringstream os;
          os << p.x() << " " << p.y() << " " << p.z() << " " << nav->GetCurrentNode()->GetName() << " "
             << nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetName();
          dd4hep::printout( dd4hep::INFO, myname, "%s", os.str().c_str() );
        }
      }
    }
  }
  return 1;
}
DECLARE_APPLY( scan_intersections, tgeo_scan_intersections )

/**
 * Scan the geometry for a number of x y z points
 *
 * geoPluginRun -input compact/run3/trunk/LHCb.xml -plugin scan 0.0 0.0 0.0 1000.0 1000 1000
 *
 */
static long tgeo_scan( dd4hep::Detector& /* description */, int argc, char** argv ) {

  ROOT::Math::XYZPoint start_point{0, 0, 0}, end_point{0, 0, 0};
  double               nb_points = 10;
  bool                 help      = false;
  bool                 error     = false;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( 0 == ::strncmp( "-help", argv[i], 4 ) ) { help = true; }
  }

  if ( argc == 7 ) {
    start_point = ROOT::Math::XYZPoint( std::stod( argv[0] ), std::stod( argv[1] ), std::stod( argv[2] ) );
    end_point   = ROOT::Math::XYZPoint( std::stod( argv[3] ), std::stod( argv[4] ), std::stod( argv[5] ) );
    nb_points   = std::stod( argv[6] );
  } else {
    error = true;
  }

  if ( help || error ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name>  xstart ystart zstart xend yend zend  nb_points\n"
                 "     name:   factory name     prints the material at a number of point between start and end\n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  auto                  nav = gGeoManager->GetCurrentNavigator();
  ROOT::Math::XYZVector direction{end_point - start_point};
  direction /= nb_points;
  if ( ( abs( direction.x() ) <= 1e-5 ) || ( abs( direction.y() ) <= 1e-5 ) || ( abs( direction.z() ) <= 1e-5 ) ) {
    std::cerr << "Scanning please specify different start and end coordinates to scan volume" << std::endl;
    ::exit( EINVAL );
  }

  double x = start_point.x();
  double y = start_point.y();
  double z = start_point.z();
  while ( z <= end_point.z() ) {
    while ( y <= end_point.y() ) {
      while ( x <= end_point.x() ) {
        TGeoNode*          node = nav->FindNode( x, y, z );
        std::ostringstream os;
        os << "( " << x << ", " << y << ", " << z << ") - " << node->GetName() << " - "
           << node->GetVolume()->GetMaterial()->GetName();
        dd4hep::printout( dd4hep::INFO, "Scan", "%s", os.str().c_str() );
        x += direction.x();
      }
      x = start_point.x();
      y += direction.y();
    }
    y = start_point.y();
    z += direction.z();
  }

  return 0;
}
DECLARE_APPLY( scan, tgeo_scan )

/**
 * Simple plugin that creates a plot of the density of the materials found in a x/y plane for the given z
 * e.g. by doing:
 * geoPluginRun -input compact/run3/trunk/LHCb.xml -interpreter -plugin get_slice -50 50 -50 50 50 1000
 */
static long tgeo_get_slice( dd4hep::Detector& /* description */, int argc, char** argv ) {

  double xmin{0}, xmax{0}, ymin{0}, ymax{0}, z{0}, x{0}, y{0};
  int    nb_points = 10;
  bool   help      = false;
  bool   error     = false;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( 0 == ::strncmp( "-help", argv[i], 4 ) ) { help = true; }
  }

  if ( argc == 6 ) {
    xmin      = std::stod( argv[0] );
    xmax      = std::stod( argv[1] );
    ymin      = std::stod( argv[2] );
    ymax      = std::stod( argv[3] );
    z         = std::stod( argv[4] );
    nb_points = std::stod( argv[5] );
  } else {
    error = true;
  }

  if ( help || error ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name>  xmin xmax ymin ymax z nb_points              \n"
                 "     name:   factory name     density_slice                         \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  TGraph2D* plot = new TGraph2D( nb_points * nb_points );
  plot->SetNpx( nb_points );
  plot->SetNpy( nb_points );
  plot->SetTitle( "" );

  int    p     = 0;
  double stepx = ( xmax - xmin ) / nb_points;
  double stepy = ( ymax - ymin ) / nb_points;

  for ( x = xmin + stepx / 2; x < xmax; x += stepx ) {
    for ( y = ymin + stepy / 2; y < ymax; y += stepy ) {
      auto node = gGeoManager->FindNode( x, y, z );
      if ( node ) {
        double d = node->GetMedium()->GetMaterial()->GetDensity();
        plot->SetPoint( p++, x, y, d );
      }
    }
  }
  plot->Draw( "COLZ" );
  TImage* img = TImage::Create();
  img->FromPad( gPad );
  img->WriteImage( "slice.png" );
  return 0;
}
DECLARE_APPLY( density_slice, tgeo_get_slice )

/**
 * Simple plugin that dumps the capacity, density and mass of each volume in the volume tree
 * e.g. by doing:
 *
 * geoPluginRun -input compact/components/debug/VP.xml -plugin dump_volume_masses
 *
 * This is intended to be used as an interactive tool to aid geometry development, rather than being used in automated
 * tests
 */
static long tgeo_dump_masses( dd4hep::Detector& description, int argc, char** argv ) {
  struct MassChecker {
    dd4hep::Detector& m_description;

    MassChecker( dd4hep::Detector& desc, int /*ac*/, char** /*av*/ ) : m_description{desc} {}

    double dump( TGeoNode* node, int level ) {
      if ( !node ) { return 0.0; }
      TGeoVolume* volume = node->GetVolume();
      if ( !volume ) { return 0.0; }
      TGeoMaterial* material = volume->GetMaterial();

      const TString volName{volume->GetName()};
      const TString matName{material ? material->GetName() : "N/A"};

      const double capacity{volume->Capacity() / 1000.0};
      const double density{material->GetDensity()};
      const double mass{capacity * density};

      bool isAssembly{false};
      if ( !material || matName == "dummy" || matName == "Vacuum" ) { isAssembly = true; }

      double assemblyMass{isAssembly ? 0.0 : mass};

      for ( Int_t idau = 0, ndau = volume->GetNdaughters(); idau < ndau; ++idau ) {
        TGeoNode* daughter = volume->GetNode( idau );
        assemblyMass += dump( daughter, level + 1 );
      }

      if ( isAssembly ) {
        std::cout << "At level " << level << " assembly/mother volume = " << volName
                  << ", total mass = " << assemblyMass << " g" << std::endl;
      } else {
        std::cout << "At level " << level << " volume = " << volName << ", capacity = " << capacity
                  << " cm3, material = " << matName << ", density = " << density << " g/cm3, mass = " << mass << " g"
                  << std::endl;
      }

      return assemblyMass;
    }

    long operator()() {
      dd4hep::DetElement top = m_description.world();
      this->dump( top.placement().ptr(), 0 );
      return 1;
    }
  };

  MassChecker massCheck{description, argc, argv};
  return massCheck();
}
DECLARE_APPLY( dump_volume_masses, tgeo_dump_masses )

/**
 * Simple plugin that creates histograms of both the radiation length and interaction length
 * of materials in a cartesian 2D plane integrated over the 3rd cartesian coordinate.
 * e.g. a histogram in x vs y integrated between z_min and z_max
 *
 * Example usage:
 *
 * geoPluginRun -input compact/components/debug/VP.xml -plugin rad_scan -150.0 -150.0 -7.0 150.0 150.0 -4.0, 300
 *
 * will do a scan in x (300 points from -150mm to 150mm) vs y (300 points from -150mm to 150mm) between z = -7mm and
 * -4mm
 *
 * Additional arguments allow to specify the number of scan points in y and z, to specify which coordinate to integrate
 * over (z by default), to toggle verbose mode (defaults to false), and to toggle whether the RF foil should be ignored
 * (defaults to true)
 *
 * This is intended to be used as an interactive tool to aid geometry development, rather than being used in automated
 * tests
 */
static long tgeo_rad_scan( dd4hep::Detector& /* description */, int argc, char** argv ) {

  ROOT::Math::XYZPoint start_point{0, 0, 0}, end_point{0, 0, 0};
  double               x_points     = 10;
  double               y_points     = 10;
  double               z_points     = 10;
  char                 integrateVar = 'z';
  bool                 verbose      = false;
  bool                 ignoreFoil   = true;
  bool                 help         = false;
  bool                 error        = false;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( 0 == ::strncmp( "-help", argv[i], 4 ) ) { help = true; }
  }

  if ( argc == 7 ) {
    start_point = ROOT::Math::XYZPoint( atof( argv[0] ), atof( argv[1] ), atof( argv[2] ) );
    end_point   = ROOT::Math::XYZPoint( atof( argv[3] ), atof( argv[4] ), atof( argv[5] ) );
    x_points    = atof( argv[6] );
    y_points    = atof( argv[6] );
    z_points    = atof( argv[6] );
  } else if ( argc == 9 ) {
    start_point = ROOT::Math::XYZPoint( atof( argv[0] ), atof( argv[1] ), atof( argv[2] ) );
    end_point   = ROOT::Math::XYZPoint( atof( argv[3] ), atof( argv[4] ), atof( argv[5] ) );
    x_points    = atof( argv[6] );
    y_points    = atof( argv[7] );
    z_points    = atof( argv[8] );
  } else if ( argc == 12 ) {
    start_point  = ROOT::Math::XYZPoint( atof( argv[0] ), atof( argv[1] ), atof( argv[2] ) );
    end_point    = ROOT::Math::XYZPoint( atof( argv[3] ), atof( argv[4] ), atof( argv[5] ) );
    x_points     = atof( argv[6] );
    y_points     = atof( argv[7] );
    z_points     = atof( argv[8] );
    integrateVar = argv[9][0];
    verbose      = atoi( argv[10] );
    ignoreFoil   = atoi( argv[11] );
  } else {
    error = true;
  }

  if ( help || error ) {
    /// Help printout describing the basic command line interface
    std::cout
        << "Usage: -plugin <name>  xstart ystart zstart xend yend zend xpoints ypoints zpoints integratevar verbose "
           "ignoreRFFoil\n"
           "     name:   factory name     rad_scan                                                                 \n"
           "     -help                    Show this help.                                                          \n"
           "\tArguments given: "
        << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  double xStart  = start_point.x();
  double yStart  = start_point.y();
  double zStart  = start_point.z();
  double xEnd    = end_point.x();
  double yEnd    = end_point.y();
  double zEnd    = end_point.z();
  bool   changed = false;

  if ( start_point.x() > end_point.x() ) {
    std::cout << "End point should come after start point! Swapping xstart and xend " << start_point.x() << " <-> "
              << end_point.x() << std::endl;
    const double xtmp = xEnd;
    xEnd              = xStart;
    xStart            = xtmp;
    changed           = true;
  }
  if ( start_point.y() > end_point.y() ) {
    std::cout << "End point should come after start point! Swapping ystart and yend " << start_point.y() << " <-> "
              << end_point.y() << std::endl;
    const double ytmp = yEnd;
    yEnd              = yStart;
    yStart            = ytmp;
    changed           = true;
  }
  if ( start_point.z() > end_point.z() ) {
    std::cout << "End point should come after start point! Swapping zstart and zend " << start_point.z() << " <-> "
              << end_point.z() << std::endl;
    const double ztmp = zEnd;
    zEnd              = zStart;
    zStart            = ztmp;
    changed           = true;
  }

  if ( changed ) {
    start_point.SetCoordinates( xStart, yStart, zStart );
    end_point.SetCoordinates( xEnd, yEnd, zEnd );
  }

  auto                  nav = gGeoManager->GetCurrentNavigator();
  ROOT::Math::XYZVector direction{end_point - start_point};
  // direction /= nb_points;
  double dx = ( direction.x() ) / x_points;
  double dy = ( direction.y() ) / y_points;
  double dz = ( direction.z() ) / z_points;
  double z  = start_point.z();
  double y  = start_point.y();
  double x  = start_point.x();
  int    p  = 0;

  // So that the slices can be done in any direction
  double a, b, da, db, a_points, b_points;
  double start_point_a, start_point_b, start_point_c, end_point_a, end_point_b;

  if ( integrateVar == 'z' ) {
    a             = x;
    da            = dx;
    b             = y;
    db            = dy;
    a_points      = x_points;
    b_points      = y_points;
    start_point_a = start_point.x();
    start_point_b = start_point.y();
    start_point_c = start_point.z();
    end_point_a   = end_point.x();
    end_point_b   = end_point.y();
    direction     = ROOT::Math::XYZVector( 0, 0, 1 );
  } else if ( integrateVar == 'x' ) {
    a             = y;
    da            = dy;
    b             = z;
    db            = dz;
    a_points      = y_points;
    b_points      = z_points;
    start_point_a = start_point.y();
    start_point_b = start_point.z();
    start_point_c = start_point.x();
    end_point_a   = end_point.y();
    end_point_b   = end_point.z();
    direction     = ROOT::Math::XYZVector( 1, 0, 0 );
  } else if ( integrateVar == 'y' ) {
    a             = x;
    da            = dx;
    b             = z;
    db            = dz;
    a_points      = x_points;
    b_points      = z_points;
    start_point_a = start_point.x();
    start_point_b = start_point.z();
    start_point_c = start_point.y();
    end_point_a   = end_point.x();
    end_point_b   = end_point.z();
    direction     = ROOT::Math::XYZVector( 0, 1, 0 );
  } else {
    std::cout << "Integrate variable needs to be x, y or z" << std::endl;
    ::exit( EINVAL );
  }

  TGraph2D* plotRadLen = new TGraph2D( a_points * b_points );
  plotRadLen->SetNpx( a_points );
  plotRadLen->SetNpy( b_points );
  plotRadLen->SetTitle( "Radiation Length" );

  TGraph2D* plotIntLen = new TGraph2D( a_points * b_points );
  plotIntLen->SetNpx( a_points );
  plotIntLen->SetNpy( b_points );
  plotIntLen->SetTitle( "Interaction Length" );
  // save also the histogram
  TH2F* rad_hist = new TH2F( "rad_hist", "Radiation length", a_points, start_point_a, end_point_a, b_points,
                             start_point_b, end_point_b );
  std::map<std::string, TH2F*> materialRadHists;
  TH2F* int_hist = new TH2F( "int_hist", "Interaction length", a_points, start_point_a, end_point_a, b_points,
                             start_point_b, end_point_b );
  std::map<std::string, TH2F*> materialIntHists;

  while ( a <= end_point_a ) {
    while ( b <= end_point_b ) {
      // Track totals of both radiation and interaction length
      double               totalRadLength{0.0};
      double               totalIntLength{0.0};
      ROOT::Math::XYZPoint currentPoint;
      if ( integrateVar == 'z' ) {
        currentPoint = ROOT::Math::XYZPoint( a, b, start_point_c );
      } else if ( integrateVar == 'x' ) {
        currentPoint = ROOT::Math::XYZPoint( start_point_c, a, b );
      } else if ( integrateVar == 'y' ) {
        currentPoint = ROOT::Math::XYZPoint( a, start_point_c, b );
      } else {
        ::exit( EINVAL );
      }
      // Keep track of the radiation and interaction length for each material
      std::map<std::string, double> materialRadLengths;
      std::map<std::string, double> materialIntLengths;
      nav->SetCurrentPoint( currentPoint.x(), currentPoint.y(), currentPoint.z() );
      nav->SetCurrentDirection( direction.x(), direction.y(), direction.z() );
      bool nav_finished = false;
      nav->FindNode();
      while ( !nav_finished ) {
        const double*      ipos     = nav->GetCurrentPoint();
        const std::string  nodeName = nav->GetCurrentNode()->GetName();
        const std::string  matName  = nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetName();
        std::ostringstream os;
        os << ipos[0] << ", " << ipos[1] << ", " << ipos[2] << " " << nodeName << " - " << matName;
        if ( verbose ) { dd4hep::printout( dd4hep::INFO, "rad_scan", "%s", os.str().c_str() ); }
        const bool           isFoil           = !( nodeName.find( "pvRFFoil" ) );
        const double         currentRadLength = nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetRadLen();
        const double         currentIntLength = nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetIntLen();
        ROOT::Math::XYZPoint i{ipos[0], ipos[1], ipos[2]};
        nav->FindNextBoundaryAndStep( sqrt( ( i - end_point ).Mag2() ) );
        const double*        pos = nav->GetCurrentPoint();
        ROOT::Math::XYZPoint p{pos[0], pos[1], pos[2]};
        const double         currentThick = sqrt( ( p - i ).Mag2() );
        const double currentRadPercent    = ( ignoreFoil && isFoil ) ? 0.0 : 100.0 * currentThick / currentRadLength;
        const double currentIntPercent    = ( ignoreFoil && isFoil ) ? 0.0 : 100.0 * currentThick / currentIntLength;
        totalRadLength += currentRadPercent;
        totalIntLength += currentIntPercent;
        if ( materialRadLengths.count( matName ) ) {
          materialRadLengths[matName] += currentRadPercent;
        } else {
          materialRadLengths.insert( std::make_pair( matName, currentRadPercent ) );
        }
        if ( materialIntLengths.count( matName ) ) {
          materialIntLengths[matName] += currentIntPercent;
        } else {
          materialIntLengths.insert( std::make_pair( matName, currentIntPercent ) );
        }
        nav_finished = ( ( p.x() >= end_point.x() ) || ( p.y() >= end_point.y() ) || ( p.z() >= end_point.z() ) );
      }
      plotRadLen->SetPoint( p++, a, b, totalRadLength );
      plotIntLen->SetPoint( p++, a, b, totalIntLength );
      rad_hist->Fill( a, b, totalRadLength );
      int_hist->Fill( a, b, totalIntLength );
      // Fill all our material-specific histograms
      for ( auto& [matName, matRadLength] : materialRadLengths ) {
        auto& matIntLength = materialIntLengths.at( matName );
        if ( materialRadHists.count( matName ) ) {
          materialRadHists[matName]->Fill( a, b, matRadLength );
          materialIntHists[matName]->Fill( a, b, matIntLength );
        } else {
          // material names have colons in sometimes e.g. VP:SiliCopper
          std::string safeMatName{matName};
          safeMatName = ( safeMatName.find( ":" ) == std::string::npos )
                            ? safeMatName
                            : safeMatName.replace( safeMatName.find( ":" ), 1, "_" );

          TH2F* newHist_rad = new TH2F( safeMatName.c_str(), ( matName + " radiation length" ).c_str(), a_points,
                                        start_point_a, end_point_a, b_points, start_point_b, end_point_b );
          newHist_rad->Fill( a, b, matRadLength );
          materialRadHists.insert( std::make_pair( matName, newHist_rad ) );

          TH2F* newHist_int = new TH2F( safeMatName.c_str(), ( matName + " interaction length" ).c_str(), a_points,
                                        start_point_a, end_point_a, b_points, start_point_b, end_point_b );
          newHist_int->Fill( a, b, matIntLength );
          materialIntHists.insert( std::make_pair( matName, newHist_int ) );
        }
      }
      b += db;
    }
    b = start_point_b;
    a += da;
  }

  plotRadLen->Draw( "COLZ" );
  TImage* img = TImage::Create();
  img->FromPad( gPad );
  img->WriteImage( "rad_scan.png" );
  std::cout << "Saved image to rad_scan.png" << std::endl;

  plotIntLen->Draw( "COLZ" );
  img->FromPad( gPad );
  img->WriteImage( "int_scan.png" );
  std::cout << "Saved image to int_scan.png" << std::endl;

  TFile* out_file = new TFile( "rad_scan.root", "recreate" );
  rad_hist->Write();
  int_hist->Write();
  for ( auto& [matName, hist] : materialRadHists ) { hist->Write(); }
  for ( auto& [matName, hist] : materialIntHists ) { hist->Write(); }
  out_file->Close();
  std::cout << "Saved histogram to rad_scan.root" << std::endl;

  return 1;
}
DECLARE_APPLY( rad_scan, tgeo_rad_scan )

/**
 * Simple plugin that creates histograms in the eta vs phi plane of the total radiation length and interaction length in
 * slices of z
 *
 * Example usage:
 *
 * geoPluginRun -input compact/components/LHCb.xml -plugin rad_scan_etaphi 0 200 835 978 2250 2750 7670 8350 9040 9420
 * 11920 12280
 *
 * will do a non-verbose scan in eta (200 points from 1.5 to 5.0) vs phi (200 points from -180 deg to 180 deg) in
 * various z slices, from 0mm to 835mm, 835mm to 978mm, etc. (z values approximately correspond to the z extents of the
 * various sub detectors)
 *
 * This is intended to be used as an interactive tool to aid geometry development, rather than being used in automated
 * tests
 */
static long tgeo_rad_scan_etaphi( dd4hep::Detector& /* description */, int argc, char** argv ) {

  unsigned int        nPoints = 200;
  std::vector<double> zPoints;
  bool                verbose = false;
  bool                help    = false;
  bool                error   = false;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( 0 == ::strncmp( "-help", argv[i], 4 ) ) { help = true; }
  }

  if ( argc > 2 ) {
    verbose = atoi( argv[0] );
    nPoints = atoi( argv[1] );
    for ( auto i = 2; i < argc; ++i ) { zPoints.push_back( atof( argv[i] ) ); }
  } else {
    error = true;
  }

  if ( help || error ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name> verbose npoints zpoints...\n"
                 "     name:   factory name     rad_scan_etaphi\n"
                 "     -help                    Show this help.\n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  auto nav = gGeoManager->GetCurrentNavigator();

  std::vector<TH2F*> hrad_eta;
  std::vector<TH2F*> hint_eta;
  const std::size_t  nZPoints{zPoints.size()};
  for ( std::size_t i = 0; i < nZPoints; ++i ) {
    std::string name_rad{"rad length eta vs. phi from z = 0 to " + std::to_string( zPoints[i] )};
    std::string name_int{"int length eta vs. phi from z = 0 to " + std::to_string( zPoints[i] )};
    if ( i > 0 ) {
      name_rad =
          "rad length eta vs. phi from z = " + std::to_string( zPoints[i - 1] ) + " to " + std::to_string( zPoints[i] );
      name_int =
          "int length eta vs. phi from z = " + std::to_string( zPoints[i - 1] ) + " to " + std::to_string( zPoints[i] );
    }
    std::string key_rad{"hrad_etaphi_" + std::to_string( i )};
    std::string key_int{"hint_etaphi_" + std::to_string( i )};
    TH2F*       newHist_rad = new TH2F( key_rad.c_str(), name_rad.c_str(), nPoints, -180., 180., nPoints, 1.5, 5. );
    TH2F*       newHist_int = new TH2F( key_int.c_str(), name_int.c_str(), nPoints, -180., 180., nPoints, 1.5, 5. );
    hrad_eta.push_back( newHist_rad );
    hint_eta.push_back( newHist_int );
  }

  double eta{0.};
  double theta{0.};
  double phi{0.};
  double phirad{0.};
  double x{0.0};
  double y{0.0};

  for ( std::size_t k = 0; k < nPoints; ++k ) {
    eta   = 1.5 + ( float( k ) + 0.5 ) * 3.5 / float( nPoints );
    theta = 2.0 * atan( exp( -eta ) );

    for ( std::size_t j = 0; j < nPoints; ++j ) {
      phi    = -180. + ( float( j ) + 0.5 ) * 360. / float( nPoints );
      phirad = phi / 180. * TMath::Pi();

      ROOT::Math::XYZPoint a{0., 0., 0.};
      ROOT::Math::XYZPoint b{0., 0., 0.};

      double totalRadLength{0.0};
      double totalIntLength{0.0};

      for ( std::size_t i = 0; i < nZPoints; ++i ) {
        const double r{zPoints[i] / cos( theta )};
        x = r * cos( phirad ) * sin( theta );
        y = r * sin( phirad ) * sin( theta );
        b.SetCoordinates( x, y, zPoints[i] );
        auto direction = b - a;
        direction /= direction.R();
        nav->SetCurrentPoint( a.x(), a.y(), a.z() );
        nav->SetCurrentDirection( direction.x(), direction.y(), direction.z() );
        nav->FindNode();
        totalRadLength = 0.0;
        totalIntLength = 0.0;
        bool navFinished{false};
        while ( !navFinished ) {
          const double*      ipos     = nav->GetCurrentPoint();
          const std::string  nodeName = nav->GetCurrentNode()->GetName();
          const std::string  matName  = nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetName();
          std::ostringstream os;
          os << ipos[0] << ", " << ipos[1] << ", " << ipos[2] << " " << nodeName << " - " << matName;
          if ( verbose ) { dd4hep::printout( dd4hep::INFO, "rad_scan_etaphi", "%s", os.str().c_str() ); }
          const double         currentRadLength = nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetRadLen();
          const double         currentIntLength = nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetIntLen();
          ROOT::Math::XYZPoint i{ipos[0], ipos[1], ipos[2]};
          nav->FindNextBoundaryAndStep( sqrt( ( i - b ).Mag2() ) );
          const double*        pos = nav->GetCurrentPoint();
          ROOT::Math::XYZPoint p{pos[0], pos[1], pos[2]};
          const double         currentThick      = sqrt( ( p - i ).Mag2() );
          const double         currentRadPercent = 100.0 * currentThick / currentRadLength;
          const double         currentIntPercent = 100.0 * currentThick / currentIntLength;
          totalRadLength += currentRadPercent;
          totalIntLength += currentIntPercent;
          navFinished = ( ( p - b ).Mag2() < 1.0 );
        }
        hrad_eta[i]->Fill( phi, eta, totalRadLength );
        hint_eta[i]->Fill( phi, eta, totalIntLength );
        a = b;
      }
    }
  }

  TFile* out_file = new TFile( "rad_scan_etaphi.root", "recreate" );
  for ( auto hist_rad : hrad_eta ) { hist_rad->Write(); }
  for ( auto hist_int : hint_eta ) { hist_int->Write(); }
  out_file->Close();
  std::cout << "Saved histogram to rad_scan_etaphi.root" << std::endl;

  return 1;
}
DECLARE_APPLY( rad_scan_etaphi, tgeo_rad_scan_etaphi )

/**
 * Various helper functions and structs for the tgeo_rad_scan_etaphi_velo plugin, defined below
 */
namespace {
  std::string tagMatName( const TString& matName, const TString& nodeName ) {

    TString sanitisedName{matName};
    sanitisedName.ReplaceAll( ":", "_" );

    if ( sanitisedName.Contains( "Aluminium" ) ) {

      // Allow separate tagging of things made of Aluminium
      if ( nodeName.Contains( "pvRFFoil" ) ) {
        sanitisedName += "_RFFoil";
      } else if ( nodeName.Contains( "RFBox" ) ) {
        sanitisedName += "_RFBox";
      } else if ( nodeName.Contains( "Foot" ) ) {
        sanitisedName += "_Foot";
      }

    } else if ( sanitisedName.Contains( "Silicon" ) ) {

      // Allow separate tagging of things made of Silicon
      if ( nodeName.Contains( "pvDet" ) ) {
        sanitisedName += "_Sensor";
      } else if ( nodeName.Contains( "pvGuardRing" ) ) {
        sanitisedName += "_GuardRing";
      } else if ( nodeName.Contains( "pvChip" ) ) {
        sanitisedName += "_ASIC";
      } else if ( nodeName.Contains( "pvSubstrate" ) ) {
        sanitisedName += "_Substrate";
      }
    }

    return sanitisedName.Data();
  }

  /*
   * Fill the radiation and interaction length histograms for each material
   */
  void fillPerMaterialHistograms( std::map<std::string, TH2*>& hists_rad,
                                  std::map<std::string, TH2*>& hists_int, // both hists should have the same keys
                                  const TH2*                   histTemplate_rad,
                                  const TH2* histTemplate_int, // possibly different min and max for radiation and
                                                               // interaction length?
                                  const std::map<std::string, double>& lengths_rad,
                                  const std::map<std::string, double>& lengths_int, const double phi_deg,
                                  const double eta, const double ipSampleWeight ) {
    for ( auto& [matName, matRadLen] : lengths_rad ) {
      auto& matIntLen = lengths_int.at( matName );

      if ( hists_rad.find( matName ) == hists_rad.end() ) {
        // Create a new histogram for this material
        // for radiation length
        const std::string histName_rad{std::string{histTemplate_rad->GetName()} + "_" + matName};
        const std::string histTitle_rad{matName + " " + histTemplate_rad->GetTitle()};
        TH2*              tmpHist_rad = static_cast<TH2*>( histTemplate_rad->Clone( histName_rad.c_str() ) );
        tmpHist_rad->SetTitle( histTitle_rad.c_str() );
        tmpHist_rad->Reset();
        hists_rad[matName] = tmpHist_rad;
        // duplicate for interaction length
        const std::string histName_int{std::string{histTemplate_int->GetName()} + "_" + matName};
        const std::string histTitle_int{matName + " " + histTemplate_int->GetTitle()};
        TH2*              tmpHist_int = static_cast<TH2*>( histTemplate_int->Clone( histName_int.c_str() ) );
        tmpHist_int->SetTitle( histTitle_int.c_str() );
        tmpHist_int->Reset();
        hists_int[matName] = tmpHist_int;
      }

      hists_rad[matName]->Fill( phi_deg, eta, matRadLen * ipSampleWeight );
      hists_int[matName]->Fill( phi_deg, eta, matIntLen * ipSampleWeight );
    }
  }

  struct MaterialEntry {
    // name
    std::string name;
    // index
    std::size_t index;
    // path length (in mm)
    double thickness;
    // percentage of X_0
    double radLength;
    // percentage of material interaction length
    double intLength;
  };

  struct HitEntry {
    // index of corresponding entry in material list
    std::size_t index;
    // transverse radius sqrt(x^2 + y^2)
    double rho;
    // z position
    double z;
  };
} // namespace

/**
 * Plugin that creates histograms of the radiation and interaction length of each material in the eta vs phi plane
 * This particular plugin is customised to provide more detail for the VELO
 *
 * Example usage:
 *
 * geoPluginRun -input compact/components/debug/VP.xml -plugin rad_scan_etaphi_velo 300 360 290 835 100 1 0
 *
 * will do a scan in eta (300 points from 2.0 to 5.0) vs phi (360 points from -180 deg to 180 deg),
 * until reaching a maximum radius of 290mm or a maximum total distance of 835mm,
 * averaged over 100 samples of the PV z position (using a Gaussian distribution mu = 0, sigma = 53),
 * (if the fifth argument (100) is set to 1 then the origin is used rather than sampling)
 * and using a random seed of 1, with verbose mode off (0).
 *
 * This is intended to be used as an interactive tool to aid geometry development, rather than being used in automated
 * tests
 */
static long tgeo_rad_scan_etaphi_velo( dd4hep::Detector& /* description */, int argc, char** argv ) {

  unsigned int nPointsEta{300};
  const double etaLow{2.0};
  const double etaHigh{5.0};
  const double etaRange{etaHigh - etaLow};

  unsigned int nPointsPhi{360};
  const double phiLow{-180.0};
  const double phiHigh{+180.0};
  const double phiRange{phiHigh - phiLow};

  double maxRadius   = 290.0;
  double maxDistance = 835.0;

  double       meanIP     = 0.0;
  double       sigmaIP    = 53.0;
  unsigned int nIPSamples = 100;

  unsigned int seed = 0;

  bool verbose = false;
  bool help    = false;
  bool error   = false;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( 0 == ::strncmp( "-help", argv[i], 4 ) ) { help = true; }
  }

  const std::vector<std::string> cmdLine{argv, argv + argc};
  const std::size_t              nArgs{cmdLine.size()};

  if ( nArgs == 0 ) {
    // This is ok but we don't need to do anything!
  } else if ( nArgs == 7 ) {
    nPointsEta  = atoi( argv[0] );
    nPointsPhi  = atoi( argv[1] );
    maxRadius   = atof( argv[2] );
    maxDistance = atof( argv[3] );
    nIPSamples  = atoi( argv[4] );
    seed        = atoi( argv[5] );
    verbose     = atoi( argv[6] );
  } else {
    error = true;
  }

  if ( help || error ) {
    /// Help printout describing the basic command line interface
    std::cout
        << "Usage: -plugin <name>  npoints_eta npoints_phi max_radius max_distance n_ip_samples seed verbose\n"
           "     name:   factory name     rad_scan                                                                 \n"
           "     -help                    Show this help.                                                          \n"
           "\tArguments given: "
        << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  auto nav = gGeoManager->GetCurrentNavigator();

  TH2* histRadLen =
      new TH2F( "RadLen", "Total radiation length [%]", nPointsPhi, phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histBefore1stHitRadLen  = new TH2F( "RadLen_Before1stHit", "Radiation length [%] before 1st hit", nPointsPhi,
                                          phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histBefore2ndHitRadLen  = new TH2F( "RadLen_Before2ndHit", "Radiation length [%] before 2nd hit", nPointsPhi,
                                          phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histBeforeLastHitRadLen = new TH2F( "RadLen_BeforeLastHit", "Radiation length [%] before last hit", nPointsPhi,
                                           phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histIntLen =
      new TH2F( "IntLen", "Total interaction length [%]", nPointsPhi, phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histBefore1stHitIntLen  = new TH2F( "IntLen_Before1stHit", "Interaction length [%] before 1st hit", nPointsPhi,
                                          phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histBefore2ndHitIntLen  = new TH2F( "IntLen_Before2ndHit", "Interaction length [%] before 2nd hit", nPointsPhi,
                                          phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histBeforeLastHitIntLen = new TH2F( "IntLen_BeforeLastHit", "Interaction length [%] before last hit", nPointsPhi,
                                           phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histHitsPerTrack =
      new TH2F( "HitsPerTrack", "Number of hits per track", nPointsPhi, phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histRadius1stHit  = new TH2F( "Radius_1stHit", "Radius of the 1st hit [mm]", nPointsPhi, phiLow, phiHigh,
                                    nPointsEta, etaLow, etaHigh );
  TH2* histRadius2ndHit  = new TH2F( "Radius_2ndHit", "Radius of the 2nd hit [mm]", nPointsPhi, phiLow, phiHigh,
                                    nPointsEta, etaLow, etaHigh );
  TH2* histRadiusLastHit = new TH2F( "Radius_LastHit", "Radius of the last hit [mm]", nPointsPhi, phiLow, phiHigh,
                                     nPointsEta, etaLow, etaHigh );
  TH2* histZ1stHit =
      new TH2F( "Z_1stHit", "Z of the 1st hit [mm]", nPointsPhi, phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histZ2ndHit =
      new TH2F( "Z_2ndHit", "Z of the 2nd hit [mm]", nPointsPhi, phiLow, phiHigh, nPointsEta, etaLow, etaHigh );
  TH2* histZLastHit =
      new TH2F( "Z_LastHit", "Z of the last hit [mm]", nPointsPhi, phiLow, phiHigh, nPointsEta, etaLow, etaHigh );

  histRadLen->SetMaximum( 50.0 );
  histRadLen->SetMinimum( 0.0 );
  histBefore1stHitRadLen->SetMaximum( 20.0 );
  histBefore1stHitRadLen->SetMinimum( 0.0 );
  histBefore2ndHitRadLen->SetMaximum( 20.0 );
  histBefore2ndHitRadLen->SetMinimum( 0.0 );
  histBeforeLastHitRadLen->SetMaximum( 20.0 );
  histBeforeLastHitRadLen->SetMinimum( 0.0 );
  histIntLen->SetMaximum( 10.0 );
  histIntLen->SetMinimum( 0.0 );
  histBefore1stHitIntLen->SetMaximum( 20 );
  histBefore1stHitIntLen->SetMinimum( 0.0 );
  histBefore2ndHitIntLen->SetMaximum( 20 );
  histBefore2ndHitIntLen->SetMinimum( 0.0 );
  histBeforeLastHitIntLen->SetMaximum( 20 );
  histBeforeLastHitIntLen->SetMinimum( 0.0 );
  histHitsPerTrack->SetMaximum( 20 );
  histHitsPerTrack->SetMinimum( 0.0 );

  std::map<std::string, TH2*> matHists_RadLen_Total;
  std::map<std::string, TH2*> matHists_RadLen_1stHit;
  std::map<std::string, TH2*> matHists_RadLen_2ndHit;
  std::map<std::string, TH2*> matHists_RadLen_LastHit;

  std::map<std::string, TH2*> matHists_IntLen_Total;
  std::map<std::string, TH2*> matHists_IntLen_1stHit;
  std::map<std::string, TH2*> matHists_IntLen_2ndHit;
  std::map<std::string, TH2*> matHists_IntLen_LastHit;

  ROOT::Math::XYZPoint ip{0.0, 0.0, meanIP};
  const double         ipSampleWeight{1.0 / nIPSamples};

  double eta{0.};
  // double theta_deg{0.};
  double theta_rad{0.};
  double cosTheta{0.};
  double sinTheta{0.};
  double phi_deg{0.};
  double phi_rad{0.};
  double cosPhi{0.};
  double sinPhi{0.};

  // Store the different materials encountered along the path:
  std::vector<MaterialEntry> materialList;

  // Store the hits on each sensor volume
  std::vector<HitEntry> hitList;

  for ( std::size_t i = 0; i < nIPSamples; ++i ) {
    if ( nIPSamples > 1 ) {
      TRandom3 random{seed};
      ip.SetZ( random.Gaus( meanIP, sigmaIP ) );
    }

    for ( std::size_t j = 0; j < nPointsPhi; ++j ) {
      phi_deg = phiLow + ( j + 0.5 ) * phiRange / nPointsPhi;
      phi_rad = phi_deg / 180. * TMath::Pi();
      cosPhi  = cos( phi_rad );
      sinPhi  = sin( phi_rad );

      for ( std::size_t k = 0; k < nPointsEta; ++k ) {
        eta       = etaLow + ( k + 0.5 ) * etaRange / nPointsEta;
        theta_rad = 2.0 * atan( exp( -eta ) );
        // theta_deg = theta_rad * 180. / TMath::Pi();
        cosTheta = cos( theta_rad );
        sinTheta = sin( theta_rad );

        ROOT::Math::XYZVector direction{sinTheta * cosPhi, sinTheta * sinPhi, cosTheta};
        nav->SetCurrentPoint( ip.x(), ip.y(), ip.z() );
        nav->SetCurrentDirection( direction.x(), direction.y(), direction.z() );

        hitList.clear();
        materialList.clear();

        bool navFinished = false;
        nav->FindNode();
        while ( !navFinished ) {
          const double*     ipos{nav->GetCurrentPoint()};
          const std::string nodeName{nav->GetCurrentNode()->GetName()};
          const std::string matName{nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetName()};
          const std::string taggedMatName{tagMatName( matName, nodeName )};
          const double      nodeRadLength{nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetRadLen()};
          const double      nodeIntLength{nav->GetCurrentNode()->GetVolume()->GetMaterial()->GetIntLen()};

          const bool isSensor{taggedMatName.find( "_Sensor" ) != std::string::npos};

          // TODO - foil scaling

          // Verbose printout
          std::ostringstream os;
          os << ipos[0] << ", " << ipos[1] << ", " << ipos[2] << " " << nodeName << " - " << matName << " - "
             << taggedMatName << " with radlen = " << nodeRadLength << " and intlen = " << nodeIntLength;
          if ( isSensor ) { os << " - A SENSOR!"; }
          if ( verbose ) { dd4hep::printout( dd4hep::INFO, "rad_scan", "%s", os.str().c_str() ); }

          const ROOT::Math::XYZPoint initPos{ipos[0], ipos[1], ipos[2]};

          nav->FindNextBoundaryAndStep();
          const double* npos = nav->GetCurrentPoint();

          const ROOT::Math::XYZPoint newPos{npos[0], npos[1], npos[2]};

          const ROOT::Math::XYZPoint avgPos{( ROOT::Math::XYZVector{initPos} + ROOT::Math::XYZVector{newPos} ) / 2.0};

          const double currentThick{( newPos - initPos ).r()};
          const double currentRadPercent{100.0 * currentThick / nodeRadLength};
          const double currentIntPercent{100.0 * currentThick / nodeIntLength};

          const std::size_t index{materialList.size()};

          if ( isSensor ) { hitList.push_back( {index, avgPos.rho(), avgPos.z()} ); }

          materialList.push_back( {taggedMatName, index, currentThick, currentRadPercent, currentIntPercent} );

          navFinished = ( ( newPos.z() > maxDistance ) || ( newPos.rho() > maxRadius ) );
        }

        // Fill hit information histograms
        const std::size_t nHits{hitList.size()};
        histHitsPerTrack->Fill( phi_deg, eta, nHits * ipSampleWeight );
        if ( nHits > 0 ) {
          // fill 1st hit info
          histRadius1stHit->Fill( phi_deg, eta, hitList.front().rho * ipSampleWeight );
          histZ1stHit->Fill( phi_deg, eta, hitList.front().z * ipSampleWeight );
          // fill last hit info
          histRadiusLastHit->Fill( phi_deg, eta, hitList.back().rho * ipSampleWeight );
          histZLastHit->Fill( phi_deg, eta, hitList.back().z * ipSampleWeight );
          if ( nHits > 1 ) {
            // fill 2nd hit info
            histRadius2ndHit->Fill( phi_deg, eta, hitList[1].rho * ipSampleWeight );
            histZ2ndHit->Fill( phi_deg, eta, hitList[1].z * ipSampleWeight );
          }
        }

        const std::size_t index1stHit{nHits > 0 ? hitList.front().index : -1};
        const std::size_t index2ndHit{nHits > 1 ? hitList[1].index : -1};
        const std::size_t indexLastHit{nHits > 0 ? hitList.back().index : -1};

        // Fill radiation length histograms
        double                        totalRadLen{0.0};
        double                        totalIntLen{0.0};
        std::map<std::string, double> perMaterialRadLen;
        std::map<std::string, double> perMaterialIntLen;

        for ( auto& [matName, matIndex, matThick, matRadLen, matIntLen] : materialList ) {

          if ( matIndex == index1stHit ) {
            histBefore1stHitRadLen->Fill( phi_deg, eta, totalRadLen * ipSampleWeight );
            histBefore1stHitIntLen->Fill( phi_deg, eta, totalIntLen * ipSampleWeight );
            fillPerMaterialHistograms( matHists_RadLen_1stHit, matHists_IntLen_1stHit, histBefore1stHitRadLen,
                                       histBefore1stHitIntLen, perMaterialRadLen, perMaterialIntLen, phi_deg, eta,
                                       ipSampleWeight );
          }
          if ( matIndex == index2ndHit ) {
            histBefore2ndHitRadLen->Fill( phi_deg, eta, totalRadLen * ipSampleWeight );
            histBefore2ndHitIntLen->Fill( phi_deg, eta, totalIntLen * ipSampleWeight );
            fillPerMaterialHistograms( matHists_RadLen_2ndHit, matHists_IntLen_2ndHit, histBefore2ndHitRadLen,
                                       histBefore2ndHitIntLen, perMaterialRadLen, perMaterialIntLen, phi_deg, eta,
                                       ipSampleWeight );
          }
          if ( matIndex == indexLastHit ) {
            histBeforeLastHitRadLen->Fill( phi_deg, eta, totalRadLen * ipSampleWeight );
            histBeforeLastHitIntLen->Fill( phi_deg, eta, totalIntLen * ipSampleWeight );
            fillPerMaterialHistograms( matHists_RadLen_LastHit, matHists_IntLen_LastHit, histBeforeLastHitRadLen,
                                       histBeforeLastHitIntLen, perMaterialRadLen, perMaterialIntLen, phi_deg, eta,
                                       ipSampleWeight );
          }

          if ( perMaterialRadLen.find( matName ) == perMaterialRadLen.end() ) {
            perMaterialRadLen[matName] = matRadLen;
            perMaterialIntLen[matName] = matIntLen;
          } else {
            perMaterialRadLen[matName] += matRadLen;
            perMaterialIntLen[matName] += matIntLen;
          }

          totalRadLen += matRadLen;
          totalIntLen += matIntLen;
        }

        // Fill total radlen histograms - total and per-material
        histRadLen->Fill( phi_deg, eta, totalRadLen * ipSampleWeight );
        histIntLen->Fill( phi_deg, eta, totalIntLen * ipSampleWeight );
        fillPerMaterialHistograms( matHists_RadLen_Total, matHists_IntLen_Total, histRadLen, histIntLen,
                                   perMaterialRadLen, perMaterialIntLen, phi_deg, eta, ipSampleWeight );
      }
    }
  }

  TFile* out_file = new TFile( "rad_scan_etaphi_velo.root", "recreate" );
  histRadLen->Write();
  histBefore1stHitRadLen->Write();
  histBefore2ndHitRadLen->Write();
  histBeforeLastHitRadLen->Write();
  histIntLen->Write();
  histBefore1stHitIntLen->Write();
  histBefore2ndHitIntLen->Write();
  histBeforeLastHitIntLen->Write();
  histHitsPerTrack->Write();
  histRadius1stHit->Write();
  histRadius2ndHit->Write();
  histRadiusLastHit->Write();
  histZ1stHit->Write();
  histZ2ndHit->Write();
  histZLastHit->Write();
  for ( auto& [_, hist] : matHists_RadLen_Total ) { hist->Write(); }
  for ( auto& [_, hist] : matHists_RadLen_1stHit ) { hist->Write(); }
  for ( auto& [_, hist] : matHists_RadLen_2ndHit ) { hist->Write(); }
  for ( auto& [_, hist] : matHists_RadLen_LastHit ) { hist->Write(); }
  for ( auto& [_, hist] : matHists_IntLen_Total ) { hist->Write(); }
  for ( auto& [_, hist] : matHists_IntLen_1stHit ) { hist->Write(); }
  for ( auto& [_, hist] : matHists_IntLen_2ndHit ) { hist->Write(); }
  for ( auto& [_, hist] : matHists_IntLen_LastHit ) { hist->Write(); }
  out_file->Close();
  std::cout << "Saved histogram to rad_scan_etaphi_velo.root" << std::endl;

  return 1;
}
DECLARE_APPLY( rad_scan_etaphi_velo, tgeo_rad_scan_etaphi_velo )
