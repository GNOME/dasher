#include "KeyboardHelper.h"

#include <X11/Xlib.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#ifdef HAVE_GDK_GDKKEYSYMS_COMPAT_H
#include <gdk/gdkkeysyms-compat.h>
#endif
#include <gdk/gdkx.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
#include <stdlib.h>

CKeyboardHelper::CKeyboardHelper(DasherAppSettings *pAppSettings) {
  m_pAppSettings = pAppSettings;

  // For now assume we either have all or nothing
  if(!m_pAppSettings || (!m_pAppSettings->GetString(SP_BUTTON_0).empty()))
    SetupDefaults();
  else
    LoadSettings();
}

void CKeyboardHelper::SetupDefaults() {
  // 1 = E
  // 2 = N
  // 3 = W
  // 4 = S

  // Left of keyboard

  SetKey(GDK_a, 1);
  SetKey(GDK_s, 3);
  SetKey(GDK_w, 2);
  SetKey(GDK_z, 4);

  SetKey(GDK_A, 1);
  SetKey(GDK_S, 3);
  SetKey(GDK_W, 2);
  SetKey(GDK_Z, 4);
  
  // Right of keyboard

  SetKey(GDK_j, 1);
  SetKey(GDK_k, 3);
  SetKey(GDK_i, 2);
  SetKey(GDK_m, 4);

  SetKey(GDK_J, 1);
  SetKey(GDK_K, 3);
  SetKey(GDK_I, 2);
  SetKey(GDK_M, 4);

  // Arrows

  SetKey(GDK_Left, 1);
  SetKey(GDK_Right, 3);
  SetKey(GDK_Up, 2);
  SetKey(GDK_Down, 4);

  // Arrows on numeric keypad

  SetKey(GDK_KP_Left, 1);
  SetKey(GDK_KP_Right, 3);
  SetKey(GDK_KP_Up, 2);
  SetKey(GDK_KP_Down, 4);

  // Numbers

  SetKey(GDK_1, 1);
  SetKey(GDK_2, 2);
  SetKey(GDK_3, 3);
  SetKey(GDK_4, 4);
  
  // 0 = keyboard start/stop

  SetKey(GDK_space, 0);

  // 10 = calibration key
  
  SetKey(GDK_F12, 10);
}

void CKeyboardHelper::LoadSettings() {
  if(!m_pAppSettings)
    return;

  int iButtons[] = {0, 1, 2, 3, 4, 10};
  int iIDs[] = {SP_BUTTON_0, SP_BUTTON_1, SP_BUTTON_2, SP_BUTTON_3, SP_BUTTON_4, SP_BUTTON_10}; 

  int iCount(sizeof(iButtons) / sizeof(int));
  
  for(int i(0); i < iCount; ++i) {
    std::string strEntry(m_pAppSettings->GetString(iIDs[i]));
    std::string strCurrent;

    for(std::string::iterator it(strEntry.begin()); it != strEntry.end(); ++it) {
      if(*it == ':') {
	SetKey(atoi(strCurrent.c_str()), iButtons[i]);
	strCurrent = "";
      }
      else {
	strCurrent.append(1, *it);
      }
    }
  }
}

void CKeyboardHelper::SaveSettings() {
  if(!m_pAppSettings)
    return;

  std::map<int, std::string> mSettings;
  
  for(std::map<int, int>::iterator it(m_mTable.begin()); it != m_mTable.end(); ++it) {
    std::stringstream ssEntry;
    ssEntry << it->first << ":";

    mSettings[it->second].append(ssEntry.str());
  }

  for(std::map<int, string>::iterator it(mSettings.begin()); it != mSettings.end(); ++it) {
    int iID;

    switch(it->first) {
    case 0:
      iID = SP_BUTTON_0;
      break;
    case 1:
      iID = SP_BUTTON_1;
      break;
    case 2:
      iID = SP_BUTTON_2;
      break;
    case 3:
      iID = SP_BUTTON_3;
      break;
    case 4:
      iID = SP_BUTTON_4;
      break;
    case 10:
      iID = SP_BUTTON_10;
      break;
    default:
      return;
    }

    m_pAppSettings->SetString(iID, it->second.c_str());
  }
}

void CKeyboardHelper::SetKey(int iCode, int iId) {
  m_mTable[iCode] = iId;
  SaveSettings();
}

void CKeyboardHelper::UnsetKey(int iCode) {
  m_mTable.erase(iCode);
  SaveSettings();
}

int CKeyboardHelper::ConvertKeycode(int iCode) {

  std::map<int,int>::iterator it(m_mTable.find(iCode));

  if(it != m_mTable.end())
    return it->second;
  else
    return -1;
}

/* Not called - commented out because of X11 lib dependence
void CKeyboardHelper::Grab(bool bGrab) {
  for(std::map<int,int>::iterator it(m_mTable.begin()); it != m_mTable.end(); ++it) {
    GdkKeymapKey *pKeys;
    int iKeysLength;

    gdk_keymap_get_entries_for_keyval(0, it->first, &pKeys, &iKeysLength);

    if(pKeys) {
      if(bGrab)
	XGrabKey(GDK_DISPLAY(), pKeys[0].keycode, 0, GDK_ROOT_WINDOW(), true, GrabModeAsync, GrabModeAsync);
      else
	XUngrabKey(GDK_DISPLAY(), pKeys[0].keycode, 0, GDK_ROOT_WINDOW());
      g_free(pKeys);
    }
  }
}
*/
