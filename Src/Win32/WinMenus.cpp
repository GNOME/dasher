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
	RootNode=new ControlTree;
	RootNode->parent=NULL;
	RootNode->children=NULL;
	RootNode->next=NULL;
	RootNode->pointer=NULL;
	RootNode->data=0;
	RootNode->text="Menus";
	for (int i=0; i<windownames.size(); i++) {
		AccessibleObjectFromWindow(windownames[i],OBJID_WINDOW,IID_IAccessible,(void**)&AccessibleObject);
		if (AccessibleObject!=0) {
			AddObjectToTree(AccessibleObject,RootNode);
		}
	}
	return RootNode;
}

bool WinMenus::AddObjectToTree(IAccessible* AccessibleObject, ControlTree* TreeParent)
{
	bool useful=false;
	VARIANT AccessibleObjectVariant;
	long NumChildren;
	ULONG NumFetched;
	_bstr_t bstrstring;

	VariantInit(&AccessibleObjectVariant);
	AccessibleObjectVariant.vt=VT_I4;
	AccessibleObjectVariant.lVal=CHILDID_SELF;


	BSTR ObjectName=NULL;
	HRESULT hr;
	hr=AccessibleObject->get_accName(AccessibleObjectVariant,&ObjectName);
	if (hr==S_OK && ObjectName!=NULL) {
		bstrstring = ObjectName;
	} else { // Nameless things are uninteresting
		return false;
	}			


	// Check if we have any children


	ControlTree* NewNode = new ControlTree;
	NewNode->parent=TreeParent;
	NewNode->children=NULL;
	NewNode->next=NULL;
	NewNode->pointer=NULL;

	// We don't need to hook it into the tree yet

	// Check how many children we have

	AccessibleObject->get_accChildCount(&NumChildren);

	if (NumChildren>0) { // if we have kids, recurse through them
		IEnumVARIANT* ParentEnum = NULL;

		AccessibleObject->QueryInterface(IID_IEnumVARIANT, (PVOID*) &ParentEnum);

		if (ParentEnum) {
			ParentEnum->Reset();
		}

		for (int i=1; i<=NumChildren; i++) {
			IDispatch* ChildDispatch=NULL;
			IAccessible* ChildAccessible=NULL;

			if (ParentEnum) {
				ParentEnum->Next(1, &AccessibleObjectVariant, &NumFetched);
			}
			else {
				AccessibleObjectVariant.vt=VT_I4;
				AccessibleObjectVariant.lVal=i;
			}
			if (AccessibleObjectVariant.vt==VT_I4) {
				HRESULT hr = AccessibleObject->get_accChild(AccessibleObjectVariant,&ChildDispatch);
			} else {
				ChildDispatch=AccessibleObjectVariant.pdispVal;
			}

			if (ChildDispatch!=NULL && int(ChildDispatch)!=0xcccccccc) {
				ChildDispatch->QueryInterface(IID_IAccessible,(void**)&ChildAccessible);
				ChildDispatch->Release();
			}

			if (ChildAccessible!=NULL) {
				if (useful==false) {
					useful=AddObjectToTree(ChildAccessible,NewNode);
				} else {
					AddObjectToTree(ChildAccessible,NewNode);
				}
			}
		}
	} else {
		// We have no kids - check whether we're a menu item
		VARIANT ObjectRole;
		VariantInit(&ObjectRole);
		ObjectRole.vt=VT_I4;
		AccessibleObject->get_accRole(AccessibleObjectVariant,&ObjectRole);
		if (ObjectRole.iVal==ROLE_SYSTEM_MENUITEM && NumChildren==0) {
			NewNode->pointer=AccessibleObject;
			NewNode->data=CHILDID_SELF;
			useful=true;
		}
	}

	// If no useful children, remove our node

	if (useful==false) {
		delete NewNode;
	} else { // If we're useful, add our node to the parent
		if (TreeParent->children==NULL) {
			TreeParent->children=NewNode;
		} else {
			ControlTree* ParentNext;
			ParentNext=TreeParent->children;
			while (ParentNext->next!=NULL) {
				ParentNext=ParentNext->next;
			}
			ParentNext->next=NewNode;
		}
		// And give it a name
		NewNode->text=bstrstring;
	}

	VariantClear(&AccessibleObjectVariant);

	return useful;
}