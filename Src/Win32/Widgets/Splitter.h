// Splitter.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2006 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Splitter_h__
#define __Splitter_h__

/////////////////////////////////////////////////////////////////////////////

// Abstract interface - callback for resize of splitter

class CSplitterOwner 
{
public:
	virtual void Layout() = 0;
};

/////////////////////////////////////////////////////////////////////////////

class CSplitter :public ATL::CWindowImpl<CSplitter>
{	

public:

	CSplitter(CSplitterOwner* pOwner,int Pos);
	
	HWND Create(HWND Parent);

	void Move(int Pos, int Width);
	
  int GetHeight();
	
	int GetPos() 
	{
		return m_iPos;
	}

  bool IsSizing() const { return m_SplitStatus == Sizing; }
	static ATL::CWndClassInfo& GetWndClassInfo() 
  { 

    static ATL::CWndClassInfo wc = 
		{ 
			{ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW , StartWindowProc, 
			  0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_WINDOW), NULL, _T("HSplitter"), NULL }, 
			NULL, NULL, MAKEINTRESOURCE(IDC_SIZENS), TRUE, 0, _T("") 
		};
		return wc;
	}

	BEGIN_MSG_MAP( CSplitter )
	    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	    MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	END_MSG_MAP()

protected:

	enum SplitStatus
	{ 
		None, 
		Sizing 
	};
	
	LRESULT OnLButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
  CSplitterOwner* m_pOwner;
  int m_iPos;
  SplitStatus m_SplitStatus;
};

#endif  /* #ifndef __Splitter_h__ */
