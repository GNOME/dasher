#pragma once
#include "../DasherCore/DasherInput.h"

namespace Dasher {
	class CDasherMouseInput;
}

class Dasher::CDasherMouseInput :
	public CDasherInput
{
public:
	CDasherMouseInput(void);
	~CDasherMouseInput(void);

	virtual int GetCoordinates( int iN, myint *pCoordinates ) { 
		
		 pCoordinates[0] = m_iDasherX;
    pCoordinates[1] = m_iDasherY;

		return 0; };

  // Get the number of co-ordinates that this device supplies

	virtual int GetCoordinateCount() { return 2; };

	void SetCoordinates( myint iDasherX, myint iDasherY ) {
		m_iDasherX = iDasherX;
		m_iDasherY = iDasherY;
	}
private:
	myint m_iDasherX;
	myint m_iDasherY;

};
