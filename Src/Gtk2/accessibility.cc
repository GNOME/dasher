#include "accessibility.h"
#include "libdasher.h"
#include <libintl.h>
#include <iostream>

ControlTree *menutree;
ControlTree *buttontree;
ControlTree *paneltree;
ControlTree *dummy; // This one is used to fake another control node
#define _(x) gettext(x)

extern gboolean textentry;
extern GtkWidget *the_canvas;

gboolean panels;

#ifdef GNOME_A11Y
std::vector<Accessible*> menuitems;
Accessible *desktop=NULL;
Accessible *focusedwindow=NULL;
Accessible *dasherwindow=NULL;
static void dasher_focus_listener (const AccessibleEvent *event,
                                    void *user_data);
static AccessibleEventListener* focusListener;
#endif

void setupa11y() {
#ifdef GNOME_A11Y
  Accessible *tempaccessible;
  //  Accessible *accessible = gtk_widget_get_accessible(the_canvas);  
  focusListener = SPI_createAccessibleEventListener(dasher_focus_listener,
						    NULL);
  // FIXME - this gives us an AtkObject, not an Accessible
  gboolean success=SPI_registerGlobalEventListener (focusListener, "focus:");
  /*  while (dasher_check_window(Accessible_getRole(accessible))!=TRUE) {
    tempaccessible=Accessible_getParent(accessible);
    if (tempaccessible==NULL) {
      break;
    }
    //    Accessible_unref(accessible);
    accessible=tempaccessible;
  }
  if (accessible!=gtk_widget_get_accessible(the_canvas)) {
    dasherwindow=accessible;
    } */
#endif
}

ControlTree* gettree() {
#ifdef GNOME_A11Y
  menutree = new ControlTree;
  buttontree = new ControlTree;
  paneltree = new ControlTree;
  Accessible *child;
  ControlTree *controltree;
  int numchildren;
  desktop = SPI_getDesktop(0);
  menutree->parent=NULL;
  menutree->children=NULL;
  menutree->pointer=NULL;
  menutree->data=0;
  menutree->text=_("Menus");
  menutree->colour=-1;
  menutree->next=NULL;
  buttontree->parent=NULL;
  buttontree->children=NULL;
  buttontree->pointer=NULL;
  buttontree->data=0;
  buttontree->text=_("Buttons");
  buttontree->colour=-1;
  buttontree->next=NULL;
  paneltree->parent=NULL;
  paneltree->children=NULL;
  paneltree->pointer=NULL;
  paneltree->text=_("Panels");
  paneltree->colour=-1;
  paneltree->next=menutree;
  numchildren = Accessible_getChildCount(desktop);
  for (int i=0; i<numchildren; i++) {
    if (findpanels(Accessible_getChildAtIndex(desktop,i))==TRUE) {
      buildmenutree(Accessible_getChildAtIndex(desktop,i),paneltree,menus);
      panels=TRUE;
    }
  }
  
  if (focusedwindow!=NULL) {
    buildmenutree(focusedwindow,menutree,menus);
    buildmenutree(focusedwindow,buttontree,pushbuttons);
    menutree->next=buttontree;
    buttontree->next=buildcontroltree();
  } else {
    ControlTree *menutree=buildcontroltree();
  }
#else
  ControlTree *menutree=buildcontroltree();
#endif
  if (panels==TRUE) {
    return paneltree;
  } else {
    return menutree;
  }
}

gboolean findpanels(Accessible *parent) {
  Accessible *child;
  gboolean useful=FALSE;

  if(!strcmp(Accessible_getName(parent),"gnome-panel")) {
    return TRUE;
  }

  int numchildren=Accessible_getChildCount(parent);
  if (numchildren>0) {
    for (int i=0; i<numchildren; i++) {
      useful=(findpanels(Accessible_getChildAtIndex(parent,i))||useful);
    }
  }
  return useful;
}

ControlTree* buildcontroltree() {
  dummy=new ControlTree;
  ControlTree *stoptree=new ControlTree;
  ControlTree *pausetree=new ControlTree;
  ControlTree *movetree=new ControlTree;
  ControlTree *deletetree=new ControlTree;
  ControlTree *speaktree=new ControlTree;
  ControlTree *paneltree=new ControlTree;
#ifndef GNOME_A11Y
  // Otherwise menutree hasn't been set yet, and we end up with a bunch of
  // null pointers rather than children
  menutree=stoptree;
#endif
  dummy->pointer=NULL;
  dummy->data=0;
  dummy->next=NULL;
  dummy->children=menutree;
  dummy->text=_("Control");
  dummy->colour=8;
  stoptree->pointer=(void*)1;
  stoptree->data=2;
  stoptree->children=dummy;
  stoptree->text=_("Stop");
  stoptree->next=pausetree;
  stoptree->colour=242;
  pausetree->pointer=(void*)1;
  pausetree->data=3;
  pausetree->children=dummy;
  pausetree->text=_("Pause");
  pausetree->colour=241;
  if (textentry==FALSE) {
    pausetree->next=movetree;
    movetree->pointer=NULL;
    movetree->data=0;
    movetree->children=menutree;
    movetree->text=_("Move");
    movetree->next=deletetree;
    movetree->children=buildmovetree(movetree);
    movetree->colour=-1;
    deletetree->pointer=NULL;
    deletetree->data=0;
    deletetree->children=builddeletetree(deletetree);
    deletetree->text=_("Delete");
    deletetree->colour=-1;
#ifndef GNOME_SPEECH
    deletetree->next=NULL;
#else
    deletetree->next=speaktree;
    speaktree->pointer=(void*)1;
    speaktree->data=0;
    speaktree->children=buildspeaktree(speaktree);
    speaktree->text=_("Speak");
    speaktree->next=NULL;
    speaktree->colour=-1;
#endif
  } else {
    pausetree->next=NULL;
  }
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
  lefttree->text=_("<");
  lefttree->next=righttree;
  lefttree->colour=-1;
  righttree->pointer=(void*)1;
  righttree->data=12;
  righttree->children=lefttree;
  righttree->text=_(">");
  righttree->next=beginningtree;
  righttree->colour=-1;
  beginningtree->pointer=(void*)1;
  beginningtree->data=13;
  beginningtree->children=lefttree;
  beginningtree->text=_("<<<");
  beginningtree->next=endtree;
  beginningtree->colour=-1;
  endtree->pointer=(void*)1;
  endtree->data=14;
  endtree->children=lefttree;
  endtree->text=_(">>>");
  endtree->next=NULL;
  endtree->colour=-1;
  return lefttree;
}

ControlTree* buildspeaktree(ControlTree *speaktree) {
  ControlTree *alltree=new ControlTree;
  ControlTree *newtree=new ControlTree;
  ControlTree *lasttree=new ControlTree;
  alltree->pointer=(void*)1;
  alltree->data=4;
  alltree->children=dummy;
  alltree->text=_("Everything");
  alltree->next=newtree;
  alltree->colour=-1;
  newtree->pointer=(void*)1;
  newtree->data=5;
  newtree->children=dummy;
  newtree->text=_("New");
  newtree->next=lasttree;
  newtree->colour=-1;
  lasttree->pointer=(void*)1;
  lasttree->data=6;
  lasttree->children=dummy;
  lasttree->text=_("Repeat");
  lasttree->next=NULL;
  lasttree->colour=-1;
  return alltree;
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
  forwardtree->text=_(">");
  forwardtree->colour=-1;
  backwardtree->pointer=NULL;
  backwardtree->data=0;
  backwardtree->next=NULL;
  backwardtree->children=backwardchar;
  backwardtree->text=_("<");
  backwardtree->colour=-1;

  forwardchar->pointer=(void*)1;
  forwardchar->data=21;
  forwardchar->children=forwardtree;
  forwardchar->next=forwardword;
  forwardchar->text=_("Character");
  forwardchar->colour=-1;
  forwardword->pointer=(void*)1;
  forwardword->data=22;
  forwardword->colour=-1;
  forwardword->children=forwardtree;
  forwardword->next=forwardline;
  forwardword->text=_("Word");
  forwardline->pointer=(void*)1;
  forwardline->data=23;
  forwardline->children=forwardtree;
  forwardline->next=NULL;
  forwardline->text=_("Line");
  forwardline->colour=-1;

  backwardchar->pointer=(void*)1;
  backwardchar->data=24;
  backwardchar->children=forwardtree;
  backwardchar->next=backwardword;
  backwardchar->text=_("Character");
  backwardchar->colour=-1;
  backwardword->pointer=(void*)1;
  backwardword->data=25;
  backwardword->children=forwardtree;
  backwardword->next=backwardline;
  backwardword->text=_("Word");
  backwardword->colour=-1;
  backwardline->pointer=(void*)1;
  backwardline->data=26;
  backwardline->children=forwardtree;
  backwardline->next=NULL;
  backwardline->text=_("Line");
  backwardline->colour=-1;

  return forwardtree;
}

#ifdef GNOME_A11Y
bool buildmenutree(Accessible *parent,ControlTree *ctree,accessibletype Type) {  
  // This code is not desperately nice, and probably ought to be rewritten
  // In any case, the job is as follows: accept an accessible object (parent)
  // and a tree of Dasher nodes (ctree) and build a tree underneath that.
  // We need to search every widget in case there's a menu lurking under
  // it, so go right down to the bottom of the tree and then return true if
  // something is found and false otherwise. Each branch that returns true gets
  // added to the tree, the ones that return false are dropped on the floor.
  int numchildren;
  // Whether or not any of our children are useful - if not (and if we're not
  // useful ourselves) then return false.
  bool useful=false;
  Accessible *child;
  ControlTree *childnode;

  // This is the node that will represent us if we're useful
  // We don't insert ourselves into the tree just yet, though
  ControlTree* NewNode = new ControlTree;
  NewNode->parent=ctree;
  NewNode->children=NULL;
  NewNode->next=NULL;
  NewNode->pointer=NULL;
  NewNode->colour=-1;

  numchildren=Accessible_getChildCount(parent);

  if (numchildren>0) {
    for (int i=0; i<numchildren; i++) {
      child=Accessible_getChildAtIndex(parent,i);
      // If we already have one useful child, then we want to remain useful
      useful=(buildmenutree(child,NewNode,Type)||useful);
    }
    NewNode->text=Accessible_getName(parent);
    menuitems.push_back(parent);
    Accessible_ref(parent);
  } else {
    // We have no kids - check if we're a menu item
    if (Type==menus) {
      if (Accessible_getRole(parent)==SPI_ROLE_MENU_ITEM||Accessible_getRole(parent)==SPI_ROLE_CHECK_MENU_ITEM) {      
	NewNode->pointer=parent;
	NewNode->data=1;
	NewNode->children=menutree;
	NewNode->text=Accessible_getName(parent);      
	useful=true;
      } 
    } else if (Type==pushbuttons) {
      if (Accessible_getRole(parent)==SPI_ROLE_PUSH_BUTTON||Accessible_getRole(parent)==SPI_ROLE_RADIO_BUTTON||Accessible_getRole(parent)==SPI_ROLE_TOGGLE_BUTTON) {
	NewNode->pointer=parent;
	NewNode->data=1;
	NewNode->children=menutree;
	NewNode->text=Accessible_getName(parent);      
	useful=true;
      } 
    }
    menuitems.push_back(parent);
    Accessible_ref(parent);
  }
  if (useful==false) {
    delete NewNode;    
  } else {
    if (ctree->children==NULL) {
      if (NewNode->text=="") {
	// We have no text - attach our child instead of ourselves      
      	ctree->children=NewNode->children;
      } else {	  
	ctree->children=NewNode;
      }
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
#endif
void deletemenutree() {  
#ifdef GNOME_A11Y
  while (menuitems.size()>0) {
    int i=menuitems.size()-1;
    Accessible_unref(menuitems[i]);
    menuitems.pop_back();    
  }
#else
  return;
#endif
}

#ifdef GNOME_A11Y
void dasher_focus_listener (const AccessibleEvent *event, void *user_data)
{
  Accessible *tempaccessible;
  Accessible *accessible = event->source;
  while (dasher_check_window(Accessible_getRole(accessible))!=TRUE) {
    tempaccessible=Accessible_getParent(accessible);
    if (tempaccessible==NULL) {
      break;
    }
    Accessible_unref(accessible);
    accessible=tempaccessible;
  }
  if (accessible!=focusedwindow) { // The focused window has changed
    if (focusedwindow!=NULL) {
      Accessible_unref(focusedwindow);
    }
    // FIXME - setup code above needs to work
    //    if (focusedwindow==dasherwindow) {
    //      return;
    //    }
    deletemenutree();
    focusedwindow=accessible;
    add_control_tree(gettree());
    dasher_start();
  }
}

//FIXME - ripped straight from gok. The same qualms as they have apply.

gboolean dasher_check_window(AccessibleRole role)
{
  /* TODO - improve efficiency here? Also, roles get added and we need
     to maintain this...  maybe we need to go about this differently */
  if ((role == SPI_ROLE_WINDOW) ||
      (role == SPI_ROLE_DIALOG) ||
      (role == SPI_ROLE_FILE_CHOOSER) ||
      (role == SPI_ROLE_FRAME) ||
      (role == SPI_ROLE_DESKTOP_FRAME) ||
      (role == SPI_ROLE_FONT_CHOOSER) ||
      (role == SPI_ROLE_COLOR_CHOOSER) ||
      (role == SPI_ROLE_APPLICATION) ||
      (role == SPI_ROLE_ALERT)
      )
    {
      return TRUE;
    }
  return FALSE;
}
#endif

