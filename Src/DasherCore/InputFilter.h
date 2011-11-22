#ifndef __INPUT_FILTER_H__
#define __INPUT_FILTER_H__

#include "DasherView.h"
#include "DasherModel.h"
#include "ModuleManager.h"
#include "UserLogBase.h"
#include "DasherInput.h"

namespace Dasher {
  class CDasherInterfaceBase;
}
/// \defgroup InputFilter Input filters
/// @{
namespace Dasher {
class CInputFilter : public CDasherModule {
 public:
  CInputFilter(CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName)
    : CDasherModule(iID, InputMethod, szName), m_pInterface(pInterface) {
  };

  ///Called after nodes have been rendered, to draw any decorations on the view.
  ///\return True if the decorations were (potentially) different from the last
  /// frame; false if the decorations drawn are definitely the same as those
  /// drawn in the previous frame.
  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput) { return false; };

  virtual void KeyDown(unsigned long Time, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel) {}
  
  virtual void KeyUp(unsigned long Time, int iId, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel) {}

  ///Called for each frame, before rendering. Filter thus has the opportunity
  /// to schedule one or more steps with the model (or cancel scheduled steps!);
  /// after calling, model will move to the next step that has been scheduled -
  /// whether by Timer(), or elsewhere. Thus, a filter in which movement is
  /// continuous while the filter is in some unpaused state, can ScheduleOneStep
  /// in each call to Timer; alternatively, a filter responding to clicks, can
  /// schedule a whole bunch of steps when a click occurs, and then Timer need
  /// do nothing (as the model will run through those steps, one per frame).
  ///\param pView useful for co-ordinate conversions; however, no drawing should
  /// be performed here (that should be done in DecorateView).
  virtual void Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CExpansionPolicy **pol) {};

  ///Called to tell the Filter to halt any movement that may be in progress:
  /// e.g. if some UI action has occurred taking focus/control away from the
  /// Dasher canvas. Thus, filters should (a) ensure they do not schedule any
  /// movement in subsequent Timer() calls; and (b) also ensure the model is
  /// cleared of any steps already scheduled with it.
  virtual void pause() = 0;

  virtual void Activate() {};
  virtual void Deactivate() {};

  virtual bool GetMinWidth(int &iMinWidth) {
    return false;
  }

  ///Used by Control Mode to determine whether to display a Pause node (perhaps
  /// in addition to a Stop/Done). If general, this is desirable if the filter
  /// will continuously enter bits in the absence of user input, and thus there
  /// should be the option to tell the filter to stop doing this (without the
  /// user necessarily having finished the phrase); no if the filter will not
  /// enter bits without the user actively pushing buttons / performing input.
  virtual bool supportsPause() {return false;}
  
 protected:
  CDasherInterfaceBase * const m_pInterface;
};
  
  ///Simple class, basis for filters using ScheduleZoom rather than
  /// ScheduleOneStep, providing a ScheduleZoom method wrapping the
  /// DasherModel one, using LP_ZOOMSTEPS steps and such that pause()
  /// cancels any such zoom in progress.
  class CStaticFilter : public CInputFilter, protected CSettingsUser {
  public:
    CStaticFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pIntf, ModuleID_t iId, const char *szName)
    : CInputFilter(pIntf, iId, szName), CSettingsUser(pCreator), m_pModel(NULL) {
    }
    void pause() {if (m_pModel) m_pModel->ClearScheduledSteps();}
  protected:
    void ScheduleZoom(CDasherModel *pModel, myint y1, myint y2) {
      (m_pModel = pModel)->ScheduleZoom(y1,y2,GetLongParameter(LP_ZOOMSTEPS));
    }
  private:
    CDasherModel *m_pModel;
  };
}
/// @}

#endif
