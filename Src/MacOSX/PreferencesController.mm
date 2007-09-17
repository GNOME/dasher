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

/*
 IMPORTANT: this code (and associated nib(s)) should not access the user defaults.  It stores and retrieves preferences via the core.  COSXSettingsStore accesses user defaults and core talks directly to it.  The rest of the Cocoa code should get values of parameters via the core and, if necessary, ask for user default notification of parameter changes.
 */

// any keys for which there are no ivars or methods will be caught as undefined keys,
// and we can just shove them straight into the core
- (id)valueForUndefinedKey:(NSString *)aKey {
  return [dasherApp getParameterValueForKey:aKey];
}

- (void)setValue:(id)aValue forUndefinedKey:(NSString *)aKey {
  [dasherApp setParameterValue:aValue forKey:aKey];
}

+ (id)preferencesController
{
  if (preferencesController == nil)
    {
    preferencesController = [[self alloc] init];  // retain to use as singleton
    [NSValueTransformer setValueTransformer:[[LineWidthTransformer alloc] init] forName:@"LineWidthTransformer"];
    }
  
  return preferencesController;
}

- (NSDictionary *)parameterDictionary {
  return [dasherApp parameterDictionary];
}

- (id)init
{
  if (self = [super init])
    {
    dasherApp = [[NSApplication sharedApplication] delegate];
    }
  
  return self;
}


- (void)makeKeyAndOrderFront:(id)sender
{
  if (panel == nil)
    {
    [NSBundle loadNibNamed:@"Preferences" owner:self];
    }
  
  [panel makeKeyAndOrderFront:self];
}

- (NSArray *)permittedValuesForAlphabetID {
  return [dasherApp permittedValuesForParameter:SP_ALPHABET_ID];
}

- (NSIndexSet *)selectionIndexesForAlphabetID {
  return [NSIndexSet indexSetWithIndex:[[self permittedValuesForAlphabetID] indexOfObject:[self valueForKey:@"AlphabetID"]]];
}

- (void)setSelectionIndexesForAlphabetID:(NSIndexSet *)anIndexSet {
  [self setValue:[[[self permittedValuesForAlphabetID] objectsAtIndexes:anIndexSet] lastObject] forKey:@"AlphabetID"];
}


- (NSArray *)permittedValuesForColourID {
  return [dasherApp permittedValuesForParameter:SP_COLOUR_ID];
}

- (NSIndexSet *)selectionIndexesForColourID {
  return [NSIndexSet indexSetWithIndex:[[self permittedValuesForColourID] indexOfObject:[self valueForKey:@"ColourID"]]];
}

- (void)setSelectionIndexesForColourID:(NSIndexSet *)anIndexSet {
  [self setValue:[[[self permittedValuesForColourID] objectsAtIndexes:anIndexSet] lastObject] forKey:@"ColourID"];
}


- (NSArray *)permittedValuesForInputFilter {
  return [dasherApp permittedValuesForParameter:SP_INPUT_FILTER];
}

- (NSIndexSet *)selectionIndexesForInputFilter {
  return [NSIndexSet indexSetWithIndex:[[self permittedValuesForInputFilter] indexOfObject:[self valueForKey:@"InputFilter"]]];
}

- (void)setSelectionIndexesForInputFilter:(NSIndexSet *)anIndexSet {
  [self setValue:[[[self permittedValuesForInputFilter] objectsAtIndexes:anIndexSet] lastObject] forKey:@"InputFilter"];
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
