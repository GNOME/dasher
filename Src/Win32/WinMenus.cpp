#include "WinMenus.h"
#include <comutil.h>

using namespace WinMenus;

std::vector<HWND> windownames;

ControlTree* WinMenus::GetWindowMenus() {
	EnumDesktopWindows(GetThreadDesktop(GetCurrentThreadId()),WNDENUMPROC(WindowProc),LPARAM(0));
	return (ProcessWindows());
}

BOOL CALLBACK WinMenus::WindowProc(HWND hwnd, LPARAM lParam)
{
	windownames.push_back(hwnd);
	return TRUE;
}

ControlTree* WinMenus::ProcessWindows() {
	IAccessible* AccessibleObject=0;
	VARIANT AccessibleChild;
	VariantInit(&AccessibleChild);
	std::vector<IAccessible*> AccessibleWindows;
	ControlTree* RootNode;
	for (int i=0; i<windownames.size(); i++) {
		AccessibleObjectFromWindow(windownames[i],OBJID_WINDOW,IID_IAccessible,(void**)&AccessibleObject);
		if (AccessibleObject!=0) {
			if (ProcessObject(AccessibleObject)==true) {
				AccessibleWindows.push_back(AccessibleObject);
			}
		}
	}
	RootNode=new ControlTree;
	RootNode->parent=NULL;
	RootNode->children=NULL;
	RootNode->next=NULL;
	RootNode->data=0;
	RootNode->text="Menus";
	for (int i=0; i<AccessibleWindows.size(); i++) {
		AddObjectToTree(AccessibleWindows[i],RootNode);
	}
	return RootNode;
}

bool WinMenus::ProcessObject(IAccessible* AccessibleObject)
{
	long NumChildren;
	unsigned long NumFetched;
	bool useful=false;
	VARIANT ChildVariant;
	IEnumVARIANT* ParentEnum = NULL;

	AccessibleObject->QueryInterface(IID_IEnumVARIANT, (PVOID*) &ParentEnum);

	if (ParentEnum) {
		ParentEnum->Reset();
	}

	AccessibleObject->get_accChildCount(&NumChildren);

	for (int i=1; i<=NumChildren; i++) {
		IDispatch* ChildDispatch=NULL;
		IAccessible* ChildAccessible=NULL;

		if (ParentEnum) {
			ParentEnum->Next(1, &ChildVariant, &NumFetched);
		}
		else {
			ChildVariant.vt=VT_I4;
			ChildVariant.lVal=i;
		}
		if (ChildVariant.vt==VT_I4) {
			HRESULT hr = AccessibleObject->get_accChild(ChildVariant,&ChildDispatch);
			if (hr==S_FALSE) {			
				VARIANT ChildRole;
				VariantInit(&ChildRole);
				ChildRole.vt=VT_I4;

				AccessibleObject->get_accRole(ChildVariant,&ChildRole);
				if (ChildRole.iVal==ROLE_SYSTEM_MENUITEM) {
					useful=true;
					VariantClear(&ChildRole);
				}
				continue;
			}
		}
		else {
			ChildDispatch=ChildVariant.pdispVal;
		}

		if (ChildDispatch!=NULL && int(ChildDispatch)!=0xcccccccc) {
			ChildDispatch->QueryInterface(IID_IAccessible,(void**)&ChildAccessible);
			ChildDispatch->Release();
		}

		if (ChildAccessible!=NULL) {
			VariantInit(&ChildVariant);
			ChildVariant.vt=VT_I4;
			ChildVariant.lVal=CHILDID_SELF;

			BSTR ChildName;
			ChildAccessible->get_accName(ChildVariant,&ChildName);
			if(VarBstrCmp(ChildName,L"System",LOCALE_USER_DEFAULT,NORM_IGNORECASE)==1) {
				ChildAccessible->Release();
				continue;
			}

			if (useful!=true)
				useful=ProcessObject(ChildAccessible);
			else
				ProcessObject(ChildAccessible);
		}
	}
	return useful;
}

void WinMenus::AddObjectToTree(IAccessible* AccessibleObject, ControlTree* TreeParent)
{
	ControlTree* ChildTree = new ControlTree;
	ChildTree->children=NULL;
	ChildTree->next=NULL;
	std::vector<IAccessible*> UsefulChildren;

	// Add our name
	
	VARIANT ChildVariant;
	BSTR ChildName=NULL;
	HRESULT hr;
	ChildVariant.vt=VT_I4;
	ChildVariant.iVal=CHILDID_SELF;
	VariantInit(&ChildVariant);
	hr=AccessibleObject->get_accName(ChildVariant,&ChildName);
	if (hr==S_OK) {
		_bstr_t bstrstring = ChildName;
		ChildTree->text=bstrstring;
	} else {
		ChildTree->text="";
	}

	VariantClear(&ChildVariant);

	if (ChildTree->text=="") {
		return; // uninteresting
	}

	// Add ourselves to the tree in the appropriate location
	if (TreeParent->children==NULL) {
		TreeParent->children=ChildTree;
	} else {
		ControlTree* ParentNext;
		ParentNext=TreeParent;
		while (ParentNext->next!=NULL) {
			ParentNext=ParentNext->next;
		}
		ParentNext->next=ChildTree;
	}
	ChildTree->pointer=NULL;
	ChildTree->data=0;
	// Now do the same for our children

	long NumChildren;
	unsigned long NumFetched;
	bool useful=false;
	IEnumVARIANT* ParentEnum = NULL;

	AccessibleObject->QueryInterface(IID_IEnumVARIANT, (PVOID*) &ParentEnum);

	if (ParentEnum) {
		ParentEnum->Reset();
	}

	AccessibleObject->get_accChildCount(&NumChildren);

	for (int i=1; i<=NumChildren; i++) {
		IDispatch* ChildDispatch=NULL;
		IAccessible* ChildAccessible=NULL;

		if (ParentEnum) {
			ParentEnum->Next(1, &ChildVariant, &NumFetched);
		}
		else {
			ChildVariant.vt=VT_I4;
			ChildVariant.lVal=i;
		}
		if (ChildVariant.vt==VT_I4) {
			HRESULT hr = AccessibleObject->get_accChild(ChildVariant,&ChildDispatch);
			if (hr==S_FALSE) {			
				VARIANT ChildRole;
				VariantInit(&ChildRole);
				ChildRole.vt=VT_I4;

				AccessibleObject->get_accRole(ChildVariant,&ChildRole);
				if (ChildRole.iVal==ROLE_SYSTEM_MENUITEM) {
					VariantClear(&ChildRole);
					ControlTree* MenuNode = new ControlTree;
					MenuNode->next=NULL;
					MenuNode->parent=ChildTree;
					MenuNode->children=NULL;
					MenuNode->pointer=NULL;
					MenuNode->data=0;
					if (ChildTree->children==NULL) {
						ChildTree->children=MenuNode;
					} else {
						ControlTree* MenuNext;
						MenuNext=ChildTree;
						while (MenuNext->next!=NULL) {
							MenuNext=MenuNext->next;
						}
						MenuNext->next=MenuNode;
					}

					BSTR ChildName=NULL;
					HRESULT hr;
					hr=AccessibleObject->get_accName(ChildVariant,&ChildName);
					if (hr==S_OK) {
						_bstr_t bstrstring = ChildName;
						MenuNode->text=bstrstring;
					} else {
						MenuNode->text="";
					}
					MenuNode->pointer=AccessibleObject;
					MenuNode->data=i;
				}
				continue;
			}
		}
		else {
			ChildDispatch=ChildVariant.pdispVal;
		}

		if (ChildDispatch!=NULL && int(ChildDispatch)!=0xcccccccc) {
			ChildDispatch->QueryInterface(IID_IAccessible,(void**)&ChildAccessible);
			ChildDispatch->Release();
		}

		if (ChildAccessible!=NULL) {
			VariantInit(&ChildVariant);
			ChildVariant.vt=VT_I4;
			ChildVariant.lVal=CHILDID_SELF;

			BSTR ChildName;
			ChildAccessible->get_accName(ChildVariant,&ChildName);
			if(VarBstrCmp(ChildName,L"System",LOCALE_USER_DEFAULT,NORM_IGNORECASE)==1) {
				ChildAccessible->Release();
				continue;
			}		
		}
		if (ChildAccessible!=NULL) {
			if (ProcessObject(ChildAccessible)==true) {
				UsefulChildren.push_back(ChildAccessible);
			}
		}
	}

	for (int i=0; i<UsefulChildren.size(); i++) {
		AddObjectToTree(UsefulChildren[i],ChildTree);
	}

	return;
}