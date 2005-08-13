// DasherButtons.cpp, build a set of boxes for Button Dasher.
// Copyright 2005, Chris Ball and David MacKay.  GPL.

#include "DasherButtons.h"
#include <valarray>

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CDasherButtons::CDasherButtons(CSettingsStore *pSettingsStore)
{
  InitMemberVars();
  SetupBoxes();

  m_pSettingsStore = pSettingsStore;
}

CDasherButtons::~CDasherButtons()
{
}

// Setup navigation boxes for Button Dasher.
void CDasherButtons::SetupBoxes()
{  
  // Other variables to which this routine must have access:
  int dasherheight = m_pSettingsStore->GetLongParameter(LP_SCREENHEIGHT);
  int B            = m_pSettingsStore->GetLongParameter(LP_B);
  int s            = m_pSettingsStore->GetLongParameter(LP_S);
  int Z            = m_pSettingsStore->GetLongParameter(LP_Z);
  int r            = m_pSettingsStore->GetLongParameter(LP_R);
  int rightzoom    = m_pSettingsStore->GetLongParameter(LP_RIGHTZOOM);

  int Y = dasherheight;
  int N = 1 << 16; 
  // A normalization constant for use when defining 
  // probabilities, etc. This should be similar to 
  // sqrt(biggest possible integer), and perhaps it's 
  // best to make it a private constant in this button 
  // menu area.

  int S = 1024;	 // Defines the scale for the safety margin parameter.
  int ZOOMDENOM = 1 << 10;	// normalizing constant for defining a zoom: 
  
  // Additional important inputs are parameters that define where in the Dasher
  // world the official zoom-in-able region is. It might make sense for this
  // zoom-in-able region to be similar to the region defined by the regular-Dasher
  // parameters Y2 and Y3, which define the points in the Dasher world beyond
  // which non-linear squishing happens in the dasher2screen mapping. The two
  // parameters defining this zoom-in-able region are

  int BY2 = int (Y * 40 / 1000);	  // An int that might be set equal to Y2. 
  int BY3 = int (Y * 960 / 1000);     // Y3. 

  // Zoom out - relevant if option R=1
  int defaultzoomoutfactor = 2 * ZOOMDENOM;

  struct box boxes_ptr[256];

  // whether this code should be verbose
  int v = 1;

  if (v == 1) {
    cout << Y << endl;
    cout << BY2 << endl;
    cout << BY3 << endl;
  }

  // Find a representation of the ratio $f between adjacent boxes in the 
  // form $rnum/$rdenom
  int rdenom = N;

  // Start with f = 1.
  int rnum = rdenom;
    cout << "rnum: " << rnum << ", rdenom: " << rdenom << endl;

  if (r > 0) {
    // decrement the numerator by (1/128) r times
    for (int i = 1; i <= r; i++) {
      rnum -= (rnum >> 7);
    }
  } else if (r < 0) {
    // increment the numerator by (1/128) r times
    for (int i = -1; i >= r; i--) {
      rnum += (rnum >> 7);
    }
  }

  double ratio = (double) rnum / rdenom;
  if (v == 1) {
    cout << "rnum: " << rnum << ", rdenom: " << rdenom << endl;
    cout << "f = " << rnum << " / " << rdenom << endl;
    cout << "  = " << ratio << endl;
  }
  // Define the unnormalized probabilities

  std::valarray<double> p(B+Z+1);
  double sum;
  int b;
  for (sum = 0, b = 1; b <= B; b++) {
    if (b == 1) {		// the first box.
      p[b] = N;
    } else {
      p[b] = int (p[b - 1] * ratio);
    }

    sum += p[b];
		
    if (v == 1) {
      cout << p[b] << endl;
    }
  }

  // TODO - unfloat this
  float smallestzoom = 1000.0;

  int smallestzoomfound = 0;
  int biggestzoomfound = 0;
	
  // Define the normalized probabilities and the zooms and the four output 
  // quantities.  also find the biggest probability, in order to define a 
  // candidate zoom-out factor

  int fullrange = BY3 - BY2;

  std::valarray<int> displayedtop(B+Z+1);
  std::valarray<int> displayedbottom(B+Z+1);
  std::valarray<int> pdash(B+Z+1);
  std::valarray<int> displayedcentre(B+Z+1);
  std::valarray<int> truecentre(B+Z+1);
  std::valarray<int> zoom(B+Z+1);

  for (b = 1; b <= B; b++) {
    if (b == 1) {		// fix top of the first box.
      displayedtop[b] = BY2;
    } else {
      displayedtop[b] = displayedbottom[b - 1];
    }

	// COMPUTE THE TRUE ZOOM FACTOR
	// the zoom in is by a factor which I render here as a float because
	// I don't know the right integer representation yet.
	// (The float p would be p[b]/sum; and the zoom is 1/(p+s).)

    zoom[b] = int (ZOOMDENOM * S / (p[b] * S / sum + s));

    if (zoom[b] < smallestzoomfound || smallestzoomfound == 0) {
      smallestzoomfound = zoom[b];
    }

    //cout << "smallestzoomfound: " << smallestzoomfound << endl;
    //cout << "zoom[b]: " << zoom[b] << endl;
    if (zoom[b] < smallestzoom) {
      smallestzoom = static_cast<float>(zoom[b]);
    }			// this finds the biggest box in the list.

    if (zoom[b] < ZOOMDENOM) {
      cout <<
        "warning, absurd parameters gave a zoom that is an unzoom"
           << endl;
    }

		

    // express the normalized probabilities in such a way that they sum to fullrange
    pdash[b] = int (p[b] * fullrange / sum);
    displayedbottom[b] = displayedtop[b] + pdash[b];
    displayedcentre[b] = int ((displayedbottom[b] + displayedtop[b]) / 2);
    truecentre[b] = displayedcentre[b];

  }

  if (Z > 0) {
	// assume $b is B+1 now.
	// use the smallestzoom to set the zoom factor
	// but if the smallest zoom is absurdly small (eg <2.0), go for a factor 
	// of 2.0 instead

    if (smallestzoomfound < defaultzoomoutfactor) {
      smallestzoomfound = defaultzoomoutfactor;
    }
		
    ///cout << "smallestzoom is " << smallestzoom << endl;

      // Old rule:  Backwards zoom == half ZOONDENOM.
      //zoom[b] = (int) (ZOOMDENOM * ZOOMDENOM / smallestzoom);

      // New rule:  Backwards zoom == factor of smallestzoomfound.
      if (b == B+Z) {   // If the current box is the backwards one.
        zoom[b] = (int) (ZOOMDENOM * ZOOMDENOM / smallestzoomfound);
      }

      displayedcentre[b] = Y / 2;
      truecentre[b] = displayedcentre[b];

      // it's likely that the zoom-out box does not fit on the canvas, so we fake it
      // - one idea would be to use $BY2 and $BY3 here.
      // but most informative of all is to show the true boundaries 
      // of perpetual zooming up and down, which are...
      // 
      // this, when multiplied by s/2,
      // is the amount by which the true accessible top protrudes above the alleged top BY2

      int topboundary;
      int botboundary;
      // give safe behaviour in silly cases
      if (zoom[1] > ZOOMDENOM) {
        topboundary = fullrange * zoom[1] / (zoom[1] - ZOOMDENOM);
      } else {
        topboundary = fullrange;
      }

      if (zoom[B] > ZOOMDENOM) {
        botboundary = fullrange * zoom[B] / (zoom[B] - ZOOMDENOM);
      } else {
        botboundary = fullrange;
      }

      // multiply by s 

      displayedtop[b] = int (BY2 - s * topboundary / S / 2);
      displayedbottom[b] = int (BY3 + s * botboundary / S / 2);
	
      // So the idea here is that IF your target is OUTSIDE this box, then you 
      // should select IT. For safety ... 

      if (displayedtop[b] < 0) {
        displayedtop[b] = 0;
        //cout << "Warning 0, overrode the accurate 'zoom-out-needed' box with a box guaranteed to be visible. Perhaps s should be smaller!" << endl;
      }
	
      if (displayedbottom[b] > Y) {
        displayedbottom[b] = Y;
        //cout << "Warning Y, overrode the accurate 'zoom-out-needed' box with a box guaranteed to be visible. Perhaps s should be smaller!" << endl;
      }

  }
  // report all outputs

  for ( b = 1; b <= B+Z; b++) {
    boxes_ptr[b].top = displayedtop[b];
    boxes_ptr[b].bottom = displayedbottom[b];
    boxes_ptr[b].zoomloc = truecentre[b];
    boxes_ptr[b].zoomfactor = zoom[b];

    cout << "box: " << b << ", zoomloc: " << truecentre[b] << ", zoomfac: " << zoom[b] << endl;
  }
}



// Just inits all our member variables, called by the constructors
void CDasherButtons::InitMemberVars()
{
}

