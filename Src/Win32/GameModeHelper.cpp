#include "GameModeHelper.h"
#include "../DasherCore/GameMessages.h"

CGameModeHelper::CGameModeHelper(Dasher::CDasherInterfaceBase *pInterface) {
  m_pInterface = pInterface;

  m_strTarget = "this is a test";

  m_pInterface->AddGameModeString(m_strTarget);
  m_strOutput = "";
}

void CGameModeHelper::Message(int message, const void *messagedata) {
  using namespace Dasher::GameMode;
  const std::string* pStr;
  std::string strText ="<span background=\"purple\" foreground=\"white\">";
  switch(message) {
  case GAME_MESSAGE_SET_TARGET_STRING:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    m_strTarget = (*pStr);
    /*
    gtk_misc_set_alignment(GTK_MISC(pPrivate->pGameInfoLabel), 0.0, 0.0);
    gtk_label_set_justify(pPrivate->pGameInfoLabel, GTK_JUSTIFY_CENTER);
    gtk_label_set_text(pPrivate->pGameInfoLabel, pPrivate->pstrTarget->c_str());
    */
    break;
  case GAME_MESSAGE_DISPLAY_TEXT:
    /*
    gtk_misc_set_alignment(GTK_MISC(pPrivate->pGameInfoLabel), 0.5, 0.0);
    gtk_label_set_use_markup(pPrivate->pGameInfoLabel, true);
    gtk_label_set_justify(pPrivate->pGameInfoLabel, GTK_JUSTIFY_CENTER);
    */
    
    strText+=(reinterpret_cast<const char*>(messagedata));
    strText+="</span>";
    /*
    gtk_label_set_markup(pPrivate->pGameInfoLabel, strText.c_str());
    */
    break;
  case GAME_MESSAGE_EDIT:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    m_strOutput += (*pStr);
    UpdateTargetLabel();
    break;
  case GAME_MESSAGE_EDIT_DELETE:
    m_strOutput = m_strOutput.substr( 0, m_strOutput.size() -
							    *reinterpret_cast<const int *>(messagedata));
    UpdateTargetLabel();
    break;
  case GAME_MESSAGE_SET_SCORE:
    /*
    SetScore(pPrivate->pScore, reinterpret_cast<const char*>(messagedata));
    */
    break;
  case GAME_MESSAGE_SET_LEVEL:
    /*
    gtk_entry_set_text(pPrivate->pLevel, reinterpret_cast<const char*>(messagedata));
    */
    break;
  case GAME_MESSAGE_CLEAR_BUFFER:
    /*
    dasher_editor_internal_cleartext(pPrivate->pEditor);
    */
    m_strOutput.clear();
    break;
  case GAME_MESSAGE_HELP_MESSAGE:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    /*
    game_mode_helper_dialog_box(pSelf, pStr->c_str());
    */
    break;
  }
}

void CGameModeHelper::Output(std::string strText) {
  m_strOutput.append(strText);

  if(m_strOutput == m_strTarget) {
    int iDummy;
    iDummy = 0;
  }
}

void CGameModeHelper::Delete(int iCount) {
  m_strOutput = m_strOutput.substr(0, m_strOutput.size() - iCount);
}

void CGameModeHelper::UpdateTargetLabel()
{

}