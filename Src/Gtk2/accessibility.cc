#include "accessibility.h"
#include <iostream>

std::vector<ControlTree*> mytree;
GNode* gtree;

ControlTree* gettree() {
  Accessible *desktop;
  desktop = SPI_getDesktop(0);
  gtree = buildtree(desktop,NULL);
  buildmenutree(gtree,NULL);
  return mytree[0];
}

int scantree(Accessible *parent) {
  Accessible *child;
  int children, i, useful=0;
  children = Accessible_getChildCount(parent);
  for (i=0; i<children; i++) {
    child = Accessible_getChildAtIndex(parent,i);
    if (useful==0) 
      useful = scantree(child);
    else
      scantree(child);
    if (Accessible_getRole(child)==SPI_ROLE_MENU_ITEM||Accessible_getRole(child)==SPI_ROLE_CHECK_MENU_ITEM) {
      useful=1;
    }
  }
  return useful;  
}

GNode* buildtree(Accessible *parent, GNode *treeparent) {
  Accessible *child;
  int children, i;
  GNode *me;

  if(treeparent==NULL) {
    treeparent=g_node_new(NULL);
    me=treeparent;
  } else {
    me=g_node_new(parent);
    g_node_append(treeparent,me);
  }

  children = Accessible_getChildCount(parent);
  for (i=0; i<children; i++) {
    child = Accessible_getChildAtIndex(parent,i);
    if (scantree(child)==1) {
      buildtree(child,me);
    }
    if (Accessible_getRole(child)==SPI_ROLE_MENU_ITEM||Accessible_getRole(child)==SPI_ROLE_CHECK_MENU_ITEM) {
      g_node_append(me,g_node_new(child));
    }
  }
  if(me->parent!=NULL)
    return 0;
  else {
    return me;
  }
}

ControlTree* buildmenutree(GNode* tree, ControlTree *ctree) {  
  ControlTree *childnode;
  if (ctree==NULL) {
    ctree = new ControlTree;
    ctree->parent=NULL;
    ctree->children=NULL;
    ctree->next=NULL;
    ctree->pointer=NULL;
    ctree->data=0;
    ctree->text="Menus";
    mytree.push_back(ctree);
  }
  if (tree->children!=NULL) {
    childnode=new ControlTree;    
    childnode->parent=ctree;
    childnode->children=NULL;
    childnode->next=NULL;
    childnode->pointer=NULL;
    childnode->data=0;
    ctree->children=childnode;
    if(tree->data!=NULL)
      ctree->text=Accessible_getName((Accessible*)tree->data);
    mytree.push_back(childnode);
    buildmenutree(tree->children,childnode);    
  }
  if (tree->next!=NULL) {
    childnode=new ControlTree;
    childnode->parent=ctree->parent;
    childnode->children=NULL;
    childnode->next=NULL;
    childnode->pointer=NULL;
    childnode->data=0;
    ctree->next=childnode;
    //    ctree->text=Accessible_getName((Accessible*)tree->data);
    mytree.push_back(childnode);
    buildmenutree(tree->next,childnode);
  }
  if (tree->children==NULL) {
    ctree->pointer=tree->data;
    ctree->data=1;
    ctree->text=Accessible_getName((Accessible*)tree->data);
  }
  if(ctree->parent==NULL)
    return ctree;
  else
    return 0;
}
  
  
