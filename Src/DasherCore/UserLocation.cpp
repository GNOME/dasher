
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
    //CFunctionLogger f1("CUserLocation::CUserLocation(1)", gLogger);

    InitMemeberVars();

    m_strTime               = CTimeSpan::GetTimeStamp();
    m_locationX             = x;
    m_locationY             = y;
    m_bHasInteger           = true;
    m_nats                  = nats;
}

// Stores only the normalized floating point data.
CUserLocation::CUserLocation(float x, float y, float nats)
{
    //CFunctionLogger f1("CUserLocation::CUserLocation(2)", gLogger);

    InitMemeberVars();

    m_strTime               = CTimeSpan::GetTimeStamp();
    m_normalizedLocationX   = x;
    m_normalizedLocationY   = y;
    m_bHasNormalized        = true;
    m_nats                  = nats;
}

// Stores only the normalized floating point data. 
// This version calculates the normalization itself.
CUserLocation::CUserLocation(int x, int y, int top, int left, int bottom, int right, bool bStoreIntegerRep, float nats)
{
    //CFunctionLogger f1("CUserLocation::CUserLocation(3)", gLogger);

    InitMemeberVars();

    m_strTime               = CTimeSpan::GetTimeStamp();
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
    //CFunctionLogger f1("CUserLocation::CUserLocation(4)", gLogger);

    InitMemeberVars();

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
    //CFunctionLogger f1("CUserLocation::~CUserLocation", gLogger);
}

string CUserLocation::GetXML(const string& prefix)
{
    //CFunctionLogger f1("CUserLocation::GetXML", gLogger);

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
    //CFunctionLogger f1("CUserLocation::ComputeNormalizedX", gLogger);

    return (double) (x - left) / (double) abs(right - left);
}

// Static helper method for computing normalized Y coordinate
double CUserLocation::ComputeNormalizedY(int y, int top, int bottom)
{
    //CFunctionLogger f1("CUserLocation::ComputeNormalizedY", gLogger);

    return (double) (y - top) / (double) abs(bottom - top);
}

void CUserLocation::InitMemeberVars()
{
    //CFunctionLogger f1("CUserLocation::InitMemeberVars", gLogger);

    m_strTime               = "";
    m_locationX             = 0;
    m_locationY             = 0;
    m_normalizedLocationX   = 0.0;
    m_normalizedLocationY   = 0.0;
    m_bHasNormalized        = false;
    m_bHasInteger           = false;
    m_nats                  = 0.0;
}

#ifdef USER_LOG_TOOL
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
    m_locationX             = XMLUtil::GetElementInt("X", strXML, &bFoundX);
    m_locationY             = XMLUtil::GetElementInt("Y", strXML, &bFoundY);
    m_normalizedLocationX   = (float) XMLUtil::GetElementFloat("XNorm", strXML, &bFoundNormX);
    m_normalizedLocationY   = (float) XMLUtil::GetElementFloat("YNorm", strXML, &bFoundNormY);
    
    // Convert the bits back to nats
    m_nats                  = (float) ((double) XMLUtil::GetElementFloat("Bits", strXML) * (double) log(2.0));

    // If there weren't X, Y elements, we want them set to 0 and mark
    // ourselves as not having them.
    if ((!bFoundX) && (!bFoundY))
    {
        m_locationX     = 0;
        m_locationY     = 0;
        m_bHasInteger   = false;
    }
    else
        m_bHasInteger   = true;
    
    // Require that we find both XNorm and YNorm in order to count
    if ((!bFoundNormX) || (!bFoundNormY))
    {
        m_normalizedLocationX   = 0.0;
        m_normalizedLocationY   = 0.0;
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
    char strNum[256];

    if (bReturnNormalized)
        sprintf(strNum, "%0.4f\t%0.4f\n", m_normalizedLocationX, m_normalizedLocationY);
    else
        sprintf(strNum, "%0.4f\t%0.4f\n", m_locationX, m_locationY);

    strResult += strNum;

    return strResult;
}

// Figure out what grid location this normalized mouse coordinate should go.
void CUserLocation::GetMouseGridLocation(int gridSize, int* i, int* j)
{
    //CFunctionLogger f1("CUserLocation::GetMouseGridLocation", gLogger);

    if ((i == NULL) || (j == NULL))
        return;

    *i = (int) (m_normalizedLocationX * (double) gridSize);
    *j = (int) (m_normalizedLocationY * (double) gridSize);
}

#endif


