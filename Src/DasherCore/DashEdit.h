/*
An abstract DashEditbox class is described here.
An implementation will handle interaction between Dasher and an actual Edit control.
e.g. - output characters to the edit control
     - tapping on the edit box updates the Dasher display
*/

#ifndef __DashEdit_h__
#define __DashEdit_h__

#include "DasherWidgetInterface.h"

namespace Dasher {class CDashEditbox;}
class Dasher::CDashEditbox
{
public:
	CDashEditbox() : m_iFlushed(0), m_DasherInterface(0), m_dirty(false) {}
	
	virtual void SetInterface(CDasherWidgetInterface* DasherInterface) {m_DasherInterface = DasherInterface;}
	
	// write some buffered output to file
	virtual void write_to_file()=0;
	
	// to reset the flushed flag
	void set_flushed(int i) {m_iFlushed=i;}
	
	// get the context from the current cursor position with max history
	virtual void get_new_context(std::string& str, int max)=0;
	
	// delete flushed text from the edit control
	virtual inline void unflush()=0;
	
	// called when characters fall of the LHS of the screen
	virtual void output(symbol Symbol)=0;
	
	// flush text from Dasher display to edit control
	virtual void flush(symbol Symbol)=0;
	
	// File I/O (optional)
	virtual void TimeStampNewFiles(bool Value) {}
	bool IsDirty() {return m_dirty;}
	virtual void New(const std::string& filename) {}; // filename can be "", but you cannot call Save() without having set a filename.
	virtual bool Open(const std::string& filename) {return false;};
	virtual bool OpenAppendMode(const std::string& filename) {return false;};
	virtual bool SaveAs(const std::string& filename) {return false;};
	virtual bool Save() {return false;}; // returns false if there is no filename set, or if saving fails
	
	// Clipboard (optional)
	virtual void Cut() {};
	virtual void Copy() {};
	virtual void CopyAll() {};
	virtual void Paste() {};
	virtual void SelectAll() {};
	virtual void Clear()=0; // Must at least be able to clear edit box
	
	virtual void SetEncoding(Opts::FileEncodingFormats Encoding)=0;
	virtual void SetFont(std::string Name, long Size)=0;
	
// TODO sort relationship between CDashEditbox and derived classes
protected:
	bool m_dirty;
	int m_iFlushed; // how many characters have been flushed
	CDasherWidgetInterface* m_DasherInterface;
};


#endif /* #ifndef __DashEdit_h__ */