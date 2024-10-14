/*****************************************************************************\
* (c) Copyright 2000-2021 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once
#include "DD4hep/DD4hepUnits.h"
#include "Detector/Calo/CaloCellID.h"
#include "Math/Point3D.h"

using CaloNeighbors = std::vector<LHCb::Detector::Calo::CellID>;

namespace LHCb::Detector::Calo {

  namespace CellQuality {
    enum Flag {
      OK                = 0,
      Dead              = 1,
      Noisy             = 2,
      Shifted           = 4,
      DeadLED           = 8,
      VeryNoisy         = 16,
      VeryShifted       = 32,
      LEDSaturated      = 64,
      BadLEDTiming      = 128,
      VeryBadLEDTiming  = 256,
      BadLEDRatio       = 512,
      BadLEDOpticBundle = 1024,
      UnstableLED       = 2048,
      StuckADC          = 4096,
      OfflineMask       = 8192
    };
    inline const int         Number       = 15;
    inline const std::string Name[Number] = {
        "OK",          "Dead",         "Noisy",        "Shifted",          "DeadLED",     "VeryNoisy",
        "VeryShifted", "LEDSaturated", "BadLEDTiming", "VeryBadLEDTiming", "BadLEDRatio", "BadLEDOpticBundle",
        "UnstableLED", "StuckADC",     "OfflineMask"};
    inline const std::string qName( int i ) {
      if ( i < Number && i >= 0 ) return Name[i];
      return std::string( "??" );
    }
  } // namespace CellQuality

  struct CellParam final {

    CellParam( LHCb::Detector::Calo::CellID id = LHCb::Detector::Calo::CellID() ) : m_cellID( std::move( id ) ) {}

    bool                         valid() const { return m_valid; }
    LHCb::Detector::Calo::CellID cellID() const { return m_cellID; }
    double                       x() const { return m_center.x(); }
    double                       y() const { return m_center.y(); }
    double                       z() const { return m_center.z() + m_zShower; }
    const ROOT::Math::XYZPoint   center() const { return ROOT::Math::XYZPoint( x(), y(), z() ); }
    double                       size() const { return m_size; }
    double                       sine() const { return m_sine; }
    double                       nominalGain() const { return m_gain; }
    double                       time() const { return m_time + m_dtime; }
    int                          cardNumber() const { return m_cardNumber; }
    int                          cardRow() const { return m_cardRow; }
    int                          cardColumn() const { return m_cardColumn; }
    double                       deltaTime() const { return m_dtime; }
    double                       zShower() const { return m_zShower; }
    int                          quality() const { return m_quality; }
    int                          l0Constant() const { return m_l0constant; }
    double calibration() const { return m_calibration; } // absolute calibration from 'Calibration' condition (T0)
    double ledDataRef() const { return m_ledDataRef; }   // <LED> data from Calibration condition (Ref T0)
    double ledMoniRef() const { return m_ledMoniRef; }   // <LED/PIN> data from Calibration condition (Ref T0)
    double ledData() const { return m_ledData; }         // <LED> data from Quality condition (current T)
    double ledDataRMS() const { return m_ledDataRMS; }   // RMS(LED) from Quality condition  (current T)
    double ledMoni() const { return m_ledMoni; }         // <LED/PIN> data from Quality condition (current T)
    double ledMoniRMS() const { return m_ledMoniRMS; }   // RMS(LED/PIN) from Quality condition (current T)
    double ledDataShift() const { return ( ledDataRef() > 0 && ledData() > 0 ) ? ledData() / ledDataRef() : 1; }
    double gainShift() const { return ( ledMoniRef() > 0 && ledMoni() > 0 ) ? ledMoni() / ledMoniRef() : 1; }
    double gain() const { return nominalGain() * calibration() / gainShift(); }
    int    numericGain() const { return m_nGain; } // for Prs only
    double pileUpOffset() const { return m_offset; }
    double pileUpOffsetSPD() const { return m_offsetSPD; }
    double pileUpOffsetRMS() const { return m_eoffset; }
    double pileUpOffsetSPDRMS() const { return m_eoffsetSPD; }

    const std::vector<LHCb::Detector::Calo::CellID>& pins() const { return m_pins; }
    const std::vector<int>&                          leds() const { return m_leds; }
    const CaloNeighbors&                             neighbors() const { return m_neighbors; }
    const CaloNeighbors&                             zsupNeighbors() const { return m_zsupNeighbors; }

    // ** To initialize the cell: Geometry, neighbours, gain

    void setValid( bool valid ) { m_valid = valid; }

    void setCenterSize( const ROOT::Math::XYZPoint& point, double S ) {
      m_center = point;
      m_size   = S;
      m_sine   = 0;
      if ( point.R() > std::numeric_limits<double>::epsilon() )
        m_sine = point.Rho() / point.R();   // MathCore methods (Cartesian3D)
      m_time = point.R() / dd4hep::c_light; // R=sqrt(Mag2)
    }
    void setDeltaTime( double dtime ) { m_dtime = dtime; }
    void setZshower( double dz ) { m_zShower = dz; }

    void addZsupNeighbor( const LHCb::Detector::Calo::CellID& ID ) { m_zsupNeighbors.push_back( ID ); }
    void addNeighbor( const LHCb::Detector::Calo::CellID& ID ) { m_neighbors.push_back( ID ); }
    void setNominalGain( const double gain ) { m_gain = gain; }
    void setFeCard( const int num, const int relCol, const int relRow ) {
      m_cardNumber = num;
      m_cardColumn = relCol;
      m_cardRow    = relRow;
    }
    void addPin( LHCb::Detector::Calo::CellID id ) { m_pins.push_back( id ); }
    void addLed( int id ) { m_leds.push_back( id ); }
    void resetPins() { m_pins.clear(); }
    void resetLeds() { m_leds.clear(); }

    // Calibration & quality
    void setQualityFlag( int quality ) { m_quality = quality; }
    void addQualityFlag( int quality ) { m_quality = m_quality | quality; }
    void setLedData( double ledData, double ledDataRMS ) {
      m_ledData    = ledData;
      m_ledDataRMS = ledDataRMS;
    }
    void setLedMoni( double ledMoni, double ledMoniRMS ) {
      m_ledMoni    = ledMoni;
      m_ledMoniRMS = ledMoniRMS;
    }
    void setLedDataRef( double ledDataRef, double ledMoniRef ) {
      m_ledDataRef = ledDataRef;
      m_ledMoniRef = ledMoniRef;
    }
    void setCalibration( double calib ) { m_calibration = calib; }
    void setL0Constant( int cte ) { m_l0constant = cte; }
    void setNumericGain( int ng ) { m_nGain = ng; } // for Prs only
    void setPileUpOffset( double off, double eoff = 0 ) {
      m_offset  = off;
      m_eoffset = eoff;
    }
    void setPileUpOffsetSPD( double off, double eoff = 0 ) {
      m_offsetSPD  = off;
      m_eoffsetSPD = eoff;
    }

    std::string cellStatus() {
      if ( (CellQuality::Flag)m_quality == CellQuality::OK ) return CellQuality::qName( CellQuality::OK );
      std::ostringstream s( "" );
      s << "| ";
      int q = m_quality;
      int d = 1;
      while ( q > 0 ) {
        if ( ( q & 0x1 ) == 1 ) s << CellQuality::qName( d ) << " | ";
        d += 1;
        q /= 2;
      }
      return s.str();
    }

  private:
    LHCb::Detector::Calo::CellID              m_cellID;                      //< ID of the cell
    double                                    m_size{0.0};                   //< Cell size
    ROOT::Math::XYZPoint                      m_center{-999999, -999999, 0}; //< Cell centre
    double                                    m_sine{0.0};                   //< To transform E to Et
    double                                    m_gain{0.0};                   //< MeV per ADC count
    double                                    m_time{0.0};                   //< Nominal time of flight from Vertex (ns)
    int                                       m_cardNumber{-1};              //< Front-end card number
    int                                       m_cardRow{-1};                 //< card row and column
    int                                       m_cardColumn{-1};
    CaloNeighbors                             m_neighbors{};     //< List of neighbors
    CaloNeighbors                             m_zsupNeighbors{}; //< List of neighbors in same area
    bool                                      m_valid{false};
    std::vector<LHCb::Detector::Calo::CellID> m_pins;
    std::vector<int>                          m_leds;
    double                                    m_dtime{0.0};
    double                                    m_zShower{0.0};
    int                                       m_quality{CellQuality::OK};
    double                                    m_calibration{1.0};
    int                                       m_l0constant{0};
    double                                    m_ledDataRef{-1.0};
    double                                    m_ledMoniRef{-1.0};
    double                                    m_ledData{-1.0};
    double                                    m_ledMoni{-1.0};
    double                                    m_ledDataRMS{0.0};
    double                                    m_ledMoniRMS{0.0};
    int                                       m_nGain{0}; // numeric gains (for Prs only)
    double                                    m_offset{0.0};
    double                                    m_eoffset{0.0};
    double                                    m_offsetSPD{0.0};
    double                                    m_eoffsetSPD{0.0};
  };

} // namespace LHCb::Detector::Calo
