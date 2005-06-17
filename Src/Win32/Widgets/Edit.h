// Edit.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __Edit_h__
#define __Edit_h__


#include "../../Common/MSVC_Unannoy.h"
#include <string>
#include <vector>

#define _ATL_APARTMENT_THREADED
#include <atlbase.h>

//You may derive a class from CComModule and use it if you want to override something, 

//but do not change the name of _Module

extern CComModule _Module;

#include <atlcom.h>

#ifndef DASHER_WINCE
#include <sapi.h>
#endif 

#include "../../DasherCore/DashEdit.h"

#include "FilenameGUI.h"
#include <Oleacc.h>

class CCanvas;

class CEdit : public Dasher::CDashEditbox, public CWinWrap
{
public:
	CEdit(HWND Parent);
	~CEdit();
	
	void Move(int x, int y, int Width, int Height);
	HWND GetHwnd() {return m_hwnd;} // As EN_UPDATE message go to parent, need this.
	
	void UserSave(HANDLE FileHandle);
	void UserOpen(HANDLE FileHandle);
	
	// Overriding file virtual functions
	void TimeStampNewFiles(bool Value);
	void New(const std::string& filename);
	bool Open(const std::string& filename);
	bool OpenAppendMode(const std::string& filename);
	bool SaveAs(const std::string& filename);
	bool Save();
	// Functions for Windows GUI to call
	void New();
	void Open();
	void OpenAppendMode();
	void SaveAs();
	std::string Import();
	void SetDirty(); // Parent window gets notification Edit window has changed.
	
	void Cut();
	void Copy();
	void CopyAll();
	void Paste();
	void SelectAll();
	void Clear();
	
	void SetEncoding(Dasher::Opts::FileEncodingFormats Encoding);
	void SetFont(std::string Name, long Size);
	
	void SetInterface(Dasher::CDasherWidgetInterface* DasherInterface);
	
	// write some buffered output to file
	void write_to_file();
	
	// get the context from the current cursor position with max history
	void get_new_context(std::string& str, int max);
		
	// called when characters fall of the LHS of the screen
	void output(Dasher::symbol Symbol);
	
	// called when outputting a control symbol
	void outputcontrol (void* pointer, int data, int type);
	

	// remove the previous character

	void deletetext(Dasher::symbol);



	// set the window that text should be entered into

	void SetWindow(HWND window);

	// toggle text entry mode
	void TextEntry(bool Value) {textentry=Value;}
	bool GetTextEntry() {return textentry;}

	// speak text
	void speak(int what);

	// set canvas
	void SetEditCanvas(CCanvas* canvas) {Canvas=canvas;}

protected:
	bool m_dirty;
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	HWND Parent;
	WNDPROC TextWndFunc;
	
	HANDLE FileHandle;   // Keeping a lock on files makes File I/O safer,
	                     // especially for the append mode!
	CFilenameGUI* m_FilenameGUI;
	Tstring m_filename;
	HWND textwindow;
	bool AppendMode;
	void TNew(const Tstring& filename);
	bool TOpen(const Tstring& filename);
	bool TOpenAppendMode(const Tstring& filename);
	bool TSaveAs(const Tstring& filename);

	HFONT m_Font;
	std::string m_FontName;
	long m_FontSize;
	
	std::string m_Output; // UTF-8 to go to training file
	UINT CodePage; // for font and possible for finding the encoding
	Dasher::Opts::FileEncodingFormats m_Encoding; // file encoding option (may say to use codepage or user setting)

	DWORD threadid;
	HWND targetwindow;
	bool textentry;
#ifdef _UNICODE
	INPUT fakekey[2];
#endif

#ifndef DASHER_WINCE
	ISpVoice * pVoice;
#endif
	Tstring speech;
	Tstring lastspeech;
	Tstring newchar;

	void InsertText(Tstring InsertText); // add symbol to edit control

	CCanvas* Canvas;
};


#endif /* #ifndef __Edit_h__ */
