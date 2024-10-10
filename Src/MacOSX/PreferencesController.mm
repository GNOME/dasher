//
//  PreferencesController.mm
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "PreferencesController.h"
#import "ModuleSettingsController.h"
#import "DasherApp.h"
#import "DasherUtil.h"
#import "config.h"

static PreferencesController *preferencesController = nil;

@interface LineWidthTransformer : NSValueTransformer
{
}
@end

//The key which "Start With Mouse Position:" checkbox writes its value to, as specified in InterfaceBuilder
static NSString * const AnyStartHandlerEnabled = @"values.StartWithMousePosition";
//The key which the drop-down box (circle/two-box) writes its selected-index to, as specified in InterfaceBuilder
static NSString * const StartHandlerIndex = @"values.StartHandlerIdx";
//The names (as written out to persistent settings file) of the bool parameters corresponding to each
// of the entries in that drop-down box, in the order they appear in the drop-down.
static NSString * const StartHandlerParamNames[2] = {
  @"CircleStart", @"StartOnMousePosition"
};
#define numStartHandlerParams (sizeof(StartHandlerParamNames)/sizeof(StartHandlerParamNames[0]))


@interface PreferencesController ()

@property BOOL panelLoaded;

@end

@implementation PreferencesController

@synthesize panel;
@synthesize languageTableView;
@synthesize inputFilterTableView;
@synthesize colorTableView;
@synthesize dasherApp;
@synthesize panelLoaded;

- (id)defaultsValueForKey:(NSString *)aKey {
  return [[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:[NSString stringWithFormat:@"values.%@", aKey]];
}

- (void)setDefaultsValue:(id)aValue forKey:(NSString *)aKey {
  [[NSUserDefaultsController sharedUserDefaultsController] setValue:aValue forKeyPath:[NSString stringWithFormat:@"values.%@", aKey]];
}

+ (id)preferencesController {
  if (preferencesController == nil)
    {
    preferencesController = [[self alloc] init];  // retain to use as singleton
    [NSValueTransformer setValueTransformer:[[LineWidthTransformer alloc] init] forName:@"LineWidthTransformer"];
    }
  
  return preferencesController;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
  
  if ([keyPath isEqualTo:AnyStartHandlerEnabled] || [keyPath isEqualTo:StartHandlerIndex]) {
    bool bOn[numStartHandlerParams];
    for (unsigned int i=0; i<numStartHandlerParams; i++) bOn[i]=NO;
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    if ([[udc valueForKeyPath:AnyStartHandlerEnabled] boolValue]) {
      int which = [[udc valueForKeyPath:StartHandlerIndex] intValue];
      if (which>=0 && which<numStartHandlerParams)
        bOn[ which ]=YES;
    }
    for (unsigned int i=0; i<numStartHandlerParams; i++)
      [dasherApp setParameterValue:[NSNumber numberWithBool:bOn[i]] forKey:StartHandlerParamNames[i]];
  } else {
    NSString *key = [keyPath substringFromIndex:[@"values." length]];
    NSString *value = [self defaultsValueForKey:key];
    [dasherApp setParameterValue:value forKey:key];
  }
}

- (void)observeDefaults {
  NSEnumerator *e = [[dasherApp parameterDictionary] keyEnumerator];
  NSString *key = nil;
  NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
  
  while (key = [e nextObject]) {
    if (![key isEqualToString:@"FrameRate"])
      [udc addObserver:self forKeyPath:[NSString stringWithFormat:@"values.%@", key] options:0 context:NULL];
  }
  [udc addObserver:self forKeyPath:AnyStartHandlerEnabled options:0 context:NULL];
  [udc addObserver:self forKeyPath:StartHandlerIndex options:0 context:NULL];
}

- (void)awakeFromNib {
  [self observeDefaults];
}

- (NSDictionary *)parameterDictionary {
  return [dasherApp parameterDictionary];
}

- (id)init {
  if (self = [super init]) {
    dasherApp = (DasherApp *)[[NSApplication sharedApplication] delegate];
  }
  
  return self;
}


- (void)makeKeyAndOrderFront:(id)sender {
  if (panel == nil) {
    [[NSBundle mainBundle] loadNibNamed:@"Preferences"
                                         owner:self
                               topLevelObjects:nil];
    panelLoaded = YES;
  }

  [panel makeKeyAndOrderFront: self];
      
  // Restore saved value.
  NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
  NSString *alphabetID = [userDefaults objectForKey: DASHER_ALPHABET_ID];
  
  if (alphabetID) {
    [self setDefaultsValue: alphabetID forKey:@"AlphabetID"];
    NSUInteger index = [[self permittedValuesForAlphabetID] indexOfObject: alphabetID];
    NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex: index];
    [self.languageTableView selectRowIndexes:indexSet byExtendingSelection:NO];
    [self.languageTableView scrollRowToVisible: index];
  }
  
  NSString *inputFilter = [userDefaults objectForKey: DASHER_INPUTFILTER];
  if (inputFilter) {
    [self setDefaultsValue: inputFilter forKey:@"InputFilter"];
    NSUInteger index = [[self permittedValuesForInputFilter] indexOfObject: inputFilter];
    NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex: index];
    [self.inputFilterTableView selectRowIndexes:indexSet byExtendingSelection:NO];
    [self.inputFilterTableView scrollRowToVisible: index];
  }
  
  NSString *colourId = [userDefaults objectForKey: DASHER_COLOURID];
  if (colourId) {
    [self setDefaultsValue: colourId forKey:@"ColourID"];
    NSUInteger index = [[self permittedValuesForColourID] indexOfObject: colourId];
    NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex: index];
    [self.colorTableView selectRowIndexes:indexSet byExtendingSelection:NO];
    [self.colorTableView scrollRowToVisible: index];
  }
}

-(void)inputFilterSettings:(NSString *)selectedObject {
  SModuleSettings *pSettings; int iCount;
  COSXDasherControl *intf = [dasherApp aquaDasherControl];
  CDasherModule *inputFilter(intf->GetModuleByName(StdStringFromNSString(selectedObject)));
  if (inputFilter && inputFilter->GetSettings(&pSettings, &iCount)) {
    [[[[ModuleSettingsController alloc] initWithTitle:NSStringFromStdString(inputFilter->GetName()) Interface:intf Settings:pSettings Count:iCount] autorelease] showModal];
  }
  
}

- (NSArray *)permittedValuesForAlphabetID {
  return [dasherApp permittedValuesForParameter:SP_ALPHABET_ID];
}

- (NSIndexSet *)selectionIndexesForAlphabetID {
  //If the stored preference names an alphabet which isn't available, return the empty set of indices
  NSUInteger idx = [[self permittedValuesForAlphabetID] indexOfObject:[self defaultsValueForKey:@"AlphabetID"]];
  return (idx==NSNotFound) ? [NSIndexSet indexSet] : [NSIndexSet indexSetWithIndex:idx];
}

- (void)setSelectionIndexesForAlphabetID:(NSIndexSet *)anIndexSet {
  //Every time we show the preferences dialog, this gets called twice:
  // First there is an initial call to selectionIndexesForAlphabetID;
  // then this gets called with an empty indexset;
  // then this gets called again with an indexset containing a single index, 0
  // then selectionIndexesForAlphabetID is called again.
  // WHY?!?!?!
  // So: ignore the call with an empty set; but what to do about the '0'?
    if (panelLoaded && [anIndexSet count]==1) {
        NSString *alphabetID = [[[self permittedValuesForAlphabetID] objectsAtIndexes:anIndexSet] lastObject];
        [self setDefaultsValue: alphabetID forKey:@"AlphabetID"];
        
        // Save selected alphabetID on local.
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        [userDefaults setObject: alphabetID forKey: DASHER_ALPHABET_ID];
    }
}

- (NSArray *)permittedValuesForColourID {
  return [dasherApp permittedValuesForParameter:SP_COLOUR_ID];
}

- (NSIndexSet *)selectionIndexesForColourID {
  //If the stored preference names a colour scheme which isn't available, return the empty set of indices
  NSUInteger idx = [[self permittedValuesForColourID] indexOfObject:[self defaultsValueForKey:@"ColourID"]];
  return (idx==NSNotFound) ? [NSIndexSet indexSet] : [NSIndexSet indexSetWithIndex:idx];
}

- (void)setSelectionIndexesForColourID:(NSIndexSet *)anIndexSet {
  //Every time we show the preferences dialog, this gets called twice:
  // First there is an initial call to selectionIndexesForColourID;
  // then this gets called with an empty indexset;
  // then this gets called again with an indexset containing a single index, 0
  // then selectionIndexesForColourID is called again.
  // WHY?!?!?!
  // So: ignore the call with an empty set; but what to do about the '0'?  if ([anIndexSet count]==1)
    if (panelLoaded && [anIndexSet count]==1) {
        NSString *colourId = [[[self permittedValuesForColourID] objectsAtIndexes:anIndexSet] lastObject];
        [self setDefaultsValue: colourId forKey:@"ColourID"];
        
        // Save selected colourId on local.
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        [userDefaults setObject: colourId forKey: DASHER_COLOURID];
    }
}


- (NSArray *)permittedValuesForInputFilter {
  return [dasherApp permittedValuesForParameter:SP_INPUT_FILTER];
}

- (NSIndexSet *)selectionIndexesForInputFilter {
  //If the stored preference names an input filter which isn't available, return the empty set of indices
  NSUInteger idx = [[self permittedValuesForInputFilter] indexOfObject:[self defaultsValueForKey:@"InputFilter"]];
  return (idx==NSNotFound) ? [NSIndexSet indexSet] : [NSIndexSet indexSetWithIndex:idx];
}

- (void)setSelectionIndexesForInputFilter:(NSIndexSet *)anIndexSet {
  //Every time we show the preferences dialog, this gets called twice:
  // First there is an initial call to selectionIndexesForInputFilter
  // then this gets called with an empty indexset;
  // then this gets called again with an indexset containing a single index, 0
  // then selectionIndexesForInputFilter is called again.
  // WHY?!?!?!
  // So: ignore the call with an empty set; but what to do about the '0'?
    if (panelLoaded && [anIndexSet count]==1) {
        NSString *inputFilter = [[[self permittedValuesForInputFilter] objectsAtIndexes:anIndexSet] lastObject];
        [self setDefaultsValue: inputFilter forKey:@"InputFilter"];
        
        // Save selected input filter on local.
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        [userDefaults setObject: inputFilter forKey: DASHER_INPUTFILTER];
    }
}


- (NSArray *)permittedValuesForDasherFont {
  return [[[NSFontManager sharedFontManager] availableFontFamilies] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
}


@end



@implementation LineWidthTransformer
+ (Class)transformedValueClass { 
  return [NSNumber class]; 
}

+ (BOOL)allowsReverseTransformation {
  return YES;
}

- (id)transformedValue:(id)value {
  // convert integer  to boolean
  return [NSNumber numberWithBool:(value != nil && [value intValue] == 3)];
}

- (id)reverseTransformedValue:(id)value {
  // convert boolean to integer
  return [NSNumber numberWithInt:(value == nil || [value boolValue] == false) ? 1 : 3];
}

@end
