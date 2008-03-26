#include "AppSettingsHeader.h"

// This file is an include file purely for the purposes of
// cross-platform consistency. IT WILL NOT LINK IF INCLUDED IN MORE
// THAN ONE PLACE! You probably want to obtain values via the
// application settings object instead.

#define PERS true

struct app_bp_table {
  int key;
  const char *regName;
  bool persistent;
  bool bDefaultValue;
  bool value;
  const char *humanReadable;
};

struct app_lp_table {
  int key;
  const char *regName;
  bool persistent;
  long iDefaultValue;
  long value;
  const char *humanReadable;
};

struct app_sp_table {
  int key;
  const char *regName;
  bool persistent;
  const char *szDefaultValue;
  char *value;
  const char *humanReadable;
};

// The following tables contain details of the application specific
// settings. Fields are:
//
// Key: See corresponding enum in AppSettingsHeader.h - entries *must*
// be in the same order as in the enum
// 
// Registry Name: Short text, without spaces. Used as the registry key
// and for other 'machine readable' purposes. Please make the name
// indicative of the purpose of the setting.
//
// Persistence: Whether this should be persistent or set at the begining of each setting.
//
// Default value: Hopefully pretty obvious
//
// Human-readable Name: Used for help text etc. Please make sure this
// is useful

app_bp_table app_boolparamtable[] = {
  {APP_BP_TIME_STAMP, "TimeStampNewFiles", PERS, true, true, "TimeStampNewFiles"},
  {APP_BP_SHOW_TOOLBAR, "ViewToolbar", PERS, true, true, "ViewToolbar"},
  {APP_BP_SHOW_TOOLBAR_TEXT, "ShowToolbarText", PERS, true, true, "ShowToolbarText"},
  {APP_BP_SHOW_LARGE_ICONS, "ShowLargeIcons", PERS, true, true, "ShowLargeIcons"},
  {APP_BP_FIX_LAYOUT, "FixLayout", PERS, false, false, "FixLayout"},
  {APP_BP_COPY_ALL_ON_STOP, "CopyAllOnStop", PERS, false, false, "CopyAllOnStop"},
  {APP_BP_WINDOW_PAUSE, "PauseOutsideWindow", PERS, false, false, "PauseOutsideWindow"},
  {APP_BP_SPEECH_MODE, "SpeechEnabled", PERS, false, false, "Speak on stop"},
  {APP_BP_KEYBOARD_MODE, "KeyboardMode", PERS, false, false, "KeyboardMode"},
  {APP_BP_DOCK, "Dock", PERS, false, false, "Dock main window"},
  {APP_BP_SPEECH_WORD, "SpeechWord", PERS, false, false, "Speak on word boundaries"}
};

app_lp_table app_longparamtable[] = {
  {APP_LP_FILE_ENCODING, "FileEncodingFormat", PERS, -2, -2, "FileEncodingFormat"},
  {APP_LP_EDIT_FONT_SIZE, "EditFontSize", PERS, 0, 0, "EditFontSize"},
  {APP_LP_EDIT_HEIGHT, "EditHeight", PERS, 75, 75, "The height of the edit window"},
  {APP_LP_EDIT_WIDTH, "EditWidth", PERS, 200, 200, "EditWidth"},
  {APP_LP_SCREEN_WIDTH, "ScreenWidth", PERS, 400, 400, "ScreenWidth"},
  {APP_LP_SCREEN_HEIGHT, "ScreenHeight", PERS, 500, 500, "ScreenHeight"},
  {APP_LP_SCREEN_WIDTH_H, "ScreenWidthH", PERS, 625, 625, "Screen Width for application style compose"},
  {APP_LP_SCREEN_HEIGHT_H, "ScreenHeightH", PERS, 250, 250, "Screen Height for application style compose"},
  {APP_LP_STYLE, "AppStyle", PERS, 0, 0, "Application style"},
  {APP_LP_DOCK_STYLE, "DockStyle", PERS, 4, 4, "Dock Style, 0-3 = docked, 4 = floating, 5 = fullscreen"},
  {APP_LP_DOCK_POSITION, "DockPosition", PERS, 100, 100, "Dock Position"},
  {APP_LP_X, "XPosition", PERS, 100, 100, "X location of window"},
  {APP_LP_Y, "YPosition", PERS, 100, 100, "Y location of window"},
#ifdef WITH_MAEMO
  {APP_LP_MAEMO_SIZE, "MaemoSize", PERS, 0, 0, "Size of Maemo input window"},
#endif
};

app_sp_table app_stringparamtable[] = {
#ifdef WITH_MAEMO
  {APP_SP_EDIT_FONT, "EditFont", PERS, "Sans 20", NULL, "EditFont"},
#else
  {APP_SP_EDIT_FONT, "EditFont", PERS, "Sans 10", NULL, "EditFont"},
#endif
#ifndef DASHER_WINCE
  {APP_SP_WINDOW_STATE, "WindowState", PERS, "", NULL, "WindowState"},
#endif
};
