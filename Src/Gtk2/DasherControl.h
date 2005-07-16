#ifndef __dashercontrol_h__
#define __dashercontrol_h__

#include "PangoCache.h"
#include "Canvas.h"
#include "mouse_input.h"

#include "GtkDasherControl.h"

#include "../DasherCore/DasherSettingsInterface.h"
#include "../DasherCore/DashEdit.h"
#include "../DasherCore/DasherInterface.h"

class CDasherControl : public CDasherSettingsInterface, public CDashEditbox {

 public:
  CDasherControl( GtkVBox *pVbox, GtkDasherControl *pDasherControl );
  ~CDasherControl();

  // Event handlers
  // FIXME - we should probably pass all parameters to these from the "C" callbacks

  /// Called when the canvas gets realized so we can finalise setup

  void RealizeCanvas();
  int TimerEvent();
  gboolean ButtonPressEvent(GdkEventButton *event);
  void CanvasConfigureEvent();
  void SliderEvent();
  gint KeyPressEvent( GdkEventKey *event );

  void scan_alphabet_files();
  void scan_colour_files();
  int alphabet_filter(const gchar* filename, GPatternSpec *alphabet);
  int colour_filter(const gchar* filename, GPatternSpec *colourglob);
  
  GArray *GetAllowedValues( int iParameter );

  void Train( const gchar *filename ) {
    m_pInterface->TrainFile( filename );
  };

  CDasherInterface* GetInterface() {
    return m_pInterface;
  };

  // CDasherSettingsInterface methods

  virtual void HandleParameterNotification( int iParameter );

  // CDashEditbox methods

  void HandleEvent( CEvent *pEvent );

  // The following functions are obsolete and will soon be deleted from the interface

  void write_to_file() {};
  void get_new_context(std::string& str, int max) {};
  void output( const std::string &strText ) {};
  void outputcontrol(void* pointer, int data, int type) {};
  void deletetext( const std::string &strText ) {};
  void Clear() {};
  void SetEncoding(Opts::FileEncodingFormats Encoding) {};
  void SetFont(std::string Name, long Size) {};
  void Cut() {};
  void Copy() {};
  void Paste() {};
  void CopyAll() {};
  void SelectAll() {};

 private:
  CDasherInterface *m_pInterface;

  GtkWidget *m_pVBox;
  GtkWidget *m_pCanvas;
  GtkWidget *m_pSpeedHScale;
  GtkWidget *m_pSpeedFrame;

  CDasherMouseInput *m_pMouseInput;

  CPangoCache *m_pPangoCache;

  CCanvas *m_pScreen;

  GtkDasherControl *m_pDasherControl;

};

#endif
