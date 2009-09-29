/*
 *  ClickDragFilter.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 18/05/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#ifndef __CLICK_DRAG_FILTER_H__
#define __CLICK_DRAG_FILTER_H__

#include "DefaultFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
class CPlainDragFilter : public CDefaultFilter {
public:
	CPlainDragFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName);
	
	virtual void KeyDown(int iTime, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog);
	virtual void KeyUp(int Time, int iId, CDasherView *pDasherView, CDasherModel *pModel, bool bPos, int iX, int iY);
	
};
}
/// @}

#endif
