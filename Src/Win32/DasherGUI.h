#ifndef __DasherGUI_h__
#define __DasherGUI_h__


#include "../DasherCore/DasherInterface.h"


/*
	CDasherGUI
	
	Provides:
		CDasherGUI(CDasherInterface* AttachInterface)
		The GUI is constructed with an interface to the core Dasher engine
		
		void mainloop()
		This is the GUI loop that handles messages for the duration of the application
*/
class CDasherGUI
{
public:
	CDasherGUI(CDasherInterface* AttachInterface);
	
	void MainLoop();
	
private:
	CDasherInterface* DasherInterface;
};


#endif /* #ifndef __DasherGUI_h__ */