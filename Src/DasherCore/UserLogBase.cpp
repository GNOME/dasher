/*
 *  UserLogBase.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 28/03/2011.
 *  Copyright 2011 Cavendish Laboratory. All rights reserved.
 *
 */

#include "UserLogBase.h"
#include "Event.h"
#include "DasherNode.h"

using namespace Dasher;

void CUserLogBase::HandleEvent(CEvent *pEvent) {
  if (pEvent->m_iEventType == EV_EDIT) {
    Dasher::CEditEvent *evt(static_cast<Dasher::CEditEvent *>(pEvent));
    if (evt->m_iEditType == 1) {
      m_vAdded.push_back(evt->m_pNode->GetSymbolProb(GetLongParameter(LP_NORMALIZATION)));
      //output
    } else if (evt->m_iEditType == 2) {
      //delete
      m_iNumDeleted++;
    }
  }
}

void CUserLogBase::FrameEnded() {
  //pass on added/deleted if any, and get ready for next frame
  if (m_iNumDeleted) {
    DeleteSymbols(m_iNumDeleted);
    m_iNumDeleted=0;
  }
  if (!m_vAdded.empty()) {
   AddSymbols(&m_vAdded);
    m_vAdded.clear();
  }
}