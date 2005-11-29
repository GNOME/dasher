
ToshTilt ReadMe
===============

ToshTilt is a tiny Windows console application that reads the
accelerometers built in to the Toshiba Portege m200 Tablet PC (and
perhaps other Toshiba notebooks with accelerometers?), and sends
messages to Dasher's UDP socket input to allow the user to control the
Dasher cursor by tilting the tablet.

See also http://www.inference.phy.cam.ac.uk/saw27/dasher/toshtilt/
(there's a ready-built binary there too).

To use, go to the Socket page in Dasher's preferences (or, if that's
superceded`, however you configure the Network Socket control of
Dasher). Enable UDP socket control of Dasher on port 20320. Set the X
coordinate label to "x", Y to "y" and the range to -1 (min) to 1 (max)
for X and Y.

Launch toshtilt.exe, and follow the instructions to calibrate. After
the calibration, toshtilt should print out values varying between
about -1 and +1, and Dasher's cursor should move around as you tilt
the tablet.

See comments in toshtilt.cpp for some technicalities.

Thanks to Larry O'Brien for http://www.devx.com/TabletPC/Article/28483

Seb Wills <saw27@mrao.cam.ac.uk>
November 2005
