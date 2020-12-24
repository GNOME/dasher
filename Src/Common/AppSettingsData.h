#include "AppSettingsHeader.h"
#include "../DasherCore/Parameters.h"

// This file is an include file purely for the purposes of
// cross-platform consistency. IT WILL NOT LINK IF INCLUDED IN MORE
// THAN ONE PLACE! You probably want to obtain values via the
// application settings object instead.

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

Dasher::Settings::bp_table app_boolparamtable[] = {
  { APP_BP_TIME_STAMP, "TimeStampNewFiles", Persistence::PERSISTENT, true, "TimeStampNewFiles" },
  { APP_BP_CONFIRM_UNSAVED, "ConfirmUnsavedFiles", Persistence::PERSISTENT, true, "ConfirmUnsavedFiles" },
  {APP_BP_SHOW_TOOLBAR, "ViewToolbar", Persistence::PERSISTENT, true, "ViewToolbar"},
  { APP_BP_SHOW_STATUSBAR, "ViewStatusbar", Persistence::PERSISTENT, true, "ViewStatusbar" },
  { APP_BP_MIRROR_LAYOUT, "MirrorLayout", Persistence::PERSISTENT, false, "MirrorLayout" },
  { APP_BP_FULL_SCREEN, "FullScreen", Persistence::PERSISTENT, false, "FullScreen" },
  { APP_BP_RESET_ON_FOCUS_CHANGE, "ResetOnFocusChange", Persistence::PERSISTENT, true, "ResetOnFocusChange" },
};

Dasher::Settings::lp_table app_longparamtable[] = {
  {APP_LP_FILE_ENCODING, "FileEncodingFormat", Persistence::PERSISTENT, -1, "FileEncodingFormat"},
  {APP_LP_EDIT_FONT_SIZE, "EditFontSize", Persistence::PERSISTENT, 0, "EditFontSize"},
  {APP_LP_EDIT_SIZE, "EditSize", Persistence::PERSISTENT, 75, "The size of the edit window"},
  {APP_LP_SCREEN_WIDTH, "ScreenWidth", Persistence::PERSISTENT, 400, "ScreenWidth"},
  {APP_LP_SCREEN_HEIGHT, "ScreenHeight", Persistence::PERSISTENT, 500, "ScreenHeight"},
  {APP_LP_STYLE, "AppStyle", Persistence::PERSISTENT, 0, "Application style"},
  {APP_LP_X, "XPosition", Persistence::PERSISTENT, 100, "X location of window"},
  {APP_LP_Y, "YPosition", Persistence::PERSISTENT, 100, "Y location of window"},
};

Dasher::Settings::sp_table app_stringparamtable[] = {
  {APP_SP_EDIT_FONT, "EditFont", Persistence::PERSISTENT, "Sans 10", "EditFont"},
  {APP_SP_TOOLBAR_ID, "ToolbarID", Persistence::PERSISTENT, "", "ToolbarID"},
};
