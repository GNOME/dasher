/*
 *  ClickDragFilter.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 18/05/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#include "../Common/Common.h"
#include "PlainDragFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

using namespace Dasher;

CPlainDragFilter::CPlainDragFilter(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName)
: CDefaultFilter(pEventHandler, pSettingsStore, pInterface, iID, szName) {
}


void CPlainDragFilter::KeyDown(int iTime, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog)
{
	if(iId == 100)
  {
    DASHER_ASSERT(GetLongParameter(BP_DASHER_PAUSED));
    m_pInterface->Unpause(iTime);
  }
}

void CPlainDragFilter::KeyUp(int iTime, int iId, CDasherView *pDasherView, CDasherModel *pModel, bool bPos, int iX, int iY)
{
	if(iId == 100)
	{
    DASHER_ASSERT(!GetBoolParameter(BP_DASHER_PAUSED));
    m_pInterface->PauseAt(0, 0);
  }
}