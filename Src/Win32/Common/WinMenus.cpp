
#include "WinCommon.h"
#include "WinMenus.h"

#include "../../Common/Allocators/SimplePooledAlloc.h"

#include <comutil.h>

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CSimplePooledAlloc < ControlTree > gControlAlloc(20);

using namespace WinMenus;
std::vector < HWND > windownames;
ControlTree *menutree;
ControlTree *dummytree;
ControlTree *stoptree;

ControlTree *WinMenus::GetWindowMenus() {
  dummytree = gControlAlloc.Alloc();
  stoptree = gControlAlloc.Alloc();
  dummytree->pointer = NULL;
  dummytree->data = 0;
  dummytree->type = 1;
  dummytree->next = NULL;
  dummytree->colour = 8;
  dummytree->text = "Control";
  dummytree->children = stoptree;
  menutree = buildcontroltree();

#ifdef ACCESSIBLE
  EnumDesktopWindows(GetThreadDesktop(GetCurrentThreadId()), WNDENUMPROC(WindowProc), LPARAM(0));
  return (ProcessWindows());
#else

  return menutree;

#endif

}

BOOL CALLBACK WinMenus::WindowProc(HWND hwnd, LPARAM lParam) {
  windownames.push_back(hwnd);
  return TRUE;
}

ControlTree *WinMenus::buildcontroltree() {
  ControlTree *pausetree = gControlAlloc.Alloc();
  ControlTree *movetree = gControlAlloc.Alloc();
  ControlTree *deletetree = gControlAlloc.Alloc();
  ControlTree *speaktree = gControlAlloc.Alloc();
  stoptree->pointer = (void *)1;
  stoptree->data = 2;
  stoptree->children = menutree;
  stoptree->text = "Stop";
  stoptree->next = pausetree;
  stoptree->type = 1;
  stoptree->colour = 242;
  stoptree->children = dummytree;
  pausetree->pointer = (void *)1;
  pausetree->data = 3;
  pausetree->children = menutree;
  pausetree->text = "Pause";
  pausetree->next = movetree;
  pausetree->type = 1;
  pausetree->colour = 241;
  pausetree->children = dummytree;
  movetree->pointer = NULL;
  movetree->data = 0;
  movetree->children = menutree;
  movetree->text = "Move";
  movetree->next = deletetree;
  movetree->children = buildmovetree(movetree);
  movetree->type = 1;
  movetree->colour = -1;
  deletetree->pointer = NULL;
  deletetree->data = 0;
  deletetree->children = builddeletetree(deletetree);
  deletetree->text = "Delete";
  deletetree->next = speaktree;
  deletetree->type = 1;
  deletetree->colour = -1;
  speaktree->pointer = 0;
  speaktree->data = 0;
  speaktree->children = buildspeaktree(speaktree);
  speaktree->text = "Speak";
  speaktree->next = NULL;
  speaktree->type = 1;
  speaktree->colour = -1;
  return stoptree;
}

ControlTree *WinMenus::buildmovetree(ControlTree *movetree) {
  ControlTree *lefttree = gControlAlloc.Alloc();
  ControlTree *righttree = gControlAlloc.Alloc();
  ControlTree *beginningtree = gControlAlloc.Alloc();
  ControlTree *endtree = gControlAlloc.Alloc();
  lefttree->pointer = (void *)1;
  lefttree->data = 11;
  lefttree->children = lefttree;
  lefttree->text = "Previous";
  lefttree->next = righttree;
  lefttree->type = 1;
  lefttree->colour = -1;
  righttree->pointer = (void *)1;
  righttree->data = 12;
  righttree->children = lefttree;
  righttree->text = "Next";
  righttree->next = beginningtree;
  righttree->type = 1;
  righttree->colour = -1;
  beginningtree->pointer = (void *)1;
  beginningtree->data = 13;
  beginningtree->children = lefttree;
  beginningtree->text = "Beginning";
  beginningtree->next = endtree;
  beginningtree->type = 1;
  beginningtree->colour = -1;
  endtree->pointer = (void *)1;
  endtree->data = 14;
  endtree->children = lefttree;
  endtree->text = "End";
  endtree->next = NULL;
  endtree->type = 1;
  endtree->colour = -1;
  return lefttree;
}

ControlTree *WinMenus::builddeletetree(ControlTree *deletetree) {
  ControlTree *forwardtree = gControlAlloc.Alloc();
  ControlTree *backwardtree = gControlAlloc.Alloc();
  ControlTree *forwardchar = gControlAlloc.Alloc();
  ControlTree *forwardword = gControlAlloc.Alloc();
  ControlTree *backwardchar = gControlAlloc.Alloc();
  ControlTree *backwardword = gControlAlloc.Alloc();

  forwardtree->pointer = NULL;
  forwardtree->data = 0;
  forwardtree->next = backwardtree;
  forwardtree->children = forwardchar;
  forwardtree->text = "Forward";
  forwardtree->type = 1;
  forwardtree->colour = -1;
  backwardtree->pointer = NULL;
  backwardtree->data = 0;
  backwardtree->next = NULL;
  backwardtree->children = backwardchar;
  backwardtree->text = "Backward";
  backwardtree->type = 1;
  backwardtree->colour = -1;

  forwardchar->pointer = (void *)1;
  forwardchar->data = 21;
  forwardchar->children = forwardtree;
  forwardchar->next = forwardword;
  forwardchar->text = "Character";
  forwardchar->type = 1;
  forwardchar->colour = -1;
  forwardword->pointer = (void *)1;
  forwardword->data = 22;
  forwardword->children = forwardtree;
  forwardword->next = NULL;
  forwardword->text = "Word";
  forwardword->type = 1;
  forwardword->colour = -1;

  backwardchar->pointer = (void *)1;
  backwardchar->data = 24;
  backwardchar->children = forwardtree;
  backwardchar->next = backwardword;
  backwardchar->text = "Character";
  backwardchar->type = 1;
  backwardchar->colour = -1;
  backwardword->pointer = (void *)1;
  backwardword->data = 25;
  backwardword->children = forwardtree;
  backwardword->next = NULL;
  backwardword->text = "Word";
  backwardword->type = 1;
  backwardword->colour = -1;

  return forwardtree;
}

ControlTree *WinMenus::buildspeaktree(ControlTree *speaktree) {
  ControlTree *everythingtree = gControlAlloc.Alloc();
  ControlTree *newtree = gControlAlloc.Alloc();
  ControlTree *repeattree = gControlAlloc.Alloc();

  everythingtree->pointer = (void *)1;
  everythingtree->data = 4;
  everythingtree->next = newtree;
  everythingtree->children = dummytree;
  everythingtree->text = "Everything";
  everythingtree->type = 1;
  everythingtree->colour = -1;
  newtree->pointer = (void *)1;
  newtree->data = 5;
  newtree->next = repeattree;
  newtree->children = dummytree;
  newtree->text = "New";
  newtree->type = 1;
  newtree->colour = -1;
  repeattree->pointer = (void *)1;
  repeattree->data = 6;
  repeattree->children = dummytree;
  repeattree->next = NULL;
  repeattree->text = "Repeat";
  repeattree->type = 1;
  repeattree->colour = -1;

  return everythingtree;
}

#ifdef ACCESSIBLE

ControlTree *WinMenus::ProcessWindows() {
  IAccessible *AccessibleObject = 0;
  VARIANT AccessibleChild;
  VariantInit(&AccessibleChild);
  std::vector < IAccessible * >AccessibleWindows;
  ControlTree *RootNode;
  RootNode = gControlAlloc.Alloc();
  RootNode->parent = NULL;
  RootNode->children = NULL;
  RootNode->next = NULL;
  RootNode->pointer = NULL;
  RootNode->data = 0;
  RootNode->type = 0;
  RootNode->text = "Menus";
  RootNode->colour = -1;

  for(int i = 0; i < windownames.size(); i++) {
    AccessibleObjectFromWindow(windownames[i], OBJID_WINDOW, IID_IAccessible, (void **)&AccessibleObject);
    if(AccessibleObject != 0) {
      AddObjectToTree(AccessibleObject, RootNode);
    }
  }
  return RootNode;
}

bool WinMenus::AddObjectToTree(IAccessible *AccessibleObject, ControlTree *TreeParent) {
  bool useful = false;
  VARIANT AccessibleObjectVariant;
  long NumChildren;
  ULONG NumFetched;
  _bstr_t bstrstring;

  VariantInit(&AccessibleObjectVariant);
  AccessibleObjectVariant.vt = VT_I4;
  AccessibleObjectVariant.lVal = CHILDID_SELF;

  BSTR ObjectName = NULL;
  HRESULT hr;
  hr = AccessibleObject->get_accName(AccessibleObjectVariant, &ObjectName);
  if(hr == S_OK && ObjectName != NULL) {
    bstrstring = ObjectName;
  }
  else {                        // Nameless things are uninteresting
    return false;
  }

  // Check if we have any children

  ControlTree *NewNode = gControlAlloc.Alloc();
  NewNode->parent = TreeParent;
  NewNode->children = NULL;
  NewNode->next = NULL;
  NewNode->pointer = NULL;
  NewNode->type = 0;
  NewNode->colour = -1;

  // We don't need to hook it into the tree yet

  // Check how many children we have

  AccessibleObject->get_accChildCount(&NumChildren);

  if(NumChildren > 0) {         // if we have kids, recurse through them

    IEnumVARIANT *ParentEnum = NULL;
    AccessibleObject->QueryInterface(IID_IEnumVARIANT, (PVOID *) & ParentEnum);

    if(ParentEnum) {
      ParentEnum->Reset();
    }

    for(int i = 1; i <= NumChildren; i++) {

      IDispatch *ChildDispatch = NULL;
      IAccessible *ChildAccessible = NULL;

      if(ParentEnum) {
        ParentEnum->Next(1, &AccessibleObjectVariant, &NumFetched);
      }
      else {
        AccessibleObjectVariant.vt = VT_I4;
        AccessibleObjectVariant.lVal = i;
      }

      if(AccessibleObjectVariant.vt == VT_I4) {
        HRESULT hr = AccessibleObject->get_accChild(AccessibleObjectVariant, &ChildDispatch);
      }
      else {
        ChildDispatch = AccessibleObjectVariant.pdispVal;
      }

      if(ChildDispatch != NULL && int (ChildDispatch) != 0xcccccccc) {
        ChildDispatch->QueryInterface(IID_IAccessible, (void **)&ChildAccessible);
        ChildDispatch->Release();
      }

      if(ChildAccessible != NULL) {
        if(useful == false) {
          useful = AddObjectToTree(ChildAccessible, NewNode);
        }
        else {
          AddObjectToTree(ChildAccessible, NewNode);
        }
      }
    }
  }
  else {

    // We have no kids - check whether we're a menu item
    VARIANT ObjectRole;
    VariantInit(&ObjectRole);
    ObjectRole.vt = VT_I4;
    AccessibleObject->get_accRole(AccessibleObjectVariant, &ObjectRole);
    if((ObjectRole.iVal == ROLE_SYSTEM_MENUITEM || ObjectRole.iVal == ROLE_SYSTEM_PUSHBUTTON) && NumChildren == 0) {
      NewNode->pointer = AccessibleObject;
      NewNode->data = CHILDID_SELF;
      useful = true;
    }
  }

  // If no useful children, remove our node

  if(useful == false) {
    delete NewNode;
  }
  else {                        // If we're useful, add our node to the parent
    if(TreeParent->children == NULL) {
      TreeParent->children = NewNode;
    }
    else {
      ControlTree *ParentNext;
      ParentNext = TreeParent->children;
      while(ParentNext->next != NULL) {
        ParentNext = ParentNext->next;
      }
      ParentNext->next = NewNode;
    }

    // And give it a name
    NewNode->text = bstrstring;

  }

  VariantClear(&AccessibleObjectVariant);

  return useful;
}

#endif
