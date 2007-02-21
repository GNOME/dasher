// DasherView.inl
//
// Copyright (c) 2001-2002 David Ward

#include "DasherView.h"
#include "DasherScreen.h"
using namespace Dasher;

// inline void Dasher::CDasherView::MapScreen(screenint *DrawX, screenint *DrawY) {
//   using namespace Dasher::Opts;

//   switch (ScreenOrientations(GetLongParameter(LP_ORIENTATION))) {
//   case (LeftToRight):
//     break;
//   case (RightToLeft):
//     *DrawX = Screen()->GetWidth() - *DrawX;
//     break;
//   case (TopToBottom):{
//       screenint Swapper = (*DrawX * Screen()->GetHeight()) / Screen()->GetWidth();
//       *DrawX = (*DrawY * Screen()->GetWidth()) / Screen()->GetHeight();
//       *DrawY = Swapper;
//       break;
//     }
//   case (BottomToTop):{
//       // Note rotation by 90 degrees not reversible like others
//       screenint Swapper = Screen()->GetHeight() - (*DrawX * Screen()->GetHeight()) / Screen()->GetWidth();
//       *DrawX = (*DrawY * Screen()->GetWidth()) / Screen()->GetHeight();
//       *DrawY = Swapper;
//       break;
//     }
//   default:
//     break;
//   }
// }

// inline void Dasher::CDasherView::UnMapScreen(screenint *MouseX, screenint *MouseY) {
//   using namespace Dasher::Opts;

//   switch (ScreenOrientations(GetLongParameter(LP_ORIENTATION))) {
//   case (LeftToRight):
//     break;
//   case (RightToLeft):
//     *MouseX = Screen()->GetWidth() - *MouseX;
//     break;
//   case (TopToBottom):{
//       screenint Swapper = (*MouseX * Screen()->GetHeight()) / Screen()->GetWidth();
//       *MouseX = (*MouseY * Screen()->GetWidth()) / Screen()->GetHeight();;
//       *MouseY = Swapper;
//       break;
//     }
//   case (BottomToTop):{
//       screenint Swapper = (*MouseX * Screen()->GetHeight()) / Screen()->GetWidth();
//       *MouseX = ((Screen()->GetHeight() - *MouseY) * Screen()->GetWidth()) / Screen()->GetHeight();
//       *MouseY = Swapper;
//       break;
//     }
//   default:
//     break;
//   }
// }
