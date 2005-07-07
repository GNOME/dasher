#ifndef __parameters_h__
#define __parameters_h__

// All parameters go into the enums here
// They are unique across the different types
enum {BP_DRAW_MOUSE_LINE, BP_DRAW_MOUSE, BP_TIME_STAMP, BP_SHOW_TOOLBAR, 
        BP_SHOW_TOOLBAR_TEXT, BP_SHOW_LARGE_ICONS, BP_FIX_LAYOUT, 
        BP_SHOW_SLIDER, BP_COPY_ALL_ON_STOP, BP_START_MOUSE, 
        BP_START_SPACE, BP_KEY_CONTROL, BP_WINDOW_PAUSE,
        BP_CONTROL_MODE, BP_COLOUR_MODE, BP_KEYBOARD_MODE, BP_MOUSEPOS_MODE,
        BP_SPEECH_MODE, BP_OUTLINE_MODE, BP_PALETTE_CHANGE, BP_NUMBER_DIMENSIONS, 
        BP_EYETRACKER_MODE, BP_AUTOCALIBRATE, BP_DASHER_PAUSED, END_OF_BPS};

enum {LP_ORIENTATION=END_OF_BPS, LP_MAX_BITRATE, LP_FILE_ENCODING, 
        LP_VIEW_ID, LP_LANGUAGE_MODEL_ID, LP_EDIT_FONT_SIZE, LP_EDIT_HEIGHT,
        LP_SCREEN_WIDTH, LP_SCREEN_HEIGHT, LP_DASHER_FONTSIZE, 
        LP_DASHER_DIMENSIONS, LP_DASHER_EYETRACKER, LP_UNIFORM, LP_YSCALE,
        LP_MOUSEPOSDIST, LP_TRUNCATION, LP_TRUNCATIONTYPE, LP_LM_MAX_ORDER,
        LP_LM_EXCLUSION, LP_LM_UPDATE_EXCLUSION, LP_LM_ALPHA, LP_LM_BETA,
        LP_LM_MIXTURE, LP_MOUSE_POS_BOX, LP_NORMALIZATION, END_OF_LPS};

enum {SP_ALPHABET_ID=END_OF_LPS, SP_COLOUR_ID, SP_DASHER_FONT,
        SP_EDIT_FONT, SP_SYSTEM_LOC, SP_USER_LOC, END_OF_SPS};

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

// First level structures with only basic data types because you
// cannot initialize struct tables with objects
// These will be turned into std::strings in the ParamTables() object 
struct bp_table {int key; char* regName; bool persistent; bool defaultValue;  char* humanReadable; };
struct lp_table {int key; char* regName; bool persistent; long defaultValue;  char* humanReadable; };
struct sp_table {int key; char* regName; bool persistent; char* defaultValue; char* humanReadable; };

// The only important thing here is that these are in the same order
// as the enum declarations (could add check in class that enforces this instead)
static bp_table boolparamtable[] = 
{
    {BP_DRAW_MOUSE_LINE,    "DrawMouse",           true,   true,   "Draw Mouse"},
    {BP_DRAW_MOUSE,         "DrawMouseLine",       true,   true,   "Draw Mouse Line"},
    {BP_TIME_STAMP,         "TimeStampNewFiles",   true,   true,   "TimeStampNewFiles"},
    {BP_SHOW_TOOLBAR,       "ViewToolbar",         true,   true,   "ViewToolbar"},
    {BP_SHOW_TOOLBAR_TEXT,  "ShowToolbarText",     true,   true,   "ShowToolbarText"},
    {BP_SHOW_LARGE_ICONS,   "ShowLargeIcons",      true,   true,   "ShowLargeIcons"},
    {BP_FIX_LAYOUT,         "FixLayout",           true,   true,   "FixLayout"},
    {BP_SHOW_SLIDER,        "ShowSpeedSlider",     true,   true,   "ShowSpeedSlider"},
    {BP_COPY_ALL_ON_STOP,   "CopyAllOnStop",       true,   true,   "CopyAllOnStop"},
    {BP_START_MOUSE,        "StartOnLeft",         true,   true,   "StartOnLeft"},
    {BP_START_SPACE,        "StartOnSpace",        true,   true,   "StartOnSpace"},
    {BP_KEY_CONTROL,        "KeyControl",          true,   true,   "KeyControl"},
    {BP_WINDOW_PAUSE,       "PauseOutsideWindow",  true,   true,   "PauseOutsideWindow"},
    {BP_CONTROL_MODE,       "ControlMode",         true,   true,   "ControlMode"},
    {BP_COLOUR_MODE,        "ColourMode",          true,   true,   "ColourMode"},
    {BP_KEYBOARD_MODE,      "KeyboardMode",        true,   true,   "KeyboardMode"},
    {BP_MOUSEPOS_MODE,      "StartOnMousePosition",true,   true,   "StartOnMousePosition"},
    {BP_SPEECH_MODE,        "SpeechEnabled",       true,   true,   "SpeechEnabled"},
    {BP_OUTLINE_MODE,       "OutlineBoxes",        true,   true,   "OutlineBoxes"},
    {BP_PALETTE_CHANGE,     "PaletteChange",       true,   true,   "PaletteChange"},
    {BP_NUMBER_DIMENSIONS,  "NumberDimensions",    true,   true,   "NumberDimensions"},
    {BP_EYETRACKER_MODE,    "EyetrackerMode",      true,   false,   "EyetrackerMode"},
    {BP_AUTOCALIBRATE,      "Autocalibrate",       true,   false,   "Autocalibrate"},
    {BP_DASHER_PAUSED,      "DasherPaused",        false,  false,   "Dasher Paused"}
};

static lp_table longparamtable[] =
{
    {LP_ORIENTATION,        "ScreenOrientation",    true,   1,      "Screen Orientation"},
    {LP_MAX_BITRATE,        "MaxBitRateTimes100",   true,   150,    "Max Bit Rate Times 100"},
    {LP_FILE_ENCODING,      "FileEncodingFormat",   true,   1,    "FileEncodingFormat"},
    {LP_VIEW_ID,            "ViewID",               true,   1,    "ViewID"},
    {LP_LANGUAGE_MODEL_ID,  "LanguageModelID",      true,   1,    "LanguageModelID"},
    {LP_EDIT_FONT_SIZE,     "EditFontSize",         true,   1,    "EditFontSize"},
    {LP_EDIT_HEIGHT,        "EditHeight",           true,   1,    "EditHeight0"},
    {LP_SCREEN_WIDTH,       "ScreenWidth",          true,   1,    "ScreenWidth"},
    {LP_SCREEN_HEIGHT,      "ScreenHeight",         true,   1,    "ScreenHeight"},
    {LP_DASHER_FONTSIZE,    "DasherFontSize",       true,   1,    "DasherFontSize"},
    {LP_DASHER_DIMENSIONS,  "NumberDimensions",     true,   1,    "NumberDimensions"},
    {LP_DASHER_EYETRACKER,  "EyetrackerMode",       true,   1,    "EyetrackerMode"},
    {LP_UNIFORM,            "UniformTimes1000",     true,   1,    "UniformTimes1000"},
    {LP_YSCALE,             "YScaling",             true,   1,    "YScaling"},
    {LP_MOUSEPOSDIST,       "MousePositionBoxDistance",   true,   1,    "MousePositionBoxDistance"},
    {LP_TRUNCATION,         "Truncation",           true,   1,    "Truncation"},
    {LP_TRUNCATIONTYPE,     "TruncationType",       true,   1,    "TruncationType"},
    {LP_LM_MAX_ORDER,       "LMMaxOrder",          true,   5,    "LMMaxOrder"},
    {LP_LM_EXCLUSION,       "LMExclusion",          true,   0,    "LMExclusion"},
    {LP_LM_UPDATE_EXCLUSION,"LMUpdateExclusion",    true,   1,    "LMUpdateExclusion"},
    {LP_LM_ALPHA,           "LMAlpha",              true,   100,  "LMAlpha"},
    {LP_LM_BETA,            "LMBeta",               true,   100,  "LMBeta"},
    {LP_LM_MIXTURE,         "LMMixture",            true,   50,   "LMMixture"},
    {LP_MOUSE_POS_BOX,      "MousePosBox",          false,  -1,   "Mouse Position Box Indicator"},
    {LP_NORMALIZATION,      "Normalization",        false,  1<<16, "Interval for child nodes"}
};
static sp_table stringparamtable[] =
{
    {SP_ALPHABET_ID, "AlphabetID", true, "", "AlphabetID"},
    {SP_COLOUR_ID, "ColourID", true, "", "ColourID"},
    {SP_DASHER_FONT, "DasherFont", true, "", "DasherFont"},
    {SP_EDIT_FONT, "EditFont", true, "", "EditFont"},
    {SP_SYSTEM_LOC, "SystemLocation", false, "", "System Directory"},
    {SP_USER_LOC,   "UserLocation", false, "", "User Directory"}
};
    

// This is the structure of each table that the settings will access
// Everything is const except the current value of the setting
struct bp_info {int key; std::string regName; bool persistent; bool value; std::string humanReadable; };
struct lp_info {int key; std::string regName; bool persistent; long value; std::string humanReadable; };
struct sp_info {int key; std::string regName; bool persistent; std::string value; std::string humanReadable; };

namespace Dasher {
  class CParamTables;
}

class Dasher::CParamTables
{

// These are the parameter tables that store everything
public:
    bp_info BoolParamTable[NUM_OF_BPS];
    lp_info LongParamTable[NUM_OF_LPS];
    sp_info StringParamTable[NUM_OF_SPS];
    
public:
    CParamTables()
    {
        // Initialize all the tables with default values
        // and convert the char* to std::string in the object
        for(int ii = 0; ii<NUM_OF_BPS; ii++)
        {
            BoolParamTable[ii].key = boolparamtable[ii].key;
            BoolParamTable[ii].value = boolparamtable[ii].defaultValue;
            BoolParamTable[ii].humanReadable = boolparamtable[ii].humanReadable;
            BoolParamTable[ii].persistent = boolparamtable[ii].persistent;
            BoolParamTable[ii].regName = boolparamtable[ii].regName;
        }

        for(int ii = 0; ii<NUM_OF_LPS; ii++)
        {
            LongParamTable[ii].key = longparamtable[ii].key;
            LongParamTable[ii].value = longparamtable[ii].defaultValue;
            LongParamTable[ii].humanReadable = longparamtable[ii].humanReadable;
            LongParamTable[ii].persistent = longparamtable[ii].persistent;
            LongParamTable[ii].regName = longparamtable[ii].regName;
        }

        for(int ii = 0; ii<NUM_OF_SPS; ii++)
        {
            StringParamTable[ii].key = stringparamtable[ii].key;
            StringParamTable[ii].value = stringparamtable[ii].defaultValue;
            StringParamTable[ii].humanReadable = stringparamtable[ii].humanReadable;
            StringParamTable[ii].persistent = stringparamtable[ii].persistent;
            StringParamTable[ii].regName = stringparamtable[ii].regName;
        }
    };

};

#endif
