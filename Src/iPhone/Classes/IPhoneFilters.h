/*
 *  IPhoneFilters.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 29/04/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#include "OneDimensionalFilter.h"

using namespace Dasher;

#define ONE_D_FILTER "IPhone1D Filter"
#define POLAR_FILTER "Polar Filter"
class CIPhone1DFilter : public COneDimensionalFilter {
public:
	CIPhone1DFilter(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID);

	virtual bool Timer(int iTime, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol);
protected:
	virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);
private:
	int m_iSlow;
	double m_dRad;
};

class CIPhonePolarFilter : public COneDimensionalFilter {
public:
	CIPhonePolarFilter(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID);
	
	virtual void KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog);
	virtual void KeyUp(int iTime, int iId, CDasherView *pView, CDasherModel *pModel);
protected:
	virtual void ApplyTransform(myint &iDasherX, myint &iDasherY);
};
/// @}