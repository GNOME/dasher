#include "accessibility.h"
#include <iostream>

ControlTree *menutree;

ControlTree* gettree() {
  Accessible *desktop, *child;
  int numchildren;
  desktop = SPI_getDesktop(0);
  menutree = new ControlTree;
  menutree->parent=NULL;
  menutree->children=NULL;
  menutree->next=NULL;
  menutree->pointer=NULL;
  menutree->data=0;
  menutree->text="Menus";
  numchildren = Accessible_getChildCount(desktop);
  for (int i=0; i<numchildren; i++) {
    child=Accessible_getChildAtIndex(desktop,i);
    buildmenutree(child,menutree);
  }
  return menutree;
}

bool buildmenutree(Accessible *parent,ControlTree *ctree) {  
  int numchildren;
  bool useful=false;
  Accessible *child;
  ControlTree *childnode;

  ControlTree* NewNode = new ControlTree;
  NewNode->parent=ctree;
  NewNode->children=NULL;
  NewNode->next=NULL;
  NewNode->pointer=NULL;
    
  // We don't insert ourselves just yet, though

  numchildren=Accessible_getChildCount(parent);

  if (numchildren>0) {
    for (int i=0; i<numchildren; i++) {
      child=Accessible_getChildAtIndex(parent,i);
      if (numchildren==1 && Accessible_getName(child)=="") {
	// ignore nodes that have one child and are nameless
	if (useful==false) {
	  useful=buildmenutree(child,ctree);
	} else {
	  buildmenutree(child,ctree);
	}
      } else {
	if (useful==false) {
	  useful=buildmenutree(child,NewNode);
	} else {
	  buildmenutree(child,NewNode);
	}
      }
    }
  } else {
    // We have no kids - check if we're a menu item
    if (Accessible_getRole(parent)==SPI_ROLE_MENU_ITEM||Accessible_getRole(parent)==SPI_ROLE_CHECK_MENU_ITEM) {      
      NewNode->pointer=parent;
      NewNode->data=1;
      NewNode->children=menutree;
      useful=true;
    }
  }
  if (useful==false) {
    delete NewNode;
  } else {
    if (ctree->children==NULL) {
      ctree->children=NewNode;
    } else {
      ControlTree* parentnext;
      parentnext=ctree->children;
      while (parentnext->next!=NULL) {
	parentnext=parentnext->next;
      };
      parentnext->next=NewNode;
    }
    NewNode->text=Accessible_getName(parent);
  }
  return useful;
}
  
  

