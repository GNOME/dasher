// GameModule.h

#ifndef __GameModule_h__
#define __GameModule_h__

#include <string>
#include <cstring>

using namespace std;

#include "DasherScreen.h"
#include "DasherModel.h"
#include "../Common/ModuleSettings.h"
#include "DasherNode.h"
#include "DasherView.h"
#include "DasherTypes.h"
#include "DasherInterfaceBase.h"
#include "WordGeneratorBase.h"
#include "DemoFilter.h"

namespace Dasher {

/**
 * This Dasher Module encapsulates all game mode logic. In game mode, users will be given
 * a target string to type as well as visual feedback for their progress and a helpful 
 * arrow to guide them in the right path through the dasher model.
 * 
 * The way target strings will be displayed and reasoned about in code is in terms
 * of chunks. Chunks represent the collection of strings that is displayed at once
 * on the screen. After typing all the words in a given chunk, a new chunk of size
 * m_iTargetChunkSize is retrieved from the word generator and displayed.
 *
 * This class handles logic and drawing code with respect to the above.
 */
class CGameModule : protected CSettingsUser, protected TransientObserver<const CEditEvent *>, protected TransientObserver<CGameNodeDrawEvent*>, private TransientObserver<CDasherNode*>, private TransientObserver<CDasherView*> {
 public:
  friend class CDemoFilter;
  /**
   * Constructor
   * @param pEventHandler A pointer to the event handler
   * @param pSettingsStore A pointer to the settings store
   * @param pInterface A pointer to a Dasher interface
   * @param iID The ID of this module.
   * @param szName The name of this module
   * @param pWordGenerator A pointer to the word generator
   */
  CGameModule(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pInterface, CDasherView *pView, CDasherModel *pModel);

  ~CGameModule();

  void StartWriting(unsigned long lTime);
  
  /**
   * Draws Game Mode specific visuals to the screen.
   * \param pView The Dasher View to be modified
   */
  void DecorateView(unsigned long lTime, CDasherView *pView, CDasherModel *pModel);

  /**
   * Set the word generator for this instance to draw words from.
   * @param pWordGenerator the word generator to be used
   */ 
  void SetWordGenerator(const CAlphInfo *pAlph, CWordGeneratorBase *pWordGenerator);
  
  /// The "GameModule" isn't actually a DasherModule, and/so this will be never called,
  /// but for uniformity with existing module settings API, I'm using this to record
  /// what preferences there are that affect Game Mode - really, these should be
  /// displayed to the user each time (s)he enters Game Mode.
  bool GetSettings(SModuleSettings **sets, int *count);

protected:
  ///Called after each successful call to GenerateChunk. Subclasses may override
  /// to do any necessary extra processing given the new chunk. Default does nothing.
  virtual void ChunkGenerated() {}
  
  /// Called when a node has been populated. Look for Game children.
  virtual void HandleEvent(CDasherNode *pNode);
  
  void DrawBrachistochrone(Dasher::CDasherView* pView);
  void DrawHelperArrow(Dasher::CDasherView* pView);
  myint ComputeBrachCenter();
    
  /// Called when a node has been output/deleted. Update string (to be/) written.
  virtual void HandleEvent(const CEditEvent *);
  
  /// Called when a NF_GAME node has been drawn.
  virtual void HandleEvent(CGameNodeDrawEvent *evt); 
  
  /// Called when screen geometry has changed. We just use this to look for when the View changes
  /// (to re-register for CGameNodeDrawEvents - a bit of a hack...)
  virtual void HandleEvent(CDasherView *);

  ///Draw the target and currently-entered text for the user to follow.
  /// Subclasses should implement using appropriate GUI components, maybe using
  /// m_strWrong, and lastCorrectSym() as an index into targetSyms()
  /// (any of which can be converted to text using m_pAlph).
  virtual void DrawText(CDasherView *pView)=0;
  
  ///Any text wrongly entered since the last on-target character
  std::string m_strWrong;
  const vector<symbol> &targetSyms() {return m_vTargetSymbols;}
  int lastCorrectSym() {return m_iLastSym;}
  const CAlphInfo *m_pAlph;
  CDasherInterfaceBase * const m_pInterface;
private:

  ///
  /// Gets a new line from the generator into m_vTargetSymbols
  /// and positions us at the beginning.
  /// \return true if a new line was obtained; false if the wordgenerator
  /// indicated EOF.
  bool GenerateChunk();

   
  /**
   * Pointer to the object that encapsulates the word generation
   * algorithm being used.
   */
  CWordGeneratorBase *m_pWordGenerator;
  
  /**
   * The target string the user must type.
   */ 
  vector<symbol> m_vTargetSymbols;

  /**
   * The last correct symbol we have seen
   */
  int m_iLastSym;
  
  /**
   * Min and max dasher coordinates of the smallest (known) game node
   */
  myint m_y1, m_y2;

  ///Best-known Location of target sentence in each frame
  vector<myint> m_vTargetY;
  ///Last element of above, i.e. current location of target sentence
  myint m_iTargetY;
  ///Time at which we first needed help, or numeric_limits<unsigned long>::max()
  /// if we don't.
  unsigned long m_uHelpStart;
  
  ///Statistics over all _previous_ sentences: total time, total nats, total syms
  unsigned long m_ulTotalTime;
  double m_dTotalNats;
  unsigned int m_uiTotalSyms;
  
  ///Time and nats at which this sentence started
  unsigned long m_ulSentenceStartTime;
  double m_dSentenceStartNats;

/* ---------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------
 */

  /**
   * The color (in Dasher colors) to make the crosshair.
   */
  const int m_iCrosshairColor;

  /**
   * The font size used to draw the target string.
   */
  const int m_iFontSize;
  
};

}




#endif
