#include "AppSettings.h"
#include "dasher.h"

// Define first int value of the first element of each type.
// Useful for offsetting into specific arrays,
// since each setting is a unique int, but all 3 arrays start at 0
#define FIRST_APP_BP 0
#define FIRST_APP_LP END_OF_APP_BPS
#define FIRST_APP_SP END_OF_APP_LPS

// Define the number of each type of setting
#define NUM_OF_APP_BPS END_OF_APP_BPS
#define NUM_OF_APP_LPS (END_OF_APP_LPS - END_OF_APP_BPS)
#define NUM_OF_APP_SPS (END_OF_APP_SPS - END_OF_APP_LPS)

#define PERS true

// First level structures with only basic data types because you
// cannot initialize struct tables with objects
// These will be turned into std::strings in the ParamTables() object 
struct bp_table {
  int key;
  char *regName;
  bool persistent;
  bool value;
  char *humanReadable;
};
struct lp_table {
  int key;
  char *regName;
  bool persistent;
  long value;
  char *humanReadable;
};
struct sp_table {
  int key;
  char *regName;
  bool persistent;
  char *value;
  char *humanReadable;
};

// The only important thing here is that these are in the same order
// as the enum declarations (could add check in class that enforces this instead)
static bp_table boolparamtable[] = {
  {APP_BP_DRAW_MOUSE_LINE, "DrawMouseLine", PERS, false, "Draw Mouse Line"},
  {APP_BP_DRAW_MOUSE, "DrawMouse", PERS, true, "Draw Mouse Position"},
  {APP_BP_TIME_STAMP, "TimeStampNewFiles", PERS, true, "TimeStampNewFiles"},
  {APP_BP_SHOW_TOOLBAR, "ViewToolbar", PERS, true, "ViewToolbar"},
  {APP_BP_SHOW_TOOLBAR_TEXT, "ShowToolbarText", PERS, true, "ShowToolbarText"},
  {APP_BP_SHOW_LARGE_ICONS, "ShowLargeIcons", PERS, true, "ShowLargeIcons"},
  {APP_BP_FIX_LAYOUT, "FixLayout", PERS, false, "FixLayout"},
  {APP_BP_SHOW_SLIDER, "ShowSpeedSlider", PERS, true, "ShowSpeedSlider"},
  {APP_BP_COPY_ALL_ON_STOP, "CopyAllOnStop", PERS, false, "CopyAllOnStop"},
  {APP_BP_START_MOUSE, "StartOnLeft", PERS, true, "StartOnLeft"},
  {APP_BP_START_SPACE, "StartOnSpace", PERS, false, "StartOnSpace"},
  {APP_BP_KEY_CONTROL, "KeyControl", PERS, false, "KeyControl"},
  {APP_BP_WINDOW_PAUSE, "PauseOutsideWindow", PERS, false, "PauseOutsideWindow"},
  {APP_BP_CONTROL_MODE, "ControlMode", PERS, false, "ControlMode"},
  {APP_BP_COLOUR_MODE, "ColourMode", PERS, true, "ColourMode"},
  {APP_BP_KEYBOARD_MODE, "KeyboardMode", PERS, false, "KeyboardMode"},
  {APP_BP_MOUSEPOS_MODE, "StartOnMousePosition", PERS, false, "StartOnMousePosition"},
  {APP_BP_SPEECH_MODE, "SpeechEnabled", PERS, false, "SpeechEnabled"},
  {APP_BP_OUTLINE_MODE, "OutlineBoxes", PERS, true, "OutlineBoxes"},
  {APP_BP_PALETTE_CHANGE, "PaletteChange", PERS, true, "PaletteChange"},
  {APP_BP_NUMBER_DIMENSIONS, "NumberDimensions", PERS, false, "NumberDimensions"},
  {APP_BP_EYETRACKER_MODE, "EyetrackerMode", PERS, false, "EyetrackerMode"},
  {APP_BP_AUTOCALIBRATE, "Autocalibrate", PERS, false, "Autocalibrate"},
  {APP_BP_DASHER_PAUSED, "DasherPaused", !PERS, true, "Dasher Paused"},
  {APP_BP_GAME_MODE, "GameMode", PERS, false, "Dasher Game Mode"},
  {APP_BP_TRAINING, "Training", !PERS, false, "Provides locking during training"},
  {APP_BP_REDRAW, "Redraw", !PERS, false, "Force a full redraw at the next timer event"},
  {APP_BP_LM_DICTIONARY, "Dictionary", PERS, true, "Whether the word-based language model uses a dictionary"},
  {APP_BP_LM_LETTER_EXCLUSION, "LetterExclusion", PERS, false, "Whether to do letter exclusion in the word-based model"}
};

static lp_table longparamtable[] = {
  {APP_LP_ORIENTATION, "ScreenOrientation", PERS, 0, "Screen Orientation"},
  {APP_LP_REAL_ORIENTATION, "RealOrientation", !PERS, 0, "Actual screen orientation (allowing for alphabet default)"},
  {APP_LP_MAX_BITRATE, "MaxBitRateTimes100", PERS, 150, "Max Bit Rate Times 100"},
  {APP_LP_FILE_ENCODING, "FileEncodingFormat", PERS, -2, "FileEncodingFormat"},
  {APP_LP_VIEW_ID, "ViewID", PERS, 1, "ViewID"},
  {APP_LP_LANGUAGE_MODEL_ID, "LanguageModelID", PERS, 1, "LanguageModelID"},
  {APP_LP_EDIT_FONT_SIZE, "EditFontSize", PERS, 1, "EditFontSize"},
  {APP_LP_EDIT_HEIGHT, "EditHeight", PERS, 75, "EditHeight0"},
  {APP_LP_SCREEN_WIDTH, "ScreenWidth", PERS, 400, "ScreenWidth"},
  {APP_LP_SCREEN_HEIGHT, "ScreenHeight", PERS, 500, "ScreenHeight"},
  {APP_LP_DASHER_FONTSIZE, "DasherFontSize", PERS, 1, "DasherFontSize"},
  {APP_LP_UNIFORM, "UniformTimes1000", PERS, 50, "UniformTimes1000"},
  {APP_LP_YSCALE, "YScaling", PERS, 0, "YScaling"},
  {APP_LP_MOUSEPOSDIST, "MousePositionBoxDistance", PERS, 50, "MousePositionBoxDistance"},
  {APP_LP_TRUNCATION, "Truncation", PERS, 0, "Truncation"},
  {APP_LP_TRUNCATIONTYPE, "TruncationType", PERS, 0, "TruncationType"},
  {APP_LP_LM_MAX_ORDER, "LMMaxOrder", PERS, 5, "LMMaxOrder"},
  {APP_LP_LM_EXCLUSION, "LMExclusion", PERS, 0, "LMExclusion"},
  {APP_LP_LM_UPDATE_EXCLUSION, "LMUpdateExclusion", PERS, 1, "LMUpdateExclusion"},
  {APP_LP_LM_ALPHA, "LMAlpha", PERS, 100, "LMAlpha"},
  {APP_LP_LM_BETA, "LMBeta", PERS, 100, "LMBeta"},
  {APP_LP_LM_MIXTURE, "LMMixture", PERS, 50, "LMMixture"},
  {APP_LP_MOUSE_POS_BOX, "MousePosBox", !PERS, -1, "Mouse Position Box Indicator"},
  {APP_LP_NORMALIZATION, "Normalization", !PERS, 1 << 16, "Interval for child nodes"},
  {APP_LP_LINE_WIDTH, "LineWidth", PERS, 1, "Width to draw crosshair and mouse line"},
  {APP_LP_LM_WORD_ALPHA, "WordAlpha", PERS, 50, "Alpha value for word-based model"}
};

static sp_table stringparamtable[] = {
  {APP_SP_ALPHABET_ID, "AlphabetID", PERS, "", "AlphabetID"},
  {APP_SP_COLOUR_ID, "ColourID", PERS, "", "ColourID"},
  {APP_SP_DASHER_FONT, "DasherFont", PERS, "", "DasherFont"},
  {APP_SP_EDIT_FONT, "EditFont", PERS, "", "EditFont"},
  {APP_SP_SYSTEM_LOC, "SystemLocation", !PERS, "sys_", "System Directory"},
  {APP_SP_USER_LOC, "UserLocation", !PERS, "usr_", "User Directory"},
  {APP_SP_GAME_TEXT_FILE, "GameTextFile", !PERS, "", "File with strings to practice writing"},
  {APP_SP_TRAIN_FILE, "TrainingFile", !PERS, "", "Training text for alphabet"}
};

void handle_parameter_change( int iParameter );

bool get_app_parameter_bool( int iParameter ) {
  return boolparamtable[ iParameter - FIRST_APP_BP ].value;
}

void set_app_parameter_bool( int iParameter, bool bValue ) {
  boolparamtable[ iParameter - FIRST_APP_BP ].value = bValue;
  handle_parameter_change( iParameter );
}

void handle_parameter_change( int iParameter ) {
  // TODO - pass this to individual components? Do we want a full event framework?

  switch( iParameter ) {
  case APP_BP_SHOW_TOOLBAR: // TODO - update UI widget here
    if( get_app_parameter_bool( APP_BP_SHOW_TOOLBAR ))
      gtk_widget_show( toolbar );
    else
      gtk_widget_hide( toolbar );
    break;
  }

}
