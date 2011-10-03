#include "Parameters.h"

namespace Dasher{
  namespace Settings {
    
// The important thing here is that these are in the same order
// as the enum declarations (could add check in class that enforces this instead)
const bp_table boolparamtable[] = {
  {BP_DRAW_MOUSE_LINE, "DrawMouseLine", PERS, true, "Draw Mouse Line"},
  {BP_DRAW_MOUSE, "DrawMouse", PERS, false, "Draw Mouse Position"},
  {BP_CURVE_MOUSE_LINE, "CurveMouseLine", PERS, false, "Curve mouse line according to screen nonlinearity"},
#ifdef WITH_MAEMO
  {BP_SHOW_SLIDER, "ShowSpeedSlider", PERS, false, "ShowSpeedSlider"},
#else
  {BP_SHOW_SLIDER, "ShowSpeedSlider", PERS, true, "ShowSpeedSlider"},
#endif
  {BP_START_MOUSE, "StartOnLeft", PERS, true, "StartOnLeft"},
  {BP_START_SPACE, "StartOnSpace", PERS, false, "StartOnSpace"},
  {BP_STOP_IDLE, "StopOnIdle", PERS, false, "StopOnIdle"},
  {BP_CONTROL_MODE, "ControlMode", PERS, false, "ControlMode"},
  {BP_COLOUR_MODE, "ColourMode", PERS, true, "ColourMode"},
  {BP_MOUSEPOS_MODE, "StartOnMousePosition", PERS, false, "StartOnMousePosition"},
  {BP_PALETTE_CHANGE, "PaletteChange", PERS, true, "PaletteChange"},
  {BP_TURBO_MODE, "TurboMode", PERS, true, "Boost speed when holding key1 or right mouse button"},
  {BP_AUTOCALIBRATE, "Autocalibrate", PERS, false, "Automatically learn TargetOffset e.g. gazetracking"},
  {BP_REMAP_XTREME, "RemapXtreme", PERS, false, "Pointer at extreme Y translates more and zooms less"},
  {BP_LM_DICTIONARY, "Dictionary", PERS, true, "Whether the word-based language model uses a dictionary"},
  {BP_LM_LETTER_EXCLUSION, "LetterExclusion", PERS, true, "Whether to do letter exclusion in the word-based model"},
  {BP_AUTO_SPEEDCONTROL, "AutoSpeedControl", PERS, true, "AutoSpeedControl"},
  {BP_LM_ADAPTIVE, "LMAdaptive", PERS, true, "Whether language model should learn as you enter text"},
  {BP_SOCKET_INPUT_ENABLE, "SocketInputEnable", PERS, false, "Read pointer coordinates from network socket instead of mouse"},
  {BP_SOCKET_DEBUG, "SocketInputDebug", PERS, false, "Print information about socket input processing to console"},
  {BP_CIRCLE_START, "CircleStart", PERS, false, "Start on circle mode"},
  {BP_GLOBAL_KEYBOARD, "GlobalKeyboard", PERS, false, "Whether to assume global control of the keyboard"},
#ifdef WITH_MAEMO
  {BP_NONLINEAR_Y, "NonlinearY", PERS, false, "Apply nonlinearities to Y axis (i.e. compress top &amp; bottom)"},
#else
  {BP_NONLINEAR_Y, "NonlinearY", PERS, true, "Apply nonlinearities to Y axis (i.e. compress top &amp; bottom)"},
#endif
  {BP_STOP_OUTSIDE, "PauseOutside", PERS, false, "Whether to stop when pointer leaves canvas area"},
#ifdef TARGET_OS_IPHONE
  {BP_BACKOFF_BUTTON, "BackoffButton", PERS, false, "Whether to enable the extra backoff button in dynamic mode"},
#else
  {BP_BACKOFF_BUTTON, "BackoffButton", PERS, true, "Whether to enable the extra backoff button in dynamic mode"},
#endif
  {BP_TWOBUTTON_REVERSE, "TwoButtonReverse", PERS, false, "Reverse the up/down buttons in two button mode"},
  {BP_2B_INVERT_DOUBLE, "TwoButtonInvertDouble", PERS, false, "Double-press acts as opposite button in two-button mode"},
  {BP_SLOW_START, "SlowStart", PERS, false, "Start at low speed and increase"},
  {BP_COPY_ALL_ON_STOP, "CopyOnStop", PERS, false, "Copy all text to clipboard whenever we stop"},
  {BP_SPEAK_ALL_ON_STOP, "SpeakOnStop", PERS, false, "Speak all text whenever we stop"},
  {BP_SPEAK_WORDS, "SpeakWords", PERS, false, "Speak words as they are written"},
  {BP_CONTROL_MODE_HAS_HALT, "ControlHasHalt", PERS, false, "Force Control Mode to provide a stop action (triggering clipboard/speech)"},
#ifdef TARGET_OS_MAC
  {BP_CONTROL_MODE_HAS_EDIT, "ControlHasEdit", PERS, false, "Provide editing functions in control mode (forward &amp; backward movement &amp; deletion)"},
#else
  {BP_CONTROL_MODE_HAS_EDIT, "ControlHasEdit", PERS, true, "Provide editing functions in control mode (forward &amp; backward movement &amp; deletion)"},
#endif
  {BP_CONTROL_MODE_HAS_COPY, "ControlHasCopy", PERS, true, "Provide copy-to-clipboard actions in Control Mode (if platforms supports)"},
  {BP_CONTROL_MODE_HAS_SPEECH, "ControlHasSpeech", PERS, true, "Provide speech actions in Control Mode (if platform supports)"},
  {BP_GAME_HELP_DRAW_PATH, "GameDrawPath", PERS, true, "When we give help, show the shortest path to the target sentence"},
};

const lp_table longparamtable[] = {
  {LP_ORIENTATION, "ScreenOrientation", PERS, -2, "Screen Orientation"},
  {LP_MAX_BITRATE, "MaxBitRateTimes100", PERS, 80, "Max Bit Rate Times 100"},
  {LP_FRAMERATE, "FrameRate", PERS, 3200, "Decaying average of last known frame rates, *100"},
  {LP_VIEW_ID, "ViewID", PERS, 1, "ViewID"},
  {LP_LANGUAGE_MODEL_ID, "LanguageModelID", PERS, 0, "LanguageModelID"},
  {LP_DASHER_FONTSIZE, "DasherFontSize", PERS, 2, "DasherFontSize"},
  {LP_MESSAGE_FONTSIZE, "MessageFontSize", PERS, 14, "Size of font for messages (in points)"},
  {LP_SHAPE_TYPE, "RenderStyle", PERS, 1, "Shapes to render in (0/1=disjoint/overlapping rects, 2/3=triangles/truncated, 4=quadrics, 5=circles)"},
  {LP_UNIFORM, "UniformTimes1000", PERS, 50, "UniformTimes1000"},
  {LP_YSCALE, "YScaling", PERS, 0, "YScaling"},
  {LP_MOUSEPOSDIST, "MousePositionBoxDistance", PERS, 50, "MousePositionBoxDistance"},
  {LP_STOP_IDLETIME, "StopIdleTime", PERS, 1000, "StopIdleTime" },
  {LP_PY_PROB_SORT_THRES, "PYProbabilitySortThreshold", PERS, 85, "Sort converted syms in descending probability order up to this %age"},
  {LP_MESSAGE_TIME, "MessageTime", PERS, 2500, "Time for which non-modal messages are displayed, in ms"},
  {LP_LM_MAX_ORDER, "LMMaxOrder", PERS, 5, "LMMaxOrder"},
  {LP_LM_EXCLUSION, "LMExclusion", PERS, 0, "LMExclusion"},
  {LP_LM_UPDATE_EXCLUSION, "LMUpdateExclusion", PERS, 1, "LMUpdateExclusion"},
  {LP_LM_ALPHA, "LMAlpha", PERS, 49, "LMAlpha"},
  {LP_LM_BETA, "LMBeta", PERS, 77, "LMBeta"},
  {LP_LM_MIXTURE, "LMMixture", PERS, 50, "LMMixture"},
  {LP_LINE_WIDTH, "LineWidth", PERS, 1, "Width to draw crosshair and mouse line"},
  {LP_GEOMETRY, "Geometry", PERS, 0, "Screen geometry (mostly for tall thin screens) - 0=old-style, 1=square no-xhair, 2=squish, 3=squish+log"},
  {LP_LM_WORD_ALPHA, "WordAlpha", PERS, 50, "Alpha value for word-based model"},
  {LP_USER_LOG_LEVEL_MASK, "UserLogLevelMask", PERS, 0, "Controls level of user logging, 0 = none, 1 = short, 2 = detailed, 3 = both"},
  {LP_ZOOMSTEPS, "Zoomsteps", PERS, 32, "Integerised ratio of zoom size for click/button mode, denom 64."},
  {LP_B, "ButtonMenuBoxes", PERS, 4, "Number of boxes for button menu mode"},
  {LP_S, "ButtonMenuSafety", PERS, 25, "Safety parameter for button mode, in percent."},
#ifdef TARGET_OS_IPHONE
  {LP_BUTTON_SCAN_TIME, "ButtonMenuScanTime", PERS, 600, "Scanning time in menu mode (0 = don't scan), in ms"},
#else
  {LP_BUTTON_SCAN_TIME, "ButtonMenuScanTime", PERS, 0, "Scanning time in menu mode (0 = don't scan), in ms"},
#endif
  {LP_R, "ButtonModeNonuniformity", PERS, 0, "Button mode box non-uniformity"},
  {LP_RIGHTZOOM, "ButtonCompassModeRightZoom", PERS, 5120, "Zoomfactor (*1024) for compass mode"},
#if defined(WITH_MAEMO) || defined (TARGET_OS_IPHONE)
  {LP_NODE_BUDGET, "NodeBudget", PERS, 1000, "Target (min) number of node objects to maintain"},
#else
  {LP_NODE_BUDGET, "NodeBudget", PERS, 3000, "Target (min) number of node objects to maintain"},
#endif
  {LP_OUTLINE_WIDTH, "OutlineWidth", PERS, 0, "Absolute value is line width to draw boxes (fill iff >=0)" },
  {LP_MIN_NODE_SIZE, "MinNodeSize", PERS, 50, "Minimum size of node (in dasher coords) to draw" }, 
#ifdef WITH_MAEMO
  {LP_NONLINEAR_X, "NonLinearX", PERS, 0, "Nonlinear compression of X-axis (0 = none, higher = more extreme)"},
#else
  {LP_NONLINEAR_X, "NonLinearX", PERS, 5, "Nonlinear compression of X-axis (0 = none, higher = more extreme)"},
#endif
  {LP_AUTOSPEED_SENSITIVITY, "AutospeedSensitivity", PERS, 100, "Sensitivity of automatic speed control (percent)"},
  {LP_SOCKET_PORT, "SocketPort", PERS, 20320, "UDP/TCP socket to use for network socket input"},
  {LP_SOCKET_INPUT_X_MIN, "SocketInputXMinTimes1000", PERS, 0, "Bottom of range of X values expected from network input"},
  {LP_SOCKET_INPUT_X_MAX, "SocketInputXMaxTimes1000", PERS, 1000, "Top of range of X values expected from network input"},
  {LP_SOCKET_INPUT_Y_MIN, "SocketInputYMinTimes1000", PERS, 0, "Bottom of range of Y values expected from network input"},
  {LP_SOCKET_INPUT_Y_MAX, "SocketInputYMaxTimes1000", PERS, 1000, "Top of range of Y values expected from network input"},
  {LP_INPUT_FILTER, "InputFilterID", PERS, 3, "Module ID of input filter"},
  {LP_CIRCLE_PERCENT, "CirclePercent", PERS, 10, "Percentage of nominal vertical range to use for radius of start circle"},
  {LP_TWO_BUTTON_OFFSET, "TwoButtonOffset", PERS, 1638, "Offset for two button dynamic mode"},
  {LP_HOLD_TIME, "HoldTime", PERS, 1000, "Time for which buttons must be held to count as long presses, in ms"},
  {LP_MULTIPRESS_TIME, "MultipressTime", PERS, 1000, "Time in which multiple presses must occur, in ms"},
  {LP_SLOW_START_TIME, "SlowStartTime", PERS, 1000, "Time over which slow start occurs"},
  {LP_CONVERSION_ORDER, "ConversionOrder", PERS, 0, "Conversion ordering"},
  {LP_CONVERSION_TYPE, "ConversionType", PERS, 0, "Conversion type"},
  {LP_TWO_PUSH_OUTER, "TwoPushOuter", PERS, 1792, "Offset for one button dynamic mode outer marker"},
  {LP_TWO_PUSH_UP, "TwoPushUp", PERS, 1536, "Offset to up marker in one button dynamic"},
  {LP_TWO_PUSH_DOWN, "TwoPushDown", PERS, 1280, "Offset to down marker in one button dynamic"},
  {LP_TWO_PUSH_TOLERANCE, "TwoPushTolerance", PERS, 100, "Tolerance of two-push-mode pushes, in ms"},
  {LP_DYNAMIC_BUTTON_LAG, "DynamicButtonLag", PERS, 50, "Lag of pushes in dynamic button mode (ms)"},
  {LP_STATIC1B_TIME, "Static1BTime", PERS, 2000, "Time for static-1B mode to scan from top to bottom (ms)"},
  {LP_STATIC1B_ZOOM, "Static1BZoom", PERS, 8, "Zoom factor for static-1B mode"},
  {LP_DEMO_SPRING, "DemoSpring", PERS, 100, "Springyness in Demo-mode"},
  {LP_DEMO_NOISE_MEM, "DemoNoiseMem", PERS, 100, "Memory parameter for noise in Demo-mode"},
  {LP_DEMO_NOISE_MAG, "DemoNoiseMag", PERS, 325, "Magnitude of noise in Demo-mode"},
  {LP_MAXZOOM, "ClickMaxZoom", PERS, 200, "Maximum zoom possible in click mode (times 10)"},
  {LP_DYNAMIC_SPEED_INC, "DynamicSpeedInc", PERS, 3, "%age by which dynamic mode auto speed control increases speed"},
  {LP_DYNAMIC_SPEED_FREQ, "DynamicSpeedFreq", PERS, 10, "Seconds after which dynamic mode auto speed control increases speed"},
  {LP_DYNAMIC_SPEED_DEC, "DynamicSpeedDec", PERS, 8, "%age by which dynamic mode auto speed control decreases speed on reverse"},
  {LP_TAP_TIME, "TapTime", PERS, 200, "Max length of a stylus 'tap' rather than hold (ms)"},
#ifdef TARGET_OS_IPHONE
  {LP_MARGIN_WIDTH, "MarginWidth", PERS, 500, "Width of RHS margin (in Dasher co-ords)"},
#else
  {LP_MARGIN_WIDTH, "MarginWidth", PERS, 300, "Width of RHS margin (in Dasher co-ords)"},
#endif
  {LP_TARGET_OFFSET, "TargetOffset", PERS, 0, "Vertical distance between mouse pointer and target (400=screen height)"},
  {LP_GAME_HELP_DIST, "GameHelpDistance", PERS, 1920, "Distance of sentence from center to decide user needs help"},
  {LP_GAME_HELP_TIME, "GameHelpTime", PERS, 0, "Time for which user must need help before help drawn"},
};

const sp_table stringparamtable[] = {
  {SP_ALPHABET_ID, "AlphabetID", PERS, "", "AlphabetID"},
  {SP_ALPHABET_1, "Alphabet1", PERS, "", "Alphabet History 1"},
  {SP_ALPHABET_2, "Alphabet2", PERS, "", "Alphabet History 2"},
  {SP_ALPHABET_3, "Alphabet3", PERS, "", "Alphabet History 3"},
  {SP_ALPHABET_4, "Alphabet4", PERS, "", "Alphabet History 4"},
  {SP_COLOUR_ID, "ColourID", PERS, "", "ColourID"}, 
  {SP_DASHER_FONT, "DasherFont", PERS, "", "DasherFont"},
  {SP_GAME_TEXT_FILE, "GameTextFile", PERS, "", "User-specified file with strings to practice writing"},
  {SP_SOCKET_INPUT_X_LABEL, "SocketInputXLabel", PERS, "x", "Label preceding X values for network input"},
  {SP_SOCKET_INPUT_Y_LABEL, "SocketInputYLabel", PERS, "y", "Label preceding Y values for network input"},
#if defined(WITH_MAEMO) || defined(TARGET_OS_IPHONE)
  {SP_INPUT_FILTER, "InputFilter", PERS, "Stylus Control", "Input filter used to provide the current control mode"},
#else
  {SP_INPUT_FILTER, "InputFilter", PERS, "Normal Control", "Input filter used to provide the current control mode"},
#endif
  {SP_INPUT_DEVICE, "InputDevice", PERS, "Mouse Input", "Driver for the input device"},
  {SP_BUTTON_0, "Button0", PERS, "", "Assignment to button 0"},
  {SP_BUTTON_1, "Button1", PERS, "", "Assignment to button 1"},
  {SP_BUTTON_2, "Button2", PERS, "", "Assignment to button 2"},
  {SP_BUTTON_3, "Button3", PERS, "", "Assignment to button 3"},
  {SP_BUTTON_4, "Button4", PERS, "", "Assignment to button 4"},
  {SP_BUTTON_10, "Button10", PERS, "", "Assignment to button 10"},
  {SP_JOYSTICK_DEVICE, "JoystickDevice", PERS, "/dev/input/js0", "Joystick device"},
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
