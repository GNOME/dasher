//
//  ModuleSettingsController.mm
//  Dasher
//
//  Created by Alan Lawrence on 10/02/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import "ModuleSettingsController.h"
#import "DasherUtil.h"

using namespace Dasher;
using Dasher::Settings::GetParameterName;

//private methods we actually need to call directly
@interface ModuleSettingsController ()
-(void)updateBoxTitle:(NSSlider *)slider;
@end

@implementation ModuleSettingsController

-(id)initWithTitle:(NSString *)title Interface:(COSXDasherControl *)_intf Settings:(SModuleSettings *)_settings Count:(int)_count {
  int height=0;
  for (int i=0; i<_count; i++)
    if (_settings[i].iType == T_BOOL) height+=25;
    else if (_settings[i].iType == T_LONG) height+=57;
  
  NSWindow *window = [[[NSPanel alloc] initWithContentRect:NSMakeRect(200.0f, 100.0f, 400.0f, height)
                                                 styleMask:( NSClosableWindowMask | NSTitledWindowMask) 
                                                   backing:NSBackingStoreBuffered defer:YES] autorelease];
  if (self = [super initWithWindow:window]) {
    settings = _settings;
    count = _count;
    intf=_intf;
    [window setTitle:title];
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    for (int i=0, y=0; i<_count; i++) {
      id ctrl; //either an NSButton, for bool parameters, or an NSSlider, for long params.
      if (settings[i].iType == T_BOOL) {
        NSButton *b = [[[NSButton alloc] initWithFrame:NSMakeRect(5.0, y+5, 390.0, 15.0)] autorelease];
        [b setButtonType:NSSwitchButton];
        [b setTitle:NSStringFromStdString(settings[i].szDescription)];
        [[window contentView] addSubview:b];
        [b setState:(intf->GetBoolParameter(settings[i].iParameter) ? NSOnState : NSOffState)];
        [b setTag:settings[i].iParameter]; 
        [b setTarget:self];
        [b setAction:@selector(boolParamChanged:)];
        ctrl = b;
        y += 25;
      } else if (settings[i].iType == T_LONG) {
        NSBox *box = [[[NSBox alloc] initWithFrame:NSMakeRect(5.0, y+5, 390.0, 50.0)] autorelease];
        //NSRect r=[[box contentView] bounds];
        NSSlider *slider = [[[NSSlider alloc] initWithFrame:NSMakeRect(5.0, 2.0, 380.0, 20.0)] autorelease];
        [slider setTag:i];
        [slider setMinValue:settings[i].iMin]; [slider setMaxValue:settings[i].iMax];
        //[slider setNumberOfTickMarks:(settings[i].iMax-settings[i].iMin)/settings[i].iDivisor+1];
        //[slider setAllowsTickMarkValuesOnly:YES];
        [slider setIntegerValue:intf->GetLongParameter(settings[i].iParameter)];
        [slider setTarget:self];
        [slider setAction:@selector(longParamChanged:)];
        [[box contentView] addSubview:slider];
        [self updateBoxTitle:slider];
        [[window contentView] addSubview:box];
        ctrl = slider;
        y += 57;
      }
      NSString *paramName = NSStringFromStdString(GetParameterName(settings[i].iParameter));
      [udc addObserver:self forKeyPath:[NSString stringWithFormat:@"values.%@", paramName] options:0 context:ctrl];
    }
  } //else, window was autoreleased.
  return self;
}

-(void)showModal {
  NSWindow *window = [self window];
  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:window];  
  [[NSApplication sharedApplication] runModalForWindow:window];
}

-(void)windowWillClose:(NSNotification *)notification {
  //This gets invoked only when we've registered with the NSNotificationCenter,
  // which we do only when we're about to show the window modally.
  [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowWillCloseNotification object:[self window]];
  NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
  for (int i=0; i<count; i++) {
    NSString *paramName = NSStringFromStdString(GetParameterName(settings[i].iParameter));
    [udc removeObserver:self forKeyPath:[NSString stringWithFormat:@"values.%@",paramName]];
  }
  [[NSApplication sharedApplication] stopModal];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
  id ctx = (id)context;
  if ([ctx isKindOfClass:[NSButton class]]) {
    //boolean param
    NSButton *btn = (NSButton *)ctx;
    int iParameter = btn.tag;
    [btn setState:(intf->GetBoolParameter(iParameter) ? NSOnState : NSOffState)];
  } else if ([ctx isKindOfClass:[NSSlider class]]) {
    //long param
    NSSlider *slider = (NSSlider *)ctx;
    int iParameter=settings[slider.tag].iParameter;
    [slider setIntegerValue:intf->GetLongParameter(iParameter)];
    [self updateBoxTitle:slider];
  } else DASHER_ASSERT(false);
}

-(void)boolParamChanged:(id)nsbutton {
  NSButton *b = (NSButton *)nsbutton;
  intf->SetBoolParameter(b.tag, b.state==NSOnState);
}

-(void)longParamChanged:(id)nsslider {
  NSSlider *slider = (NSSlider *)nsslider;
  long val = [slider integerValue];
  SModuleSettings *setting = &settings[slider.tag];
  if (val != intf->GetLongParameter(setting->iParameter)) {
    intf->SetLongParameter(setting->iParameter, val);
    [self updateBoxTitle:slider];
  }
}

-(void)updateBoxTitle:(NSSlider *)slider {
  NSBox *box;
  for (NSView *s = slider; ;) {
    s = [s superview];
    if ([s isKindOfClass:[NSBox class]]) {
      box = (NSBox *)s;
      break;
    }
  }
  SModuleSettings *setting = &settings[slider.tag];
  
  int iPlaces=0, iDivisor(setting->iDivisor);
  while (iDivisor > 1) {
    iPlaces++;
    iDivisor/=10;
  }
  NSString *format =[@"%@: %." stringByAppendingFormat:@"%df",iPlaces];  
  box.title = [NSString stringWithFormat:format,NSStringFromStdString(setting->szDescription),(intf->GetLongParameter(setting->iParameter) / (float)setting->iDivisor)];
}



@end
