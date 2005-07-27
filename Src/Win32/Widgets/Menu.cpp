// Menu.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "Menu.h"
#include "../resource.h"

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

void CMenu::SetStatus(UINT Command, bool Grayed, bool Checked) {
  MenuOptions Options;
  Options.Grayed = Grayed;
  Options.Checked = Checked;
  m_Items[Command] = Options;
}

bool CMenu::GetCheck(UINT Command) {
  return m_Items[Command].Checked;
}

void CMenu::SortOut(HMENU Menu) {
  // Clipboard special case (not a saved option - depends on
  // current state of Windows's clipboard)
  if(IsClipboardFormatAvailable(CF_TEXT))
    SetStatus(ID_EDIT_PASTE, false, false);
  else
    SetStatus(ID_EDIT_PASTE, true, false);

  int NumberItems = GetMenuItemCount(Menu);

  for(int i = 0; i < NumberItems; i++) {
    UINT MenuItem = GetMenuItemID(Menu, i);

    // If we don't know, do nothing
    if(m_Items.find(MenuItem) == m_Items.end())
      continue;

    if(m_Items[MenuItem].Checked)
      CheckMenuItem(Menu, MenuItem, MF_CHECKED);
    else
      CheckMenuItem(Menu, MenuItem, MF_UNCHECKED);

    if(m_Items[MenuItem].Grayed)
      EnableMenuItem(Menu, MenuItem, MF_GRAYED);
    else
      EnableMenuItem(Menu, MenuItem, MF_ENABLED);
  }
}
