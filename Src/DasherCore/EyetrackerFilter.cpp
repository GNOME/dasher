#include "EyetrackerFilter.h"

CEyetrackerFilter::CEyetrackerFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel)
  : CDefaultFilter(pEventHandler, pSettingsStore, pInterface, m_pDasherModel, 5) {
}

void CEyetrackerFilter::ApplyTransform(myint &iDasherX, myint &iDasherY) {
  double disty=(myint)GetLongParameter(LP_OY)-iDasherY;
  
  myint x( iDasherX );
  
  myint dasherOX=(myint)GetLongParameter(LP_OX); 
  
  //  if( iDasherX < dasherOX ) {
  
  //cout << "dasherOX: " << dasherOX << endl; 
  myint dasherOY=(myint)GetLongParameter(LP_OY); 
  
  // X co-ordinate changes. 
  double double_x = (x/dasherOX);  // Fraction of way over to crosshair
  double double_y = -((iDasherY-dasherOY)/(double)(dasherOY) ); // Fraction above the crosshair
  
  // FIXME - I have *no* idea how this is supposed to work - someone else fix it and comment the code please!
  
  double xmax_y = xmax(double_x, double_y); 
  
  if(double_x < xmax_y) { 
    double_x = xmax_y; 
  } 
  
  //      std::cout << xmax_y << std::endl;
  
  x = myint(dasherOX*double_x);
  
  // Finished x-coord changes.
  
  double repulsionparameter=0.5;
  iDasherY = myint(dasherOY - (1.0+ double_y*double_y* repulsionparameter ) * disty);
  
  if( iDasherX < x )
    iDasherX = x;
}

double CEyetrackerFilter::xmax(double x, double y) {
  // DJCM -- define a function xmax(y) thus:
  // xmax(y) = a*[exp(b*y*y)-1] 
  // then:  if(x<xmax(y) [if the mouse is to the RIGHT of the line xmax(y)]
  // set x=xmax(y).  But set xmax=c if(xmax>c).
  // I would set a=1, b=1, c=16, to start with. 
  
  int a = 1, b = 1, c = 100;
  double xmax = a * (exp(b * y * y) - 1);
  //cout << "xmax = " << xmax << endl;
  
  if(xmax > c)
    xmax = c;
  
  return xmax;
}
