// Edit.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

/*
	File I/O is very simplistic. It relies on the fact that there isn't
	going to be loads of text in the edit box. Otherwise I'm sure performance
	would be unacceptable.
*/

#include "Edit.h"
#include "Canvas.h"
#include <mbstring.h>

using namespace Dasher;
using namespace std;

#include "DasherEncodingToCP.h"
#include "../resource.h"
#include "../WinLocalisation.h"
using namespace WinLocalisation;
#include "../WinUTF8.h"
using namespace WinUTF8;



CEdit::CEdit(HWND Parent) : Parent(Parent), m_FontSize(0), m_FontName(""),
	FileHandle(INVALID_HANDLE_VALUE), m_FilenameGUI(0), threadid(0), targetwindow(0), pVoice(0)
{
	Tstring WindowTitle;
	WinLocalisation::GetResourceString(IDS_APP_TITLE, &WindowTitle);
	m_FilenameGUI = new CFilenameGUI(Parent, WindowTitle.c_str());
	
	CodePage = GetUserCodePage();
	m_Font = GetCodePageFont(CodePage, 14);
	m_hwnd=CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), NULL,
		ES_NOHIDESEL | WS_CHILD | ES_MULTILINE | WS_VSCROLL,
		0,0,0,0, Parent,NULL, WinHelper::hInstApp, NULL );
	
	// subclass the Window Procedure so we can add functionality
	WinWrapMap::add(m_hwnd, this);
	TextWndFunc = (WNDPROC)SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG) WinWrapMap::WndProc);
	ShowWindow(m_hwnd,SW_SHOW);

	// Initialise speech support
	speech="";
	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
	if (hr!=S_OK)
		pVoice=0;
	if (pVoice!=0) {
		pVoice->Speak(L"",SPF_ASYNC,NULL);
	}
}


CEdit::~CEdit()
{
	delete m_FilenameGUI;
	if (FileHandle!=INVALID_HANDLE_VALUE)
		CloseHandle(FileHandle);
}


void CEdit::Move(int x, int y, int Width, int Height)
{
	MoveWindow(m_hwnd, x , y, Width, Height, TRUE);
}


void CEdit::New(const string& filename)
{
	Tstring newFilename;
	UTF8string_to_Tstring(filename, &newFilename, GetACP());
	TNew(newFilename);
}


bool CEdit::Open(const string& filename)
{
	Tstring openFilename;
	UTF8string_to_Tstring(filename, &openFilename, GetACP());
	return TOpen(openFilename);
}


bool CEdit::OpenAppendMode(const string& filename)
{
	Tstring openFilename;
	UTF8string_to_Tstring(filename, &openFilename, GetACP());
	return TOpenAppendMode(openFilename);
}


bool CEdit::SaveAs(const string& filename)
{
	Tstring saveFilename;
	UTF8string_to_Tstring(filename, &saveFilename, GetACP());
	return TSaveAs(saveFilename);
}


/*  CEdit::Save() - Save to file: {{{
	
	Write a Byte Order Mark (BOM) if writing a Unicode file.
	(Convert to wide in ANSI version and then) convert to desired codepage.
	Dump to file
}}}*/
bool CEdit::Save()
{
	if (FileHandle==INVALID_HANDLE_VALUE) {
		if (m_filename==TEXT(""))
			return false;
		FileHandle = CreateFile(m_filename.c_str(), GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
			(HANDLE) NULL);
		
		if (FileHandle==INVALID_HANDLE_VALUE)
			return false;
	}
	
	// Truncate File to 0 bytes.
	SetFilePointer(FileHandle, NULL, NULL, FILE_BEGIN);
	SetEndOfFile(FileHandle);
	
	// Get all the text from the edit control
	LRESULT EditLength = 1 + SendMessage(m_hwnd, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* EditText = new TCHAR[EditLength];
	EditLength = SendMessage(m_hwnd, WM_GETTEXT, (WPARAM) EditLength, (LPARAM) EditText);
	
	DWORD NumberOfBytesWritten; // Used by WriteFile
	
	// This is Windows therefore we tag Unicode files with BOMs (Byte Order Marks) {{{
	// Then notepad and other Windows apps can recognise the files.
	// Do NOT write BOMs in a UNIX version, they are not welcome there.
	// The BOM is just an encoding of U+FEFF (ZERO WIDTH NO-BREAK SPACE)
	// This is unambiguous as U+FFFE is not a valid Unicode character.
	// There could be a menu option for this, but most users won't know what a BOM is. }}}
	unsigned int WideLength=0;
	wchar_t* WideText=0;
	if ((m_Encoding==Opts::UTF16LE) || (m_Encoding==Opts::UTF16BE)) {
		// These are the UTF-16 formats. If the string isn't already in UTF-16 we need
		// it to be so.
		#ifdef _UNICODE
			WideLength = EditLength;
			WideText = EditText;
		#else
			WideText = new wchar_t[EditLength+1];
			WideLength = MultiByteToWideChar(CodePage, 0, EditText, -1, WideText, EditLength+1);
		#endif
	}
	switch (m_Encoding) {
	case Opts::UTF8: {// there is no byte order, but BOM tags it as a UTF-8 file
		unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};
		WriteFile(FileHandle, &BOM, 3, &NumberOfBytesWritten, NULL);
		Tstring Tmp = EditText;
		string Output;
		Tstring_to_UTF8string(EditText, &Output, CodePage);
		WriteFile(FileHandle, Output.c_str(), Output.size(), &NumberOfBytesWritten, NULL);
		break;
	}
	case Opts::UTF16LE: {
		// TODO I am assuming this machine is LE. Do any windows (perhaps CE) machines run on BE?
		unsigned char BOM[2] = {0xFF, 0xFE};
		WriteFile(FileHandle, &BOM, 2, &NumberOfBytesWritten, NULL);
		WriteFile(FileHandle, WideText, WideLength*2, &NumberOfBytesWritten, NULL);
		#ifndef _UNICODE
			delete[] WideText;
		#endif
		break;
	}
	case Opts::UTF16BE: { // UTF-16BE
		// TODO I am again assuming this machine is LE.
		unsigned char BOM[2] = {0xFE, 0xFF};
		WriteFile(FileHandle, &BOM, 2, &NumberOfBytesWritten, NULL);
		// There will be a better way. Perhaps use _swab instead.
		for (unsigned int i=0; i<WideLength; i++) {
			const char* Hack = (char*) &WideText[i];
			WriteFile(FileHandle, Hack+1, 1, &NumberOfBytesWritten, NULL);
			WriteFile(FileHandle, Hack, 1, &NumberOfBytesWritten, NULL);
		}
		#ifndef _UNICODE
			delete[] WideText;
		#endif
		break;
	}
	default:
		#ifdef _UNICODE
			char* MultiByteText = new char[EditLength*4];
			int MultiByteLength = WideCharToMultiByte(CodePage, 0, EditText, EditLength, MultiByteText, EditLength*4, NULL, NULL);
			WriteFile(FileHandle, MultiByteText, MultiByteLength, &NumberOfBytesWritten, NULL);
			delete[] MultiByteText;
		#else
			WriteFile(FileHandle, EditText, EditLength, &NumberOfBytesWritten, NULL);
		#endif
		break; // do nothing
	}
	
	delete[] EditText;
	// The file handle is not closed here. We keep a write-lock on the file to stop other programs confusing us.
	
	m_FilenameGUI->SetDirty(false);
	m_dirty = false;
	return true;
}


void CEdit::TimeStampNewFiles(bool Value)
{
	m_FilenameGUI->SetNewWithDate(Value);
}


void CEdit::New()
{
	switch (m_FilenameGUI->QuerySaveFirst()) {
	case IDYES:
		if (!Save())
			if (!TSaveAs(m_FilenameGUI->SaveAs()))
				return;
		break;
	case IDNO:
		break;
	default:
		return;
	}
	TNew(TEXT(""));
}


void CEdit::Open()
{
	switch (m_FilenameGUI->QuerySaveFirst()) {
	case IDYES:
		if (!Save())
			if (!TSaveAs(m_FilenameGUI->SaveAs()))
				return;
		break;
	case IDNO:
		break;
	default:
		return;
		break;
	}
	TOpen(m_FilenameGUI->Open());
}


void CEdit::OpenAppendMode()
{
	switch (m_FilenameGUI->QuerySaveFirst()) {
	case IDYES:
		if (!Save())
			if (!TSaveAs(m_FilenameGUI->SaveAs()))
				return;
		break;
	case IDNO:
		break;
	default:
		return;
		break;
	}
	TOpenAppendMode(m_FilenameGUI->Open());
}


void CEdit::SaveAs()
{
	TSaveAs(m_FilenameGUI->SaveAs());
}

std::string CEdit::Import()
{
        string filename;
	Tstring_to_UTF8string(m_FilenameGUI->Open(),&filename,CodePage);
	return filename;
}


void CEdit::SetDirty()
{
	m_dirty = true;
	m_FilenameGUI->SetDirty(true);
}


void CEdit::TNew(const Tstring& filename)
{
	if (filename==TEXT(""))
		m_filename = m_FilenameGUI->New();
	else
		m_filename = filename;
	if (FileHandle!=INVALID_HANDLE_VALUE)
		CloseHandle(FileHandle);
	FileHandle=INVALID_HANDLE_VALUE;
	AppendMode = false;
	Clear();
	m_DasherInterface->Start();
	m_DasherInterface->Redraw();
}


bool CEdit::TOpen(const Tstring& filename)
{
	// Could try and detect unicode formats from BOMs like notepad.
	// Could also base codepage on menu.
	// Best thing is probably to trust any BOMs at the beginning of file, but otherwise
	// to believe menu. Unicode files don't necessarily have BOMs, especially from Unix.

	HANDLE TmpHandle = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
		(HANDLE) NULL);
	
	if (TmpHandle==INVALID_HANDLE_VALUE)
		return false;
	
	if (FileHandle!=INVALID_HANDLE_VALUE)
		CloseHandle(FileHandle);
	FileHandle = TmpHandle;
	m_filename = filename;
	
	SetFilePointer(FileHandle, NULL, NULL, FILE_BEGIN);

	DWORD filesize = GetFileSize(FileHandle,NULL);
	unsigned long amountread;

	char* filebuffer = new char[filesize];
	
	// Just read in whole file as char* and cast later.
	
	ReadFile(FileHandle,filebuffer,filesize,&amountread,NULL);

	string text;
	text = text+filebuffer;
	Tstring inserttext;
	UTF8string_to_Tstring(text,&inserttext, GetACP());
	InsertText(inserttext);

	AppendMode = false;
	m_FilenameGUI->SetFilename(m_filename);
	m_FilenameGUI->SetDirty(false);
	m_dirty = false;
	return true;
}


bool CEdit::TOpenAppendMode(const Tstring& filename)
{
	//
	AppendMode = true;
	return true;
}


bool CEdit::TSaveAs(const Tstring& filename)
{
	HANDLE TmpHandle = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
		(HANDLE) NULL);
	
	if (TmpHandle==INVALID_HANDLE_VALUE)
		return false;
	
	if (FileHandle!=INVALID_HANDLE_VALUE)
		CloseHandle(FileHandle);
	FileHandle = TmpHandle;
	
	m_filename = filename;
	if (Save()) {
		m_FilenameGUI->SetFilename(m_filename);
		return true;
	} else
		return false;
}

void CEdit::Cut()
{
	SendMessage(m_hwnd, WM_CUT, 0, 0);
}


void CEdit::Copy()
{
	SendMessage(m_hwnd, WM_COPY, 0, 0);
/*
#ifndef _UNICODE
	HGLOBAL handle;
	DWORD* foo;
	handle = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DWORD));
	foo = (DWORD*) GlobalLock(handle);
	*foo = MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT), SORT_DEFAULT);
	GlobalUnlock(handle);
	OpenClipboard(m_hwnd);
	SetClipboardData(CF_LOCALE, handle);
	CloseClipboard();
#endif
*/
}


void CEdit::CopyAll()
{
	// One might think this would lead to flickering of selecting and
	// unselecting. It doesn't seem to. Using the clipboard directly
	// is fiddly, so this cheat is useful.
	DWORD start,finish;
	SendMessage(m_hwnd, EM_GETSEL, (LONG)&start,(LONG)&finish);
	SendMessage(m_hwnd, EM_SETSEL, 0,-1);
	SendMessage(m_hwnd, WM_COPY, 0, 0);
	SendMessage(m_hwnd, EM_SETSEL, (LONG)start,(LONG)finish);
}


void CEdit::Paste()
{
	SendMessage(m_hwnd, WM_PASTE, 0, 0);
}


void CEdit::SelectAll()
{
	SendMessage(m_hwnd, EM_SETSEL, 0,-1);
}


void CEdit::Clear()
{
	SendMessage(m_hwnd, WM_SETTEXT, 0, (LPARAM) TEXT(""));
}


void CEdit::SetEncoding(Dasher::Opts::FileEncodingFormats Encoding)
{
	m_Encoding = Encoding;
}


void CEdit::SetFont(string Name, long Size)
{

#ifndef _WIN32_WCE

	m_FontName = Name;
	m_FontSize = Size;
	
	Tstring FontName;
	UTF8string_to_Tstring(Name, &FontName);
	
	if (Size==0)
		Size=14;
	
	DeleteObject(m_Font);
	if (Name=="")
		m_Font = GetCodePageFont(CodePage, -Size);
	else
		m_Font = CreateFont(-Size, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		         OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,
		         FontName.c_str()); // DEFAULT_CHARSET => font made just from Size and FontName
	
	SendMessage(m_hwnd, WM_SETFONT, (WPARAM)m_Font, true);
#else 
	// not implemented
	#pragma message ( "CEdit::SetFot not implemented on WinCE")
	assert(0);
#endif


}


void CEdit::SetInterface(CDasherWidgetInterface* DasherInterface)
{
	CDashEditbox::SetInterface(DasherInterface);
	
	CodePage = EncodingToCP(m_DasherInterface->GetAlphabetType());
	SetFont(m_FontName, m_FontSize);
	
	unsigned int NumberSymbols = m_DasherInterface->GetNumberSymbols();
	
	DisplayStrings.resize(NumberSymbols);
	
	for (unsigned int i=0; i<NumberSymbols; i++) {
		WinUTF8::UTF8string_to_Tstring(m_DasherInterface->GetEditText(i), &DisplayStrings[i], CodePage);
	}
}

void CEdit::write_to_file()
{
	const string& TrainFile = m_DasherInterface->GetTrainFile();
	if (TrainFile=="")
		return;
	Tstring TTrainFile;
	UTF8string_to_Tstring(TrainFile, &TTrainFile);
	
	HANDLE hFile = CreateFile(TTrainFile.c_str(),
		GENERIC_WRITE, 0, NULL, OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL,0);
	
	if (hFile == INVALID_HANDLE_VALUE) {
		OutputDebugString(TEXT("Can not open file\n"));
	} else {
		DWORD NumberOfBytesWritten;
		SetFilePointer (hFile, 0, NULL, FILE_END);
		for (unsigned int i=0;i<m_Output.size();i++) {
			WriteFile(hFile, &m_Output[i], 1, &NumberOfBytesWritten, NULL);
		}
		
		m_Output = "";
		CloseHandle(hFile);
	}
}


void CEdit::get_new_context(string& str, int max)
{
	// Currently all of the edit box up to the caret is copied
	// and then a few characters taken from that.
	// This is a bit silly, but works for now. IAM 2002/08
	
	int iStart,iFinish;
	SendMessage(m_hwnd, EM_GETSEL, (LONG)&iStart,(LONG)&iFinish );
	
	TCHAR *tString = new TCHAR[iFinish+1];
	
	SendMessage(m_hwnd, WM_GETTEXT, (LONG)(iStart+1), (LONG)tString);
	
	string Wasteful;
	Tstring_to_UTF8string(tString, &Wasteful, CodePage);
	
	if (Wasteful.size()>max)
		str = Wasteful.substr(Wasteful.size()-max, max);
	else
		str = Wasteful;
	
	delete[] tString;
}


void CEdit::output(symbol Symbol)
{
	if (m_DasherInterface==0)
		return;
	
	InsertText(DisplayStrings[Symbol]);
	if (targetwindow!=NULL) {
		const char* DisplayText=m_DasherInterface->GetEditText(Symbol).c_str();
#ifdef UNICODE
		if( DisplayText[0]==0xd && DisplayText[1]==0xa) {
			// Newline, so we want to fake an enter
			fakekey[0].type=fakekey[1].type=INPUT_KEYBOARD;
			fakekey[0].ki.wVk=fakekey[1].ki.wVk=VK_RETURN;
			fakekey[0].ki.time=fakekey[1].ki.time=0;
			fakekey[1].ki.dwFlags=KEYEVENTF_KEYUP;

			SetFocus(targetwindow);
			SendInput(2,fakekey,sizeof(INPUT));
		}
		wchar_t outputstring[256];
		int i=mbstowcs(outputstring,DisplayText,255);

		for (int j=0; j<i; j++) {
			fakekey[0].type=INPUT_KEYBOARD;
			fakekey[0].ki.dwFlags=KEYEVENTF_UNICODE;
			fakekey[0].ki.wVk=0;
			fakekey[0].ki.time=NULL;
			fakekey[0].ki.wScan=outputstring[j];
			SetFocus(targetwindow);
			SendInput(1,fakekey,sizeof(INPUT));
		}
#else
		if( DisplayText[0]==0xd && DisplayText[1]==0xa) {
			// Newline, so we want to fake an enter
			SetFocus(targetwindow);
			keybd_event(VK_RETURN,0,NULL,NULL);
			keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,NULL);
		}
		Tstring character;
		WinUTF8::UTF8string_to_Tstring(DisplayText,&character,1252); 
		TCHAR test=character[0];
		SHORT outputvk=VkKeyScan(char(character[0]));
		SetFocus(targetwindow);
		if(HIBYTE(outputvk)&&6) {
			keybd_event(VK_SHIFT,0,NULL,NULL);
			keybd_event(LOBYTE(outputvk),0,NULL,NULL);
			keybd_event(LOBYTE(outputvk),0,KEYEVENTF_KEYUP,NULL);
			keybd_event(VK_SHIFT,0,KEYEVENTF_KEYUP,NULL);
		} else {
			keybd_event(LOBYTE(outputvk),0,NULL,NULL);
			keybd_event(LOBYTE(outputvk),0,KEYEVENTF_KEYUP,NULL);
		}
#endif
	}
	m_Output += m_DasherInterface->GetEditText(Symbol);

	speech+=m_DasherInterface->GetEditText(Symbol);
}

LRESULT CEdit::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = CallWindowProc(TextWndFunc, Window, message, wParam, lParam);
	
	switch (message) {
	case WM_LBUTTONUP:
		// if we click the mouse in the edit control, update the Dasher display
		m_DasherInterface->ChangeEdit();
		InvalidateRect(Window, NULL, FALSE);
		break;
	case WM_KEYUP:
		// if we enter text or move around the edit control, update the Dasher display
		if (Canvas->Running()==false) {
			m_DasherInterface->ChangeEdit();
		}
		InvalidateRect (Window, NULL, FALSE);
		break;
	case WM_COMMAND:
		SendMessage(Parent, message, wParam, lParam);
		break;
	}
	return result;
}


void CEdit::InsertText(Tstring InsertText)
{
	SendMessage(m_hwnd, EM_REPLACESEL, TRUE, (LPARAM)InsertText.c_str()); 
}


// TODO The following were inline in DJW's code. Think about reinstating

/*
void CEdit::dumpedit(int i) const
{

	TCHAR deb[32];
	wsprintf(deb,TEXT("edit %d %x\n"),i,m_hwnd);
	OutputDebugString(deb);
}
*/

void CEdit::deletetext()
{
	DWORD start,finish;
	SendMessage(m_hwnd, EM_GETSEL, (LONG)&start, (LONG)&finish);
	start-=1;
	SendMessage(m_hwnd, EM_SETSEL, (LONG)start, (LONG)finish);
	TCHAR out [2];
	wsprintf(out,TEXT(""));
	SendMessage(m_hwnd, EM_REPLACESEL, TRUE, (LONG)out);
	if (targetwindow!=NULL) {
#ifdef UNICODE
		fakekey[0].type=fakekey[1].type=INPUT_KEYBOARD;
		fakekey[0].ki.wVk=fakekey[1].ki.wVk=VK_BACK;
		fakekey[0].ki.time=fakekey[1].ki.time=0;
		fakekey[1].ki.dwFlags=KEYEVENTF_KEYUP;
		
		SetFocus(targetwindow);
		SendInput(2,fakekey,sizeof(INPUT));
#else
		SetFocus(targetwindow);
		keybd_event(VK_BACK,0,NULL,NULL);
		keybd_event(VK_BACK,0,KEYEVENTF_KEYUP,NULL);
#endif
	}
	if (speech.length()>0) 
		speech.resize(speech.length()-1);
}

void CEdit::SetWindow(HWND window)
{
	targetwindow=window;
	if (threadid!=NULL) {
		AttachThreadInput(GetCurrentThreadId(),threadid,FALSE);
		SetFocus(Parent);
	}
	if (window!=NULL) {
		AttachThreadInput(GetCurrentThreadId(),GetWindowThreadProcessId(window,NULL),TRUE);
		SetFocus(window);
	}
}

void CEdit::outputcontrol (void* pointer, int data, int type)
{
	if (type==1) {
		BYTE pbKeyState[256];
		switch (data) {
		  case 2:
			  // stop
			  Canvas->StartStop();
			  break;
		  case 3:
			  //	pause
			  Canvas->Pause();
			  break;
		  case 4:
			  speak();
			  break;
		  case 11:
			  // move left
		    SendMessage(m_hwnd, WM_KEYDOWN, VK_LEFT, NULL);
			SendMessage(m_hwnd, WM_KEYUP, VK_LEFT, NULL);
			  break;
		  case 12:
			  // move right
			SendMessage(m_hwnd, WM_KEYDOWN, VK_RIGHT, NULL);
			SendMessage(m_hwnd, WM_KEYUP, VK_RIGHT, NULL);
			  break;
		  case 13:
			// move to the start of the document
			GetKeyboardState((LPBYTE) &pbKeyState);
			pbKeyState[VK_CONTROL] |= 0x80;
			SetKeyboardState((LPBYTE) &pbKeyState);
			SendMessage(m_hwnd, WM_KEYDOWN, VK_HOME, NULL);
			SendMessage(m_hwnd, WM_KEYUP, VK_HOME, NULL);
			pbKeyState[VK_CONTROL] &= ~0x80;
			SetKeyboardState((LPBYTE) &pbKeyState);
			  break;
		  case 14:
			  // go to end
			GetKeyboardState((LPBYTE) &pbKeyState);
			pbKeyState[VK_CONTROL] |= 0x80;
			SetKeyboardState((LPBYTE) &pbKeyState);
			SendMessage(m_hwnd, WM_KEYDOWN, VK_END, NULL);
			SendMessage(m_hwnd, WM_KEYUP, VK_END, NULL);
			pbKeyState[VK_CONTROL] &= ~0x80;
			SetKeyboardState((LPBYTE) &pbKeyState);
			  break;
		  case 21:
				//delete next character
			SendMessage(m_hwnd, WM_KEYDOWN, VK_DELETE, NULL);
			SendMessage(m_hwnd, WM_KEYUP, VK_DELETE, NULL);
			  break;
		  case 22:
			BYTE pbKeyState[256];
			GetKeyboardState((LPBYTE) &pbKeyState);
			pbKeyState[VK_CONTROL] |= 0x80;
			pbKeyState[VK_SHIFT] |= 0x80;
			SetKeyboardState((LPBYTE) &pbKeyState);
			SendMessage(m_hwnd, WM_KEYDOWN, VK_RIGHT, NULL);
			SendMessage(m_hwnd, WM_KEYUP, VK_RIGHT, NULL);
			pbKeyState[VK_SHIFT] &= ~0x80;
			pbKeyState[VK_CONTROL] &= ~0x80;
			SetKeyboardState((LPBYTE) &pbKeyState);
			SendMessage(m_hwnd, WM_KEYDOWN, VK_DELETE, NULL);
			SendMessage(m_hwnd, WM_KEYUP, VK_DELETE, NULL);
			  break;
		  case 24:
			  SendMessage(m_hwnd, WM_KEYDOWN, VK_BACK, NULL);
			  SendMessage(m_hwnd, WM_KEYUP, VK_BACK, NULL);
			  break;
		  case 25:
			GetKeyboardState((LPBYTE) &pbKeyState);
			pbKeyState[VK_CONTROL] |= 0x80;
			pbKeyState[VK_SHIFT] |= 0x80;
			SetKeyboardState((LPBYTE) &pbKeyState);
			SendMessage(m_hwnd, WM_KEYDOWN, VK_LEFT, NULL);
			SendMessage(m_hwnd, WM_KEYUP, VK_LEFT, NULL);
			pbKeyState[VK_SHIFT] &= ~0x80;
			pbKeyState[VK_CONTROL] &= ~0x80;
			SetKeyboardState((LPBYTE) &pbKeyState);
			SendMessage(m_hwnd, WM_KEYDOWN, VK_DELETE, NULL);
			SendMessage(m_hwnd, WM_KEYUP, VK_DELETE, NULL);
			  break;
		}
	return;
	}

	if (pointer==NULL) {
		return;
	}
	IAccessible* AccessibleObject=(IAccessible*)pointer;
	BSTR AccessibleAction;
	VARIANT AccessibleVariant;
	HRESULT hr;
	VariantInit(&AccessibleVariant);
	AccessibleVariant.vt=VT_I4;
	AccessibleVariant.lVal=data;
	hr=AccessibleObject->get_accDefaultAction(AccessibleVariant,&AccessibleAction);
	hr=AccessibleObject->accDoDefaultAction(AccessibleVariant);
	VariantClear(&AccessibleVariant);
}

void CEdit::speak() {
	if (pVoice!=0) {
		wchar_t* widespeech=new wchar_t[4096];
		mbstowcs(widespeech,speech.c_str(),speech.length()+1);
		pVoice->Speak(widespeech,SPF_ASYNC,NULL);
		delete(widespeech);
		speech="";
	}
}
