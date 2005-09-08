#ifndef __parameters_h__
#define __parameters_h__

#include <string>

// All parameters go into the enums here
// They are unique across the different types
enum { 
  BP_DRAW_MOUSE_LINE, BP_DRAW_MOUSE,
  BP_SHOW_SLIDER, BP_START_MOUSE,
  BP_START_SPACE, BP_START_STYLUS, BP_STOP_IDLE, BP_KEY_CONTROL,  BP_CONTROL_MODE, 
  BP_COLOUR_MODE, BP_MOUSEPOS_MODE,
  BP_OUTLINE_MODE, BP_PALETTE_CHANGE, BP_NUMBER_DIMENSIONS,
  BP_EYETRACKER_MODE, BP_AUTOCALIBRATE, BP_DASHER_PAUSED,
  BP_GAME_MODE, BP_TRAINING, BP_REDRAW, BP_LM_DICTIONARY, 
  BP_LM_LETTER_EXCLUSION, BP_AUTO_SPEEDCONTROL, BP_CLICK_MODE, 
  BP_LM_ADAPTIVE, BP_BUTTONONESTATIC, BP_BUTTONONEDYNAMIC,
  BP_BUTTONMENU, BP_BUTTONPULSING, BP_BUTTONSTEADY, 
  BP_BUTTONDIRECT, BP_BUTTONFOURDIRECT, BP_BUTTONALTERNATINGDIRECT,
  BP_COMPASSMODE, END_OF_BPS
};

enum { 
  LP_ORIENTATION = END_OF_BPS, LP_REAL_ORIENTATION, LP_MAX_BITRATE,
  LP_VIEW_ID, LP_LANGUAGE_MODEL_ID, LP_DASHER_FONTSIZE,
  LP_UNIFORM, LP_YSCALE, LP_MOUSEPOSDIST, LP_STOP_IDLETIME, LP_TRUNCATION, 
  LP_TRUNCATIONTYPE, LP_LM_MAX_ORDER, LP_LM_EXCLUSION,
  LP_LM_UPDATE_EXCLUSION, LP_LM_ALPHA, LP_LM_BETA,
  LP_LM_MIXTURE, LP_MOUSE_POS_BOX, LP_NORMALIZATION, LP_LINE_WIDTH, 
  LP_LM_WORD_ALPHA, LP_USER_LOG_LEVEL_MASK, LP_SPEED_DIVISOR, 
  LP_ZOOMSTEPS, LP_B, LP_S, LP_Z, LP_R, LP_RIGHTZOOM, LP_SCREENWIDTH,
  LP_SCREENHEIGHT, LP_BOOSTFACTOR, END_OF_LPS
};

enum {
  SP_ALPHABET_ID = END_OF_LPS, SP_COLOUR_ID, SP_DASHER_FONT,
  SP_SYSTEM_LOC, SP_USER_LOC, SP_GAME_TEXT_FILE,
  SP_TRAIN_FILE, END_OF_SPS
};

// Define first int value of the first element of each type.
// Useful for offsetting into specific arrays,
// since each setting is a unique int, but all 3 arrays start at 0
#define FIRST_BP 0
#define FIRST_LP END_OF_BPS
#define FIRST_SP END_OF_LPS

// Define the number of each type of setting
#define NUM_OF_BPS END_OF_BPS
#define NUM_OF_LPS (END_OF_LPS - END_OF_BPS)
#define NUM_OF_SPS (END_OF_SPS - END_OF_LPS)

#define PERS true

// First level structures with only basic data types because you
// cannot initialize struct tables with objects
// These will be turned into std::strings in the ParamTables() object 
struct bp_table {
  int key;
  const char *regName;
  bool persistent;
  bool defaultValue;
  const char *humanReadable;
};
struct lp_table {
  int key;
  const char *regName;
  bool persistent;
  long defaultValue;
  const char *humanReadable;
};
struct sp_table {
  int key;
  const char *regName;
  bool persistent;
  const char *defaultValue;
  const char *humanReadable;
};

// The only important thing here is that these are in the same order
// as the enum declarations (could add check in class that enforces this instead)
static bp_table boolparamtable[] = {
  {BP_DRAW_MOUSE_LINE, "DrawMouseLine", PERS, false, "Draw Mouse Line"},
  {BP_DRAW_MOUSE, "DrawMouse", PERS, true, "Draw Mouse Position"},
  {BP_SHOW_SLIDER, "ShowSpeedSlider", PERS, true, "ShowSpeedSlider"},
  {BP_START_MOUSE, "StartOnLeft", PERS, true, "StartOnLeft"},
  {BP_START_SPACE, "StartOnSpace", PERS, false, "StartOnSpace"},
  {BP_START_STYLUS, "StartOnStylus", PERS, false, "StartOnStylus"},
  {BP_STOP_IDLE, "StopOnIdle", PERS, false, "StopOnIdle"},
  {BP_KEY_CONTROL, "KeyControl", PERS, false, "KeyControl"},
  {BP_CONTROL_MODE, "ControlMode", PERS, false, "ControlMode"},
  {BP_COLOUR_MODE, "ColourMode", PERS, true, "ColourMode"},
  {BP_MOUSEPOS_MODE, "StartOnMousePosition", PERS, false, "StartOnMousePosition"},
  {BP_OUTLINE_MODE, "OutlineBoxes", PERS, true, "OutlineBoxes"},
  {BP_PALETTE_CHANGE, "PaletteChange", PERS, true, "PaletteChange"},
  {BP_NUMBER_DIMENSIONS, "NumberDimensions", PERS, false, "NumberDimensions"},
  {BP_EYETRACKER_MODE, "EyetrackerMode", PERS, false, "EyetrackerMode"},
  {BP_AUTOCALIBRATE, "Autocalibrate", PERS, false, "Autocalibrate"},
  {BP_DASHER_PAUSED, "DasherPaused", !PERS, true, "Dasher Paused"},
  {BP_GAME_MODE, "GameMode", PERS, false, "Dasher Game Mode"},
  {BP_TRAINING, "Training", !PERS, false, "Provides locking during training"},
  {BP_REDRAW, "Redraw", !PERS, false, "Force a full redraw at the next timer event"},
  {BP_LM_DICTIONARY, "Dictionary", PERS, true, "Whether the word-based language model uses a dictionary"},
  {BP_LM_LETTER_EXCLUSION, "LetterExclusion", PERS, true, "Whether to do letter exclusion in the word-based model"},
  {BP_AUTO_SPEEDCONTROL, "AutoSpeedControl", PERS, true, "AutoSpeedControl"},
  {BP_CLICK_MODE, "ClickMode", PERS, false, "Dasher Click Mode"},
  {BP_LM_ADAPTIVE, "LMAdaptive", PERS, true, "Whether language model should learn as you enter text"},
  {BP_BUTTONONESTATIC, "ButtonOneStaticMode", PERS, false, "One-button static mode"},
  {BP_BUTTONONEDYNAMIC, "ButtonOneDynamicMode", PERS, false, "One-button dynamic mode"},
  {BP_BUTTONMENU, "ButtonMenuMode", PERS, false, "Button menu mode"},
  {BP_BUTTONPULSING, "ButtonPulsingMode", PERS, false, "One-button dynamic pulsing mode"},
  {BP_BUTTONSTEADY, "ButtonSteadyMode", PERS, true, "One-button dynamic steady mode"},
  {BP_BUTTONDIRECT, "ButtonDirectMode", PERS, false, "Three-button direct mode"},
  {BP_BUTTONFOURDIRECT, "ButtonFourDirectMode", PERS, false, "Four-button direct mode"},
  {BP_BUTTONALTERNATINGDIRECT, "ButtonAlternatingDirectMode", PERS, true, "Alternating direct mode"},
  {BP_COMPASSMODE, "ButtonCompassMode", PERS, false, "Compass mode"}
};

static lp_table longparamtable[] = {
  {LP_ORIENTATION, "ScreenOrientation", PERS, 0, "Screen Orientation"},
  {LP_REAL_ORIENTATION, "RealOrientation", !PERS, 0, "Actual screen orientation (allowing for alphabet default)"},
  {LP_MAX_BITRATE, "MaxBitRateTimes100", PERS, 150, "Max Bit Rate Times 100"},
  {LP_VIEW_ID, "ViewID", PERS, 1, "ViewID"},
  {LP_LANGUAGE_MODEL_ID, "LanguageModelID", PERS, 0, "LanguageModelID"},
  {LP_DASHER_FONTSIZE, "DasherFontSize", PERS, 1, "DasherFontSize"},
  {LP_UNIFORM, "UniformTimes1000", PERS, 50, "UniformTimes1000"},
  {LP_YSCALE, "YScaling", PERS, 0, "YScaling"},
  {LP_MOUSEPOSDIST, "MousePositionBoxDistance", PERS, 50, "MousePositionBoxDistance"},
  {LP_STOP_IDLETIME, "StopIdleTime", PERS, 1000, "StopIdleTime" },
  {LP_TRUNCATION, "Truncation", PERS, 0, "Truncation"},
  {LP_TRUNCATIONTYPE, "TruncationType", PERS, 0, "TruncationType"},
  {LP_LM_MAX_ORDER, "LMMaxOrder", PERS, 5, "LMMaxOrder"},
  {LP_LM_EXCLUSION, "LMExclusion", PERS, 0, "LMExclusion"},
  {LP_LM_UPDATE_EXCLUSION, "LMUpdateExclusion", PERS, 1, "LMUpdateExclusion"},
  {LP_LM_ALPHA, "LMAlpha", PERS, 100, "LMAlpha"},
  {LP_LM_BETA, "LMBeta", PERS, 100, "LMBeta"},
  {LP_LM_MIXTURE, "LMMixture", PERS, 50, "LMMixture"},
  {LP_MOUSE_POS_BOX, "MousePosBox", !PERS, -1, "Mouse Position Box Indicator"},
  {LP_NORMALIZATION, "Normalization", !PERS, 1 << 16, "Interval for child nodes"},
  {LP_LINE_WIDTH, "LineWidth", PERS, 1, "Width to draw crosshair and mouse line"},
  {LP_LM_WORD_ALPHA, "WordAlpha", PERS, 50, "Alpha value for word-based model"},
  {LP_USER_LOG_LEVEL_MASK, "UserLogLevelMask", PERS, 1, "Controls level of user logging, 0 = none, 1 = short, 2 = detailed, 3 = both"},
  {LP_SPEED_DIVISOR, "SpeedDivisor", !PERS, 100, "Factor by which to slow down (multiplied by 100)"},
  {LP_ZOOMSTEPS, "Zoomsteps", PERS, 32, "Integerised ratio of zoom size for click/button mode, denom 64."},
  {LP_B, "ButtonMenuBoxes", PERS, 4, "Number of boxes for button menu mode"},
  {LP_S, "ButtonMenuSafety", PERS, 25, "Safety parameter for button mode, in percent."},
  {LP_Z, "ButtonMenuBackwardsBox", PERS, 1, "Number of back-up boxes for button menu mode"},
  {LP_R, "ButtonModeNonuniformity", PERS, 0, "Button mode box non-uniformity"},
  {LP_RIGHTZOOM, "ButtonCompassModeRightZoom", PERS, 5, "Zoomfactor (*1024) for compass mode"},
  {LP_SCREENWIDTH, "ScreenWidth", PERS, 1, "Width of screen, in pixels."},
  {LP_SCREENHEIGHT, "ScreenHeight", PERS, 1, "Heightof screen, in pixels."},
  {LP_BOOSTFACTOR, "BoostFactor", !PERS, 100, "Boost/brake factor (multiplied by 100)"}
};

static sp_table stringparamtable[] = {
  {SP_ALPHABET_ID, "AlphabetID", PERS, "", "AlphabetID"},
  {SP_COLOUR_ID, "ColourID", PERS, "", "ColourID"},
  {SP_DASHER_FONT, "DasherFont", PERS, "", "DasherFont"},
  {SP_SYSTEM_LOC, "SystemLocation", !PERS, "sys_", "System Directory"},
  {SP_USER_LOC, "UserLocation", !PERS, "usr_", "User Directory"},
  {SP_GAME_TEXT_FILE, "GameTextFile", !PERS, "gamemode_english_GB.txt", "File with strings to practice writing"},
  {SP_TRAIN_FILE, "TrainingFile", !PERS, "", "Training text for alphabet"}
};

// This is the structure of each table that the settings will access
// Everything is const except the current value of the setting
struct bp_info {
  int key;
  std::string regName;
  bool persistent;
  bool value;
  bool defaultVal;
  std::string humanReadable;
};
struct lp_info {
  int key;
  std::string regName;
  bool persistent;
  long value;
  long defaultVal;
  std::string humanReadable;
};
struct sp_info {
  int key;
  std::string regName;
  bool persistent;
  std::string value;
  std::string defaultVal;
  std::string humanReadable;
};

namespace Dasher {
  class CParamTables;
} 

class Dasher::CParamTables {

// These are the parameter tables that store everything
public:
  bp_info BoolParamTable[NUM_OF_BPS];
  lp_info LongParamTable[NUM_OF_LPS];
  sp_info StringParamTable[NUM_OF_SPS];

public:
  CParamTables() {
    // Initialize all the tables with default values
    // and convert the char* to std::string in the object
    for(int ii = 0; ii < NUM_OF_BPS; ii++) {
      BoolParamTable[ii].key = boolparamtable[ii].key;
      BoolParamTable[ii].value = boolparamtable[ii].defaultValue;
      BoolParamTable[ii].defaultVal = boolparamtable[ii].defaultValue;
      BoolParamTable[ii].humanReadable = boolparamtable[ii].humanReadable;
      BoolParamTable[ii].persistent = boolparamtable[ii].persistent;
      BoolParamTable[ii].regName = boolparamtable[ii].regName;
    } 
    
    for(int ii = 0; ii < NUM_OF_LPS; ii++) {
      LongParamTable[ii].key = longparamtable[ii].key;
      LongParamTable[ii].value = longparamtable[ii].defaultValue;
      LongParamTable[ii].defaultVal = longparamtable[ii].defaultValue;
      LongParamTable[ii].humanReadable = longparamtable[ii].humanReadable;
      LongParamTable[ii].persistent = longparamtable[ii].persistent;
      LongParamTable[ii].regName = longparamtable[ii].regName;
    }

    for(int ii = 0; ii < NUM_OF_SPS; ii++) {
      StringParamTable[ii].key = stringparamtable[ii].key;
      StringParamTable[ii].value = stringparamtable[ii].defaultValue;
      StringParamTable[ii].defaultVal = stringparamtable[ii].defaultValue;
      StringParamTable[ii].humanReadable = stringparamtable[ii].humanReadable;
      StringParamTable[ii].persistent = stringparamtable[ii].persistent;
      StringParamTable[ii].regName = stringparamtable[ii].regName;
    }
  };

};

#endif
