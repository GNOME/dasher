// CDasherIM.h : Declaration of the CCDasherIM

#pragma once
#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif
#ifdef POCKETPC2003_UI_MODEL
#include "resourceppc.h"
#endif
#ifdef SMARTPHONE2003_UI_MODEL
#include "resourcesp.h"
#endif
#ifdef AYGSHELL_UI_MODEL
#include "resourceayg.h"
#endif

#include "DasherIM.h"
#include "IMContainer.h"

#include <sip.h>


// CCDasherIM

class ATL_NO_VTABLE CCDasherIM :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CCDasherIM, &CLSID_CDasherIM>,
	public IDispatchImpl<ICDasherIM, &IID_ICDasherIM, &LIBID_DasherIMLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
  public IInputMethod
{
public:
	CCDasherIM()
	{
    m_hWndSip = NULL;
    m_pCallback = NULL;

    m_pContainer = NULL;
	}

#ifndef _CE_DCOM
DECLARE_REGISTRY_RESOURCEID(IDR_CDASHERIM)
#endif


BEGIN_COM_MAP(CCDasherIM)
	COM_INTERFACE_ENTRY(ICDasherIM)
	COM_INTERFACE_ENTRY(IDispatch)
  COM_INTERFACE_ENTRY(IInputMethod)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

  // Methods from IInputMethod
  HRESULT STDMETHODCALLTYPE Select(HWND hwndSip);
  HRESULT STDMETHODCALLTYPE Deselect(void);
  HRESULT STDMETHODCALLTYPE Showing(void);
  HRESULT STDMETHODCALLTYPE Hiding(void);
  HRESULT STDMETHODCALLTYPE GetInfo(IMINFO *pimi);
  HRESULT STDMETHODCALLTYPE ReceiveSipInfo(SIPINFO *psi);
  HRESULT STDMETHODCALLTYPE RegisterCallback(IIMCallback *lpIMCallback);
  HRESULT STDMETHODCALLTYPE GetImData(DWORD dwSize, void *pvImData);
  HRESULT STDMETHODCALLTYPE SetImData(DWORD dwSize, void *pvImData);
  HRESULT STDMETHODCALLTYPE UserOptionsDlg(HWND hwndParent);

private:
  HWND m_hWndSip;
  IIMCallback *m_pCallback;
  CIMContainer *m_pContainer;
};

OBJECT_ENTRY_AUTO(__uuidof(CDasherIM), CCDasherIM)
