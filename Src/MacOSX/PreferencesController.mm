//
//  PreferencesController.mm
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//

#import "PreferencesController.h"
#import "DasherApp.h"
#import "DasherUtil.h"

static PreferencesController *preferencesController = nil;

@interface LineWidthTransformer : NSValueTransformer
{
}
@end

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
  NSString *key = [keyPath substringFromIndex:[@"values." length]];
  NSString *value = [self defaultsValueForKey:key];
  
  [dasherApp setParameterValue:value forKey:key];
}

- (void)observeDefaults {
  NSEnumerator *e = [[dasherApp parameterDictionary] keyEnumerator];
  NSString *key = nil;
  NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
  
  while (key = [e nextObject]) {
    [udc addObserver:self forKeyPath:[NSString stringWithFormat:@"values.%@", key] options:0 context:NULL];
  }
}

- (void)awakeFromNib {
  [self observeDefaults];
}

- (NSDictionary *)parameterDictionary {
  return [dasherApp parameterDictionary];
}

- (id)init {
  if (self = [super init]) {
    dasherApp = [[NSApplication sharedApplication] delegate];
  }
  
  return self;
}


- (void)makeKeyAndOrderFront:(id)sender {
  if (panel == nil) {
    [NSBundle loadNibNamed:@"Preferences" owner:self];
  }
  
  [panel makeKeyAndOrderFront:self];
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
