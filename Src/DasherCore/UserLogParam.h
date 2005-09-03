
#ifndef __USER_LOG_PARAM_H__
#define __USER_LOG_PARAM_H__

// Class that stores a particular parameter
// 
// Could be a struct, but we want to sort and need to
// overload the < operator.
//
// Copyright 2005 by Keith Vertanen

#include <string>
#include <vector>
#include "Parameters.h"

using namespace std;

// Bit mask options that are sent in when logging parameters
enum eUserLogParam
{
  userLogParamTrackMultiple   = 1,    // Should we track multiple values of a given parameter?
  userLogParamOutputToSimple  = 2,    // Does this parameter get sent to the simple log file?
  userLogParamTrackInTrial    = 4,    // Do we also store a copy of the parameter value within a trial?
  userLogParamForceInTrial    = 8,    // Do we always log the value of this parameter when a new trial is created?
  userLogParamShortInCycle    = 16    // In short logging, does the value get added to the end of a cycle stats line?
};

// We need to have a lookup table that maps parameters we want to track in 
// the UserLog object and what their behavior is.
struct UserLogParamMask {
  int key;
  int mask;
};

static UserLogParamMask s_UserLogParamMaskTable [] = {
  {SP_ALPHABET_ID,          userLogParamOutputToSimple},
  {SP_COLOUR_ID,            userLogParamOutputToSimple},
  {LP_MAX_BITRATE,          userLogParamOutputToSimple | 
                            userLogParamTrackMultiple | 
                            userLogParamTrackInTrial |
                            userLogParamForceInTrial |
                            userLogParamShortInCycle},
  {BP_CONTROL_MODE,         userLogParamOutputToSimple},
  {LP_UNIFORM,              userLogParamOutputToSimple},
  {LP_YSCALE,               userLogParamOutputToSimple},
  {BP_NUMBER_DIMENSIONS,    userLogParamOutputToSimple},
  {BP_EYETRACKER_MODE,      userLogParamOutputToSimple},
  {LP_LANGUAGE_MODEL_ID,    userLogParamOutputToSimple},
  {LP_LM_MAX_ORDER,         userLogParamOutputToSimple},
  {LP_LM_EXCLUSION,         userLogParamOutputToSimple},
  {LP_LM_UPDATE_EXCLUSION,  userLogParamOutputToSimple},
  {LP_LM_ALPHA,             userLogParamOutputToSimple},
  {LP_LM_BETA,              userLogParamOutputToSimple},
  {LP_LM_MIXTURE,           userLogParamOutputToSimple},
  {LP_LM_WORD_ALPHA,        userLogParamOutputToSimple},
  {-1, -1}  // Flag value that should always be at the end
};

class CUserLogParam;

typedef vector<CUserLogParam*>                      VECTOR_USER_LOG_PARAM_PTR;
typedef vector<CUserLogParam*>::iterator            VECTOR_USER_LOG_PARAM_PTR_ITER;
typedef vector<CUserLogParam*>::reverse_iterator    VECTOR_USER_LOG_PARAM_PTR_REV_ITER;

class CUserLogParam
{
public:
  string          strName;                // Name of the parameter
  string          strValue;               // String version of the value
  string          strTimeStamp;           // Optional timestamp if we want to know when a parameter was changed
  int             options;                // The options that were used on the parameter

  static bool     ComparePtr(CUserLogParam* pA, CUserLogParam* pB);
};

#endif

