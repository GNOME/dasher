#include "accessibility.h"
#include "libdasher.h"
#include "edit.h"
#include <libintl.h>
#include <iostream>
#include <libwnck/libwnck.h>

ControlTree *menutree;
ControlTree *buttontree;
ControlTree *paneltree;
ControlTree *edittree;
ControlTree *widgettree;
ControlTree *windowtree;
ControlTree *stoptree;
ControlTree *dummy; // This one is used to fake another control node
extern ControlTree *controltree;
#define _(x) gettext(x)

extern gboolean textentry;
extern gboolean training;
extern gboolean quitting;
extern GtkWidget *the_canvas;
extern GtkWidget *window;

WnckScreen *wnckscreen;

gboolean panels=FALSE;
gboolean building=FALSE;

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
  focusListener = SPI_createAccessibleEventListener(dasher_focus_listener,
						    NULL);
  SPI_registerGlobalEventListener (focusListener, "focus:");
#endif
  // FIXME - should check for correct screen
  wnckscreen = wnck_screen_get_default();
}

void setuptree(ControlTree *tree, ControlTree *parent, ControlTree *children, 
	       ControlTree *next, void* pointer, int data, std::string string, 
	       int colour)
{
	tree->parent=parent;
	tree->children=children;
	tree->pointer=pointer;
	tree->text=string;
	tree->next=next;
	tree->data=data;
	tree->colour=colour;
}

ControlTree* gettree() {	
#ifdef GNOME_A11Y
  building=TRUE;
  menutree = new ControlTree;
  buttontree = new ControlTree;
  paneltree = new ControlTree;
  edittree = new ControlTree;
  widgettree = new ControlTree;
  windowtree = new ControlTree;
  int numchildren;
  desktop = SPI_getDesktop(0);

  /* Paneltree
         |         /-menutree
     Widgettree------buttontree
                   \-edittree
  */

  /* Tree, parent, children, next, pointer, data, string, colour */
  setuptree(paneltree, NULL, NULL, widgettree, NULL, 0, _("Panels"), -1);
  setuptree(widgettree, NULL, menutree, NULL, NULL, 0, _("Application"),
	    -1);
  setuptree(menutree, widgettree, NULL, buttontree, NULL, 0, _("Menus"), -1);
  setuptree(buttontree, NULL, NULL, edittree, NULL, 0, _("Buttons"), -1);
  setuptree(edittree, NULL, NULL, NULL, NULL, 0, _("Text"), -1);

  numchildren = Accessible_getChildCount(desktop);
  for (int i=0; i<numchildren; i++) {
    if (findpanels(Accessible_getChildAtIndex(desktop,i))==TRUE) {
      buildmenutree(Accessible_getChildAtIndex(desktop,i),paneltree,menus);
      panels=TRUE;
    }
  }

  widgettree->children = menutree;
  
  if (focusedwindow!=NULL) {
    buildmenutree(focusedwindow,menutree,menus);
    buildmenutree(focusedwindow,buttontree,pushbuttons);
    buildmenutree(focusedwindow,edittree,textenter);
    menutree->next=buttontree;
    widgettree->next=buildcontroltree();
  } else {
    widgettree=buildcontroltree();
  }
#else
  widgettree=buildcontroltree();
#endif
  building=FALSE;
  if (panels==TRUE) {
    return paneltree;
  } else {
    return widgettree;
  }
}

#ifdef GNOME_A11Y
gboolean findpanels(Accessible *parent) {
  gboolean useful=FALSE;

  if (parent==NULL) {
    return FALSE;
  }

  if(!strcmp(Accessible_getName(parent),"gnome-panel")) {
    Accessible_unref(parent);
    return TRUE;
  }

  int numchildren=Accessible_getChildCount(parent);
  if (numchildren>0) {
    for (int i=0; i<numchildren; i++) {
      useful=(findpanels(Accessible_getChildAtIndex(parent,i))||useful);
    }
  }
  Accessible_unref(parent);
  return useful;
}
#endif

ControlTree* buildcontroltree() {
  dummy=new ControlTree;
  stoptree=new ControlTree;
  windowtree = new ControlTree;
  ControlTree *pausetree=new ControlTree;
  ControlTree *movetree=new ControlTree;
  ControlTree *deletetree=new ControlTree;
  ControlTree *speaktree=new ControlTree;
//  ControlTree *paneltree=new ControlTree;
//  ControlTree *speedtree=new ControlTree;
#ifndef GNOME_A11Y
  // Otherwise menutree hasn't been set yet, and we end up with a bunch of
  // null pointers rather than children
  menutree=windowtree;
#endif
  dummy->pointer=NULL;
  dummy->data=0;
  dummy->next=NULL;
  dummy->children=menutree;
  dummy->text=_("Control");
  dummy->colour=8;

  windowtree->pointer=NULL;
  windowtree->data=0;
  windowtree->children=buildwindowtree();
  windowtree->text=_("Windows");
  windowtree->colour=-1;
  windowtree->next=stoptree;

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
//    deletetree->next=speedtree;
    deletetree->next=NULL;
#else
    deletetree->next=speaktree;
    speaktree->pointer=(void*)1;
    speaktree->data=0;
    speaktree->children=buildspeaktree(speaktree);
    speaktree->text=_("Speak");
    speaktree->next=NULL;
    speaktree->colour=-1;
//    speaktree->next=speedtree;
    speaktree->next=NULL;
#endif
/*    speedtree->pointer=(void*)1;
    speedtree->data=0;
    speedtree->children=buildspeedtree(speedtree);
    speedtree->text=_("Speed");
    speedtree->next=NULL;
    speedtree->colour=-1;
*/
} else {
    pausetree->next=NULL;
}
  return windowtree;
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

ControlTree* buildspeedtree(ControlTree *speedtree) {

  ControlTree *onetree=new ControlTree;
  ControlTree *twotree=new ControlTree;
  ControlTree *threetree=new ControlTree;
  ControlTree *fourtree=new ControlTree;
  ControlTree *fivetree=new ControlTree;
  ControlTree *sixtree=new ControlTree;
  ControlTree *seventree=new ControlTree;
  ControlTree *eighttree=new ControlTree;
  ControlTree *ninetree=new ControlTree;
  ControlTree *tentree=new ControlTree;
  
  onetree->pointer=(void*)1;
  onetree->data=41;
  onetree->children=dummy;
  onetree->text=_("1");
  onetree->next=twotree;
  onetree->colour=-1;

  twotree->pointer=(void*)1;
  twotree->data=42;
  twotree->children=dummy;
  twotree->text=_("2");
  twotree->next=threetree;
  twotree->colour=-1;

  threetree->pointer=(void*)1;
  threetree->data=43;
  threetree->children=dummy;
  threetree->text=_("3");
  threetree->next=fourtree;
  threetree->colour=-1;
  
  fourtree->pointer=(void*)1;
  fourtree->data=44;
  fourtree->children=dummy;
  fourtree->text=_("4");
  fourtree->next=fivetree;
  fourtree->colour=-1;
  
  fivetree->pointer=(void*)1;
  fivetree->data=45;
  fivetree->children=dummy;
  fivetree->text=_("5");
  fivetree->next=sixtree;
  fivetree->colour=-1;
  
  sixtree->pointer=(void*)1;
  sixtree->data=46;
  sixtree->children=dummy;
  sixtree->text=_("6");
  sixtree->next=seventree;
  sixtree->colour=-1;
  
  seventree->pointer=(void*)1;
  seventree->data=47;
  seventree->children=dummy;
  seventree->text=_("7");
  seventree->next=eighttree;
  seventree->colour=-1;
  
  eighttree->pointer=(void*)1;
  eighttree->data=48;
  eighttree->children=dummy;
  eighttree->text=_("8");
  eighttree->next=ninetree;
  eighttree->colour=-1;
  
  ninetree->pointer=(void*)1;
  ninetree->data=49;
  ninetree->children=dummy;
  ninetree->text=_("9");
  ninetree->next=tentree;
  ninetree->colour=-1;
  
  tentree->pointer=(void*)1;
  tentree->data=50;
  tentree->children=dummy;
  tentree->text=_("10");
  tentree->next=NULL;
  tentree->colour=-1;
  
 return onetree;
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
  forwardchar->text=_("Character");
  forwardchar->colour=-1;
  if (a11y_text_entry()==TRUE) {
    forwardchar->next=NULL;
  } else {
    forwardchar->next=forwardword;
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
  }
  
  backwardchar->pointer=(void*)1;
  backwardchar->data=24;
  backwardchar->children=forwardtree;
  backwardchar->text=_("Character");
  backwardchar->colour=-1;
  if (a11y_text_entry()==TRUE) {
    backwardchar->next=NULL;
  } else {
    backwardchar->next=backwardword;
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
  }
  return forwardtree;
}

ControlTree* buildwindowtree()
{
	GList *l;
	wnck_screen_force_update(wnckscreen);
	ControlTree *firstchild = new ControlTree;
	ControlTree *tmptree = firstchild;
	WnckWindow *window;
	std::string name;
	GList *windows = wnck_screen_get_windows(wnckscreen);

	for (l = windows; l; l = l->next) {
		window = (WnckWindow*)l->data;
		name = wnck_window_get_name (window);
		setuptree(tmptree, NULL, controltree, NULL, (void *)window, 31, name, -1);
		tmptree->next = new ControlTree;
		tmptree->next->parent = tmptree;
		tmptree = tmptree->next;

	}
	
	if (tmptree->parent == NULL) {
	  /* wnck didn't provide us with any windows */
	  delete tmptree;
	  return NULL;
	}

	tmptree->parent->next = NULL;
	delete tmptree;
	return firstchild;
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

  AccessibleStateSet *state_set;
  state_set=Accessible_getStateSet(parent);
  AccessibleRole role;

  role=Accessible_getRole(parent);

  if (!AccessibleStateSet_contains(state_set,SPI_STATE_ENABLED) && role!=SPI_ROLE_APPLICATION) {
    return false;
  }

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
  } else {
    // We have no kids - check if we're a menu item
    if (Type==menus) {
      if (role==SPI_ROLE_MENU_ITEM||role==SPI_ROLE_CHECK_MENU_ITEM||role==SPI_ROLE_RADIO_MENU_ITEM) {      
	NewNode->pointer=parent;
	NewNode->data=1;
	NewNode->children=stoptree;
	NewNode->text=Accessible_getName(parent);      
	useful=true;
      } 
    } else if (Type==pushbuttons) {
      if (role==SPI_ROLE_PUSH_BUTTON||role==SPI_ROLE_RADIO_BUTTON||role==SPI_ROLE_TOGGLE_BUTTON) {
	NewNode->pointer=parent;
	NewNode->data=1;
	NewNode->children=stoptree;
	NewNode->text=Accessible_getName(parent);      
	useful=true;
      } 
    } else if (Type==textenter) {
      if (role==SPI_ROLE_TEXT) {
	AccessibleStateSet *state_set;
	state_set=Accessible_getStateSet(parent);
	if (AccessibleStateSet_contains(state_set,SPI_STATE_EDITABLE)) {  
	  AccessibleRelation **relations;
	  NewNode->pointer=parent;
	  NewNode->data=30;
	  NewNode->children=stoptree;
	  relations=Accessible_getRelationSet(parent);
	  if (relations == NULL || *relations == NULL) {
	    NewNode->text=_("Unknown");
	  } else {
	    for (int i=0; relations[i]; i++) {
	      if (AccessibleRelation_getRelationType(relations[i])==SPI_RELATION_LABELED_BY) {
		Accessible *label=AccessibleRelation_getTarget(relations[i],0);		
		NewNode->text=Accessible_getName(label);
		Accessible_unref(label);
		break;
	      }	      
	    }
	    delete relations;
	  }
	  useful=true;
	}
	AccessibleStateSet_unref(state_set);
      } 
    }
    menuitems.push_back(parent);
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
  Accessible_unref(desktop);
  set_textbox(NULL);
#else
  return;
#endif
}

#ifdef GNOME_A11Y
void dasher_focus_listener (const AccessibleEvent *event, void *user_data)
{
  char *name;
  // Don't do this if we're in the middle of doing something else - it'll
  // just result in badness
  if (training==TRUE || building==TRUE || quitting==TRUE) {
    return;
  }
  
  building=TRUE;

  Accessible *tempaccessible;
  Accessible *textaccessible=NULL;
  Accessible *accessible = event->source;
  while (dasher_check_window(Accessible_getRole(accessible))!=TRUE) {
    if (Accessible_getRole(accessible)==SPI_ROLE_TEXT) {
      AccessibleStateSet *state_set=Accessible_getStateSet(accessible);
      if (AccessibleStateSet_contains(state_set,SPI_STATE_EDITABLE)) {
	textaccessible=accessible;
      }
    }
    tempaccessible=Accessible_getParent(accessible);
    if (tempaccessible==NULL) {
      break;
    }
    Accessible_unref(accessible);
    accessible=tempaccessible;
  }
  name=Accessible_getName(accessible);
  if (accessible!=focusedwindow) { // The focused window has changed
    if (!strcmp(name,"Dasher")) {
      Accessible_unref(accessible);
    } else {
      if (focusedwindow!=NULL) {
	Accessible_unref(focusedwindow);
      }
      focusedwindow=accessible;
      deletemenutree();
      controltree=gettree();
      add_control_tree(controltree);
      dasher_start();      
      set_textbox(textaccessible);
    }
  }
  SPI_freeString(name);
  building=FALSE;
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

