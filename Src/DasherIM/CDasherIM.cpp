// CDasherIM.cpp : Implementation of CCDasherIM

#include "stdafx.h"
#include "CDasherIM.h"

// CCDasherIM

HRESULT STDMETHODCALLTYPE CCDasherIM::Select(HWND hwndSip) {
  m_hWndSip = hwndSip;

  m_pContainer = new CIMContainer(hwndSip);
  m_pContainer->Create();
  m_pContainer->SetIMCallback(m_pCallback);

  return S_OK;
}

HRESULT STDMETHODCALLTYPE CCDasherIM::Deselect(void) {
  if(m_pContainer) {
    m_pContainer->DestroyWindow();
    delete m_pContainer;
    m_pContainer = NULL;
  }

  m_hWndSip = NULL;

  return S_OK;
}

HRESULT STDMETHODCALLTYPE CCDasherIM::Showing(void) {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE CCDasherIM::Hiding(void) {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE CCDasherIM::GetInfo(IMINFO *pimi) {
  pimi->cbSize = sizeof(IMINFO);
  // TODO: Bitmaps etc here

  return S_OK;
}

HRESULT STDMETHODCALLTYPE CCDasherIM::ReceiveSipInfo(SIPINFO *psi) {

  if(m_pContainer)
    m_pContainer->MoveWindow(0, 0, psi->rcSipRect.right - psi->rcSipRect.left, psi->rcSipRect.bottom - psi->rcSipRect.top);

  return S_OK;
}

HRESULT STDMETHODCALLTYPE CCDasherIM::RegisterCallback(IIMCallback *lpIMCallback) {
  if(m_pCallback) {
    m_pCallback->Release();
  }

  m_pCallback = lpIMCallback;
  m_pCallback->AddRef();

  if(m_pContainer) {
    m_pContainer->SetIMCallback(m_pCallback);
  }

  return S_OK;
}

HRESULT STDMETHODCALLTYPE CCDasherIM::GetImData(DWORD dwSize, void *pvImData) {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE CCDasherIM::SetImData(DWORD dwSize, void *pvImData) {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE CCDasherIM::UserOptionsDlg(HWND hwndParent) {
  return S_OK;
}