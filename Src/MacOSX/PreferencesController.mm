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

static PreferencesController *preferencesController = nil;

@interface LineWidthTransformer : NSValueTransformer
{
}
@end

//The key which "Start With Mouse Position:" checkbox writes its value to, as specified in InterfaceBuilder
static const NSString *AnyStartHandlerEnabled = @"values.StartWithMousePosition";
//The key which the drop-down box (circle/two-box) writes its selected-index to, as specified in InterfaceBuilder
static const NSString *StartHandlerIndex = @"values.StartHandlerIdx";
//The names (as written out to persistent settings file) of the bool parameters corresponding to each
// of the entries in that drop-down box, in the order they appear in the drop-down.
static const NSString *StartHandlerParamNames[2] = {
  @"CircleStart", @"StartOnMousePosition"
};
#define numStartHandlerParams (sizeof(StartHandlerParamNames)/sizeof(StartHandlerParamNames[0]))


@implementation PreferencesController

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
    [NSBundle loadNibNamed:@"Preferences" owner:self];
  }
  
  [panel makeKeyAndOrderFront:self];
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
  return [NSIndexSet indexSetWithIndex:[[self permittedValuesForAlphabetID] indexOfObject:[self defaultsValueForKey:@"AlphabetID"]]];
}

- (void)setSelectionIndexesForAlphabetID:(NSIndexSet *)anIndexSet {
  [self setDefaultsValue:[[[self permittedValuesForAlphabetID] objectsAtIndexes:anIndexSet] lastObject] forKey:@"AlphabetID"];
}


- (NSArray *)permittedValuesForColourID {
  return [dasherApp permittedValuesForParameter:SP_COLOUR_ID];
}

- (NSIndexSet *)selectionIndexesForColourID {
  return [NSIndexSet indexSetWithIndex:[[self permittedValuesForColourID] indexOfObject:[self defaultsValueForKey:@"ColourID"]]];
}

- (void)setSelectionIndexesForColourID:(NSIndexSet *)anIndexSet {
  [self setDefaultsValue:[[[self permittedValuesForColourID] objectsAtIndexes:anIndexSet] lastObject] forKey:@"ColourID"];
}


- (NSArray *)permittedValuesForInputFilter {
  return [dasherApp permittedValuesForParameter:SP_INPUT_FILTER];
}

- (NSIndexSet *)selectionIndexesForInputFilter {
  return [NSIndexSet indexSetWithIndex:[[self permittedValuesForInputFilter] indexOfObject:[self defaultsValueForKey:@"InputFilter"]]];
}

- (void)setSelectionIndexesForInputFilter:(NSIndexSet *)anIndexSet {
  [self setDefaultsValue:[[[self permittedValuesForInputFilter] objectsAtIndexes:anIndexSet] lastObject] forKey:@"InputFilter"];
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
