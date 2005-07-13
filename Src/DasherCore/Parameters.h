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
        BP_EYETRACKER_MODE, BP_AUTOCALIBRATE, BP_DASHER_PAUSED, 
        BP_GAME_MODE, END_OF_BPS};

enum {LP_ORIENTATION=END_OF_BPS, LP_MAX_BITRATE, LP_FILE_ENCODING, 
        LP_VIEW_ID, LP_LANGUAGE_MODEL_ID, LP_EDIT_FONT_SIZE, LP_EDIT_HEIGHT,
        LP_SCREEN_WIDTH, LP_SCREEN_HEIGHT, LP_DASHER_FONTSIZE, 
        LP_DASHER_DIMENSIONS, LP_DASHER_EYETRACKER, LP_UNIFORM, LP_YSCALE,
        LP_MOUSEPOSDIST, LP_TRUNCATION, LP_TRUNCATIONTYPE, LP_LM_MAX_ORDER,
        LP_LM_EXCLUSION, LP_LM_UPDATE_EXCLUSION, LP_LM_ALPHA, LP_LM_BETA,
        LP_LM_MIXTURE, LP_MOUSE_POS_BOX, LP_NORMALIZATION, END_OF_LPS};

enum {SP_ALPHABET_ID=END_OF_LPS, SP_COLOUR_ID, SP_DASHER_FONT,
        SP_EDIT_FONT, SP_SYSTEM_LOC, SP_USER_LOC, SP_GAME_TEXT_FILE, 
        SP_TRAIN_FILE, END_OF_SPS};

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
struct bp_table {int key; char* regName; bool persistent; bool defaultValue;  char* humanReadable; };
struct lp_table {int key; char* regName; bool persistent; long defaultValue;  char* humanReadable; };
struct sp_table {int key; char* regName; bool persistent; char* defaultValue; char* humanReadable; };

// The only important thing here is that these are in the same order
// as the enum declarations (could add check in class that enforces this instead)
static bp_table boolparamtable[] = 
{
    {BP_DRAW_MOUSE_LINE,    "DrawMouse",           PERS,   false,   "Draw Mouse"},
    {BP_DRAW_MOUSE,         "DrawMouseLine",       PERS,   true,   "Draw Mouse Line"},
    {BP_TIME_STAMP,         "TimeStampNewFiles",   PERS,   true,   "TimeStampNewFiles"},
    {BP_SHOW_TOOLBAR,       "ViewToolbar",         PERS,   true,   "ViewToolbar"},
    {BP_SHOW_TOOLBAR_TEXT,  "ShowToolbarText",     PERS,   true,   "ShowToolbarText"},
    {BP_SHOW_LARGE_ICONS,   "ShowLargeIcons",      PERS,   true,   "ShowLargeIcons"},
    {BP_FIX_LAYOUT,         "FixLayout",           PERS,   false,   "FixLayout"},
    {BP_SHOW_SLIDER,        "ShowSpeedSlider",     PERS,   true,   "ShowSpeedSlider"},
    {BP_COPY_ALL_ON_STOP,   "CopyAllOnStop",       PERS,   false,   "CopyAllOnStop"},
    {BP_START_MOUSE,        "StartOnLeft",         PERS,   true,   "StartOnLeft"},
    {BP_START_SPACE,        "StartOnSpace",        PERS,   false,   "StartOnSpace"},
    {BP_KEY_CONTROL,        "KeyControl",          PERS,   false,   "KeyControl"},
    {BP_WINDOW_PAUSE,       "PauseOutsideWindow",  PERS,   false,   "PauseOutsideWindow"},
    {BP_CONTROL_MODE,       "ControlMode",         PERS,   false,   "ControlMode"},
    {BP_COLOUR_MODE,        "ColourMode",          PERS,   true,   "ColourMode"},
    {BP_KEYBOARD_MODE,      "KeyboardMode",        PERS,   false,   "KeyboardMode"},
    {BP_MOUSEPOS_MODE,      "StartOnMousePosition",PERS,   false,   "StartOnMousePosition"},
    {BP_SPEECH_MODE,        "SpeechEnabled",       PERS,   false,   "SpeechEnabled"},
    {BP_OUTLINE_MODE,       "OutlineBoxes",        PERS,   true,   "OutlineBoxes"},
    {BP_PALETTE_CHANGE,     "PaletteChange",       PERS,   true,   "PaletteChange"},
    {BP_NUMBER_DIMENSIONS,  "NumberDimensions",    PERS,   false,   "NumberDimensions"},
    {BP_EYETRACKER_MODE,    "EyetrackerMode",      PERS,   false,   "EyetrackerMode"},
    {BP_AUTOCALIBRATE,      "Autocalibrate",       PERS,   false,   "Autocalibrate"},
    {BP_DASHER_PAUSED,      "DasherPaused",        !PERS,  false,   "Dasher Paused"},
    {BP_GAME_MODE,          "GameMode",            PERS,   false,   "Dasher Game Mode"}
};

static lp_table longparamtable[] =
{
    {LP_ORIENTATION,        "ScreenOrientation",    PERS,   0,      "Screen Orientation"},
    {LP_MAX_BITRATE,        "MaxBitRateTimes100",   PERS,   150,    "Max Bit Rate Times 100"},
    {LP_FILE_ENCODING,      "FileEncodingFormat",   PERS,   -2,    "FileEncodingFormat"},
    {LP_VIEW_ID,            "ViewID",               PERS,   1,    "ViewID"},
    {LP_LANGUAGE_MODEL_ID,  "LanguageModelID",      PERS,   1,    "LanguageModelID"},
    {LP_EDIT_FONT_SIZE,     "EditFontSize",         PERS,   1,    "EditFontSize"},
    {LP_EDIT_HEIGHT,        "EditHeight",           PERS,   75,    "EditHeight0"},
    {LP_SCREEN_WIDTH,       "ScreenWidth",          PERS,   400,    "ScreenWidth"},
    {LP_SCREEN_HEIGHT,      "ScreenHeight",         PERS,   500,    "ScreenHeight"},
    {LP_DASHER_FONTSIZE,    "DasherFontSize",       PERS,   1,    "DasherFontSize"},
    {LP_DASHER_DIMENSIONS,  "NumberDimensions",     PERS,   1,    "NumberDimensions"},
    {LP_DASHER_EYETRACKER,  "EyetrackerMode",       PERS,   1,    "EyetrackerMode"},
    {LP_UNIFORM,            "UniformTimes1000",     PERS,   50,    "UniformTimes1000"},
    {LP_YSCALE,             "YScaling",             PERS,   0,    "YScaling"},
    {LP_MOUSEPOSDIST,       "MousePositionBoxDistance",   PERS,   50,    "MousePositionBoxDistance"},
    {LP_TRUNCATION,         "Truncation",           PERS,   0,    "Truncation"},
    {LP_TRUNCATIONTYPE,     "TruncationType",       PERS,   0,    "TruncationType"},
    {LP_LM_MAX_ORDER,       "LMMaxOrder",           PERS,   5,    "LMMaxOrder"},
    {LP_LM_EXCLUSION,       "LMExclusion",          PERS,   0,    "LMExclusion"},
    {LP_LM_UPDATE_EXCLUSION,"LMUpdateExclusion",    PERS,   1,    "LMUpdateExclusion"},
    {LP_LM_ALPHA,           "LMAlpha",              PERS,   100,  "LMAlpha"},
    {LP_LM_BETA,            "LMBeta",               PERS,   100,  "LMBeta"},
    {LP_LM_MIXTURE,         "LMMixture",            PERS,   50,   "LMMixture"},
    {LP_MOUSE_POS_BOX,      "MousePosBox",          !PERS,  -1,   "Mouse Position Box Indicator"},
    {LP_NORMALIZATION,      "Normalization",        !PERS,  1<<16, "Interval for child nodes"}
};
static sp_table stringparamtable[] =
{
    {SP_ALPHABET_ID,    "AlphabetID",       PERS, "", "AlphabetID"},
    {SP_COLOUR_ID,      "ColourID",         PERS, "", "ColourID"},
    {SP_DASHER_FONT,    "DasherFont",       PERS, "", "DasherFont"},
    {SP_EDIT_FONT,      "EditFont",         PERS, "", "EditFont"},
    {SP_SYSTEM_LOC,     "SystemLocation",   !PERS, "sys_", "System Directory"},
    {SP_USER_LOC,       "UserLocation",     !PERS, "usr_", "User Directory"},
    {SP_GAME_TEXT_FILE, "GameTextFile",     !PERS, "", "File with strings to practice writing"},
    {SP_TRAIN_FILE,     "TrainingFile",     !PERS, "", "Training text for alphabet"}
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
