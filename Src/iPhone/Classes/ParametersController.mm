//
//  ParametersController.mm
//  Dasher
//
//  Created by Alan Lawrence on 10/02/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import "ParametersController.h"
#import "DasherAppDelegate.h"
#import "DasherUtil.h"

using namespace Dasher;
using Dasher::Settings::GetParameterName;

//private methods
@interface ParametersController ()
-(void)boolParamChanged:(id)uiswitch;
-(void)longParamChanged:(id)uislider;
@end

@implementation ParametersController

-(id)initWithTitle:(NSString *)title Settings:(SModuleSettings *)settings Count:(int)count {
  if (self = [super init]) {
    self.title=title;
    self.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"Back" style:UIBarButtonItemStyleDone target:nil action:nil];
    m_pSettings = settings;
    m_iCount = count;
  }
  return self;
}

-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation {
  return toInterfaceOrientation != UIInterfaceOrientationPortraitUpsideDown;
}

-(void)setTarget:(id)_target Selector:(SEL)_selector {
  UIBarButtonItem *item = (UIBarButtonItem *)self.navigationItem.leftBarButtonItem;
  item.target = _target;
  item.action = _selector;
}

- (void)loadView {
  UIScrollView *view = [[[UIScrollView alloc] init] autorelease];
  self.view = view;
  view.backgroundColor = [UIColor whiteColor];
  
  int y=[self layoutOptionsOn:view startingAtY:15];
  [view setContentSize:CGSizeMake([UIScreen mainScreen].applicationFrame.size.width,y-15)];
}

-(int)layoutOptionsOn:(UIView *)view startingAtY:(int)y {
  if (m_iCount==0) return [self makeNoSettingsLabelOnView:view atY:y];
  return [self layoutModuleSettings:m_pSettings count:m_iCount onView:view startingAtY:y];
}

-(int)layoutModuleSettings:(SModuleSettings *)settings count:(int)count onView:(UIView *)view startingAtY:(int)y {
  CDasherInterfaceBridge *intf = [DasherAppDelegate theApp].dasherInterface;
  CGSize appSize = [UIScreen mainScreen].applicationFrame.size;
  for (int i=0; i<count; i++) {
    if (settings[i].iType == T_BOOL) {
      UISwitch *sw=[self makeSwitch:NSStringFromStdString(GetParameterName(settings[i].iParameter)) onView:view atY:&y];
      sw.tag = settings[i].iParameter;
      sw.on = intf->GetBoolParameter(settings[i].iParameter);
      [sw addTarget:self action:@selector(boolParamChanged:) forControlEvents:UIControlEventValueChanged];
    } else if (settings[i].iType == T_LONG) {
      UILabel *label = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, y, appSize.width-20, 20.0)] autorelease];
      label.textAlignment = NSTextAlignmentCenter;
      UISlider *slider = [[[UISlider alloc] initWithFrame:CGRectMake(10.0, y+20, appSize.width-20, 20.0)] autorelease];
      slider.tag = (NSInteger)label; label.tag=(NSInteger)&settings[i];
      slider.minimumValue = settings[i].iMin; slider.maximumValue = settings[i].iMax;
      slider.value = intf->GetLongParameter(settings[i].iParameter);
      [slider addTarget:self action:@selector(longParamChanged:) forControlEvents:UIControlEventValueChanged];
      [self longParamChanged:slider];
      [view addSubview:label]; [view addSubview:slider];
      y += 70;
    }
  }
  return y;
}

-(UISwitch *)makeSwitch:(NSString *)title onView:(UIView *)view atY:(int *)pY {
  CGFloat appWidth = [UIScreen mainScreen].applicationFrame.size.width;
  const int textWidth(ceil([title sizeWithFont:[UIFont systemFontOfSize:[UIFont labelFontSize]]].width));

  UISwitch *sw = [[[UISwitch alloc] initWithFrame:CGRectMake(10.0, *pY, appWidth/3, 20.0)] autorelease];
  //UISwitch's ignore the size with which they are constructed and make themselves
  // a default "sensible" size...
  CGSize swSize = sw.frame.size;
  UILabel *label = [[[UILabel alloc] initWithFrame:CGRectMake(floor((appWidth-textWidth-swSize.width-20)/2), *pY, textWidth, swSize.height)] autorelease];
  label.text = title;
  sw.frame = CGRectMake(label.frame.origin.x + textWidth+20, *pY, swSize.width, swSize.height);
  
  [view addSubview:label];
  [view addSubview:sw];
  *pY += swSize.height+30;
  return sw;
}

-(int)makeNoSettingsLabelOnView:(UIView *)view atY:(int)y {
  UILabel *label=[[[UILabel alloc] initWithFrame:CGRectMake(10.0, y, view.bounds.size.width-20, 20.0)] autorelease];
  label.text=@"No Settings";
  [view addSubview:label];
  return y+50;
}

-(void)boolParamChanged:(id)uiswitch {
  UISwitch *sw = (UISwitch *)uiswitch;
  [DasherAppDelegate theApp].dasherInterface->SetBoolParameter(sw.tag, sw.on);
}

-(void)longParamChanged:(id)uislider {
  CDasherInterfaceBridge *intf = [DasherAppDelegate theApp].dasherInterface;
  UISlider *slider = (UISlider *)uislider;
  UILabel *label = (UILabel *)slider.tag;
  SModuleSettings *setting = (SModuleSettings *)label.tag;
  long val = slider.value;
  if (!label.text || val != intf->GetLongParameter(setting->iParameter)) {
    intf->SetLongParameter(setting->iParameter, val);
    int iPlaces=0, iDivisor(setting->iDivisor);
    while (iDivisor > 1) {
      iPlaces++;
      iDivisor/=10;
    }
    NSString *format =[@"%@: %." stringByAppendingFormat:@"%df",iPlaces];  
    label.text = [NSString stringWithFormat:format,NSStringFromStdString(GetParameterName(setting->iParameter)),(val / (float)setting->iDivisor)];
  }
}

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end
