//! Class definition for an edit box
// DashEdit.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

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

	//! Provide the Editbox with a widget interface
	virtual void SetInterface(CDasherWidgetInterface* DasherInterface) {m_DasherInterface = DasherInterface;}
	
	//! Write some buffered output to a file
	virtual void write_to_file()=0;
	
	//! Set the number of flushed characters
	//
	//! Set the number of flushed characters to an arbitrary number.
	//! Usually used to reset it to 0 after unflushing
	void set_flushed(int i) {m_iFlushed=i;}
	
	//! Provide context from the editbox for the core
	//
	//! Provide the context at the current position within the editbox to 
	//! the core. Set str to up to max characters before 
	//! the cursor position within the editbox.
	virtual void get_new_context(std::string& str, int max)=0;
	
	//! Delete flushed text from the editbox
	virtual inline void unflush()=0;
	
	//! Enter a the character Symbol into the text box
	virtual void output(symbol Symbol)=0;

	//! Delete the previous symbol from the text box
	virtual void deletetext()=0;
	
	//! Enter a character into the text box and remember that it is flushed
	//
	//! Output the character and increment m_iFlushed. When unflush is
	//! called, remove the previous m_iFlushed characters
	virtual void flush(symbol Symbol)=0;
	
	// File I/O (optional)

	//! If Value is true, timestamp all new files (optional)
	//
	//! If switched on, all new files should be timestamped, either in the
	//! filename or in file metadata
	virtual void TimeStampNewFiles(bool Value) {}

	//! Return true if any text has been modified since the last save (optional)
	bool IsDirty() {return m_dirty;}

	//! Generate a new file (optional)
	//
	//! New file - provide a file save dialogue and return the filename in
	//! filename, or provide a blank filename and present a file 
	//! save dialogue when Save() is called
	virtual void New(const std::string& filename) {}; // filename can be "", but you cannot call Save() without having set a filename.
	
	//! Open a file (optional)
	//
	//! Provide a file open dialogue and set filename to the 
	//! filename. Return true if a file is chosen and opened successfully,
	//! false otherwise
	virtual bool Open(const std::string& filename) {return false;};

	//! Open a file and append to it (optional)
	// 
	//! Provide a file open dialogue and set filename to the 
	//! filename. The file will then have any new text appended to it. 
	//! Return true if a file is chosen and opened successfully, false 
	//! otherwise
	virtual bool OpenAppendMode(const std::string& filename) {return false;};
	//! Save a file as a provided filename (optional)
	// 
	//! Provide a file save dialogue and set filename to the 
	//! filename. Return true if a file is chosen and saved successfully,
	//! false otherwise
	virtual bool SaveAs(const std::string& filename) {return false;};

	//! Save the current file (optional)
	//
	//! Save file to the current filename. If there is no current filename,
	//! or if saving fails, return false
	virtual bool Save() {return false;}; // returns false if there is no filename set, or if saving fails
	
	// Clipboard (optional)
	//! Cut selected text (optional)
	//
	//! Copy the selected text to the clipboard and remove it from the
	//! editbox
	virtual void Cut() {};

	//! Copy selected text (optional)
	//
	//! Copy the selected text to the clipboard
	virtual void Copy() {};

	//! Copy all text (optional)
	//
	//! Copy all text in the editbox to the clipboard
	virtual void CopyAll() {};

	//! Paste text from clipboard (optional)
	//
	//! Paste text from the clipboard into the editbox at the current
	//! position
	virtual void Paste() {};

	//! Select all text in the editbox (optional)
	virtual void SelectAll() {};

	//! Clear all text from the editbox (REQUIRED)
	virtual void Clear()=0; // Must at least be able to clear edit box

	//! Set the file encoding
	//
	//! Set the file encoding to the provided encoding Encoding. 
	//! The editbox is responsible for saving the file in the encoding 
	//! desired by the user. As Dasher is internally UTF8, it may well be 
	//! necessary to save in an alternative format based on locale and OS.
	virtual void SetEncoding(Opts::FileEncodingFormats Encoding)=0;

	//! Set the font used in the editbox
	//
	//! Set the font used in the editbox to Name and size 
	//! Size (in points)
	virtual void SetFont(std::string Name, long Size)=0;
	
// TODO sort relationship between CDashEditbox and derived classes
protected:
	//! Have the contents of the editbox been altered since the last save?
	bool m_dirty;

	//! Record the number of characters that have been flushed
	int m_iFlushed; // how many characters have been flushed

	//! Pointer to a DasherWidgetInterface for communication with the core
	CDasherWidgetInterface* m_DasherInterface;
};


#endif /* #ifndef __DashEdit_h__ */
