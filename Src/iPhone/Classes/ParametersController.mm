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

//private methods
@interface ParametersController ()
-(void)boolParamChanged:(id)uiswitch;
-(void)longParamChanged:(id)uislider;
@end

@implementation ParametersController

-(id)initWithTitle:(NSString *)title Settings:(SModuleSettings *)_settings Count:(int)_count {
  if (self = [super init]) {
    self.title=title;
    self.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"Back" style:UIBarButtonItemStyleDone target:nil action:nil];
    settings = _settings;
    count = _count;
  }
  return self;
}

-(void)setTarget:(id)_target Selector:(SEL)_selector {
  UIBarButtonItem *item = (UIBarButtonItem *)self.navigationItem.leftBarButtonItem;
  item.target = _target;
  item.action = _selector;
}

- (void)loadView {
  CDasherInterfaceBridge *intf = [DasherAppDelegate theApp].dasherInterface;
  
  UIScrollView *view = [[[UIScrollView alloc] init] autorelease];
  self.view = view;
  view.backgroundColor = [UIColor whiteColor];
    
  int y=15;
  for (int i=0; i<count; i++) {
    if (settings[i].iType == T_BOOL) {
      UILabel *label = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, y, 190.0, 20.0)] autorelease];
      label.text = NSStringFromStdString(intf->GetSettingsStore()->GetParameterName(settings[i].iParameter));
      UISwitch *sw = [[[UISwitch alloc] initWithFrame:CGRectMake(210.0, y, 100.0, 20.0)] autorelease];
      [view addSubview:label];
      [view addSubview:sw];
      sw.on = intf->GetBoolParameter(sw.tag = settings[i].iParameter);
      [sw addTarget:self action:@selector(boolParamChanged:) forControlEvents:UIControlEventValueChanged];
      y += 50;
    } else if (settings[i].iType == T_LONG) {
      UILabel *label = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, y, 300.0, 20.0)] autorelease];
      UISlider *slider = [[[UISlider alloc] initWithFrame:CGRectMake(10.0, y+20, 300.0, 20.0)] autorelease];
      slider.tag = (int)label; label.tag=i;
      slider.minimumValue = settings[i].iMin; slider.maximumValue = settings[i].iMax;
      slider.value = intf->GetLongParameter(settings[i].iParameter);
      [slider addTarget:self action:@selector(longParamChanged:) forControlEvents:UIControlEventValueChanged];
      [self longParamChanged:slider];
      [view addSubview:label]; [view addSubview:slider];
      y += 70;
    }
  }
  [view setContentSize:CGSizeMake(320.0,y-15)];
}

-(void)boolParamChanged:(id)uiswitch {
  UISwitch *sw = (UISwitch *)uiswitch;
  [DasherAppDelegate theApp].dasherInterface->SetBoolParameter(sw.tag, sw.on);
}

-(void)longParamChanged:(id)uislider {
  CDasherInterfaceBridge *intf = [DasherAppDelegate theApp].dasherInterface;
  UISlider *slider = (UISlider *)uislider;
  UILabel *label = (UILabel *)slider.tag;
  SModuleSettings *setting = &settings[label.tag];
  long val = slider.value;
  if (!label.text || val != intf->GetLongParameter(setting->iParameter)) {
    intf->SetLongParameter(setting->iParameter, val);
    int iPlaces=0, iDivisor(setting->iDivisor);
    while (iDivisor > 1) {
      iPlaces++;
      iDivisor/=10;
    }
    NSString *format =[@"%@: %." stringByAppendingFormat:@"%df",iPlaces];  
    label.text = [NSString stringWithFormat:format,NSStringFromStdString(intf->GetSettingsStore()->GetParameterName(setting->iParameter)),(val / (float)setting->iDivisor)];
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
