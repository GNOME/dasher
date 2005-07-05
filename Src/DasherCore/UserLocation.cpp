
#include "UserLocation.h"

#ifdef _WIN32
// In order to track leaks to line number, we need this at the top of every file
#include "MemoryLeak.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// Construct a new location at the current point in time and at the specified coordinates.
// This version only stores the integer coordinate data.
CUserLocation::CUserLocation(int x, int y, float nats)
{
    m_strTime               = CTimeSpan::GetTimeStamp();
    m_locationX             = x;
    m_locationY             = y;
    m_normalizedLocationX   = 0.0;
    m_normalizedLocationY   = 0.0;
    m_bHasNormalized        = false;
    m_bHasInteger           = true;
    m_nats                  = nats;
}

// Stores only the normalized floating point data.
CUserLocation::CUserLocation(float x, float y, float nats)
{
    m_strTime               = CTimeSpan::GetTimeStamp();
    m_locationX             = 0;
    m_locationY             = 0;
    m_normalizedLocationX   = x;
    m_normalizedLocationY   = y;
    m_bHasNormalized        = true;
    m_bHasInteger           = false;
    m_nats                  = nats;
}

// Stores only the normalized floating point data. 
// This version calculates the normalization itself.
CUserLocation::CUserLocation(int x, int y, int top, int left, int bottom, int right, bool bStoreIntegerRep, float nats)
{
    m_strTime               = CTimeSpan::GetTimeStamp();
    m_locationX             = 0;
    m_locationY             = 0;
    m_normalizedLocationX   = (float) ComputeNormalizedX(x, left, right);
    m_normalizedLocationY   = (float) ComputeNormalizedY(y, top, bottom);
    m_bHasNormalized        = true;
    m_nats                  = nats;

    if (bStoreIntegerRep)
    {
        m_bHasInteger       = true;
        m_locationX         = x;
        m_locationY         = y;
    }
    else
        m_bHasInteger       = false;
}

// We want both the integer representation and the normalized.
CUserLocation::CUserLocation(int x1, int y1, float x2, float y2, float nats)
{
    m_strTime               = CTimeSpan::GetTimeStamp();
    m_locationX             = x1;
    m_locationY             = y1;
    m_normalizedLocationX   = x2;
    m_normalizedLocationY   = y2;
    m_bHasNormalized        = true;
    m_bHasInteger           = true;
    m_nats                  = nats;
}

CUserLocation::~CUserLocation()
{
}

string CUserLocation::GetXML(const string& prefix)
{
    string strResult = "";

    strResult += prefix;
    strResult += "<Pos>\n";

    strResult += prefix;
    strResult += "\t<Time>";
    strResult += m_strTime;
    strResult += "</Time>\n";

    char strNum[256];
    if (m_bHasInteger)
    {
        strResult += prefix;
        strResult += "\t<X>";
        sprintf(strNum, "%d", m_locationX);
        strResult += strNum;
        strResult += "</X>\n";

        strResult += prefix;
        strResult += "\t<Y>";
        sprintf(strNum, "%d", m_locationY);
        strResult += strNum;
        strResult += "</Y>\n";
    }
    if (m_bHasNormalized)
    {
        strResult += prefix;
        strResult += "\t<XNorm>";
        sprintf(strNum, "%0.4f", m_normalizedLocationX);
        strResult += strNum;
        strResult += "</XNorm>\n";

        strResult += prefix;
        strResult += "\t<YNorm>";
        sprintf(strNum, "%0.4f", m_normalizedLocationY);
        strResult += strNum;
        strResult += "</YNorm>\n";    
    }

	strResult += prefix;
	strResult += "\t<Bits>";
	sprintf(strNum, "%0.3f", m_nats / log(2.0));	
	strResult += strNum;
	strResult += "</Bits>\n";

    strResult += prefix;
    strResult += "</Pos>\n";

    return strResult;
}

// Static helper method for computing normalized X coordinate
double CUserLocation::ComputeNormalizedX(int x, int left, int right)
{
    return (double) (x - left) / (double) abs(right - left);
}

// Static helper method for computing normalized Y coordinate
double CUserLocation::ComputeNormalizedY(int y, int top, int bottom)
{
    return (double) (y - top) / (double) abs(bottom - top);
}

