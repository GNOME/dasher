#include <math.h>

/* Unlike round() from libmath, this function rounds up to the highest
 * integral value, so round(-1.5)=-1 rather than -2.
 * This function tends to be called with positive values, in which case
 * the behaviour is the same: round(1.5)=2.
 */
double round(double dVal) {
  double dF = floor(dVal);
  double dC = ceil(dVal);

  if(dVal - dF < dC - dVal)
    return dF;
  else
    return dC;
}
