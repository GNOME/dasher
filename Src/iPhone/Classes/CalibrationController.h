//
//  CalibrationController.h
//  Dasher
//
//  Created by Alan Lawrence on 26/06/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Vec3.h"

enum settableParam {
	SETTING_VERT,
	SETTING_CUST_MIN,
	SETTING_CUST_MAX,
	SETTING_CUST_X
};

@interface CalibrationController : UITableViewController<UIAccelerometerDelegate, UIActionSheetDelegate> {
  UITableViewCell *vertical, *custom;
  UIView *headers[2];
  UIButton *selButtons[2];
  UITabBarController *tabCon;
	
  //vertical mode
  UILabel *vertMin, *vertMax, *vertX;
  UIButton *setRange;
  UISwitch *invert;
  id<UIAccelerometerDelegate> oldDeleg;

  //custom mode
  UIButton *custMin, *custMax, *custX;
	
  ///If tilt sensor active, prevent autorotation away from this orientation
  UIInterfaceOrientation m_fixOrientation;
  
  settableParam settingParam;
  UILabel *settingLabel; //if SETTING_CUST_X, is the label in the UIActionSheet.
  Vec3 setVec; //the vector of which settingLabel displays a text representation
  float minY, maxY, minX, maxX; //if SETTING_VERT, the min/max values seen so far (also displayed immediately in vertMin, vertMax, vertX)
}

-(id)init;
+(void)doSetup;

@end
