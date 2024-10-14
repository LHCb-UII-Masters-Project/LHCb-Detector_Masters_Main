/*****************************************************************************\
* (c) Copyright 2021 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#pragma once

#include "../../../Core/include/Core/UpgradeTags.h"
#include "DD4hep/DetElement.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "DDCond/ConditionsTags.h"
#include "XML/XMLElements.h"
#include <map>
#include <numeric>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Small helper function
std::string GetLogVolName( std::string logvol_path ) {

  // Get position of the logvol name in the string
  std::size_t match   = logvol_path.find( "/lv" );
  std::string lv_name = logvol_path.substr( match + 1, logvol_path.size() - match + 1 );

  return lv_name;
}

// Check for the existence of a logvol in the map
// The compact description of each filter is self sufficient, so that each filter can be build indipendently from the
// presence of others This lead to volume redundancies. Avoid to add the same volume multiple times using this function
bool FindLogVolInMap( std::map<std::string, dd4hep::Volume>& volumes_map, std::string logvol_name ) {

  bool                                            match = false;
  std::map<std::string, dd4hep::Volume>::iterator itr;
  for ( itr = volumes_map.begin(); itr != volumes_map.end(); itr++ ) {

    std::string map_el_name = itr->first;
    if ( map_el_name.compare( logvol_name ) == 0 ) {
      match = true;
      break;
    }
  }

  return match;
}

// Helper function to sum two vectors component by component and store the results in a vector
std::vector<double> SumVectors( std::vector<double> pv_vect, std::vector<double> transf_vect ) {

  std::vector<double> sum( 3 );
  for ( int i = 0; i < 3; i++ ) { sum[i] = pv_vect[i] + transf_vect[i]; }

  return sum;
}

// Helper function to sum two vectors component by component and store the results in a vector
void SumVectors( std::vector<double>& pv_vect, std::vector<double>& transf_vect, std::vector<double>& sum ) {

  for ( int i = 0; i < 3; i++ ) { sum[i] = pv_vect[i] + transf_vect[i]; }
}

// Function to parse all the compact descriptions that contain paramphysvol volumes.
// Store the logical volumes in a map to be used later to put togethere higher level logvols
void ParseAndLoadVols( std::map<std::string, dd4hep::Volume>& volumes_map, xml_comp_t logvol,
                       dd4hep::Assembly lvMotherVol ) {

  //  ************************************************  //
  //      Load volumes inside the paramphysvol tag      //
  //  ************************************************  //
  for ( xml_coll_t ppv1D( logvol, _LBU( paramphysvol ) ); ppv1D; ppv1D++ ) {

    std::vector<double> PhysVolPos( 3 );
    std::vector<double> TransfPos( 3 );
    double              PhysVolRotAng, TransfRotAng;
    bool                PhysVolRotY = false;
    bool                PhysVolRotZ = false;
    bool                TransfRotY  = false;
    bool                TransfRotZ  = false;

    xml_comp_t parphysvol1D = ppv1D;

    unsigned int     nD = 1;
    std::vector<int> numbers( nD );
    numbers[0] = parphysvol1D.attr<unsigned int>( _U( number ) );
    // Number of volumes to place(replicas of the physvol inside)
    unsigned int nV = std::accumulate( numbers.begin(), numbers.end(), 1u, std::multiplies<unsigned int>() ); //

    // First, evaluate the physvol tag
    xml_comp_t  physvol        = parphysvol1D.child( _U( physvol ) );
    std::string physvol_name   = physvol.nameStr();
    std::string pv_logvol_path = physvol.attr<std::string>( _U( logvol ) );
    std::string pv_logvol_name = GetLogVolName( pv_logvol_path );

    xml_dim_t physvol_pos = physvol.child( _LBU( posXYZ ) );
    if ( physvol_pos.hasAttr( _U( x ) ) ) {
      PhysVolPos[0] = physvol_pos.x();
    } else {
      PhysVolPos[0] = 0.;
    }
    if ( physvol_pos.hasAttr( _U( y ) ) ) {
      PhysVolPos[1] = physvol_pos.y();
    } else {
      PhysVolPos[1] = 0.;
    }
    if ( physvol_pos.hasAttr( _U( z ) ) ) {
      PhysVolPos[2] = physvol_pos.z();
    } else {
      PhysVolPos[2] = 0.;
    }

    // Check if there is a rotation to be applied to the physvol. No rotations around the x axix.
    if ( physvol.hasChild( _LBU( rotXYZ ) ) ) {

      xml_dim_t PhysVolRot = physvol.child( _LBU( rotXYZ ) );
      if ( PhysVolRot.hasAttr( _LBU( rotY ) ) ) {
        PhysVolRotY   = true;
        PhysVolRotAng = PhysVolRot.attr<double>( _LBU( rotY ) );
      } else {
        PhysVolRotZ   = true;
        PhysVolRotAng = PhysVolRot.attr<double>( _LBU( rotZ ) );
      }

    } else {
      PhysVolRotAng = 0.;
    }

    // Get the tranformation step
    xml_dim_t transf_pos = parphysvol1D.child( _LBU( posXYZ ) );
    if ( transf_pos.hasAttr( _U( x ) ) ) {
      TransfPos[0] = transf_pos.x();
    } else {
      TransfPos[0] = 0.;
    }
    if ( transf_pos.hasAttr( _U( y ) ) ) {
      TransfPos[1] = transf_pos.y();
    } else {
      TransfPos[1] = 0.;
    }
    if ( transf_pos.hasAttr( _U( z ) ) ) {
      TransfPos[2] = transf_pos.z();
    } else {
      TransfPos[2] = 0.;
    }

    // Check if there is an extra rotation to apply
    if ( parphysvol1D.hasChild( _LBU( rotXYZ ) ) ) {

      xml_dim_t TransfRot = parphysvol1D.child( _LBU( rotXYZ ) );
      if ( TransfRot.hasAttr( _LBU( rotY ) ) ) {
        TransfRotY   = true;
        TransfRotAng = TransfRot.attr<double>( _LBU( rotY ) );
      } else {
        TransfRotZ   = true;
        TransfRotAng = TransfRot.attr<double>( _LBU( rotZ ) );
      }

    } else {
      TransfRotAng = 0.;
    }

    // Make a pair with the translation step and the rot angle
    std::vector<std::pair<std::vector<double>, double>> stepTransRot( nD );
    // auto& p = stepTransRot[0];
    // p = std::make_pair(TransfPos, TransfRotAng);
    stepTransRot[0] = std::make_pair( TransfPos, TransfRotAng );

    // Make vectors to store all the translation steps and rotation to apply to the nV volumes
    std::vector<std::vector<double>> translations( nV );
    std::vector<double>              rotations( nV );

    // Fill vectors. Same loop as in the DetDesc code:
    // https://gitlab.cern.ch/lhcb/LHCb/-/blob/master/Det/DetDescCnv/src/component/XmlLVolumeCnv.cpp#L685-697
    {
      // Zero comp of translations and rotations must be initialized by hand to a null tranformation
      std::vector<double> no_transf( 3, 0. );
      translations[0] = no_transf;
      rotations[0]    = 0.;

      unsigned int numberOfItems = 1;
      for ( unsigned int dimension = 0; dimension < nD; dimension++ ) {
        const auto& transRot = stepTransRot[dimension];
        for ( unsigned int item = 0; item < numberOfItems; item++ ) {
          for ( int i = 1; i < numbers[dimension]; i++ ) {
            translations[numberOfItems * i + item] =
                SumVectors( transRot.first, translations[numberOfItems * ( i - 1 ) + item] );
            rotations[numberOfItems * i + item] = transRot.second + rotations[numberOfItems * ( i - 1 ) + item];
          }
        }
        numberOfItems *= numbers[dimension];
      }
    }

    // Now place all these volumes
    dd4hep::Volume PhysVol( physvol_name );
    PhysVol = volumes_map.find( pv_logvol_name )->second;
    std::vector<double> ReplicaPos( 3 );
    for ( unsigned int i = 0; i < nV; i++ ) {

      SumVectors( PhysVolPos, translations[i], ReplicaPos );
      dd4hep::Position Pos( ReplicaPos[0], ReplicaPos[1], ReplicaPos[2] );

      // There are no rotation around the X axix in the compact description of this logvol.
      // Also, there can only be one rotation tag, either inside the physvol tag or outside.
      if ( PhysVolRotY ) {
        lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationY( PhysVolRotAng ), Pos ) );
      } else if ( PhysVolRotZ ) {
        lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationZ( PhysVolRotAng ), Pos ) );
      } else if ( TransfRotY ) {
        lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationY( rotations[i] ), Pos ) );
      } else if ( TransfRotZ ) {
        lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationZ( rotations[i] ), Pos ) );
      } else {
        lvMotherVol.placeVolume( PhysVol, Pos );
      }
    }

    // For debug purposes only
    // std::cout << "<paramphysvol>" << std::endl;
    // std::cout << "physvol name = " << physvol_name << ", logvol name = " << pv_logvol_name << std::endl;
    // std::cout << "physvol posXYZ: x = " << PhysVolPos[0] << ", y = " << PhysVolPos[1] << ", z = " << PhysVolPos[2]
    //           << std::endl;
    // if ( PhysVolRotY ) {
    //   std::cout << "physvol rotXYZ: rotY = " << PhysVolRotAng << std::endl;
    // } else if ( PhysVolRotY ) {
    //   std::cout << "physvol rotXYZ: rotZ = " << PhysVolRotAng << std::endl;
    // }
    // std::cout << "transf  posXYZ: x = " << TransfPos[0] << ", y = " << TransfPos[1] << ", z = " << TransfPos[2]
    //           << std::endl;
    // if ( TransfRotY ) {
    //   std::cout << "transf  rotXYZ: rotY = " << TransfRotAng << std::endl;
    // } else if ( TransfRotZ ) {
    //   std::cout << "transf  rotXYZ: rotZ = " << TransfRotAng << std::endl;
    // }
    // std::cout << "</paramphysvol>" << std::endl;

  } // End of loop on <paramphysvol> tag

  //  **************************************************  //
  //      Load volumes inside the paramphysvol2D tag      //
  //  **************************************************  //
  for ( xml_coll_t ppv2D( logvol, _LBU( paramphysvol2D ) ); ppv2D; ppv2D++ ) {

    std::vector<double> PhysVolPos( 3 );
    std::vector<double> TransfPos1( 3 );
    std::vector<double> TransfPos2( 3 );
    double              PhysVolRotAng, TransfRotAng;
    bool                PhysVolRotY = false;
    bool                PhysVolRotZ = false;
    bool                TransfRotY  = false;
    bool                TransfRotZ  = false;

    xml_comp_t parphysvol2D = ppv2D;

    unsigned int     nD = 2;
    std::vector<int> numbers( nD );
    numbers[0] = parphysvol2D.attr<unsigned int>( _LBU( number1 ) );
    numbers[1] = parphysvol2D.attr<unsigned int>( _LBU( number2 ) );
    // Number of volumes to place(replicas of the physvol inside)
    unsigned int nV = std::accumulate( numbers.begin(), numbers.end(), 1u, std::multiplies<unsigned int>() );

    // First, evaluate the physvol tag
    xml_comp_t  physvol        = parphysvol2D.child( _U( physvol ) );
    std::string physvol_name   = physvol.nameStr();
    std::string pv_logvol_path = physvol.attr<std::string>( _U( logvol ) );
    std::string pv_logvol_name = GetLogVolName( pv_logvol_path );

    xml_dim_t physvol_pos = physvol.child( _LBU( posXYZ ) );
    if ( physvol_pos.hasAttr( _U( x ) ) ) {
      PhysVolPos[0] = physvol_pos.x();
    } else {
      PhysVolPos[0] = 0.;
    }
    if ( physvol_pos.hasAttr( _U( y ) ) ) {
      PhysVolPos[1] = physvol_pos.y();
    } else {
      PhysVolPos[1] = 0.;
    }
    if ( physvol_pos.hasAttr( _U( z ) ) ) {
      PhysVolPos[2] = physvol_pos.z();
    } else {
      PhysVolPos[2] = 0.;
    }

    // Check if there is a rotation to be applied to the physvol
    if ( physvol.hasChild( _LBU( rotXYZ ) ) ) {

      xml_dim_t PhysVolRot = physvol.child( _LBU( rotXYZ ) );
      if ( PhysVolRot.hasAttr( _LBU( rotY ) ) ) {
        PhysVolRotY   = true;
        PhysVolRotAng = PhysVolRot.attr<double>( _LBU( rotY ) );
      } else {
        PhysVolRotZ   = true;
        PhysVolRotAng = PhysVolRot.attr<double>( _LBU( rotZ ) );
      }

    } else {
      PhysVolRotAng = 0.;
    }

    // Get the tranformation steps, two in this case
    int step_index = 1;
    for ( xml_coll_t pos( parphysvol2D, _LBU( posXYZ ) ); pos; pos++ ) {
      xml_dim_t transf_pos = pos;

      if ( step_index == 1 ) {
        if ( transf_pos.hasAttr( _U( x ) ) ) {
          TransfPos1[0] = transf_pos.x();
        } else {
          TransfPos1[0] = 0.;
        }
        if ( transf_pos.hasAttr( _U( y ) ) ) {
          TransfPos1[1] = transf_pos.y();
        } else {
          TransfPos1[1] = 0.;
        }
        if ( transf_pos.hasAttr( _U( z ) ) ) {
          TransfPos1[2] = transf_pos.z();
        } else {
          TransfPos1[2] = 0.;
        }
      } else {
        if ( transf_pos.hasAttr( _U( x ) ) ) {
          TransfPos2[0] = transf_pos.x();
        } else {
          TransfPos2[0] = 0.;
        }
        if ( transf_pos.hasAttr( _U( y ) ) ) {
          TransfPos2[1] = transf_pos.y();
        } else {
          TransfPos2[1] = 0.;
        }
        if ( transf_pos.hasAttr( _U( z ) ) ) {
          TransfPos2[2] = transf_pos.z();
        } else {
          TransfPos2[2] = 0.;
        }
      }

      step_index++;
    }

    // Check if there is an extra rotation to apply
    if ( parphysvol2D.hasChild( _LBU( rotXYZ ) ) ) {

      xml_dim_t TransfRot = parphysvol2D.child( _LBU( rotXYZ ) );
      if ( TransfRot.hasAttr( _LBU( rotY ) ) ) {
        TransfRotY   = true;
        TransfRotAng = TransfRot.attr<double>( _LBU( rotY ) );
      } else {
        TransfRotZ   = true;
        TransfRotAng = TransfRot.attr<double>( _LBU( rotZ ) );
      }

    } else {
      TransfRotAng = 0.;
    }

    // Make a pair with the translation step and the rot angle
    std::vector<std::pair<std::vector<double>, double>> stepTransRot( nD );
    stepTransRot[0] = std::make_pair( TransfPos1, TransfRotAng );
    stepTransRot[1] = std::make_pair( TransfPos2, TransfRotAng );

    // Make vectors to store all the translation steps and rotation to apply to the nV volumes
    std::vector<std::vector<double>> translations( nV );
    std::vector<double>              rotations( nV );

    // Fill vectors. Same loop as in the DetDesc code:
    // https://gitlab.cern.ch/lhcb/LHCb/-/blob/master/Det/DetDescCnv/src/component/XmlLVolumeCnv.cpp#L685-697
    {
      // Zero comp of translations and rotations must be initialized by hand to a null tranformation
      std::vector<double> no_transf( 3, 0. );
      translations[0] = no_transf;
      rotations[0]    = 0.;

      unsigned int numberOfItems = 1;
      for ( unsigned int dimension = 0; dimension < nD; dimension++ ) {
        const auto& transRot = stepTransRot[dimension];
        for ( unsigned int item = 0; item < numberOfItems; item++ ) {
          for ( int i = 1; i < numbers[dimension]; i++ ) {
            translations[numberOfItems * i + item] =
                SumVectors( transRot.first, translations[numberOfItems * ( i - 1 ) + item] );
            rotations[numberOfItems * i + item] = transRot.second + rotations[numberOfItems * ( i - 1 ) + item];
          }
        }
        numberOfItems *= numbers[dimension];
      }
    }

    // Now place all these volumes
    dd4hep::Volume PhysVol( physvol_name );
    PhysVol = volumes_map.find( pv_logvol_name )->second;
    std::vector<double> ReplicaPos( 3 );
    for ( unsigned int i = 0; i < nV; i++ ) {

      SumVectors( PhysVolPos, translations[i], ReplicaPos );
      dd4hep::Position Pos( ReplicaPos[0], ReplicaPos[1], ReplicaPos[2] );

      // There are no rotation around the X axix in the compact description of this logvol.
      // Also, there can only be one rotation tag, either inside the physvol tag or outside.
      if ( PhysVolRotY ) {
        lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationY( PhysVolRotAng ), Pos ) );
      } else if ( PhysVolRotZ ) {
        lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationZ( PhysVolRotAng ), Pos ) );
      } else if ( TransfRotY ) {
        lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationY( rotations[i] ), Pos ) );
      } else if ( TransfRotZ ) {
        lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationZ( rotations[i] ), Pos ) );
      } else {
        lvMotherVol.placeVolume( PhysVol, Pos );
      }
    }

    // For debug purposes only
    // std::cout << "<paramphysvol2D>" << std::endl;
    // std::cout << "physvol name = " << physvol_name << ", logvol name = " << pv_logvol_name << std::endl;
    // std::cout << "physvol posXYZ: x = " << PhysVolPos[0] << ", y = " << PhysVolPos[1] << ", z = " << PhysVolPos[2]
    //           << std::endl;
    // if ( PhysVolRotY ) {
    //   std::cout << "physvol rotXYZ: rotY = " << PhysVolRotAng << std::endl;
    // } else if ( PhysVolRotY ) {
    //   std::cout << "physvol rotXYZ: rotZ = " << PhysVolRotAng << std::endl;
    // }
    // std::cout << "transf1 posXYZ: x = " << TransfPos1[0] << ", y = " << TransfPos1[1] << ", z = " << TransfPos1[2]
    //           << std::endl;
    // std::cout << "transf2 posXYZ: x = " << TransfPos2[0] << ", y = " << TransfPos2[1] << ", z = " << TransfPos2[2]
    //           << std::endl;
    // if ( TransfRotY ) {
    //   std::cout << "transf  rotXYZ: rotY = " << TransfRotAng << std::endl;
    // } else if ( TransfRotZ ) {
    //   std::cout << "transf  rotXYZ: rotZ = " << TransfRotAng << std::endl;
    // }
    // std::cout << "</paramphysvol2D>" << std::endl;

  } // End of loop on <paramphysvol2D> tag

  //  **************************************************  //
  //      Load volumes inside the paramphysvol3D tag      //
  //  **************************************************  //

  // Check if there are any <paramphysvol3d> tags
  if ( logvol.hasChild( _LBU( paramphysvol3D ) ) ) {

    for ( xml_coll_t ppv3D( logvol, _LBU( paramphysvol3D ) ); ppv3D; ppv3D++ ) {

      std::vector<double> PhysVolPos( 3 );
      std::vector<double> TransfPos1( 3 );
      std::vector<double> TransfPos2( 3 );
      std::vector<double> TransfPos3( 3 );
      double              PhysVolRotAng, TransfRotAng;
      bool                PhysVolRotY = false;
      bool                PhysVolRotZ = false;
      bool                TransfRotY  = false;
      bool                TransfRotZ  = false;

      xml_comp_t parphysvol3D = ppv3D;

      // Get the number of replicas to place
      unsigned int     nD = 3;
      std::vector<int> numbers( nD );
      numbers[0] = parphysvol3D.attr<unsigned int>( _LBU( number1 ) );
      numbers[1] = parphysvol3D.attr<unsigned int>( _LBU( number2 ) );
      numbers[2] = parphysvol3D.attr<unsigned int>( _LBU( number3 ) );
      // Number of volumes to place(replicas of the physvol inside)
      unsigned int nV = std::accumulate( numbers.begin(), numbers.end(), 1u, std::multiplies<unsigned int>() );

      // First, evaluate the physvol tag
      xml_comp_t  physvol        = parphysvol3D.child( _U( physvol ) );
      std::string physvol_name   = physvol.nameStr();
      std::string pv_logvol_path = physvol.attr<std::string>( _U( logvol ) );
      std::string pv_logvol_name = GetLogVolName( pv_logvol_path );

      xml_dim_t physvol_pos = physvol.child( _LBU( posXYZ ) );
      if ( physvol_pos.hasAttr( _U( x ) ) ) {
        PhysVolPos[0] = physvol_pos.x();
      } else {
        PhysVolPos[0] = 0.;
      }
      if ( physvol_pos.hasAttr( _U( y ) ) ) {
        PhysVolPos[1] = physvol_pos.y();
      } else {
        PhysVolPos[1] = 0.;
      }
      if ( physvol_pos.hasAttr( _U( z ) ) ) {
        PhysVolPos[2] = physvol_pos.z();
      } else {
        PhysVolPos[2] = 0.;
      }

      // Check if there is a rotation to be applied to the physvol
      if ( physvol.hasChild( _LBU( rotXYZ ) ) ) {

        xml_dim_t PhysVolRot = physvol.child( _LBU( rotXYZ ) );
        if ( PhysVolRot.hasAttr( _LBU( rotY ) ) ) {
          PhysVolRotY   = true;
          PhysVolRotAng = PhysVolRot.attr<double>( _LBU( rotY ) );
        } else {
          PhysVolRotZ   = true;
          PhysVolRotAng = PhysVolRot.attr<double>( _LBU( rotZ ) );
        }

      } else {
        PhysVolRotAng = 0.;
      }

      // Get the tranformation steps, two in this case
      int step_index = 1;
      for ( xml_coll_t pos( parphysvol3D, _LBU( posXYZ ) ); pos; pos++ ) {
        xml_dim_t transf_pos = pos;

        if ( step_index == 1 ) {
          if ( transf_pos.hasAttr( _U( x ) ) ) {
            TransfPos1[0] = transf_pos.x();
          } else {
            TransfPos1[0] = 0.;
          }
          if ( transf_pos.hasAttr( _U( y ) ) ) {
            TransfPos1[1] = transf_pos.y();
          } else {
            TransfPos1[1] = 0.;
          }
          if ( transf_pos.hasAttr( _U( z ) ) ) {
            TransfPos1[2] = transf_pos.z();
          } else {
            TransfPos1[2] = 0.;
          }
        } else if ( step_index == 2 ) {
          if ( transf_pos.hasAttr( _U( x ) ) ) {
            TransfPos2[0] = transf_pos.x();
          } else {
            TransfPos2[0] = 0.;
          }
          if ( transf_pos.hasAttr( _U( y ) ) ) {
            TransfPos2[1] = transf_pos.y();
          } else {
            TransfPos2[1] = 0.;
          }
          if ( transf_pos.hasAttr( _U( z ) ) ) {
            TransfPos2[2] = transf_pos.z();
          } else {
            TransfPos2[2] = 0.;
          }
        } else {
          if ( transf_pos.hasAttr( _U( x ) ) ) {
            TransfPos3[0] = transf_pos.x();
          } else {
            TransfPos3[0] = 0.;
          }
          if ( transf_pos.hasAttr( _U( y ) ) ) {
            TransfPos3[1] = transf_pos.y();
          } else {
            TransfPos3[1] = 0.;
          }
          if ( transf_pos.hasAttr( _U( z ) ) ) {
            TransfPos3[2] = transf_pos.z();
          } else {
            TransfPos3[2] = 0.;
          }
        }

        step_index++;
      }

      // Check if there is an extra rotation to apply
      if ( parphysvol3D.hasChild( _LBU( rotXYZ ) ) ) {

        xml_dim_t TransfRot = parphysvol3D.child( _LBU( rotXYZ ) );
        if ( TransfRot.hasAttr( _LBU( rotY ) ) ) {
          TransfRotY   = true;
          TransfRotAng = TransfRot.attr<double>( _LBU( rotY ) );
        } else {
          TransfRotZ   = true;
          TransfRotAng = TransfRot.attr<double>( _LBU( rotZ ) );
        }

      } else {
        TransfRotAng = 0.;
      }

      // Make a pair with the translation step and the rot angle
      std::vector<std::pair<std::vector<double>, double>> stepTransRot( nD );
      stepTransRot[0] = std::make_pair( TransfPos1, TransfRotAng );
      stepTransRot[1] = std::make_pair( TransfPos2, TransfRotAng );
      stepTransRot[2] = std::make_pair( TransfPos3, TransfRotAng );

      // Make vectors to store all the translation steps and rotation to apply to the nV volumes
      std::vector<std::vector<double>> translations( nV );
      std::vector<double>              rotations( nV );

      // Fill vectors. Same loop as in the DetDesc code:
      // https://gitlab.cern.ch/lhcb/LHCb/-/blob/master/Det/DetDescCnv/src/component/XmlLVolumeCnv.cpp#L685-697
      {
        // Zero comp of translations and rotations must be initialized by hand to a null tranformation
        std::vector<double> no_transf( 3, 0. );
        translations[0] = no_transf;
        rotations[0]    = 0.;

        unsigned int numberOfItems = 1;
        for ( unsigned int dimension = 0; dimension < nD; dimension++ ) {
          const auto& transRot = stepTransRot[dimension];
          for ( unsigned int item = 0; item < numberOfItems; item++ ) {
            for ( int i = 1; i < numbers[dimension]; i++ ) {
              translations[numberOfItems * i + item] =
                  SumVectors( transRot.first, translations[numberOfItems * ( i - 1 ) + item] );
              rotations[numberOfItems * i + item] = transRot.second + rotations[numberOfItems * ( i - 1 ) + item];
            }
          }
          numberOfItems *= numbers[dimension];
        }
      }

      // Now place all these volumes
      dd4hep::Volume PhysVol( physvol_name );
      PhysVol = volumes_map.find( pv_logvol_name )->second;
      std::vector<double> ReplicaPos( 3 );
      for ( unsigned int i = 0; i < nV; i++ ) {

        SumVectors( PhysVolPos, translations[i], ReplicaPos );
        dd4hep::Position Pos( ReplicaPos[0], ReplicaPos[1], ReplicaPos[2] );

        // There are no rotation around the X axix in the compact description of this logvol.
        // Also, there can only be one rotation tag, either inside the physvol tag or outside.
        if ( PhysVolRotY ) {
          lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationY( PhysVolRotAng ), Pos ) );
        } else if ( PhysVolRotZ ) {
          lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationZ( PhysVolRotAng ), Pos ) );
        } else if ( TransfRotY ) {
          lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationY( rotations[i] ), Pos ) );
        } else if ( TransfRotZ ) {
          lvMotherVol.placeVolume( PhysVol, dd4hep::Transform3D( dd4hep::RotationZ( rotations[i] ), Pos ) );
        } else {
          lvMotherVol.placeVolume( PhysVol, Pos );
        }
      }

      // For debug purposes only
      // std::cout << "<paramphysvol3D>" << std::endl;
      // std::cout << "physvol name = " << physvol_name << ", logvol name = " << pv_logvol_name << std::endl;
      // std::cout << "physvol posXYZ: x = " << PhysVolPos[0] << ", y = " << PhysVolPos[1] << ", z = " << PhysVolPos[2]
      //           << std::endl;
      // if ( PhysVolRotY ) {
      //   std::cout << "physvol rotXYZ: rotY = " << PhysVolRotAng << std::endl;
      // } else if ( PhysVolRotY ) {
      //   std::cout << "physvol rotXYZ: rotZ = " << PhysVolRotAng << std::endl;
      // }
      // std::cout << "transf1 posXYZ: x = " << TransfPos1[0] << ", y = " << TransfPos1[1] << ", z = " << TransfPos1[2]
      //           << std::endl;
      // std::cout << "transf2 posXYZ: x = " << TransfPos2[0] << ", y = " << TransfPos2[1] << ", z = " << TransfPos2[2]
      //           << std::endl;
      // std::cout << "transf3 posXYZ: x = " << TransfPos3[0] << ", y = " << TransfPos3[1] << ", z = " << TransfPos3[2]
      //           << std::endl;
      // if ( TransfRotY ) {
      //   std::cout << "transf  rotXYZ: rotY = " << TransfRotAng << std::endl;
      // } else if ( TransfRotZ ) {
      //   std::cout << "transf  rotXYZ: rotZ = " << TransfRotAng << std::endl;
      // }
      // std::cout << "</paramphysvol3D>" << std::endl;

    } // End of loop on <paramphysvol3D> tag
  }
}
