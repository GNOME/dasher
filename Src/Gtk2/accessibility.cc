#include "accessibility.h"

ControlTree *menutree;
std::vector<Accessible*> menuitems;

#ifdef GNOME_A11Y
Accessible *desktop;
#endif

ControlTree* gettree() {
#ifdef GNOME_A11Y
  menutree = new ControlTree;
  Accessible *child;
  ControlTree *controltree;
  int numchildren;
  desktop = SPI_getDesktop(0);
  menutree->parent=NULL;
  menutree->children=NULL;
  menutree->pointer=NULL;
  menutree->data=0;
  menutree->text="Menus";
  menutree->colour=0;
  numchildren = Accessible_getChildCount(desktop);
  for (int i=0; i<numchildren; i++) {
    child=Accessible_getChildAtIndex(desktop,i);
    buildmenutree(child,menutree);
  }
  menutree->next=buildcontroltree();
#else
  ControlTree *menutree=buildcontroltree();
#endif
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
  stoptree->colour=41;
  pausetree->pointer=(void*)1;
  pausetree->data=3;
  pausetree->children=menutree;
  pausetree->text="Pause";
  pausetree->next=movetree;
  pausetree->colour=42;
  movetree->pointer=NULL;
  movetree->data=0;
  movetree->children=menutree;
  movetree->text="Move";
  movetree->next=deletetree;
  movetree->children=buildmovetree(movetree);
  movetree->colour=0;
  deletetree->pointer=NULL;
  deletetree->data=0;
  deletetree->children=builddeletetree(deletetree);
  deletetree->text="Delete";
  deletetree->next=speaktree;
  deletetree->colour=0;
  speaktree->pointer=(void*)1;
  speaktree->data=4;
  speaktree->children=menutree;
  speaktree->text="Speak";
  speaktree->next=NULL;
  speaktree->colour=0;
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
  lefttree->colour=0;
  righttree->pointer=(void*)1;
  righttree->data=12;
  righttree->children=lefttree;
  righttree->text="Next";
  righttree->next=beginningtree;
  righttree->colour=0;
  beginningtree->pointer=(void*)1;
  beginningtree->data=13;
  beginningtree->children=lefttree;
  beginningtree->text="Beginning";
  beginningtree->next=endtree;
  beginningtree->colour=0;
  endtree->pointer=(void*)1;
  endtree->data=14;
  endtree->children=lefttree;
  endtree->text="End";
  endtree->next=NULL;
  endtree->colour=0;
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
  forwardtree->colour=0;
  backwardtree->pointer=NULL;
  backwardtree->data=0;
  backwardtree->next=NULL;
  backwardtree->children=backwardchar;
  backwardtree->text="Backward";
  backwardtree->colour=0;

  forwardchar->pointer=(void*)1;
  forwardchar->data=21;
  forwardchar->children=forwardtree;
  forwardchar->next=forwardword;
  forwardchar->text="Character";
  forwardchar->colour=0;
  forwardword->pointer=(void*)1;
  forwardword->data=22;
  forwardword->colour=0;
  forwardword->children=forwardtree;
  forwardword->next=forwardline;
  forwardword->text="Word";
  forwardline->pointer=(void*)1;
  forwardline->data=23;
  forwardline->children=forwardtree;
  forwardline->next=NULL;
  forwardline->text="Line";
  forwardline->colour=0;

  backwardchar->pointer=(void*)1;
  backwardchar->data=24;
  backwardchar->children=forwardtree;
  backwardchar->next=backwardword;
  backwardchar->text="Character";
  backwardchar->colour=0;
  backwardword->pointer=(void*)1;
  backwardword->data=25;
  backwardword->children=forwardtree;
  backwardword->next=backwardline;
  backwardword->text="Word";
  backwardword->colour=0;
  backwardline->pointer=(void*)1;
  backwardline->data=26;
  backwardline->children=forwardtree;
  backwardline->next=NULL;
  backwardline->text="Line";
  backwardline->colour=0;

  return forwardtree;
}

#ifdef GNOME_A11Y
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
  NewNode->colour=0;
  // We don't insert ourselves just yet, though

  numchildren=Accessible_getChildCount(parent);

  if (numchildren>0) {
    for (int i=0; i<numchildren; i++) {
      child=Accessible_getChildAtIndex(parent,i);
      useful=(buildmenutree(child,NewNode)||useful);
    }
    NewNode->text=Accessible_getName(parent);
    Accessible_unref(parent);
  } else {
    // We have no kids - check if we're a menu item
    if (Accessible_getRole(parent)==SPI_ROLE_MENU_ITEM||Accessible_getRole(parent)==SPI_ROLE_CHECK_MENU_ITEM) {      
      NewNode->pointer=parent;
      NewNode->data=1;
      NewNode->children=menutree;
      NewNode->text=Accessible_getName(parent);      
      useful=true;
      menuitems.push_back(parent);
    }
  }
  if (useful==false) {
    delete NewNode;
    Accessible_unref(parent);
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
  }
  return useful;
}

void deletemenutree() {  
  while (menuitems.size()>0) {
    Accessible_unref(menuitems[menuitems.size()-1]);
    menuitems.pop_back();
  }
  Accessible_unref(desktop);
}
#endif
