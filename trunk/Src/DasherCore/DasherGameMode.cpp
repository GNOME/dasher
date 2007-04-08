
#include "../Common/Common.h"

#include "DasherGameMode.h"
#include "DasherInterfaceBase.h"
#include "DasherModel.h"
#include "DasherNode.h"
#include "DasherView.h"

#include <limits>

CDasherGameMode::CDasherGameMode(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pDashIface, CDasherModel *model)
:CDasherComponent(pEventHandler, pSettingsStore), m_model(model), m_DasherInterface(pDashIface) {
  if(InitializeTargetFile() == myint(-1)) {
//    DASHER_ASSERT(0);   // OOPS Can't open file with target text
  }

  if(GetNextTargetString() == myint(-1)) {
 //   DASHER_ASSERT(0);   // OOPS can't get a string to write
  }
}

myint CDasherGameMode::GetDasherCoordOfTarget() {
  bool KeyIsPrefix = false;     // For alphabet call
  std::string Context;

  // Oops - we don't have an edit box any more :-( 

  // This is the text that has been written so far
  //  m_DasherInterface->GetEditbox()->get_new_context(Context, 1000);
  Context = "foo";

  int posOfFirstDifference = 0;
  alphabet_map alphabetmap = m_model->GetAlphabet().GetAlphabetMap();
  // First, see how many characters they have written correctly so far
  while(alphabetmap.Get(CurrentTarget.substr(posOfFirstDifference, 1), &KeyIsPrefix) == alphabetmap.Get(Context.substr(posOfFirstDifference, 1), &KeyIsPrefix)) {
    posOfFirstDifference++;
    if(posOfFirstDifference == (int) CurrentTarget.length()) {
      // FINISHED WRITING CORRECTLY!
      // should choose new target and reset Dasher at this point?
      return INT64_MIN;
    }
  }

  int contextLength = Context.length();

  int symbolsAfterRoot = 0;

  CDasherNode *root, *crosshair;
  root = m_model->Root();
  crosshair = m_model->Get_node_under_crosshair();
  while(root != crosshair) {
    symbolsAfterRoot++;
    crosshair = crosshair->Parent();
  }

  int positionOfRoot = contextLength - symbolsAfterRoot;
  int positionOfMaxBackup = positionOfRoot - 6;

  if(positionOfMaxBackup >= posOfFirstDifference) {
    if(alphabetmap.Get(CurrentTarget.substr(posOfFirstDifference, 1), &KeyIsPrefix) < alphabetmap.Get(Context.substr(posOfFirstDifference, 1), &KeyIsPrefix))
      return INT64_MIN + 1;
    else
      return INT64_MAX;
  }

//////
// This section calculates the exact dasher coordinate they want to be aiming at
// and then draws the pointer there
  myint currDasherMax, currDasherMin, currDasherRange;

  // Current location in the target string that we are comparing
  // for correctness
  CDasherNode *currNode = m_model->Root();

  if(!currNode)
    return INT64_MIN;

  myint norm = GetLongParameter(LP_NORMALIZATION);
  // These represent the scaled range of dasher coordinates
  // start at the current root but grow as you backtrack
  currDasherMax = m_model->Rootmax();
  currDasherMin = m_model->Rootmin();
  currDasherRange = currDasherMax - currDasherMin;

  // Now find the normalized dasher coordinates for the node we're using as
  // the "correct path" node, the last correctly typed symbol 
  // (or going back as far as we can without overflow)
  while(positionOfRoot > posOfFirstDifference) {
    // IF no parent, but not yet at node of first difference
    // need to just draw up or down arrow again
    if(!currNode->Parent()) {
      if(alphabetmap.Get(CurrentTarget.substr(posOfFirstDifference, 1), &KeyIsPrefix) < alphabetmap.Get(Context.substr(posOfFirstDifference, 1), &KeyIsPrefix))
        return INT64_MIN + 1;
      else
        return INT64_MAX;
    }

    myint nodeupper, nodelower, noderange;

    nodeupper = currNode->Hbnd();
    nodelower = currNode->Lbnd();
    noderange = nodeupper - nodelower;

    currDasherMax += (norm - nodeupper) * currDasherRange / noderange;
    currDasherMin -= nodelower * currDasherRange / noderange;
    currDasherRange = currDasherMax - currDasherMin;

    currNode = currNode->Parent();
    positionOfRoot--;
  }

  // The location we examine first is where we last matched a symbol
  // So, matching 3 symbols means we look at element 3 (zero-based), 4th elem
  int currLocation = positionOfRoot;

  DASHER_ASSERT(currNode);

  // With the original root's dasher coordinates, find where pointer should be
  // Go through this loop until no child is found along the target string
  bool foundChild = true;
  while(foundChild) {
    foundChild = false;
    int childrencount = currNode->ChildCount();

    if(childrencount > 0) {
      CDasherNode::ChildMap &children = currNode->Children();
      for(int ii = 0; ii < childrencount; ii++) // XXX should be based on alphabet size
      {
        int symbol = m_model->GetAlphabet().GetAlphabetMap().Get(CurrentTarget.substr(currLocation, 1), &KeyIsPrefix);
        if(children[ii]->Symbol() == symbol) {
          foundChild = true;
          currLocation++;
          currNode = children[ii];

          // Calculate new dasher high/low/range coords for the child we want
          currDasherMax -= ((norm - currNode->Hbnd()) * currDasherRange) / norm;
          currDasherMin += currNode->Lbnd() * currDasherRange / norm;
          currDasherRange = currDasherMax - currDasherMin;
          break;
        }
      }
    }
    else
      break;
  }

  return (currDasherMax - currDasherRange / 2);
}

myint CDasherGameMode::InitializeTargetFile() {
  std::string TargetFile;
  FILE *Input;
  char buf[128];

  // Put together path to the game text file from settings (read in from AlphIO)
  TargetFile = GetStringParameter(SP_SYSTEM_LOC) + GetStringParameter(SP_GAME_TEXT_FILE);

  // Open file
  if((Input = fopen(TargetFile.c_str(), "r")) == (FILE *) 0) {
    // could not open file
    return -1;
  }

  // Read strings out of that file into vector
  while(fgets(buf, 128, Input) != NULL)
    TargetStrings.push_back(std::string(buf));

  // Done with file, close it up
  fclose(Input);

  return 1;
}

myint CDasherGameMode::GetNextTargetString() {
  if(TargetStrings.size() == 0)
    return -1;

  int r = rand();

  r *= TargetStrings.size();

  r /= RAND_MAX;

  // For now, pick a string at random
  CurrentTarget = TargetStrings.at(r);

  return 1;
}

void CDasherGameMode::Reset() {
  // Reset the vector of strings
  TargetStrings.clear();
  CurrentTarget = "";
}
