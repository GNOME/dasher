
#include "../Common/Common.h"

#include "UserLocation.h"

#include <sys/timeb.h>

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// Construct a new location at the current point in time and at the specified coordinates.
// This version only stores the integer coordinate data.
CUserLocation::CUserLocation(int iX, int iY, float dNats)
{
  //CFunctionLogger f1("CUserLocation::CUserLocation(1)", gLogger);

  InitMemeberVars();

  m_strTime               = CTimeSpan::GetTimeStamp();
  m_iLocationX            = iX;
  m_iLocationY            = iY;
  m_bHasInteger           = true;
  m_dNats                 = dNats;
}

// Stores only the normalized floating point data.
CUserLocation::CUserLocation(float iX, float iY, float dNats)
{
  //CFunctionLogger f1("CUserLocation::CUserLocation(2)", gLogger);

  InitMemeberVars();

  m_strTime               = CTimeSpan::GetTimeStamp();
  m_dNormalizedLocationX  = iX;
  m_dNormalizedLocationY  = iY;
  m_bHasNormalized        = true;
  m_dNats                 = dNats;
}

// Stores only the normalized floating point data. 
// This version calculates the normalization itself.
CUserLocation::CUserLocation(int iX, 
                             int iY, 
                             int iTop, 
                             int iLeft, 
                             int iBottom, 
                             int iRight, 
                             bool bStoreIntegerRep, 
                             float dNats)
{
  //CFunctionLogger f1("CUserLocation::CUserLocation(3)", gLogger);

  InitMemeberVars();

  m_strTime               = CTimeSpan::GetTimeStamp();
  m_dNormalizedLocationX  = (float) ComputeNormalizedX(iX, iLeft, iRight);
  m_dNormalizedLocationY  = (float) ComputeNormalizedY(iY, iTop, iBottom);
  m_bHasNormalized        = true;
  m_dNats                 = dNats;

  if (bStoreIntegerRep)
  {
    m_bHasInteger         = true;
    m_iLocationX          = iX;
    m_iLocationY          = iY;
  }
  else
    m_bHasInteger         = false;
}

// We want both the integer representation and the normalized.
CUserLocation::CUserLocation(int iX1, int iY1, float iX2, float iY2, float dNats)
{
  //CFunctionLogger f1("CUserLocation::CUserLocation(4)", gLogger);

  InitMemeberVars();

  m_strTime               = CTimeSpan::GetTimeStamp();
  m_iLocationX            = iX1;
  m_iLocationY            = iY1;
  m_dNormalizedLocationX  = iX2;
  m_dNormalizedLocationY  = iY2;
  m_bHasNormalized        = true;
  m_bHasInteger           = true;
  m_dNats                 = dNats;
}

CUserLocation::~CUserLocation()
{
  //CFunctionLogger f1("CUserLocation::~CUserLocation", gLogger);
}

string CUserLocation::GetXML(const string& strPrefix)
{
  //CFunctionLogger f1("CUserLocation::GetXML", gLogger);

  string strResult = "";

  strResult += strPrefix;
  strResult += "<Pos>\n";

  strResult += strPrefix;
  strResult += "\t<Time>";
  strResult += m_strTime;
  strResult += "</Time>\n";

  char strNum[256];
  if (m_bHasInteger)
  {
    strResult += strPrefix;
    strResult += "\t<X>";
    sprintf(strNum, "%d", m_iLocationX);
    strResult += strNum;
    strResult += "</X>\n";

    strResult += strPrefix;
    strResult += "\t<Y>";
    sprintf(strNum, "%d", m_iLocationY);
    strResult += strNum;
    strResult += "</Y>\n";
  }
  if (m_bHasNormalized)
  {
    strResult += strPrefix;
    strResult += "\t<XNorm>";
    sprintf(strNum, "%0.4f", m_dNormalizedLocationX);
    strResult += strNum;
    strResult += "</XNorm>\n";

    strResult += strPrefix;
    strResult += "\t<YNorm>";
    sprintf(strNum, "%0.4f", m_dNormalizedLocationY);
    strResult += strNum;
    strResult += "</YNorm>\n";    
  }

  strResult += strPrefix;
  strResult += "\t<Bits>";
  sprintf(strNum, "%0.3f", m_dNats / log(2.0));	
  strResult += strNum;
  strResult += "</Bits>\n";

  strResult += strPrefix;
  strResult += "</Pos>\n";

  return strResult;
}

// Static helper method for computing normalized X coordinate
double CUserLocation::ComputeNormalizedX(int iX, int iLeft, int iRight)
{
  //CFunctionLogger f1("CUserLocation::ComputeNormalizedX", gLogger);

  return (double) (iX - iLeft) / (double) abs(iRight - iLeft);
}

// Static helper method for computing normalized Y coordinate
double CUserLocation::ComputeNormalizedY(int iY, int iTop, int iBottom)
{
  //CFunctionLogger f1("CUserLocation::ComputeNormalizedY", gLogger);

  return (double) (iY - iTop) / (double) abs(iBottom - iTop);
}

void CUserLocation::InitMemeberVars()
{
  //CFunctionLogger f1("CUserLocation::InitMemeberVars", gLogger);

  m_strTime               = "";
  m_iLocationX            = 0;
  m_iLocationY            = 0;
  m_dNormalizedLocationX  = 0.0;
  m_dNormalizedLocationY  = 0.0;
  m_bHasNormalized        = false;
  m_bHasInteger           = false;
  m_dNats                 = 0.0;
}

// Construct based on some XML like:
//  <Pos>
//  	<Time>15:49:10.203</Time>
//  	<X>807</X>
//  	<Y>382</Y>
//  	<XNorm>0.7274</XNorm>
//  	<YNorm>0.1853</YNorm>
//  	<Bits>0.555</Bits>
//  </Pos>
CUserLocation::CUserLocation(const string& strXML)
{
  //CFunctionLogger f1("CUserLocation::CUserLocation(XML)", gLogger);

  InitMemeberVars();

  bool bFoundNormX        = false;
  bool bFoundNormY        = false;
  bool bFoundX            = false;
  bool bFoundY            = false;

  m_strTime               = XMLUtil::GetElementString("Time", strXML, true);    
  m_iLocationX            = XMLUtil::GetElementInt("X", strXML, &bFoundX);
  m_iLocationY            = XMLUtil::GetElementInt("Y", strXML, &bFoundY);
  m_dNormalizedLocationX  = (float) XMLUtil::GetElementFloat("XNorm", strXML, &bFoundNormX);
  m_dNormalizedLocationY  = (float) XMLUtil::GetElementFloat("YNorm", strXML, &bFoundNormY);

  // Convert the bits back to dNats
  m_dNats                 = (float) ((double) XMLUtil::GetElementFloat("Bits", strXML) * (double) log(2.0));

  // If there weren't X, Y elements, we want them set to 0 and mark
  // ourselves as not having them.
  if ((!bFoundX) && (!bFoundY))
  {
    m_iLocationX  = 0;
    m_iLocationY  = 0;
    m_bHasInteger = false;
  }
  else
    m_bHasInteger = true;

  // Require that we find both XNorm and YNorm in order to count
  if ((!bFoundNormX) || (!bFoundNormY))
  {
    m_dNormalizedLocationX  = 0.0;
    m_dNormalizedLocationY  = 0.0;
    m_bHasNormalized        = false;
  }
  else
    m_bHasNormalized        = true;

}

// Returns a tab delimited version of this location's X & Y coordinate
string CUserLocation::GetTabMouseXY(bool bReturnNormalized)
{
  //CFunctionLogger f1("CUserLocation::GetTabMouseXY", gLogger);

  string strResult = "";
  char szNum[256];

  if (bReturnNormalized)
    sprintf(szNum, "%0.4f\t%0.4f\n", m_dNormalizedLocationX, m_dNormalizedLocationY);
  else
    sprintf(szNum, "%0.4f\t%0.4f\n", (double)m_iLocationX, (double)m_iLocationY);

  strResult += szNum;

  return strResult;
}

// Figure out what grid location this normalized mouse coordinate should go.
void CUserLocation::GetMouseGridLocation(int iGridSize, int* pRow, int* pCol)
{
  //CFunctionLogger f1("CUserLocation::GetMouseGridLocation", gLogger);

  if ((pRow == NULL) || (pCol == NULL))
    return;

  *pRow = (int) (m_dNormalizedLocationX * (double) iGridSize);
  *pCol = (int) (m_dNormalizedLocationY * (double) iGridSize);
}


