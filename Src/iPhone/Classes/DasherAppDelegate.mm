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
#import "ParametersController.h"
#import "DasherUtil.h"
#import "Common.h"
#import "TextView.h"
#import "FliteTTS.h"
#import "ActionConfigurator.h"

#import "FileUtils.h"

@interface UITextView (MessageHider)
-(void)hideMessage:(NSNumber *)height;
@end

@implementation UITextView (MessageHider)
-(void)hideMessage:(NSNumber *)n {
  CGFloat height = [n floatValue];
  CGRect b = self.frame;
  self.frame = CGRectMake(b.origin.x, b.origin.y+height, b.size.width, b.size.height-height);
  if (b.size.height-height<=0.0) self.hidden=YES;
  else [self scrollRangeToVisible:NSMakeRange([self.text length], 0)];
}
@end


//Most of the view controllers we use, are TableViewControllers. This makes them default
// to rotating to any orientation other than upside-down.
@interface UITableViewController (MultiOrient)
-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation;
@end

@implementation UITableViewController (MultiOrient)
-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation {
  return toInterfaceOrientation != UIInterfaceOrientationPortraitUpsideDown;
}
@end


//declare some private methods!
@interface DasherAppDelegate ()
- (void)settings;
- (void)initDasherInterface;
- (void)finishStartup;
- (void)speedSlid:(id)slider;
- (CGRect)doLayout:(UIInterfaceOrientation)orient;
//calls through to [EAGLView makeContextCurrent]
- (void)selectEAGLContext;
@property (retain) UILabel *screenLockLabel;
@property (retain) NSString *lockText;
@property (nonatomic,retain) NSString *m_wordBoundary;
@property (nonatomic,retain) NSString *m_sentenceBoundary;
@property (nonatomic,retain) NSString *m_lineBoundary;
@end

//we can't call setHidden:BOOL with performSelector:withObject:, as passing [NSNumber numberWithBool:YES] (as one should)
// does not pass the supplied YES into setHidden on some/most versions of iPhone OS/SDK...hence, we call hide instead!
@interface UISlider (Hideable)
- (void)hide;
@end

static SModuleSettings _miscSettings[] = { //note iStep and string description are ignored
  {LP_NODE_BUDGET, T_LONG, 400, 10000, 1, 0, ""}, //hopefully appropriate for an iPhone 3GS?
  {LP_MARGIN_WIDTH, T_LONG, 100, 900, 1, 0, ""},
  {LP_DASHER_FONTSIZE, T_LONG, 5, 40, 1, 1, ""},
  {LP_OUTLINE_WIDTH, T_LONG, -5, 5, 1, -1, ""},
  {BP_AUTO_SPEEDCONTROL, T_BOOL, -1, -1, -1, -1, ""},
  {LP_NONLINEAR_X, T_LONG, 0, 10, 1, -1, ""},
  {LP_GAME_HELP_DIST, T_LONG, 0, 8192, 1, 0, ""},
  {LP_GAME_HELP_TIME, T_LONG, 0, 10000, 1000, 0, ""},
  {BP_GAME_HELP_DRAW_PATH, T_BOOL, -1, -1, -1, -1, ""},
};

static NSString *EDIT_FONT_SIZE = @"iPhoneEditBoxFontSize";

//MiscSettings is a normal ParametersController for the above, plus adds
// a slider for the editbox font size. A quick hack not a general mechanism!!
@interface MiscSettings : ParametersController {
	TextView *tv;
}
-(id)initForTextView:(TextView *)tv;
-(void)longUserDefChanged:(id)uislider;
@end

@implementation MiscSettings

-(id)initForTextView:(TextView *)_tv {
  if (self =[super initWithTitle:@"Misc" Settings:_miscSettings Count:sizeof(_miscSettings)/sizeof(_miscSettings[0])]) {
    tv = _tv;
    self.tabBarItem.image = [UIImage imageNamed:@"misc.png"];
  }
  return self;
}

-(int)layoutOptionsOn:(UIView *)view startingAtY:(int)y {
  if (m_iCount)
    y=[super layoutOptionsOn:view startingAtY:y];
  NSUserDefaults *ud=[NSUserDefaults standardUserDefaults];
  CGSize appSize = [UIScreen mainScreen].applicationFrame.size;
  UILabel *label = [[[UILabel alloc] initWithFrame:CGRectMake(10.0, y, appSize.width-20, 20.0)] autorelease];
  label.textAlignment = UITextAlignmentCenter;
  UISlider *slider = [[[UISlider alloc] initWithFrame:CGRectMake(10.0, y+20, appSize.width-20, 20.0)] autorelease];
  slider.tag = (NSInteger)label; label.tag=(NSInteger)EDIT_FONT_SIZE;
  slider.minimumValue = 5;
  slider.maximumValue = 40;
  slider.value = [ud integerForKey:EDIT_FONT_SIZE];
  [slider addTarget:self action:@selector(longUserDefChanged:) forControlEvents:UIControlEventValueChanged];
  [self longUserDefChanged:slider];
  [view addSubview:label]; [view addSubview:slider];
  y += 70;
  return y;
}

-(void)longUserDefChanged:(id)uislider {
  NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
  UISlider *s=(UISlider *)uislider;
  UILabel *lbl=(UILabel *)s.tag;
  NSString *key = (NSString *)lbl.tag;
  int sz = s.value;
  if (!lbl.text || sz != [ud integerForKey:key]) {
    [ud setInteger:sz forKey:key];
    lbl.text = [key stringByAppendingFormat:@": %i",sz];
    tv.font = [tv.font fontWithSize:sz];
  }
}

@end

using namespace Dasher;

@implementation DasherAppDelegate

@synthesize screenLockLabel;
@synthesize lockText;
@synthesize m_wordBoundary;
@synthesize m_sentenceBoundary;
@synthesize m_lineBoundary;
@synthesize allowsRotation = m_bAllowsRotation;

static DasherAppDelegate *s_appDelegate;

//a private method called only by CDasherInterfaceBridge
-(EAGLView *)glView {
  return glView;
}

-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
  if (interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown)
    return NO;
  if (m_bAllowsRotation || !glView.animating) return YES;
  return interfaceOrientation == self.interfaceOrientation;
}

-(void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation {
  [super didRotateFromInterfaceOrientation:fromInterfaceOrientation];
  [self doLayout:self.interfaceOrientation];
}

/// Sets sizes of toolbar and textview according to supplied orientation
/// Also computes and returns desired size of glView, and sets said _iff_ glView is non-nil
-(CGRect)doLayout:(UIInterfaceOrientation)orient {
  self.view.frame = CGRectMake(0, 0, [UIScreen mainScreen].bounds.size.width, [UIScreen mainScreen].bounds.size.height);
  CGFloat top = [UIApplication sharedApplication].statusBarFrame.size.height;
  const CGSize mainSize = self.view.bounds.size;
  //now always display toolbar, even in landscape
  const int barHeight(mainSize.height/20);
  const int mainHeight(mainSize.height - barHeight);
  tools.frame = CGRectMake(0.0, mainHeight, mainSize.width, barHeight);
  CGRect dashRect,textRect;
  switch (orient) {
    case UIInterfaceOrientationPortrait: {
      dashRect = CGRectMake(0.0, top, mainSize.width, ((mainHeight*3)/4));
      textRect = CGRectMake(0.0, top + dashRect.size.height, mainSize.width, mainHeight-dashRect.size.height-top);
      textView.bLandscape = NO;
      break;
    }
    case UIInterfaceOrientationLandscapeRight:
    case UIInterfaceOrientationLandscapeLeft: {
      textRect = CGRectMake(0.0, top, static_cast<int>((mainSize.width*2)/9), mainHeight-top);
      dashRect = CGRectMake(textRect.size.width, top, mainSize.width-textRect.size.width, mainHeight-top);
      textView.bLandscape = YES;
      break;
    }
    default:
      NSAssert(false, @"Unexpected interface orientation");
  }
  textView.frame = textRect;
  webView.frame = textRect;
  messageLabel.frame = CGRectMake(0.0, textRect.origin.y + textRect.size.height - messageLabel.frame.size.height, mainSize.width, messageLabel.frame.size.height);
  //YEUCH but unfortunately I can find no way to (programmatically) get the proper height for a UISlider.
  // Setting its height to 0 seems to be stored ok, but ends up with the slider being unresponsive -
  // yet clearly rendered >0 pixels high onscreen...
  speedSlider.frame = CGRectMake(0.0, textRect.origin.y + textRect.size.height-23.0, mainSize.width, 23.0);
  [NSObject cancelPreviousPerformRequestsWithTarget:messageLabel];
  if (glView) glView.frame=dashRect;
  return dashRect;
}

-(CDasherInterfaceBridge *)dasherInterface {return _dasherInterface;}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
  // check delegate
  id<UIApplicationDelegate> appDelegate = application.delegate;
  NSAssert ([appDelegate isMemberOfClass:[DasherAppDelegate class]], @"AppDelegate is not DasherAppDelegate!");
  s_appDelegate = (DasherAppDelegate*)appDelegate;
  
  //by default, we support landscape mode (i.e. unless the input device _disables_ it)
  // - hence, set now, before the input device is activate()d...
  m_bAllowsRotation = YES;
    
  //make object (this doesn't do anything much, initialization/Realize later
  // - but we have to set a screen before we Realize)
  _dasherInterface = new CDasherInterfaceBridge(self);
  //temporary hack: in earlier releases, font sizes were on scale 1-3.
  //adjust to new scale...
  if (_dasherInterface->GetLongParameter(LP_DASHER_FONTSIZE)<5)
    _dasherInterface->SetLongParameter(LP_DASHER_FONTSIZE, 10*max(_dasherInterface->GetLongParameter(LP_DASHER_FONTSIZE),1l));
    
    // iOS specific default settings
    std::string geometryStr("Geometry");
    if(!_dasherInterface->IsParameterSaved(geometryStr)) {
        // maybe new value needed
        // _dasherInterface->SetLongParameter(LP_GEOMETRY, 0);
    }
    // initiate window, the rest at viewDidLoad
    //sizes set in doLayout, below...
    self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
    // self.view = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, self.window.bounds.size.width, self.window.bounds.size.height)] autorelease];
    // [self.window addSubview:self.view];
    self.window.rootViewController = self;
    [self.window makeKeyAndVisible];
      //exit this routine; initDasherInterface (in separate thread) will cause this (main) thread
  // to execute finishStartup, and finally unlock the display, when it's done with training etc.
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor whiteColor];
    //create GUI components...
    textView = [[[TextView alloc] init] autorelease];
    webView = [[[UIWebView alloc] init] autorelease];
    messageLabel = [[[UITextView alloc] init] autorelease];
    speedSlider = [[[UISlider alloc] init] autorelease];
    tools = [[UIToolbar alloc] init]; //retain a reference (until dealloc) because of rotation
    glView = [[[EAGLView alloc] initWithFrame:[self doLayout:UIInterfaceOrientationPortrait] Delegate:self] autorelease];
    glView.animating = NO;
    glView.multipleTouchEnabled = YES;
    
    textView.text=@"";
    textView.editable = NO;
    textView.delegate = self;
    selectedText.location = selectedText.length = 0;
    textView.selectedRange=selectedText;
    NSInteger sz = [[NSUserDefaults standardUserDefaults] integerForKey:EDIT_FONT_SIZE];
    if (!sz)
        [[NSUserDefaults standardUserDefaults] setInteger:sz=12 forKey:EDIT_FONT_SIZE];
    
    textView.font = [UIFont systemFontOfSize:sz];
    
    webView.dataDetectorTypes = UIDataDetectorTypeNone;
    webView.delegate = self;
    
    messageLabel.editable = NO;
    messageLabel.backgroundColor = [UIColor grayColor];
    messageLabel.textColor = [UIColor whiteColor];
    messageLabel.contentInset = UIEdgeInsetsZero;
    messageLabel.hidden = YES;
    
    CGFloat maxSpeed = _dasherInterface->GetLongParameter(LP_X_LIMIT_SPEED) / 100.0;
    self.speedChangeStep = (CGFloat)maxSpeed / 15;
    speedSlider.minimumValue = 0.1;
    speedSlider.maximumValue = maxSpeed;
    speedSlider.hidden = YES;
    
    [speedSlider addTarget:self action:@selector(fadeSlider) forControlEvents:UIControlEventAllTouchEvents];
    [speedSlider addTarget:self action:@selector(speedSlid:) forControlEvents:UIControlEventValueChanged];
    
    // init speed label, it will be added to view in refreshToolbar call
    self.speedLabel = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
    self.speedLabel.userInteractionEnabled = YES;
    [self.speedLabel addGestureRecognizer:[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(fadeSlider)]];
    
    [self notifySpeedChange];
    [self refreshToolbar];
    
    [self.view addSubview:glView];
    [self.view addSubview:textView];
    [self.view addSubview:webView];
    //relying here on things added later being on top of those added earlier.
    //Seems to work ok but not sure whether this is guaranteed?!
    [self.view addSubview:speedSlider];
    [self.view addSubview:messageLabel];
    [self.view addSubview:tools];
    
    
    //start Realization i.e. training in a separate thread. (Has to be after first
    // call to doLayout, or get a black band across top of screen)
    [self doAsyncLocked:@"Initializing..." target:self selector:@selector(initDasherInterface) param:nil];
    
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    if(doneSetup) {
        glView.animating = YES;
    }
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
    glView.animating = NO;
}
-(void)refreshToolbar {
  UIBarButtonSystemItem icon = _dasherInterface->GetGameModule() ? UIBarButtonSystemItemStop : UIBarButtonSystemItemPlay;
  UIBarButtonItem *game = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem:icon target:self action:@selector(toggleGameMode)] autorelease];
  UIBarButtonItem *action = [ActionButton buttonForToolbar:tools];
  if (!toolbarItems) {
      UIBarButtonItem *flexSpaceLeft = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace target:nil action:nil] autorelease];
      flexSpaceLeft.width = 40.0;
      toolbarItems = [[NSMutableArray arrayWithObjects:
                   [[[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"cog.png"] style:UIBarButtonItemStylePlain target:self action:@selector(settings)] autorelease],
                       flexSpaceLeft,
                   // rewind icon for slowing down
                     [[[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"rewind"] style:UIBarButtonItemStylePlain target:self action:@selector(didTapSlowDown:)] autorelease],
                   // speed label
                   [[[UIBarButtonItem alloc] initWithCustomView:self.speedLabel] autorelease],
                   // fast-forward icon for speeding up
                     [[[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"fast-forward"] style:UIBarButtonItemStylePlain target:self action:@selector(didTapSpeedUp:)] autorelease],
                   [[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil] autorelease],
                   game,
                   //[[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil] autorelease],
                   action,
                   nil] retain];
  } else {
    [toolbarItems replaceObjectAtIndex:6 withObject:game];
    [toolbarItems replaceObjectAtIndex:7 withObject:action];
  }
  webView.hidden = !(textView.hidden = _dasherInterface->GetGameModule() ? YES : NO);
  [tools setItems:toolbarItems];
}

- (IBAction)didTapSlowDown:(id)sender {
    speedSlider.value = MAX(speedSlider.value - self.speedChangeStep, speedSlider.minimumValue);
    [self speedSlid:speedSlider];
    [self fadeSlider];
}

- (IBAction)didTapSpeedUp:(id)sender {
    speedSlider.value = MIN(speedSlider.value + self.speedChangeStep, speedSlider.maximumValue);
    [self speedSlid:speedSlider];
    [self fadeSlider];
}

-(void)webViewDidFinishLoad:(UIWebView *)sender {
  DASHER_ASSERT(sender==webView);
  //try for approx twice as much beneath as above
  [sender stringByEvaluatingJavaScriptFromString:[NSString stringWithFormat:@"window.scrollBy(0,Math.max(0,document.getElementById(\"here\").offsetTop-%d));",static_cast<int>(sender.frame.size.height/3)]];
}

- (void)initDasherInterface {
  //training takes too long to perform in applicationDidFinishLaunching;
  // so we do it here instead (having let the main thread display a "training" message);
  _dasherInterface->Realize();
  //the rest has to be done on the main thread to avoid problems with OpenGL contexts
  // (which are local to one thread); however, we'll have the background thread wait...
  [self performSelectorOnMainThread:@selector(finishStartup) withObject:nil waitUntilDone:YES];
  //...so that initDasherInterface doesn't finish before we're ready
  // (it returns to aSyncMain, which dismisses the modal view controller!)
}

- (void)finishStartup {
	[CalibrationController doSetup]; //restore tilt settings
	[self notifySpeedChange];
  doneSetup = YES;
  //The following will cause the text cursor to be displayed whenever
  // any change is made to the textbox...
  [textView becomeFirstResponder];
}

- (void)displayMessage:(NSString *)msg {
  CGRect b = messageLabel.frame;
  CGFloat height = [msg sizeWithFont:[messageLabel font] constrainedToSize:CGSizeMake(b.size.width, CGFLOAT_MAX)].height;
  messageLabel.text = [messageLabel hasText] ? [NSString stringWithFormat:@"%@\n%@",messageLabel.text,msg] : msg;
  messageLabel.frame = CGRectMake(b.origin.x, b.origin.y -height, b.size.width, b.size.height+height);
  [messageLabel scrollRangeToVisible:NSMakeRange([messageLabel.text length], 0)];
  messageLabel.hidden=NO;
  [messageLabel performSelector:@selector(hideMessage:) withObject:[NSNumber numberWithFloat:height] afterDelay:3.0];
}

- (void)fadeSlider {
  if ([NSThread isMainThread]) {
    speedSlider.hidden = NO;
    [NSObject cancelPreviousPerformRequestsWithTarget:speedSlider];
    [speedSlider performSelector:@selector(hide) withObject:nil afterDelay:2.0];
  } else [self performSelectorOnMainThread:@selector(fadeSlider) withObject:nil waitUntilDone:NO];
}

- (void)speedSlid:(id)sender {
	CGFloat v = ((UISlider *)sender).value;
    _dasherInterface->SetLongParameter(LP_MAX_BITRATE, (long)(v * 100.0));
	//[self notifySpeedChange];//no need, CDasherInterfaceBridge calls if SetLongParameter did anything
}

- (void)clearBtn {
	UIActionSheet *confirm = [[[UIActionSheet alloc] initWithTitle:@"Start New Document" delegate:self cancelButtonTitle:@"Keep Existing" destructiveButtonTitle:@"Discard Existing" otherButtonTitles:nil] autorelease];
	[confirm showFromToolbar:tools];
}

- (void)clearText {
  textView.text=@"";
  selectedText.location = selectedText.length = 0;
  _dasherInterface->SetBuffer(0);
}
	
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (buttonIndex == actionSheet.destructiveButtonIndex) [self clearText];
}

- (void)toggleGameMode {
  if (_dasherInterface->GetGameModule())
    _dasherInterface->LeaveGameMode();
  else
    _dasherInterface->EnterGameMode(NULL);
}

-(UIWebView *)getWebView {
  return webView;
}

- (void)settings {
  //avoid awful muddle if we change out of tap-to-start mode whilst running.... 
  _dasherInterface->GetActiveInputMethod()->pause();
	
  UITabBarController *tabs = [[[UITabBarController alloc] init] autorelease];
  tabs.title = @"Settings";
  tabs.navigationItem.leftBarButtonItem = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(settingsDone)] autorelease];
  UINavigationController *settings = [[[UINavigationController alloc] initWithRootViewController:tabs] autorelease];

  ParametersController *misc = [[[MiscSettings alloc] initForTextView:textView] autorelease];

    tabs.viewControllers = [NSArray arrayWithObjects:
							[[[InputMethodSelector alloc] init] autorelease],
						    [[[LanguagesController alloc] init] autorelease],
                [[[StringParamController alloc] initWithTitle:@"Colour" image:[UIImage imageNamed:@"palette.png"] settingParam:SP_COLOUR_ID] autorelease],
						    misc,
                [[[ActionConfigurator alloc] init] autorelease],
						    nil];
    [self presentViewController:settings animated:YES completion:nil];
}

- (void)settingsDone {
    [self dismissViewControllerAnimated:YES completion:nil];
	glView.animating=YES;
}

- (void)outputCallback:(NSString *)s {
	textView.text=[textView.text stringByReplacingCharactersInRange:selectedText withString:s];
	selectedText.location+=[s length]; 
	selectedText.length = 0;
	textView.selectedRange = selectedText;
	//This isn't quite right, it jumps up then down again once you have >3 lines...
	[textView scrollRangeToVisible:selectedText];
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
	glView.animating=NO;
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
  if (doneSetup) glView.animating=YES;//glView.animationInterval = 1.0 / 60.0;
}

-(void)applicationDidEnterBackground:(UIApplication *)application {
  [[NSUserDefaults standardUserDefaults] synchronize];
  self.dasherInterface->WriteTrainFileFull();
}

-(void)applicationWillTerminate:(UIApplication *)application {
  glView.animating=NO;
  self.dasherInterface->WriteTrainFileFull();
}

- (void)newFrameAt:(unsigned long)time ForceRedraw:(BOOL)bForce {
	self.dasherInterface->NewFrame(time, bForce);
}

- (void)notifySpeedChange {
  CGFloat speed = (CGFloat)self.dasherInterface->GetLongParameter(LP_MAX_BITRATE) / 100.0;
  speedSlider.value = speed; 
	NSString *caption = [NSString stringWithFormat:@"%.2f", speed];
    self.speedLabel.text = caption;
    [self.speedLabel sizeToFit];
}

- (void)dealloc {
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
  unsigned int tmp1 = (unsigned int)[textView.text length];
  unsigned int tmp2 = (unsigned int)max(0, (int)([textView.text length] - (int)offset));
  range.location = min(offset,tmp1);
  range.length = min(length,tmp2);
  return [textView.text substringWithRange:range];
}

- (void)setAlphabet:(const CAlphInfo *)pAlph {
  self.m_wordBoundary = pAlph->GetSpaceSymbol() ? NSStringFromStdString(pAlph->GetText(pAlph->GetSpaceSymbol())) : @" ";
  self.m_lineBoundary = (pAlph->GetParagraphSymbol())
  ? NSStringFromStdString(pAlph->GetText(pAlph->GetParagraphSymbol())) : nil;
  self.m_sentenceBoundary = (pAlph->GetDefaultContext().length()>0)
      ? NSStringFromStdString(pAlph->GetDefaultContext())
      : @".";
}

- (int)find:(CControlManager::EditDistance)amt forwards:(BOOL)bForwards {
  if (amt==CControlManager::EDIT_FILE) return bForwards ? (int)[textView.text length] : 0;
  int pos = (int)selectedText.location;
  for(;;) {
    if (bForwards) {
      if (++pos > [textView.text length]) return pos-1;
    } else {
      if (--pos < 0) return 0;
    }
    NSString *lookFor;
    switch (amt) {
      case CControlManager::EDIT_CHAR:
        //once only, never loop
        return pos;
      case CControlManager::EDIT_WORD:
        lookFor = m_wordBoundary;
        break;
      case CControlManager::EDIT_LINE:
        if (m_lineBoundary && [textView.text compare:m_lineBoundary options:0 range:NSMakeRange(pos, [m_lineBoundary length])] == NSOrderedSame)
          return pos;
        lookFor = m_lineBoundary;
        break;
    }
    if ([textView.text compare:lookFor options:0 range:NSMakeRange(pos, [lookFor length])] == NSOrderedSame)
      return pos;
  }
}

- (unsigned int)move:(CControlManager::EditDistance)amt forwards:(BOOL)bForwards {
  selectedText.location = [self find:amt forwards:bForwards];
  selectedText.length=0;
  textView.selectedRange = selectedText;
  [textView scrollRangeToVisible:selectedText];
  return selectedText.location;
}

- (unsigned int)del:(CControlManager::EditDistance)amt forwards:(BOOL)bForwards {
  int to = [self find:amt forwards:bForwards];
  if (bForwards) {
    selectedText.length = to-selectedText.location;
  } else {
    selectedText.length = selectedText.location - to;
    selectedText.location = to;
  }
  [self outputCallback:@""];
  return selectedText.location;
}

- (NSString *)allText {
  return textView.text;
}

-(void)selectEAGLContext {
  [glView makeContextCurrent];
}

-(void)updateLockText {
  NSAssert([NSThread isMainThread],@"Not on main thread?!?!");
  if (UILabel *lbl=self.screenLockLabel) {
    if (NSString *s = self.lockText) {
      [lbl setText:s];
      lbl.hidden = NO;
    } else lbl.hidden = YES;   
    [self performSelector:@selector(updateLockText) withObject:nil afterDelay:0.2];
  }
}

+ (DasherAppDelegate *)theApp {
  DasherAppDelegate *ret;
  @synchronized ([UIApplication sharedApplication]) {
    ret = s_appDelegate;
  }
  return ret;
}

#pragma mark TextViewDelegate method

-(void)textViewDidChangeSelection:(UITextView *)sender {
  DASHER_ASSERT(sender == textView);
  selectedText = textView.selectedRange;
  if (selectedText.location == NSIntegerMax) selectedText.location = textView.text.length;
  _dasherInterface->SetOffset(selectedText.location);
}

@end

/*@interface UIViewController (lockable)
- (void)aSyncMain:(NSInvocation *)action;
@end*/

@implementation UIViewController (lockable)

#define kASYNC_LOCK_VIEW_TAG -99901

- (void)doAsyncUnlock {
    NSMutableArray *rmvcs = [NSMutableArray new];
    for(UIViewController *viewController in self.childViewControllers) {
        if(viewController.view.tag == kASYNC_LOCK_VIEW_TAG) {
            [rmvcs addObject:viewController];
        }
    }
    
    [UIView animateWithDuration:0.5 animations:^{
        for(UIViewController *viewController in rmvcs) {
            viewController.view.alpha = 0;
        }
    } completion:^(BOOL finished) {
        for(UIViewController *viewController in rmvcs) {
            [viewController.view removeFromSuperview];
            [viewController removeFromParentViewController];
        }
    }];    
}

- (void)doAsyncLocked:(NSString *)msg target:(id)obj selector:(SEL)sel param:(id)param,... {
  CGSize mainSize = [UIScreen mainScreen].bounds.size;
  UIViewController *lockCon = [[[UIViewController alloc] init] autorelease];
    lockCon.view = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, mainSize.width, mainSize.height)] autorelease];
    lockCon.view.userInteractionEnabled = YES;
/*
  UIImageView *imgView = [[[UIImageView alloc] initWithFrame:CGRectMake(0.0, 0.0, mainSize.width, mainSize.height)] autorelease];
  imgView.image = [UIImage imageNamed:@"Default"];
  imgView.contentMode = UIViewContentModeBottomRight;
  [lockCon.view addSubview:imgView];
 */
  CGRect mainLabelRect = CGRectMake(40.0, (mainSize.height*5)/12, mainSize.width-80.0, mainSize.height/6);
  UILabel *lbl1 = [[[UILabel alloc] initWithFrame:mainLabelRect] autorelease];
  lbl1.backgroundColor = [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.8];
  lbl1.textColor = [UIColor whiteColor];
  lbl1.textAlignment = UITextAlignmentCenter;
  lbl1.text = msg;
  lbl1.adjustsFontSizeToFitWidth = YES;
  [lockCon.view addSubview:lbl1]; // WAS IMGVIEW
  UILabel *lbl2 = [[[UILabel alloc] initWithFrame:CGRectMake(mainLabelRect.origin.x+20.0,(mainSize.height*15)/24,mainLabelRect.size.width-40, mainSize.height/12)] autorelease];
  lbl2.backgroundColor = lbl1.backgroundColor;
  lbl2.textColor = lbl1.textColor;
  lbl2.textAlignment = UITextAlignmentCenter;
  lbl2.adjustsFontSizeToFitWidth = YES;
  lbl2.hidden = YES;
  [lockCon.view addSubview:lbl2]; // WAS IMGVIEW
  //think we just hope old value of screenLockLabel was null
  // (i.e. no nested calls to doAsyncLocked...)
  [DasherAppDelegate theApp].screenLockLabel = lbl2;
    // [self presentViewController:lockCon animated:NO completion:nil];
    [self addChildViewController:lockCon];
    lockCon.view.tag = kASYNC_LOCK_VIEW_TAG;
    [self.view addSubview:lockCon.view];
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
  [[DasherAppDelegate theApp] performSelector:@selector(updateLockText) withObject:nil afterDelay:0.2];
}

- (void)aSyncMain:(NSInvocation *)action {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  //This method is being executed on (some, unknown) background thread, i.e. not the main thread.
  // We don't _know_ it's going to do anything OpenGL-related, but to mirror the main thread:
  [[DasherAppDelegate theApp] selectEAGLContext];
  [action invoke];
  [DasherAppDelegate theApp].screenLockLabel = nil;
  [NSObject cancelPreviousPerformRequestsWithTarget:[DasherAppDelegate theApp] selector:@selector(updateLockText) object:nil];
  //passing 'nil' here, where a BOOL is expected, is a horrendous trick - nil = 0x0 is effectively reinterpret_casted... 
  // however, the 'correct' method of passing [NSNumber numberWithBool:] is erratic, resulting in either inversion, 
  // always true, or always false, on different versions of the iPhone OS/SDK...
  dispatch_async(dispatch_get_main_queue(), ^{
    // [self dismissViewControllerAnimated:YES completion:nil];
    [self doAsyncUnlock];
  });
  [pool release];
}

@end

@implementation UISlider (Hideable)

-(void)hide {
  [self setHidden:YES];
}

@end
