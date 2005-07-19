// FilenameGUI.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __FilenameGUI_h__
#define __FilenameGUI_h__

/*  CFilenameGUI Notes: {{{
	
	This class provides an interface to the Windows common dialogs for file
	opening and saving. It also (optionally) generates dated filenames and will
	manage the title bar of a Window to show the filename (formatted according to
	windows system preferences) and whether it is "dirty" (using a *).
	
	BTW I also wrote a class CFileGUI that assumes it has total control over
	setting files. It returns Windows file handles, rather than filenames and
	is neater and easier to use (as it did more). I decided it wasn't suitable
	for Dasher though as it wouldn't allow for setting of filenames through the
	Dasher interface. I'll provide CFileGUI on request. IAM 09/2002
}}}*/

class CFilenameGUI {
public:
  CFilenameGUI(HWND WindowWithTitlebar, Tstring AppName, bool NewWithDate = false);

  // Used by the Windows GUI:
  ////////////////////////////////////////////////////////////////////////////
  const Tstring & New();        // Returns a dated filename or ""
  const Tstring & Open();       // Returns filename or "" if user aborts
  const Tstring & SaveAs();     // Returns filename or "" if user aborts

  const Tstring & Save();       // Returns current filename if there is one,
  // otherwise returns FileSaveAs().

  int QuerySaveFirst();         // Called before doing something that will trash
  // unsaved text (FileNew(), FileOpen(), quitting, etc)
  // returns IDNO if you should just carry on,
  // returns IDYES if you should call save,
  // returns IDCANCEL if you should do nothing.

  void SetNewWithDate(bool Value);      // Should we automatically generate filenames
  // based on the date when a user hits New?
  // In this section as options are set by the GUI

  // Used by the client - eg an edit box control
  ////////////////////////////////////////////////////////////////////////////
  void SetFilename(const Tstring & FileName);   // Objects of this class cannot
  // know if the filename from FileOpen() or FileSaveAs()
  // worked out. A FileName may also be set in another way.
  // Therefore, the window title is not set until this
  // member is called.

  void SetDirty(bool Value);    // Needed to format the dirty * in window title
  // QuerySaveFirst also automatically returns IDNO
  // if (!Dirty). This flag is NOT altered by
  // FileOpen(), FileSave() or FileSaveAs(). We
  // cannot guess whether these requests succeeded
  // at the client end.
private:
  static const Tstring Empty;
  Tstring AppName;
  Tstring OriginalPath;

  Tstring FileAndPath;
  bool Dirty;
  bool NewWithDate;
  HWND WindowWithTitlebar;

  void SetWindowTitle();
};

#endif  /* #ifndef __FilenameGUI_h__ */
