/*
 * Created by Doug Dickinson (dougd AT DressTheMonkey DOT plus DOT com), 20 April 2003
 */

#import "PreferencesController.h"
#import "DasherApp.h"
#import "DasherUtil.h"

#include "libdasher.h"


void parameter_bool_callback( bool_param p, bool value )
{
   BOOL boolValue = value == true ? YES : NO;
   NSString *keyName = nil;
   
   switch (p)
   {
     case BOOL_DIMENSIONS:
       keyName = DASHER_DIMENSIONS;
       break;
     case BOOL_SHOWTOOLBAR:
       keyName = SHOW_TOOLBAR;
       break;
     case BOOL_SHOWTOOLBARTEXT:
       keyName = SHOW_TOOLBAR_TEXT;
       break;
     case BOOL_SHOWTOOLBARLARGEICONS:
       keyName = SHOW_LARGE_ICONS;
       break;
     case BOOL_SHOWSPEEDSLIDER:
       keyName = SHOW_SLIDER;
       break;
     case BOOL_FIXLAYOUT:
       keyName = FIX_LAYOUT;
       break;
     case BOOL_TIMESTAMPNEWFILES:
       keyName = TIME_STAMP;
       break;
     case BOOL_COPYALLONSTOP:
       keyName = COPY_ALL_ON_STOP;
       break;
     case BOOL_DRAWMOUSE:
       keyName = DRAW_MOUSE;
       break;
     case BOOL_STARTONSPACE:
       keyName = START_SPACE;
       break;
     case BOOL_STARTONLEFT:
       keyName = START_MOUSE;
       break;
     case BOOL_KEYBOARDCONTROL:
       keyName = KEY_CONTROL;
       break;
     default:
       NSLog(@"Unhandled bool parameter: %d, value = %d", p, value);
       return;
       break;
   }

   [[NSUserDefaults standardUserDefaults] setBool:boolValue forKey:keyName];
}

void parameter_int_callback( int_param p, long int value )
{
  NSString *keyName = nil;
  
  switch (p)
    {
    case INT_LANGUAGEMODEL:
      keyName = LANGUAGE_MODEL_ID;
      break;
    case INT_VIEW:
      keyName = VIEW_ID;
      break;
    case INT_SCREENWIDTH:
      keyName = SCREEN_WIDTH;
      break;
    case INT_SCREENHEIGHT:
      keyName = SHOW_LARGE_ICONS;
      break;
    case INT_EDITFONTSIZE:
      keyName = SCREEN_HEIGHT;
      break;
    case INT_DASHERFONTSIZE:
      keyName = DASHER_FONTSIZE;
      break;
    case INT_EDITHEIGHT:
      keyName = EDIT_HEIGHT;
      break;
    case INT_ORIENTATION:
      keyName = SCREEN_ORIENTATION;
      break;
    case INT_ENCODING:
      keyName = FILE_ENCODING;
      break;
    default:
      NSLog(@"Unhandled int parameter: %d, value = %d", p, value);
      return;
      break;
    }

  [[NSUserDefaults standardUserDefaults] setInteger:value forKey:keyName];
}

void parameter_double_callback( double_param p, double value )
{
  switch (p)
    {
    case DOUBLE_MAXBITRATE:
      [[NSUserDefaults standardUserDefaults] setInteger:value * 100 forKey:MAX_BITRATE_TIMES100];
      break;
    default:
      NSLog(@"Unhandled double parameter: %d, value = %g", p, value);
      return;
      break;
    }
}

void parameter_string_callback( string_param p, const char *value )
{
  NSString *stringValue = [NSString stringWithCString:value];
  NSString *keyName = nil;

  switch (p)
    {
    case STRING_ALPHABET:
      keyName = ALPHABET_ID;
      break;
    case STRING_EDITFONT:
      keyName = EDIT_FONT;
      break;
    case STRING_DASHERFONT:
      keyName = DASHER_FONT;
      break;
      /*
    case STRING_USERDIR:
      keyName = ??;
      break;
    case STRING_SYSTEMDIR:
      keyName = ??;
      break;
      */
    default:
      NSLog(@"Unhandled string parameter: %d, value = %s", p, value);
      return;
      break;
    }

  [[NSUserDefaults standardUserDefaults] setObject:stringValue forKey:keyName];
}


bool get_bool_option_callback(const std::string& Key, bool *Value)
{
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  NSString *defaultKey = NSStringFromStdString(Key);

  if ([defaults objectForKey:defaultKey] != nil)
    {
    *Value = [defaults boolForKey:defaultKey] ? true : false;
    return true;
    }

  return false;
}

void set_bool_option_callback(const std::string& Key, bool Value)
{
  [[NSUserDefaults standardUserDefaults] setBool:(Value == true ? YES : NO) forKey:NSStringFromStdString(Key)];
}

bool get_string_option_callback(const std::string& Key, std::string *Value)
{
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  NSString *defaultKey = NSStringFromStdString(Key);

  if ([defaults objectForKey:defaultKey] != nil)
    {
    *Value = std::string([[defaults stringForKey:defaultKey] cString]);
    return true;
    }

  return false;
}

void set_string_option_callback(const std::string& Key, const std::string& Value)
{
  [[NSUserDefaults standardUserDefaults] setObject:NSStringFromStdString(Value) forKey:NSStringFromStdString(Key)];
}


bool get_long_option_callback(const std::string& Key, long *Value)
{
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  NSString *defaultKey = NSStringFromStdString(Key);

  if ([defaults objectForKey:defaultKey] != nil)
    {
    *Value = [defaults integerForKey:defaultKey];
    return true;
    }

  return false;
}

void set_long_option_callback(const std::string& Key, long Value)
{
  [[NSUserDefaults standardUserDefaults] setInteger:Value forKey:NSStringFromStdString(Key)];
}


static void registerCallbacks()
{
  dasher_set_get_bool_option_callback( get_bool_option_callback );
  dasher_set_get_long_option_callback( get_long_option_callback );
  dasher_set_get_string_option_callback( get_string_option_callback );
  
  dasher_set_set_bool_option_callback( set_bool_option_callback );
  dasher_set_set_long_option_callback( set_long_option_callback );
  dasher_set_set_string_option_callback( set_string_option_callback );
  
  dasher_set_string_callback(parameter_string_callback);
  dasher_set_double_callback(parameter_double_callback);
  dasher_set_int_callback(parameter_int_callback);
  dasher_set_bool_callback(parameter_bool_callback);
}


static PreferencesController *preferencesController = nil;

@implementation PreferencesController

+ (void)initialize
{
  // set up default defaults..this was copied from Apple docs
  
  // TODO fill in all keys and values

  NSDictionary *appDefaults = [NSDictionary dictionaryWithObjectsAndKeys:
    [[NSFont userFixedPitchFontOfSize:10.0] fontName], DASHER_FONT,
    [[NSFont userFontOfSize:12.0] fontName], EDIT_FONT,
    [NSNumber numberWithFloat:12.0], EDIT_FONT_SIZE,
    nil
    ];

  [[NSUserDefaults standardUserDefaults] registerDefaults:appDefaults];
}

+ (id)preferencesController
{
  if (preferencesController == nil)
    {
    preferencesController = [[self alloc] init];  // retain to use as singleton
    }

  return preferencesController;
}


- (id)init
{
  if (self = [super init])
    {
    registerCallbacks();
    }

  return self;
}

- (void)populateAlphabetList
{
  const int alphabetlist_size = 128;
  const char *asciiAlphabetList[ alphabetlist_size ];

  int alphabet_count;

  alphabet_count = dasher_get_alphabets( asciiAlphabetList, alphabetlist_size );

  NSMutableArray *alphabetList = [NSMutableArray arrayWithCapacity:alphabet_count];

  for (int i = 0; i < alphabet_count; i++)
    {
    [alphabetList addObject:[NSString stringWithCString:asciiAlphabetList[i]]];
    }

  //TODO do I need to free asciiAlphabetList?
  [self setAlphabetList:alphabetList];
}


- (void)incorporateDefaults
{
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  
  [drawMouseUI setIntValue:[defaults boolForKey:DRAW_MOUSE]];
  [dasherDimensionsUI setIntValue:[defaults boolForKey:DASHER_DIMENSIONS]];
  [startMouseUI setIntValue:[defaults boolForKey:START_MOUSE]];
  [startSpaceUI setIntValue:[defaults boolForKey:START_SPACE]];
  [screenOrientationUI selectCellWithTag:[defaults integerForKey:SCREEN_ORIENTATION]];
  [dasherFontSizeUI selectCellWithTag:[defaults integerForKey:DASHER_FONTSIZE]];
  [keyControlUI setIntValue:[defaults boolForKey:KEY_CONTROL]];
  [copyAllOnStopUI setIntValue:[defaults boolForKey:COPY_ALL_ON_STOP]];

  [alphabetIDUI selectRow:[[self alphabetList] indexOfObject:[defaults stringForKey:ALPHABET_ID]] inColumn:0];

  [dasherFontUI setStringValue:[defaults stringForKey:DASHER_FONT]];
  [editFontUI setStringValue:[NSString stringWithFormat:@"%@ %d", [defaults stringForKey:EDIT_FONT], [defaults integerForKey:EDIT_FONT_SIZE]]];

  [maxBitRateUI setDoubleValue:(double)([defaults integerForKey:MAX_BITRATE_TIMES100] / 100.0)];
}

- (void)makeKeyAndOrderFront:(id)sender
{
  if (panel == nil)
    {
    [NSBundle loadNibNamed:@"Preferences" owner:self];
    [self populateAlphabetList];
    }

  [self incorporateDefaults];
  [panel makeKeyAndOrderFront:self];
}

- (IBAction)drawMouseAction:(id)sender
{
  dasher_set_parameter_bool(BOOL_DRAWMOUSE, [sender intValue] ? true : false);
}

- (IBAction)dasherDimensionsAction:(id)sender
{
  dasher_set_parameter_bool(BOOL_DIMENSIONS, [sender intValue] ? true : false);
}

- (IBAction)startMouseAction:(id)sender
{
  dasher_set_parameter_bool(BOOL_STARTONLEFT, [sender intValue] ? true : false);
}

- (IBAction)startSpaceAction:(id)sender
{
  dasher_set_parameter_bool(BOOL_STARTONSPACE, [sender intValue] ? true : false);
}

- (IBAction)screenOrientationAction:(id)sender
{
  // N.B. the radio buttons MUST return tags corresponding to the values of Opts::ScreenOrientations
  int i = [[sender selectedCell] tag];
  dasher_set_orientation(Dasher::Opts::ScreenOrientations(i));
  [[NSUserDefaults standardUserDefaults] setInteger:i forKey:SCREEN_ORIENTATION];
  dasher_redraw();
}

- (IBAction)maxBitRateAction:(id)sender
{
  double d = [sender doubleValue];
  dasher_set_parameter_double(DOUBLE_MAXBITRATE, d);
  [[NSUserDefaults standardUserDefaults] setInteger:(int)(d * 100) forKey:MAX_BITRATE_TIMES100];
}

- (IBAction)alphabetIDAction:(id)sender
{
  NSString *s = [[sender selectedCell] stringValue];
  dasher_set_parameter_string(STRING_ALPHABET, [s cString]);
  [[NSUserDefaults standardUserDefaults] setObject:s forKey:ALPHABET_ID];
}

- (IBAction)keyControlAction:(id)sender
{
  dasher_set_parameter_bool(BOOL_KEYBOARDCONTROL, [sender intValue] ? true : false);
}

- (IBAction)copyAllOnStopAction:(id)sender
{
  dasher_set_parameter_bool(BOOL_COPYALLONSTOP, [sender intValue] ? true : false);
}


- (IBAction)dasherFontSizeAction:(id)sender
{
  // N.B. the radio buttons MUST return tags corresponding to the values of Opts::FontSize
  dasher_set_parameter_int(INT_DASHERFONTSIZE, [[sender selectedCell] tag]);
  dasher_redraw();
}

- (IBAction)dasherFontAction:(id)sender
{
  // this only displays the font panel,  changeFont: is where the font is actually changed
  fontToAlter = ALTER_FONT_DASHER;
  [[NSFontManager sharedFontManager] orderFrontFontPanel:sender];
}

- (IBAction)editFontAction:(id)sender
{
  // this only displays the font panel,  changeFont: is where the font is actually changed
  fontToAlter = ALTER_FONT_EDIT;
  [[NSFontManager sharedFontManager] orderFrontFontPanel:sender];
}


- (IBAction)changeFont:(id)sender
{
  switch (fontToAlter) {
    case ALTER_FONT_DASHER:
      [self changeDasherFont:sender];
      break;
    case ALTER_FONT_EDIT:
      [self changeEditFont:sender];
      break;
  }
}

- (void)changeDasherFont:(id)sender
{
  // size is not important; just the font name
  NSFont *oldFont = [NSFont fontWithName:[[NSUserDefaults standardUserDefaults] objectForKey:DASHER_FONT] size:10.0];
  NSFont *newFont = [sender convertFont:oldFont];
  NSString *fontName = [newFont fontName];
  dasher_set_parameter_string(STRING_DASHERFONT, [fontName cString]);
  [[NSUserDefaults standardUserDefaults] setObject:fontName forKey:DASHER_FONT];
  [dasherFontUI setStringValue:fontName];
  dasher_redraw();
}

- (void)changeEditFont:(id)sender
{
  // this should not change the font in the edit box
  // it is the font to be used when a NEW document is created
  NSFont *oldFont = [NSFont fontWithName:[[NSUserDefaults standardUserDefaults] stringForKey:EDIT_FONT] size:(float)[[NSUserDefaults standardUserDefaults] integerForKey:EDIT_FONT_SIZE]];
  NSFont *newFont = [sender convertFont:oldFont];
  NSString *fontName = [newFont fontName];
  int pointSize = (int)[newFont pointSize];
  
  dasher_set_parameter_string(STRING_EDITFONT, [fontName cString]);
  dasher_set_parameter_int(INT_EDITFONTSIZE, pointSize);

  [[NSUserDefaults standardUserDefaults] setObject:fontName forKey:EDIT_FONT];
  [[NSUserDefaults standardUserDefaults] setInteger:pointSize forKey:EDIT_FONT_SIZE];
  
  [editFontUI setStringValue:[NSString stringWithFormat:@"%@ %d", fontName, pointSize]];
}


- (int)browser:(NSBrowser *)sender numberOfRowsInColumn:(int)column
{
  return [[self alphabetList] count];
}

- (void)browser:(NSBrowser *)sender willDisplayCell:(id)cell atRow:(int)row column:(int)column
{
  [cell setStringValue:[[self alphabetList] objectAtIndex:row]];
  [cell setLeaf:YES];
}


- (NSArray *)alphabetList {
  return _alphabetList;
}

- (void)setAlphabetList:(NSArray *)newAlphabetList {
  if (_alphabetList != newAlphabetList) {
    NSArray *oldValue = _alphabetList;
    _alphabetList = [newAlphabetList retain];
    [oldValue release];
  }
}

- (void)dealloc
{
  [_alphabetList release];
  [super dealloc];
}
@end
