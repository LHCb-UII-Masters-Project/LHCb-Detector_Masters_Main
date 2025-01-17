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

struct CardParam final {

public:
  enum Mapping { None = 0 };

  CardParam( int iArea = 0, int iRow = 0, int iCol = 0, int ilRow = 0, int ilCol = 0, int iNum = 0, int iCrate = 0,
             int iSlot = 0 )
      : m_Area( iArea )
      , m_Row( iRow )
      , m_Col( iCol )
      , m_lRow( ilRow )
      , m_lCol( ilCol )
      , m_number( iNum )
      , m_crate( iCrate )
      , m_slot( iSlot ) {}

  int     downNumber() const { return m_DownNumber; }
  int     leftNumber() const { return m_LeftNumber; }
  int     cornerNumber() const { return m_CornerNumber; }
  int     previousNumber() const { return m_PreviousNumber; }
  int     validationNumber() const { return m_validationNumber; }
  int     area() const { return m_Area; }
  int     firstRow() const { return m_Row; }
  int     firstColumn() const { return m_Col; }
  int     lastRow() const { return m_lRow; }
  int     lastColumn() const { return m_lCol; }
  bool    isPinCard() const { return m_isPin; }
  int     tell1() const { return m_tell1; }
  int     number() const { return m_number; }
  int     crate() const { return m_crate; }
  int     slot() const { return m_slot; }
  int     code() const { return 16 * m_crate + m_slot; }
  int     selectionType() const { return m_selectionType; }
  Mapping mapping() const { return m_mapping; }
  bool    isParasitic() const {
    return ( ( m_Row + m_lRow + m_Col + m_lCol ) == 0 );
  } // special FEBs (no channelID associated)
  bool isPmtCard() const { return ( !isPinCard() && !isParasitic() ); }

  const std::vector<LHCb::Detector::Calo::CellID>& ids() const { return m_ids; }
  void                                             addID( LHCb::Detector::Calo::CellID id ) { m_ids.push_back( id ); }
  void                                             setTell1( int tell1 ) { m_tell1 = tell1; }
  void                                             setValidationNumber( int num ) { m_validationNumber = num; }
  void setNeighboringCards( const int down, const int left, const int corner, const int previous ) {
    m_DownNumber     = down;
    m_LeftNumber     = left;
    m_CornerNumber   = corner;
    m_PreviousNumber = previous;
  }
  void setIsPin( bool pin ) { m_isPin = pin; }
  void setSelectionType( int num ) { m_selectionType = num; }
  void setMapping( Mapping typ ) { m_mapping = typ; }

private:
  int                                       m_Area;
  int                                       m_Row;
  int                                       m_Col;
  int                                       m_lRow;
  int                                       m_lCol;
  int                                       m_DownNumber       = -1;
  int                                       m_LeftNumber       = -1;
  int                                       m_CornerNumber     = -1;
  int                                       m_PreviousNumber   = -1;
  int                                       m_validationNumber = -1;
  bool                                      m_isPin            = false;
  int                                       m_number;
  std::vector<LHCb::Detector::Calo::CellID> m_ids;
  int                                       m_crate;
  int                                       m_slot;
  int                                       m_tell1 = -1;
  int     m_selectionType = -1; // [e,g,pi0L,pi0G] = -1 ; [hadronMaster] = 0 ; [hadronSlave1]=1 ; [hadronSlave2]=2
  Mapping m_mapping       = CardParam::None; // see CardParam::Mapping  enum
};
