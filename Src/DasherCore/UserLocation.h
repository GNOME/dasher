
// Keeps track of the user's mouse location at a certain point in time.
//
// Can store a integer x, y coordinate and/or a normalized floating
// point x, y pair where (1.0, 1.0) is the lower right corener of the
// window.
//
// Copyright 2005 by Keith Vertanen

#ifndef __USER_LOCATION_H__
#define __USER_LOCATION_H__

#include "FileLogger.h"
#include <string>
#include <sys/timeb.h>
#include <time.h>
#include "TimeSpan.h"
#include <math.h>

#ifdef USER_LOG_TOOL
    #include "XMLUtil.h"
#endif

using namespace std;

extern CFileLogger* gLogger;

class CUserLocation;

typedef vector<CUserLocation>    VECTOR_USER_LOCATION;
typedef vector<CUserLocation*>   VECTOR_USER_LOCATION_PTR;

class CUserLocation
{
public:
    CUserLocation(int x, int y, float nats);
    CUserLocation(float x, float y, float nats);
    CUserLocation(int x1, int y1, float x2, float y2, float nats);
    CUserLocation(int x, int y, int top, int left, int bottom, int right, bool bStoreIntegerRep, float nats);
    ~CUserLocation();

#ifdef USER_LOG_TOOL
    CUserLocation(const string& strXML);
#endif


    string              GetXML(const string& prefix = "");
    static double       ComputeNormalizedX(int x, int left, int right);
    static double       ComputeNormalizedY(int y, int top, int bottom);

private:
    string              m_strTime;
    int                 m_locationX;
    int                 m_locationY;
    float               m_normalizedLocationX;
    float               m_normalizedLocationY;
    bool                m_bHasNormalized;           // Are we storing a normalized representation?
    bool                m_bHasInteger;              // Are we storing an integer representation?
    float               m_nats;

    void                InitMemeberVars();
};

#endif
