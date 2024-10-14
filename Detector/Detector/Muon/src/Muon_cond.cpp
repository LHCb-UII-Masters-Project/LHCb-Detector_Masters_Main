//==========================================================================
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
#include "Core/ConditionHelper.h"
#include "Core/DeConditionCall.h"
#include "Core/DeIOV.h"
#include "Detector/Muon/DeMuon.h"

#include "DD4hep/ConditionDerived.h"
#include "DD4hep/Detector.h"

#include "Core/Keys.h"
#include "DD4hep/DetElement.h"
#include "DD4hep/Printout.h"

namespace LHCb::Detector {

  /// Condition derivation call to build the top level VP DetElement condition information
  struct DeMuonConditionCall : DeConditionCall {
    using DeConditionCall::DeConditionCall;
    virtual dd4hep::Condition operator()( const dd4hep::ConditionKey& /*key*/,
                                          dd4hep::cond::ConditionUpdateContext& ctxt ) override final {
      /*
      auto        muondet       = dd4hep::Detector::getInstance().detector( "Muon" );
      std::string name_to_print = "Muon Cond Loader";

      dd4hep::Condition m2 = ctxt.condition( dd4hep::ConditionKey::KeyMaker( muondet, "M2" ).hash);
      std::cout<<" paura dopo "<<dd4hep::ConditionKey::KeyMaker( muondet, "M2" ).hash<<std::endl;
      std::cout<<" paura dopo "<<dd4hep::ConditionKey::KeyMaker( muondet, "M3" ).hash<<std::endl;
      std::cout<<" paura dopo "<<dd4hep::ConditionKey::KeyMaker( muondet, "M3" ).hash<<std::endl;

            const auto       m2_load  = json2yaml( m2.get<nlohmann::json>() );
            int               stationNumber         = m2_load["StationNumber"].as<int>();

      */

      /*
      //TSlayout in station
      auto condTS =  m2_load["TSLayout"];
      //dd4hep::printout( dd4hep::DEBUG, name_to_print, "StationNumber: %d", condTS.size() );

      for(auto const &item : condTS){ dd4hep::printout( dd4hep::DEBUG, name_to_print, "Item: %d", item.size() );}
      for(auto const &item : condTS){
        std::vector test= item.as<std::vector<int>>() ;
        for (auto const & ele : test){
          dd4hep::printout( dd4hep::VERBOSE, name_to_print, "elemento: %d", ele );
        }
      }


      //auto testTS =  m2_load["TSLayout"][0];
      dd4hep::printout( dd4hep::DEBUG, name_to_print, "test test eteste:" );

      auto Tell40Boards =  m2_load["TELL40Board"];
      dd4hep::printout( dd4hep::DEBUG, name_to_print, "how many described Tell40 : %d", Tell40Boards.size() );
      auto nODEBoards =  m2_load["NODEBoard"];
      auto TSmaps =  m2_load["TSMap"];
      dd4hep::printout( dd4hep::DEBUG,name_to_print, "how many described nODE : %d", nODEBoards.size() );
      for(auto const &tBoard : Tell40Boards){
        const int t_number= tBoard["Tell40Number"].as<int>();
        const std::string t_name= tBoard["Tell40Name"].as<std::string>();
        //dd4hep::printout( dd4hep::DEBUG, name_to_print, " Tell40 Num and Name: %d %s", t_number, t_name.c_str() );
        auto tell40PCI =  ( tBoard )["PCI"];
        //dd4hep::printout( dd4hep::DEBUG, name_to_print, "pci number: %d", tell40PCI.size() );
        for(auto const pci : tell40PCI){
          const int active_link=pci["ActiveLinkNumber"].as<int>();
          const int pciNumber=pci["PCINumber"].as<int>();
          int described_link=0;
          const std::vector<int> odelist=pci["ODEList"].as<std::vector<int>>();
          const std::vector<int> odelink=pci["ODELink"].as<std::vector<int>>();
          auto o_link= odelink.begin();
          for(auto o : odelist){
            //dd4hep::printout( dd4hep::DEBUG, name_to_print,
            //		      "ode number and link: %d  %d", o, *o_link );
            o_link++;
            if(o>0){described_link++;}

          }
          //if(described_link!= active_link)	dd4hep::printout( dd4hep::ERROR, name_to_print,
          //"discrepancyin ODE link described in Tell40 PCI: %d %d", t_number, pciNumber );
        }
      }

      for(auto const &nBoard : nODEBoards){
        const int o_number= nBoard["ODESerialNumber"].as<int>();
        const std::string o_name= nBoard["ODEECSName"].as<std::string>();
        const int o_region= nBoard["RegionNumber"].as<int>();
        const int TSnum=nBoard["TSNumber"].as<int>();
        const std::vector<int> TSGXList=nBoard["TSGridXList"].as<std::vector<int>>();
        const std::vector<int> TSGYList=nBoard["TSGridYList"].as<std::vector<int>>();
        const std::vector<int> TSGQList=nBoard["TSGridQList"].as<std::vector<int>>();
        const std::vector<unsigned int> TSMapNumList=nBoard["TSMapNumberList"].as<std::vector<unsigned int>>();


      }
      dd4hep::printout( dd4hep::ERROR, name_to_print, "region number TSMap number: " );

      const int ts_reg=TSmaps[0]["RegionNumber"].as<int>();
      const int ts_number=TSmaps[0]["TSNumber"].as<int>();

      //dd4hep::printout( dd4hep::ERROR, name_to_print, "region number TSMap number: %d %d", ts_reg, ts_number );
*/
      /* keep as an example*/

      /*        dd4hep::Condition magnet_meascond = ctxt.condition( dd4hep::ConditionKey::KeyMaker( muondet,
         "OLDStationNumber" ).hash ); YAML::Node*       magnet_measdoc  = static_cast<YAML::Node*>(
         magnet_meascond->payload() ); double            current         = ( *magnet_measdoc )["Current"].as<double>();
            int               polarity        = ( *magnet_measdoc )["Polarity"].as<int>();
            dd4hep::printout( dd4hep::DEBUG, "Alessia", "Current value: %f", current );
            dd4hep::printout( dd4hep::DEBUG, "Alessia", "Current polarity: %d", polarity );
       */

      return DeIOV( new detail::DeMuonObject( dd4hep::Detector::getInstance().detector( "Muon" ), ctxt ) );
    }
  };
} // namespace LHCb::Detector

static long create_conditions_recipes( dd4hep::Detector& description, xml_h e ) {

  // Use the helper to load the XML, setup the callback according
  LHCb::Detector::ConditionConfigHelper<LHCb::Detector::DeMuonConditionCall> config_helper{description, "Muon", e};
  config_helper.configure();

  return 1;
}
DECLARE_XML_DOC_READER( LHCb_Muon_cond, create_conditions_recipes )
