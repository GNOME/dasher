#include "accessibility.h"

ControlTree *menutree;

ControlTree* gettree() {
  Accessible *desktop, *child;
  ControlTree *controltree;
  int numchildren;
  desktop = SPI_getDesktop(0);
  menutree = new ControlTree;
  menutree->parent=NULL;
  menutree->children=NULL;
  menutree->pointer=NULL;
  menutree->data=0;
  menutree->text="Menus";
  numchildren = Accessible_getChildCount(desktop);
  for (int i=0; i<numchildren; i++) {
    child=Accessible_getChildAtIndex(desktop,i);
    buildmenutree(child,menutree);
  }
  menutree->next=buildcontroltree();
  return menutree;
}

ControlTree* buildcontroltree() {
  ControlTree *stoptree=new ControlTree;
  ControlTree *pausetree=new ControlTree;
  ControlTree *movetree=new ControlTree;
  ControlTree *deletetree=new ControlTree;
  ControlTree *speaktree=new ControlTree;
  stoptree->pointer=(void*)1;
  stoptree->data=2;
  stoptree->children=menutree;
  stoptree->text="Stop";
  stoptree->next=pausetree;
  pausetree->pointer=(void*)1;
  pausetree->data=3;
  pausetree->children=menutree;
  pausetree->text="Pause";
  pausetree->next=movetree;
  movetree->pointer=NULL;
  movetree->data=0;
  movetree->children=menutree;
  movetree->text="Move";
  movetree->next=deletetree;
  movetree->children=buildmovetree(movetree);
  deletetree->pointer=NULL;
  deletetree->data=0;
  deletetree->children=builddeletetree(deletetree);
  deletetree->text="Delete";
  deletetree->next=speaktree;
  speaktree->pointer=(void*)1;
  speaktree->data=4;
  speaktree->children=menutree;
  speaktree->text="Speak";
  speaktree->next=NULL;
  return stoptree;
}

ControlTree* buildmovetree(ControlTree *movetree) {
  ControlTree *lefttree=new ControlTree;
  ControlTree *righttree=new ControlTree;
  ControlTree *beginningtree=new ControlTree;
  ControlTree *endtree=new ControlTree;
  lefttree->pointer=(void*)1;
  lefttree->data=11;
  lefttree->children=lefttree;
  lefttree->text="Previous";
  lefttree->next=righttree;
  righttree->pointer=(void*)1;
  righttree->data=12;
  righttree->children=lefttree;
  righttree->text="Next";
  righttree->next=beginningtree;
  beginningtree->pointer=(void*)1;
  beginningtree->data=13;
  beginningtree->children=lefttree;
  beginningtree->text="Beginning";
  beginningtree->next=endtree;
  endtree->pointer=(void*)1;
  endtree->data=14;
  endtree->children=lefttree;
  endtree->text="End";
  endtree->next=NULL;
  return lefttree;
}

ControlTree* builddeletetree(ControlTree *deletetree) {
  ControlTree *forwardtree = new ControlTree;
  ControlTree *backwardtree = new ControlTree;
  ControlTree *forwardchar = new ControlTree;
  ControlTree *forwardword = new ControlTree;
  ControlTree *forwardline = new ControlTree;
  ControlTree *backwardchar = new ControlTree;
  ControlTree *backwardword = new ControlTree;
  ControlTree *backwardline = new ControlTree;

  forwardtree->pointer=NULL;
  forwardtree->data=0;
  forwardtree->next=backwardtree;
  forwardtree->children=forwardchar;
  forwardtree->text="Forward";
  backwardtree->pointer=NULL;
  backwardtree->data=0;
  backwardtree->next=NULL;
  backwardtree->children=backwardchar;
  backwardtree->text="Backward";

  forwardchar->pointer=(void*)1;
  forwardchar->data=21;
  forwardchar->children=forwardtree;
  forwardchar->next=forwardword;
  forwardchar->text="Character";
  forwardword->pointer=(void*)1;
  forwardword->data=22;
  forwardword->children=forwardtree;
  forwardword->next=forwardline;
  forwardword->text="Word";
  forwardline->pointer=(void*)1;
  forwardline->data=23;
  forwardline->children=forwardtree;
  forwardline->next=NULL;
  forwardline->text="Line";

  backwardchar->pointer=(void*)1;
  backwardchar->data=24;
  backwardchar->children=forwardtree;
  backwardchar->next=backwardword;
  backwardchar->text="Character";
  backwardword->pointer=(void*)1;
  backwardword->data=25;
  backwardword->children=forwardtree;
  backwardword->next=backwardline;
  backwardword->text="Word";
  backwardline->pointer=(void*)1;
  backwardline->data=26;
  backwardline->children=forwardtree;
  backwardline->next=NULL;
  backwardline->text="Line";

  return forwardtree;
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
