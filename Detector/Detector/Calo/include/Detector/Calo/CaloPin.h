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

namespace LHCb::Detector::Calo {

  struct Pin final {

    Pin() : m_id( LHCb::Detector::Calo::CellID() ) {}
    Pin( LHCb::Detector::Calo::CellID id ) : m_id( id ){};

    // getters
    LHCb::Detector::Calo::CellID                     id() const { return m_id; }
    const std::vector<int>&                          leds() const { return m_leds; }
    const std::vector<LHCb::Detector::Calo::CellID>& cells() const { return m_cells; }

    std::vector<int> firstRows() const { return m_fRow; }
    std::vector<int> lastRows() const { return m_lRow; }
    std::vector<int> firstColumns() const { return m_fCol; }
    std::vector<int> lastColumns() const { return m_lCol; }
    std::vector<int> areas() const { return m_area; }

    int firstRow() const { return *( m_fRow.begin() ); }
    int lastRow() const { return *( m_lRow.begin() ); }
    int firstColumn() const { return *( m_fCol.begin() ); }
    int lastColumn() const { return *( m_lCol.begin() ); }
    int area() const { return *( m_area.begin() ); }

    int box() const { return m_box; }
    int index() const { return m_index; }
    int side() const { return m_side; }

    friend std::ostream& operator<<( std::ostream& str, Pin obj ) { return obj.fillStream( str ); }
    std::ostream&        fillStream( std::ostream& str ) {
      return str << " cellId " << m_id << " : " << m_code << " : " << m_region;
    }

    // setters
    void setId( LHCb::Detector::Calo::CellID id ) { m_id = id; }
    void addLed( int id ) { m_leds.push_back( id ); }
    void addCell( LHCb::Detector::Calo::CellID cell ) { m_cells.push_back( cell ); }
    void addCaloRegion( int area, int fCol, int fRow, int lCol, int lRow ) {
      m_area.push_back( area );
      m_fRow.push_back( fRow );
      m_fCol.push_back( fCol );
      m_lCol.push_back( lCol );
      m_lRow.push_back( lRow );
      std::stringstream s( " " );
      s << m_region << " += [" << area << ", " << fCol << "," << fRow << "," << lCol << "," << lRow << "] ";
      m_region = s.str();
    }

    void setPinLocation( int box, int index, int side ) {
      m_box                      = box;
      m_index                    = index;
      m_side                     = side;
      std::string       sideName = ( m_side == 0 ) ? "C-side" : "A-side";
      std::stringstream s( " " );
      s << m_code << "(" << sideName << " : " << box << "." << index << ")";
      m_code = s.str();
    }

  private:
    LHCb::Detector::Calo::CellID              m_id;
    std::string                               m_region = "Region ";
    std::string                               m_code   = "Code ";
    int                                       m_side   = -1;
    int                                       m_index  = -1;
    int                                       m_box    = -1;
    std::vector<int>                          m_area;
    std::vector<int>                          m_fCol;
    std::vector<int>                          m_fRow;
    std::vector<int>                          m_lCol;
    std::vector<int>                          m_lRow;
    std::vector<int>                          m_leds;
    std::vector<LHCb::Detector::Calo::CellID> m_cells;
  };

} // namespace LHCb::Detector::Calo
