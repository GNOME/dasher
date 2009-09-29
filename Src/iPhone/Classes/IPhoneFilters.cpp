/*
 *  IPhoneFilters.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 29/04/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#include "IPhoneFilters.h"

#include "../Common/Common.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

#include <iostream>

CIPhone1DFilter::CIPhone1DFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID)
: COneDimensionalFilter(pEventHandler, pSettingsStore, pInterface, iID, ONE_D_FILTER), m_iSlow(0), m_dRad(1.0) {};

bool CIPhone1DFilter::Timer(int iTime, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted)
{
	myint iDasherX,iDasherY;
	m_pDasherView->GetCoordinates(iDasherX, iDasherY);
	if (iDasherX > 3072/*ICK*/) {
    //tilted to left; slow down to 0.25* speed (suddenly!)
    m_iSlow = -1; m_dRad = 0.25;
  } else if (m_iSlow == -1 && iDasherX < 0) {
    //currently in slowed-down mode but tilted to right: start speeding up (gradually)
		m_iSlow = iTime - GetLongParameter(LP_SLOW_START_TIME)/4;
  }
  //if we're not in slowed-down mode, nor in normal mode,...
	if (m_iSlow != -1 && m_iSlow != 0)
	{
    //...then we must be gradually speeding back up to normal
		if (iTime - m_iSlow >= GetLongParameter(LP_SLOW_START_TIME))
		{
      //reached normal speed
			m_iSlow = 0;
			m_dRad = 1.0;
		}
		else
		{
      //interpolate
			m_dRad = (iTime - m_iSlow) / (double)GetLongParameter(LP_SLOW_START_TIME);
		}
	}
	return CDefaultFilter::Timer(iTime, m_pDasherView, m_pDasherModel, pAdded, pNumDeleted);
}
			
void CIPhone1DFilter::ApplyTransform(myint &iDasherX, myint &iDasherY) {
	COneDimensionalFilter::ApplyTransform(iDasherX, iDasherY);
	if (m_dRad != 1.0)
	{
		dasherint ox = GetLongParameter(LP_OX), oy = GetLongParameter(LP_OY);
		iDasherX = (iDasherX - ox)*m_dRad + ox;
		iDasherY = (iDasherY - oy)*m_dRad + oy;
	}
}

CIPhonePolarFilter::CIPhonePolarFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID)
: COneDimensionalFilter(pEventHandler, pSettingsStore, pInterface, iID, POLAR_FILTER) {};

void CIPhonePolarFilter::ApplyTransform(myint &iDasherX, myint &iDasherY) {
	myint x = iDasherX;
	COneDimensionalFilter::ApplyTransform(iDasherX, iDasherY);
	dasherint ox = GetLongParameter(LP_OX), oy = GetLongParameter(LP_OY);
	double dRad = 2.0 - (x/(double)ox);
	iDasherX = (iDasherX - ox)*dRad + ox;
	iDasherY = (iDasherY - oy)*dRad + oy;	
}

void CIPhonePolarFilter::KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog) {
	if(iId == 100)
		m_pInterface->Unpause(iTime);
}

void CIPhonePolarFilter::KeyUp(int iTime, int iId, CDasherView *pView, CDasherModel *pModel) {
	if(iId == 100)
		m_pInterface->PauseAt(0, 0);
}