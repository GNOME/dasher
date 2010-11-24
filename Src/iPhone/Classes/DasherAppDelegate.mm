//
//  DasherAppDelegate.m
//  Dasher
//
//  Created by Alan Lawrence on 18/03/2009.
//  Copyright Cavendish Laboratory 2009. All rights reserved.
//

#import "DasherAppDelegate.h"
#import "EAGLView.h"
#import "LanguagesController.h"
#import "InputMethodSelector.h"
#import "CalibrationController.h"
#import "DasherUtil.h"
#import "Common.h"
#import "TextView.h"
#import "MiscSettings.h"
#import "FliteTTS.h"

//declare some private methods!
@interface DasherAppDelegate ()
- (void)settings;
- (void)initDasherInterface;
- (void)finishStartup;
- (void)doSpeedBtnImage:(NSString *)msg;
- (void)speedSlid:(id)slider;
- (CGRect)doLayout:(UIInterfaceOrientation)orient;
@property (retain) UILabel *screenLockLabel;
@property (nonatomic,retain) NSString *m_wordBoundary;
@property (nonatomic,retain) NSString *m_sentenceBoundary;
@property (nonatomic,retain) NSString *m_lineBoundary;
@end

//we can't call setHidden:BOOL with performSelector:withObject:, as passing [NSNumber numberWithBool:YES] (as one should)
// does not pass the supplied YES into setHidden on some/most versions of iPhone OS/SDK...hence, we call hide instead!
@interface UISlider (Hideable)
- (void)hide;
@end

@implementation DasherAppDelegate

@synthesize screenLockLabel;
@synthesize m_wordBoundary;
@synthesize m_sentenceBoundary;
@synthesize m_lineBoundary;
-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
  if (interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown)
    return NO;
  if (m_bLandscapeSupported || interfaceOrientation == UIInterfaceOrientationPortrait)
    return YES;
  return NO;
}

-(void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation {
  [super didRotateFromInterfaceOrientation:fromInterfaceOrientation];
  [self doLayout:self.interfaceOrientation];
}

-(void)viewWillAppear:(BOOL)animated {
  if (m_bLandscapeSupported) {
    UIDeviceOrientation devOrient = [[UIDevice currentDevice] orientation];
    UIInterfaceOrientation intOrient;
    CGAffineTransform trans;
    switch (devOrient) {
      case UIDeviceOrientationLandscapeLeft:
        intOrient = UIInterfaceOrientationLandscapeRight;
        trans = CGAffineTransformMakeRotation(M_PI/2.0);
        break;
      case UIDeviceOrientationLandscapeRight:
        intOrient = UIInterfaceOrientationLandscapeLeft;
        trans = CGAffineTransformMakeRotation(-M_PI/2.0);
        break;
      case UIDeviceOrientationPortrait:
      case UIDeviceOrientationPortraitUpsideDown:
      default: //???
        intOrient = UIInterfaceOrientationPortrait;
        trans = CGAffineTransformIdentity;
        break;
    }
    if (self.interfaceOrientation != intOrient) {
      [[UIApplication sharedApplication] setStatusBarOrientation:intOrient];
      self.view.transform = trans;
      [self doLayout:intOrient];
    }
  }
}

-(void)setLandscapeSupported:(BOOL)bLandscapeSupported {
  m_bLandscapeSupported = bLandscapeSupported;
  //note that if we've just _enabled_ landscape support when the phone
  // was previously in landscape orientation (but the interface _not_),
  // there will be no rotation event sent by the OS; however,
  // we do our best to adjust the interface to suit (i.e., putting it into
  // landscape mode, now that it newly supports this) in viewWillAppear:, above.

  //The case of _disabling_ landscape support when the phone+interface were already in landscape
  // orientation, we do _not_ need to handle (for now): the landscape interface does not allow
  // calling up the settings in order to select a different input method (which might disable it!)
}

/// Sets sizes of toolbar and textview according to supplied orientation
/// Also computes and returns desired size of glView, and sets said _iff_ glView is non-nil
-(CGRect)doLayout:(UIInterfaceOrientation)orient {
  CGRect appFrame = [UIScreen mainScreen].applicationFrame;
  window.frame=appFrame;
  self.view.frame = CGRectMake(0.0, 0.0, appFrame.size.width, appFrame.size.height);

  CGSize mainSize = self.view.bounds.size;
  CGRect dashRect;
  switch (orient) {
    case UIInterfaceOrientationPortrait: {
      dashRect = CGRectMake(0.0, 0.0, mainSize.width, mainSize.height - 100.0);
      CGRect textRect = CGRectMake(0.0, dashRect.size.height, mainSize.width, 70.0);
      text.frame = textRect;
      text.bLandscape = NO;
      messageLabel.frame = CGRectMake(0.0, textRect.origin.y + textRect.size.height - 30.0, mainSize.width, 30.0);
      tools.frame = CGRectMake(0.0, mainSize.height - 30.0, mainSize.width, 30.0);
      [self.view addSubview:tools];
      break;
    }
    case UIInterfaceOrientationLandscapeRight:
    case UIInterfaceOrientationLandscapeLeft: {
      CGRect textRect = CGRectMake(0.0, 0.0, 100.0, mainSize.height);//-30.0);
      text.frame = textRect;
      text.bLandscape = YES;
      messageLabel.frame = CGRectMake(0.0, textRect.origin.y + textRect.size.height - 100.0, textRect.size.width, 100.0);
      dashRect = CGRectMake(textRect.size.width, 0.0, mainSize.width-textRect.size.width, mainSize.height);//-30.0);
      [tools removeFromSuperview];
      break;
    }
    default:
      NSAssert(false, @"Unexpected interface orientation");
  }
  if (glView) glView.frame=dashRect;
  return dashRect;
}

-(CDasherInterfaceBridge *)dasherInterface {return _dasherInterface;}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
  //by default, we support landscape mode (i.e. unless the input device _disables_ it)
  // - hence, set now, before the input device is activate()d...
  m_bLandscapeSupported = YES;

  //sizes set in doLayout, below...
	window = [[UIWindow alloc] init];
	self.view = [[[UIView alloc] init] autorelease];

	[window addSubview:self.view];
  
  //create GUI components...
	text = [[[TextView alloc] init] autorelease];
  messageLabel = [[[UILabel alloc] init] autorelease];
  tools = [[UIToolbar alloc] init]; //retain a reference (until dealloc) because of rotation
	glView = [[[EAGLView alloc] initWithFrame:[self doLayout:UIInterfaceOrientationPortrait] Delegate:self] autorelease];
		
  //start training in a separate thread. (Has to be after first
  // call to doLayout, or get a black band across top of screen)
  [self doAsyncLocked:@"Initializing..." target:self selector:@selector(initDasherInterface) param:nil];

	text.text=@"";
	text.editable = NO;
	text.delegate = self;
	selectedText.location = selectedText.length = 0;
  text.selectedRange=selectedText;

  messageLabel.backgroundColor = [UIColor grayColor];
  messageLabel.textColor = [UIColor whiteColor];
  messageLabel.adjustsFontSizeToFitWidth = YES;
  messageLabel.hidden = YES;

  speedSlider = [[[UISlider alloc] init] autorelease];
  speedSlider.frame = messageLabel.frame;
  speedSlider.minimumValue=0.1; speedSlider.maximumValue=12.0;
  speedSlider.hidden = YES;
  
  [speedSlider addTarget:self action:@selector(fadeSlider) forControlEvents:UIControlEventAllTouchEvents];
  [speedSlider addTarget:self action:@selector(speedSlid:) forControlEvents:UIControlEventValueChanged];
  //...and lay them out
	UIBarButtonItem *settings = [[[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"cog.png"] style:UIBarButtonItemStylePlain target:self action:@selector(settings)] autorelease];
	speedBtn = [UIButton buttonWithType:UIButtonTypeCustom];
	[speedBtn setImageEdgeInsets:UIEdgeInsetsMake(0.0, 2.0, 0.0, 2.0)];
	[speedBtn addTarget:self action:@selector(fadeSlider) forControlEvents:UIControlEventAllTouchEvents];
  
  actions = [[[ActionButton alloc] initForToolbar:tools] autorelease];
  
	UIBarButtonItem *clear = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemTrash target:self action:@selector(clearBtn)] autorelease];
	[tools setItems:[NSArray arrayWithObjects:
				settings,
				[[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil] autorelease],
				[[[UIBarButtonItem alloc] initWithCustomView:speedBtn] autorelease],
				[[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil] autorelease],
				clear,
				[[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil] autorelease],
				actions,
				nil]];
	
	[self.view addSubview:glView];
	[self.view addSubview:text];
  //relying here on things added later being on top of those added earlier.
  //Seems to work ok but not sure whether this is guaranteed?!
  [self.view addSubview:speedSlider];
  [self.view addSubview:messageLabel];
	[self.view addSubview:tools];
	[window makeKeyAndVisible];
  //exit this routine; initDasherInterface (in separate thread) will cause this (main) thread
  // to execute finishStartup, and finally unlock the display, when it's done with training etc.
}

- (void)initDasherInterface {
  //training takes too long to perform in applicationDidFinishLaunching;
  // so we do it here instead (having let the main thread display a "training" message);
  _dasherInterface = new CDasherInterfaceBridge(self);
  //the rest has to be done on the main thread to avoid problems with OpenGL contexts
  // (which are local to one thread); however, we'll have the background thread wait...
  [self performSelectorOnMainThread:@selector(finishStartup) withObject:nil waitUntilDone:YES];
  //...so that initDasherInterface doesn't finish before we're ready
  // (it returns to aSyncMain, which dismisses the modal view controller!)
}

- (void)finishStartup {
  self.dasherInterface->ChangeScreen(new CDasherScreenBridge(glView));
  
	[CalibrationController doSetup]; //restore tilt settings
	[self notifySpeedChange];
  self.dasherInterface->OnUIRealised(); //that does startAnimation...
  doneSetup = YES;
  //The following will cause the text cursor to be displayed whenever
  // any change is made to the textbox...
  [text becomeFirstResponder];
}

- (void)displayMessage:(NSString *)msg ID:(int)iId Type:(int)type {
  //set initial state - overriding any animation in progress...
  messageLabel.text = msg;
  messageLabel.alpha = 1.0;
  messageLabel.hidden = NO;
  //now setup an animation that'll fade it out...
  [UIView beginAnimations:@"MessageAnim" context:nil];
  [UIView setAnimationDuration:2.0];
  [UIView setAnimationCurve:UIViewAnimationCurveEaseIn];
  [UIView setAnimationDelegate:self];
  [UIView setAnimationDidStopSelector:@selector(animationDidStop:finished:context:)];
  [messageLabel setAlpha:0.0];
  [UIView commitAnimations]; //that'll abort any previous animation in progress
  // (and thus proceed with just this one)
}

- (void)animationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context {
  if ([finished boolValue]) {
    messageLabel.hidden = YES;
  }
  //else, we've been called because the old animation's been aborted by a newer one.
  // In which case, leave the new one to proceed without interference...
}

- (void)fadeSlider {
  if ([NSThread isMainThread]) {
    speedSlider.hidden = NO;
    [NSObject cancelPreviousPerformRequestsWithTarget:speedSlider];
    [speedSlider performSelector:@selector(hide) withObject:nil afterDelay:2.0];
  } else [self performSelectorOnMainThread:@selector(fadeSlider) withObject:nil waitUntilDone:NO];
}

- (void)speedSlid:(id)sender {
	float v = ((UISlider *)sender).value;
	_dasherInterface->SetLongParameter(LP_MAX_BITRATE, 100*v);
	//[self notifySpeedChange];//no need, CDasherInterfaceBridge calls if SetLongParameter did anything
}

- (void)clearBtn {
	UIActionSheet *confirm = [[[UIActionSheet alloc] initWithTitle:@"Start New Document" delegate:self cancelButtonTitle:@"Keep Existing" destructiveButtonTitle:@"Discard Existing" otherButtonTitles:nil] autorelease];
	[confirm showFromToolbar:tools];
}

- (void)clearText {
  text.text=@"";
  selectedText.location = selectedText.length = 0;
  _dasherInterface->SetOffset(0);
}
	
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (buttonIndex == actionSheet.destructiveButtonIndex) [self clearText];
}
	
- (void)settings {
  //avoid awful muddle if we change out of tap-to-start mode whilst running.... 
  _dasherInterface->Stop();
  [glView stopAnimation];
	
  UITabBarController *tabs = [[[UITabBarController alloc] init] autorelease];
  tabs.title = @"Settings";
  tabs.navigationItem.leftBarButtonItem = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(settingsDone)] autorelease];
  UINavigationController *settings = [[[UINavigationController alloc] initWithRootViewController:tabs] autorelease];

    tabs.viewControllers = [NSArray arrayWithObjects:
							[[[InputMethodSelector alloc] init] autorelease],
						    [[[LanguagesController alloc] init] autorelease],
                [[[StringParamController alloc] initWithTitle:@"Colour" image:[UIImage imageNamed:@"palette.png"] settingParam:SP_COLOUR_ID] autorelease],
						    [[[MiscSettings alloc] init] autorelease],
                [actions tabConfigurator],
						    nil];
  [self presentModalViewController:settings animated:YES];
}

- (void)settingsDone {
	[self dismissModalViewControllerAnimated:YES];
	[glView startAnimation];
}

- (void)startTimer {
	[glView startAnimation];
}

- (void)shutdownTimer {//Dasher closing...
	[glView stopAnimation];
}

- (void)outputCallback:(NSString *)s {
	text.text=[text.text stringByReplacingCharactersInRange:selectedText withString:s];
	selectedText.location+=[s length]; 
	selectedText.length = 0;
	text.selectedRange = selectedText;
	//This isn't quite right, it jumps up then down again once you have >3 lines...
	[text scrollRangeToVisible:selectedText];
}

- (void)deleteCallback:(NSString *)s {
  if (selectedText.length == 0) selectedText.location -= (selectedText.length = 1); //select previous character
  [self outputCallback:@""];
}

- (BOOL)supportsSpeech {
  if (!fliteEng) {
    fliteEng = [[FliteTTS alloc] init];
    if (!fliteEng) return NO;
    [fliteEng setVoice:@"cmu_us_rms"];
    [fliteEng setPitch:100.0 variance:50.0 speed:1.0];
  }
  return YES;
}

- (void)speak:(NSString *)sText interrupt:(BOOL)bInt {
  if (!fliteEng && ![self supportsSpeech]) return; //fail!
  //"speakText" automatically interrupts previous, i.e. appropriate for bInt.
  //TODO - if (!bInt), should only speak after current speech finished.
  // (However not vital, as we don't offer a setting for BP_SPEAK_WORDS on iPhone)
  [fliteEng speakText:sText];
}

- (void) copy:(NSString *)sText {
  [UIPasteboard generalPasteboard].string=sText;
}

- (void)insertText:(NSString *)sText {
  [self outputCallback:sText];
  // if any text, it came from outside, so buffer/context has changed
  _dasherInterface->SetOffset(selectedText.location, [sText length]);
}

- (void)applicationWillResignActive:(UIApplication *)application {
	[glView stopAnimation];//.animationInterval = 1.0 / 5.0;
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
  if (doneSetup) [glView startAnimation];//glView.animationInterval = 1.0 / 60.0;
}

-(void)applicationDidEnterBackground:(UIApplication *)application {
  self.dasherInterface->WriteTrainFileFull();
}

-(void)applicationWillTerminate:(UIApplication *)application {
  self.dasherInterface->StartShutdown();
}

- (void)newFrameAt:(unsigned long)time ForceRedraw:(BOOL)bForce {
	self.dasherInterface->NewFrame(time, bForce);
}

- (void)notifySpeedChange {
  double speed = self.dasherInterface->GetLongParameter(LP_MAX_BITRATE) / 100.0;
  speedSlider.value = speed; 
	NSString *caption = [NSString stringWithFormat:@"%.2f", speed];
	[self doSpeedBtnImage:caption];
}

- (void)doSpeedBtnImage:(NSString *)msg {
	CGSize size = [msg sizeWithFont:[UIFont boldSystemFontOfSize:12.0]];
	int h = size.height, w = size.width + h + 4.0;
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate(nil, w, h, 8, w*4, colorSpace, kCGImageAlphaPremultipliedLast);
	UIGraphicsPushContext(context);
	CGContextClearRect(context, CGRectMake(0.0, 0.0, w, h));
	const CGFloat whiteComps[] = {1.0, 1.0, 1.0, 1.0};
	CGColorRef white = CGColorCreate(colorSpace, whiteComps);
	CGContextSetFillColorWithColor(context, white);
	CGContextSetStrokeColorWithColor(context, white);
	CGContextTranslateCTM(context, 0.0, h);
	CGContextScaleCTM(context, 1.0, -1.0);
	[msg drawAtPoint:CGPointMake(h/2.0 + 2.0, 0.0) withFont:[UIFont boldSystemFontOfSize:12.0]];
	CGContextBeginPath(context);
	CGContextMoveToPoint(context, 0.0, h/2.0);
	CGContextAddLineToPoint(context, h/2.0, 0.0);
	CGContextAddLineToPoint(context, h/2.0, h);
	CGContextFillPath(context); //implicitly ClosePath's first

	CGContextBeginPath(context);
	
	CGContextMoveToPoint(context, w - h/2.0, 0.0);
	CGContextAddLineToPoint(context, w - h/2, h);
	CGContextAddLineToPoint(context, w, h/2.0);
	CGContextFillPath(context);
	
	CGColorRelease(white);
	UIGraphicsPopContext();
	CGImageRef whole = CGBitmapContextCreateImage(context);
	CGContextRelease(context);
	[speedBtn setImage:[UIImage imageWithCGImage:whole] forState:UIControlStateNormal];
	[speedBtn sizeToFit];
	CGImageRelease(whole);
}

- (void)dealloc {
	[window release];
  [tools release];
  [fliteEng release];
	[super dealloc];
}

- (NSString *)textAtOffset:(unsigned int)offset Length:(unsigned int)length {
  NSRange range;
  //truncate both endpoints of desired range to lie within text.
  // (Although requiring offset+length to be within text, has identified many bugs,
  // the editing functions in control mode are too broken to fix right now! Hence,
  // copying the Gtk2 behaviour...)
  range.location = max(0u,min(offset,[text.text length]));
  range.length = min(length,[text.text length] - range.location);
  return [text.text substringWithRange:range];
}

- (void)setAlphabet:(const CAlphInfo *)pAlph {
  self.m_wordBoundary = pAlph->GetSpaceSymbol() ? NSStringFromStdString(pAlph->GetText(pAlph->GetSpaceSymbol())) : @" ";
  self.m_lineBoundary = (pAlph->GetParagraphSymbol())
  ? NSStringFromStdString(pAlph->GetText(pAlph->GetParagraphSymbol())) : nil;
  self.m_sentenceBoundary = (pAlph->GetDefaultContext().length()>0)
      ? NSStringFromStdString(pAlph->GetDefaultContext())
      : @".";
}

- (int)find:(EEditDistance)amt forwards:(BOOL)bForwards {
  if (amt==EDIT_FILE) return bForwards ? [text.text length] : 0;
  int pos = selectedText.location;
  for(;;) {
    if (bForwards) {
      if (++pos > [text.text length]) return pos-1;
    } else {
      if (--pos < 0) return 0;
    }
    NSString *lookFor;
    switch (amt) {
      case EDIT_CHAR:
        //once only, never loop
        return pos;
      case EDIT_WORD:
        lookFor = m_wordBoundary;
        break;
      case EDIT_LINE:
        if (m_lineBoundary && [text.text compare:m_lineBoundary options:0 range:NSMakeRange(pos, [m_lineBoundary length])] == NSOrderedSame)
          return pos;
        lookFor = m_lineBoundary;
        break;
    }
    if ([text.text compare:lookFor options:0 range:NSMakeRange(pos, [lookFor length])] == NSOrderedSame)
      return pos;
  }
}

- (void)move:(EEditDistance)amt forwards:(BOOL)bForwards {
  selectedText.location = [self find:amt forwards:bForwards];
  selectedText.length=0;
  text.selectedRange = selectedText;
  [text scrollRangeToVisible:selectedText];
}

- (void)del:(EEditDistance)amt forwards:(BOOL)bForwards {
  int to = [self find:amt forwards:bForwards];
  if (bForwards) {
    selectedText.length = to-selectedText.location;
  } else {
    selectedText.length = selectedText.location - to;
    selectedText.location = to;
  }
  [self outputCallback:@""];
}

- (NSString *)allText {
  return text.text;
}

#pragma mark TextViewDelegate methods

-(void)textViewDidChangeSelection:(UITextView *)textView {
  DASHER_ASSERT(textView == text);
  selectedText = text.selectedRange;
  if (selectedText.location == NSIntegerMax) selectedText.location = text.text.length;
  _dasherInterface->SetOffset(selectedText.location);
}

- (void)setLockText:(NSString *)s {
  if ([NSThread isMainThread]) {
    UILabel *lbl = self.screenLockLabel;
    if (!lbl) return;
    if (s) {
      [lbl setText:s];
      lbl.hidden = NO;
    }
    else lbl.hidden = YES;
  }
  else [self performSelectorOnMainThread:@selector(setLockText:) withObject:s waitUntilDone:YES];
}

+ (DasherAppDelegate *)theApp {
	id<UIApplicationDelegate> app = [UIApplication sharedApplication].delegate;
	NSAssert ([app isMemberOfClass:[DasherAppDelegate class]], @"AppDelegate is not DasherAppDelegate!");
	return (DasherAppDelegate *)app;
}
@end

/*@interface UIViewController (lockable)
- (void)aSyncMain:(NSInvocation *)action;
@end*/

@implementation UIViewController (lockable)

- (void)doAsyncLocked:(NSString *)msg target:(id)obj selector:(SEL)sel param:(id)param,... {
  CGSize mainSize = [UIScreen mainScreen].applicationFrame.size;
  UIViewController *lockCon = [[[UIViewController alloc] init] autorelease];
  UIImageView *imgView = [[[UIImageView alloc] initWithFrame:CGRectMake(0.0, 0.0, mainSize.width, mainSize.height)] autorelease];
  imgView.image = [UIImage imageNamed:@"Locked.png"];
  lockCon.view = imgView;
  CGRect mainLabelRect = CGRectMake(40.0, 200.0, mainSize.width-80.0, 80.0);
  UILabel *lbl1 = [[[UILabel alloc] initWithFrame:mainLabelRect] autorelease];
  lbl1.backgroundColor = [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.8];
  lbl1.textColor = [UIColor whiteColor];
  lbl1.textAlignment = UITextAlignmentCenter;
  lbl1.text = msg;
  lbl1.adjustsFontSizeToFitWidth = YES;
  [imgView addSubview:lbl1];
  UILabel *lbl2 = [[[UILabel alloc] initWithFrame:CGRectMake(mainLabelRect.origin.x+20.0,mainLabelRect.origin.y+mainLabelRect.size.height+20,mainLabelRect.size.width-40, mainLabelRect.size.height-40)] autorelease];
  lbl2.backgroundColor = lbl1.backgroundColor;
  lbl2.textColor = lbl1.textColor;
  lbl2.textAlignment = UITextAlignmentCenter;
  lbl2.adjustsFontSizeToFitWidth = YES;
  lbl2.hidden = YES;
  [imgView addSubview:lbl2];
  //think we just hope old value of screenLockLabel was null
  // (i.e. no nested calls to doAsyncLocked...)
  [DasherAppDelegate theApp].screenLockLabel = lbl2;
  [self presentModalViewController:lockCon animated:NO];
  NSMethodSignature *sig = [obj methodSignatureForSelector:sel];
  NSInvocation *action = [NSInvocation invocationWithMethodSignature:sig];
  int numArgs = [sig numberOfArguments];
  [action setTarget:obj];
  [action setSelector:sel];
  if (numArgs>2) //NSInvocation/NSMethodSignature args start at 2
  {              // (0 = self, 1 = _cmd)
    [action setArgument:&param atIndex:2];  // First arg <param> isn't part of the varargs list,
    va_list argumentList;                 
    va_start(argumentList, param); //look for varargs *after* param
    for (int posn = 3; posn < numArgs ; posn++) {
      param = va_arg(argumentList, id);
      [action setArgument:&param atIndex:posn];
    }
    va_end(argumentList);
  }
  [self performSelectorInBackground:@selector(aSyncMain:) withObject:action];
}

- (void)aSyncMain:(NSInvocation *)action {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  [action invoke];
  //passing 'nil' here, where a BOOL is expected, is a horrendous trick - nil = 0x0 is effectively reinterpret_casted... 
  // however, the 'correct' method of passing [NSNumber numberWithBool:] is erratic, resulting in either inversion, 
  // always true, or always false, on different versions of the iPhone OS/SDK...
  [DasherAppDelegate theApp].screenLockLabel = nil;
  [self performSelectorOnMainThread:@selector(dismissModalViewControllerAnimated:) withObject:nil waitUntilDone:NO];
  [pool release];
}

@end

@implementation UISlider (Hideable)

-(void)hide {
  [self setHidden:YES];
}

@end
