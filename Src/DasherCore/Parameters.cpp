#include "Parameters.h"

namespace Dasher{
  namespace Settings {
    
// The important thing here is that these are in the same order
// as the enum declarations (could add check in class that enforces this instead)
const bp_table boolparamtable[] = {
  {BP_DRAW_MOUSE_LINE, "DrawMouseLine", Persistence::PERSISTENT, true, "Draw Mouse Line"},
  {BP_DRAW_MOUSE, "DrawMouse", Persistence::PERSISTENT, true, "Draw Mouse Position"},
  {BP_CURVE_MOUSE_LINE, "CurveMouseLine", Persistence::PERSISTENT, false, "Curve mouse line according to screen nonlinearity"},
  {BP_START_MOUSE, "StartOnLeft", Persistence::PERSISTENT, true, "StartOnLeft"},
  {BP_START_SPACE, "StartOnSpace", Persistence::PERSISTENT, false, "StartOnSpace"},
  {BP_CONTROL_MODE, "ControlMode", Persistence::PERSISTENT, false, "ControlMode"},
  {BP_MOUSEPOS_MODE, "StartOnMousePosition", Persistence::PERSISTENT, false, "StartOnMousePosition"},
  {BP_PALETTE_CHANGE, "PaletteChange", Persistence::PERSISTENT, true, "PaletteChange"},
  {BP_TURBO_MODE, "TurboMode", Persistence::PERSISTENT, true, "Boost speed when holding key1 or right mouse button"},
  {BP_EXACT_DYNAMICS, "ExactDynamics", Persistence::PERSISTENT, false, "Use exact computation of per-frame movement (slower)"},
  {BP_AUTOCALIBRATE, "Autocalibrate", Persistence::PERSISTENT, false, "Automatically learn TargetOffset e.g. gazetracking"},
  {BP_REMAP_XTREME, "RemapXtreme", Persistence::PERSISTENT, false, "Pointer at extreme Y translates more and zooms less"},
  {BP_AUTO_SPEEDCONTROL, "AutoSpeedControl", Persistence::PERSISTENT, true, "AutoSpeedControl"},
  {BP_LM_ADAPTIVE, "LMAdaptive", Persistence::PERSISTENT, true, "Whether language model should learn as you enter text"},
  {BP_SOCKET_DEBUG, "SocketInputDebug", Persistence::PERSISTENT, false, "Print information about socket input processing to console"},
  {BP_CIRCLE_START, "CircleStart", Persistence::PERSISTENT, false, "Start on circle mode"},
  {BP_GLOBAL_KEYBOARD, "GlobalKeyboard", Persistence::PERSISTENT, false, "Whether to assume global control of the keyboard"},
  {BP_NONLINEAR_Y, "NonlinearY", Persistence::PERSISTENT, true, "Apply nonlinearities to Y axis (i.e. compress top &amp; bottom)"},
  {BP_STOP_OUTSIDE, "PauseOutside", Persistence::PERSISTENT, false, "Whether to stop when pointer leaves canvas area"},
#ifdef TARGET_OS_IPHONE
  {BP_BACKOFF_BUTTON, "BackoffButton", Persistence::PERSISTENT, false, "Whether to enable the extra backoff button in dynamic mode"},
#else
  {BP_BACKOFF_BUTTON, "BackoffButton", Persistence::PERSISTENT, true, "Whether to enable the extra backoff button in dynamic mode"},
#endif
  {BP_TWOBUTTON_REVERSE, "TwoButtonReverse", Persistence::PERSISTENT, false, "Reverse the up/down buttons in two button mode"},
  {BP_2B_INVERT_DOUBLE, "TwoButtonInvertDouble", Persistence::PERSISTENT, false, "Double-press acts as opposite button in two-button mode"},
  {BP_SLOW_START, "SlowStart", Persistence::PERSISTENT, false, "Start at low speed and increase"},
  {BP_COPY_ALL_ON_STOP, "CopyOnStop", Persistence::PERSISTENT, false, "Copy all text to clipboard whenever we stop"},
  {BP_SPEAK_ALL_ON_STOP, "SpeakOnStop", Persistence::PERSISTENT, false, "Speak all text whenever we stop"},
  {BP_SPEAK_WORDS, "SpeakWords", Persistence::PERSISTENT, false, "Speak words as they are written"},
  {BP_GAME_HELP_DRAW_PATH, "GameDrawPath", Persistence::PERSISTENT, true, "When we give help, show the shortest path to the target sentence"},
  {BP_TWO_PUSH_RELEASE_TIME, "TwoPushReleaseTime", Persistence::PERSISTENT, false, "Use push and release times of single press rather than push times of two presses"},
  {BP_SLOW_CONTROL_BOX, "SlowControlBox", Persistence::PERSISTENT, true, "Slow down when going through control box" },
};

const lp_table longparamtable[] = {
  {LP_ORIENTATION, "ScreenOrientation", Persistence::PERSISTENT, -2, "Screen Orientation"},
  {LP_MAX_BITRATE, "MaxBitRateTimes100", Persistence::PERSISTENT, 80, "Max Bit Rate Times 100"},
  {LP_FRAMERATE, "FrameRate", Persistence::EPHEMERAL, 3200, "Decaying average of last known frame rates, *100"},
  {LP_LANGUAGE_MODEL_ID, "LanguageModelID", Persistence::PERSISTENT, 0, "LanguageModelID"},
  {LP_DASHER_FONTSIZE, "DasherFontSize", Persistence::PERSISTENT, 2, "DasherFontSize"},
  {LP_MESSAGE_FONTSIZE, "MessageFontSize", Persistence::PERSISTENT, 14, "Size of font for messages (in points)"},
  {LP_SHAPE_TYPE, "RenderStyle", Persistence::PERSISTENT, 1, "Shapes to render in (0/1=disjoint/overlapping rects, 2/3=triangles/truncated, 4=quadrics, 5=circles)"},
  {LP_UNIFORM, "UniformTimes1000", Persistence::PERSISTENT, 50, "UniformTimes1000"},
  {LP_YSCALE, "YScaling", Persistence::PERSISTENT, 0, "YScaling"},
  {LP_MOUSEPOSDIST, "MousePositionBoxDistance", Persistence::PERSISTENT, 50, "MousePositionBoxDistance"},
  {LP_PY_PROB_SORT_THRES, "PYProbabilitySortThreshold", Persistence::PERSISTENT, 85, "Sort converted syms in descending probability order up to this percentage"},
  {LP_MESSAGE_TIME, "MessageTime", Persistence::PERSISTENT, 2500, "Time for which non-modal messages are displayed, in ms"},
  {LP_LM_MAX_ORDER, "LMMaxOrder", Persistence::PERSISTENT, 5, "LMMaxOrder"},
  { LP_LM_EXCLUSION, "LMExclusion", Persistence::PERSISTENT, 0, "LMExclusion" },
  {LP_LM_UPDATE_EXCLUSION, "LMUpdateExclusion", Persistence::PERSISTENT, 1, "LMUpdateExclusion"},
  {LP_LM_ALPHA, "LMAlpha", Persistence::PERSISTENT, 49, "LMAlpha"},
  {LP_LM_BETA, "LMBeta", Persistence::PERSISTENT, 77, "LMBeta"},
  {LP_LM_MIXTURE, "LMMixture", Persistence::PERSISTENT, 50, "LMMixture"},
  {LP_LINE_WIDTH, "LineWidth", Persistence::PERSISTENT, 1, "Width to draw crosshair and mouse line"},
  {LP_GEOMETRY, "Geometry", Persistence::PERSISTENT, 0, "Screen geometry (mostly for tall thin screens) - 0=old-style, 1=square no-xhair, 2=squish, 3=squish+log"},
  {LP_LM_WORD_ALPHA, "WordAlpha", Persistence::PERSISTENT, 50, "Alpha value for word-based model"},
  {LP_USER_LOG_LEVEL_MASK, "UserLogLevelMask", Persistence::PERSISTENT, 0, "Controls level of user logging, 0 = none, 1 = short, 2 = detailed, 3 = both"},
  {LP_ZOOMSTEPS, "Zoomsteps", Persistence::PERSISTENT, 32, "Integerised ratio of zoom size for click/button mode, denom 64."},
  {LP_B, "ButtonMenuBoxes", Persistence::PERSISTENT, 4, "Number of boxes for button menu mode"},
  {LP_S, "ButtonMenuSafety", Persistence::PERSISTENT, 25, "Safety parameter for button mode, in percent."},
#ifdef TARGET_OS_IPHONE
  {LP_BUTTON_SCAN_TIME, "ButtonMenuScanTime", Persistence::PERSISTENT, 600, "Scanning time in menu mode (0 = don't scan), in ms"},
#else
  {LP_BUTTON_SCAN_TIME, "ButtonMenuScanTime", Persistence::PERSISTENT, 0, "Scanning time in menu mode (0 = don't scan), in ms"},
#endif
  {LP_R, "ButtonModeNonuniformity", Persistence::PERSISTENT, 0, "Button mode box non-uniformity"},
  {LP_RIGHTZOOM, "ButtonCompassModeRightZoom", Persistence::PERSISTENT, 5120, "Zoomfactor (*1024) for compass mode"},
#ifdef TARGET_OS_IPHONE
  {LP_NODE_BUDGET, "NodeBudget", Persistence::PERSISTENT, 1000, "Target (min) number of node objects to maintain"},
#else
  {LP_NODE_BUDGET, "NodeBudget", Persistence::PERSISTENT, 3000, "Target (min) number of node objects to maintain"},
#endif
  {LP_OUTLINE_WIDTH, "OutlineWidth", Persistence::PERSISTENT, 0, "Absolute value is line width to draw boxes (fill iff >=0)" },
  {LP_MIN_NODE_SIZE, "MinNodeSize", Persistence::PERSISTENT, 50, "Minimum size of node (in dasher coords) to draw" }, 
  {LP_NONLINEAR_X, "NonLinearX", Persistence::PERSISTENT, 5, "Nonlinear compression of X-axis (0 = none, higher = more extreme)"},
  {LP_AUTOSPEED_SENSITIVITY, "AutospeedSensitivity", Persistence::PERSISTENT, 100, "Sensitivity of automatic speed control (percent)"},
  {LP_SOCKET_PORT, "SocketPort", Persistence::PERSISTENT, 20320, "UDP/TCP socket to use for network socket input"},
  {LP_SOCKET_INPUT_X_MIN, "SocketInputXMinTimes1000", Persistence::PERSISTENT, 0, "Bottom of range of X values expected from network input"},
  {LP_SOCKET_INPUT_X_MAX, "SocketInputXMaxTimes1000", Persistence::PERSISTENT, 1000, "Top of range of X values expected from network input"},
  {LP_SOCKET_INPUT_Y_MIN, "SocketInputYMinTimes1000", Persistence::PERSISTENT, 0, "Bottom of range of Y values expected from network input"},
  {LP_SOCKET_INPUT_Y_MAX, "SocketInputYMaxTimes1000", Persistence::PERSISTENT, 1000, "Top of range of Y values expected from network input"},
  {LP_CIRCLE_PERCENT, "CirclePercent", Persistence::PERSISTENT, 10, "Percentage of nominal vertical range to use for radius of start circle"},
  {LP_TWO_BUTTON_OFFSET, "TwoButtonOffset", Persistence::PERSISTENT, 1638, "Offset for two button dynamic mode"},
  {LP_HOLD_TIME, "HoldTime", Persistence::PERSISTENT, 1000, "Time for which buttons must be held to count as long presses, in ms"},
  {LP_MULTIPRESS_TIME, "MultipressTime", Persistence::PERSISTENT, 1000, "Time in which multiple presses must occur, in ms"},
  {LP_SLOW_START_TIME, "SlowStartTime", Persistence::PERSISTENT, 1000, "Time over which slow start occurs"},
  {LP_TWO_PUSH_OUTER, "TwoPushOuter", Persistence::PERSISTENT, 1792, "Offset for one button dynamic mode outer marker"},
  {LP_TWO_PUSH_LONG, "TwoPushLong", Persistence::PERSISTENT, 512, "Distance between down markers (long gap)"},
  {LP_TWO_PUSH_SHORT, "TwoPushShort", Persistence::PERSISTENT, 80, "Distance between up markers, as percentage of long gap"},
  {LP_TWO_PUSH_TOLERANCE, "TwoPushTolerance", Persistence::PERSISTENT, 100, "Tolerance of two-push-mode pushes, in ms"},
  {LP_DYNAMIC_BUTTON_LAG, "DynamicButtonLag", Persistence::PERSISTENT, 50, "Lag of pushes in dynamic button mode (ms)"},
  {LP_STATIC1B_TIME, "Static1BTime", Persistence::PERSISTENT, 2000, "Time for static-1B mode to scan from top to bottom (ms)"},
  {LP_STATIC1B_ZOOM, "Static1BZoom", Persistence::PERSISTENT, 8, "Zoom factor for static-1B mode"},
  {LP_DEMO_SPRING, "DemoSpring", Persistence::PERSISTENT, 100, "Springyness in Demo-mode"},
  {LP_DEMO_NOISE_MEM, "DemoNoiseMem", Persistence::PERSISTENT, 100, "Memory parameter for noise in Demo-mode"},
  {LP_DEMO_NOISE_MAG, "DemoNoiseMag", Persistence::PERSISTENT, 325, "Magnitude of noise in Demo-mode"},
  {LP_MAXZOOM, "ClickMaxZoom", Persistence::PERSISTENT, 200, "Maximum zoom possible in click mode (times 10)"},
  {LP_DYNAMIC_SPEED_INC, "DynamicSpeedInc", Persistence::PERSISTENT, 3, "%age by which dynamic mode auto speed control increases speed"},
  {LP_DYNAMIC_SPEED_FREQ, "DynamicSpeedFreq", Persistence::PERSISTENT, 10, "Seconds after which dynamic mode auto speed control increases speed"},
  {LP_DYNAMIC_SPEED_DEC, "DynamicSpeedDec", Persistence::PERSISTENT, 8, "%age by which dynamic mode auto speed control decreases speed on reverse"},
  {LP_TAP_TIME, "TapTime", Persistence::PERSISTENT, 200, "Max length of a stylus 'tap' rather than hold (ms)"},
#ifdef TARGET_OS_IPHONE
  {LP_MARGIN_WIDTH, "MarginWidth", Persistence::PERSISTENT, 500, "Width of RHS margin (in Dasher co-ords)"},
#else
  {LP_MARGIN_WIDTH, "MarginWidth", Persistence::PERSISTENT, 300, "Width of RHS margin (in Dasher co-ords)"},
#endif
  {LP_TARGET_OFFSET, "TargetOffset", Persistence::PERSISTENT, 0, "Vertical distance between mouse pointer and target (400=screen height)"},
  {LP_X_LIMIT_SPEED, "XLimitSpeed", Persistence::PERSISTENT, 800, "X Co-ordinate at which maximum speed is reached (&lt;2048=xhair)"},
  {LP_GAME_HELP_DIST, "GameHelpDistance", Persistence::PERSISTENT, 1920, "Distance of sentence from center to decide user needs help"},
  {LP_GAME_HELP_TIME, "GameHelpTime", Persistence::PERSISTENT, 0, "Time for which user must need help before help drawn"},
};

const sp_table stringparamtable[] = {
  {SP_ALPHABET_ID, "AlphabetID", Persistence::PERSISTENT, "", "AlphabetID"},
  {SP_ALPHABET_1, "Alphabet1", Persistence::PERSISTENT, "", "Alphabet History 1"},
  {SP_ALPHABET_2, "Alphabet2", Persistence::PERSISTENT, "", "Alphabet History 2"},
  {SP_ALPHABET_3, "Alphabet3", Persistence::PERSISTENT, "", "Alphabet History 3"},
  {SP_ALPHABET_4, "Alphabet4", Persistence::PERSISTENT, "", "Alphabet History 4"},
  { SP_COLOUR_ID, "ColourID", Persistence::PERSISTENT, "", "ColourID" },
  { SP_CONTROL_BOX_ID, "ControlBoxID", Persistence::PERSISTENT, "", "ControlBoxID" },
  {SP_DASHER_FONT, "DasherFont", Persistence::PERSISTENT, "", "DasherFont"},
  {SP_GAME_TEXT_FILE, "GameTextFile", Persistence::PERSISTENT, "", "User-specified file with strings to practice writing"},
  {SP_SOCKET_INPUT_X_LABEL, "SocketInputXLabel", Persistence::PERSISTENT, "x", "Label preceding X values for network input"},
  {SP_SOCKET_INPUT_Y_LABEL, "SocketInputYLabel", Persistence::PERSISTENT, "y", "Label preceding Y values for network input"},
#ifdef TARGET_OS_IPHONE
  {SP_INPUT_FILTER, "InputFilter", Persistence::PERSISTENT, "Stylus Control", "Input filter used to provide the current control mode"},
#else
  {SP_INPUT_FILTER, "InputFilter", Persistence::PERSISTENT, "Normal Control", "Input filter used to provide the current control mode"},
#endif
  {SP_INPUT_DEVICE, "InputDevice", Persistence::PERSISTENT, "Mouse Input", "Driver for the input device"},
  {SP_BUTTON_0, "Button0", Persistence::PERSISTENT, "", "Assignment to button 0"},
  {SP_BUTTON_1, "Button1", Persistence::PERSISTENT, "", "Assignment to button 1"},
  {SP_BUTTON_2, "Button2", Persistence::PERSISTENT, "", "Assignment to button 2"},
  {SP_BUTTON_3, "Button3", Persistence::PERSISTENT, "", "Assignment to button 3"},
  {SP_BUTTON_4, "Button4", Persistence::PERSISTENT, "", "Assignment to button 4"},
  {SP_BUTTON_10, "Button10", Persistence::PERSISTENT, "", "Assignment to button 10"},
  {SP_JOYSTICK_DEVICE, "JoystickDevice", Persistence::PERSISTENT, "/dev/input/js0", "Joystick device"},
};

ParameterType GetParameterType(int iParameter) {
  if ((iParameter >= FIRST_BP) && (iParameter < FIRST_LP))
    return ParamBool;
  if ((iParameter >= FIRST_LP) && (iParameter < FIRST_SP))
    return ParamLong;
  if ((iParameter >= FIRST_SP) && (iParameter < END_OF_SPS))
    return ParamString;
  
  return ParamInvalid;
}

std::string GetParameterName(int iParameter) {
  // Pull the registry name out of the correct table depending on the parameter type
  switch (GetParameterType(iParameter)) {
    case (ParamBool):
      DASHER_ASSERT(iParameter == boolparamtable[iParameter - FIRST_BP].key);
      return boolparamtable[iParameter - FIRST_BP].regName;
    case (ParamLong):
      DASHER_ASSERT(iParameter == longparamtable[iParameter - FIRST_LP].key);
      return longparamtable[iParameter - FIRST_LP].regName;
    case (ParamString):
      DASHER_ASSERT(iParameter == stringparamtable[iParameter - FIRST_SP].key);
      return stringparamtable[iParameter - FIRST_SP].regName;
    case ParamInvalid:
      DASHER_ASSERT(false);
      break;
  };
  return "";
}

} //end namespace Settings
} //end namespace Dasher
