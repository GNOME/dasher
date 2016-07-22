#ifndef __StatusControl_h__
#define __StatusControl_h__

#include "../Common/WinCommon.h"
#include "../../DasherCore/SettingsStore.h"
#include "../AppSettings.h"
#include "../resource.h"

#include <atlbase.h>
#include <atlwin.h>

class CStatusControl : public ATL::CDialogImpl<CStatusControl>, public Dasher::CSettingsObserver {
public:
	CStatusControl(Dasher::CSettingsUser *pCreateFrom, CAppSettings *pAppSettings);

    enum { IDD = IDD_STATUSBAR};

  BEGIN_MSG_MAP(CStatusControl)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    COMMAND_HANDLER(IDC_ALPHABET_COMBO, CBN_SELCHANGE, OnAlphabetChanged)
    COMMAND_HANDLER(IDC_SPEED_EDIT, EN_CHANGE, OnSpeedEditChange)
    NOTIFY_HANDLER(IDC_SPEED_SPIN, UDN_DELTAPOS, OnSpeedSpinChange)
  END_MSG_MAP()

  // Message handlers:
  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnAlphabetChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnSpeedEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnSpeedSpinChange(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);


  int GetHeight() {
    return m_dialogHeight;
  }
  void HandleEvent(int iParameter);

private:
  // Update the contents of the alphabet seletion combo
  void PopulateCombo();

  // Update Dasher to reflect the new alphabet selection
  void SelectAlphabet();

  // Update the contents of the speed control
  void PopulateSpeed();

  // Update Dasher and the edit box to represent the current speed
  void UpdateSpeed(int iPos, int iDelta);

  // The Dasher interface with which this control communicates
  CAppSettings *m_pAppSettings;

  // Handles to child windows
  CWindow m_hEdit;
  CWindow m_hUpDown;
  CWindow m_hCombo;
  int m_dialogHeight;
};

#endif