//
//  DasherEdit.m
//  Dasher
//
//  Created by Doug Dickinson on Fri May 30 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import "DasherEdit.h"
#import "DasherApp.h"
#import "PreferencesController.h"
#include "libdasher.h"

// TODO this is a no-no; see similar in DasherApp
DasherEdit *XXXdasherEdit;


void edit_output_callback(symbol Symbol)
{
  [XXXdasherEdit outputCallback:NSStringFromStdString(dasher_get_edit_text(Symbol))];
}

void edit_flush_callback(symbol Symbol)
{
  [XXXdasherEdit flushCallback:NSStringFromStdString(dasher_get_edit_text(Symbol))];
}

void edit_unflush_callback()
{
  [XXXdasherEdit unflushCallback];
}

void edit_delete_callback()
{
  [XXXdasherEdit deleteCallback];
}

void get_new_context_callback( std::string &str, int max )
{
  NSString *s = [XXXdasherEdit getNewContextCallback:max];
  str = (s == nil || [s length] == 0) ? new std::string() : new std::string([s UTF8String]);
}

void clipboard_callback( clipboard_action act )
{
  [XXXdasherEdit clipboardCallbackWithAction:(clipboard_action)act];
}

static void registerCallbacks()
{

  dasher_set_edit_output_callback( edit_output_callback );
  dasher_set_edit_flush_callback( edit_flush_callback );
  dasher_set_edit_unflush_callback( edit_unflush_callback );
  dasher_set_edit_delete_callback(edit_delete_callback);
  dasher_set_get_new_context_callback( get_new_context_callback );

  dasher_set_clipboard_callback( clipboard_callback );

}




@implementation DasherEdit

- (void)textViewDidChangeSelection:(NSNotification *)aNotification
{
  // only start and redraw if it was the user who manipulated the text
  if (dasherIsModifyingText == NO) {
    dasher_start();
    dasher_redraw();
  }
}

- init
{
  if (self = [super init]) {
    registerCallbacks();
  }

  return self;
}

- (void)awakeFromNib {

  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

  dasherIsModifyingText = YES;

  XXXdasherEdit = self;

  // TODO this should actually happen on creating a new document
  [dasherTextUI setFont:[NSFont fontWithName:[defaults stringForKey:EDIT_FONT] size:(float)[defaults integerForKey:EDIT_FONT_SIZE]]];

  dasherIsModifyingText = NO;
}

- (void)outputCallback:(NSString *)aString
{
  dasherIsModifyingText = YES;
  [dasherTextUI insertText:aString];
  dasherIsModifyingText = NO;
}

- (void)flushCallback:(NSString *)aString
{
  dasherIsModifyingText = YES;

  if (aString != nil && ![aString isEqualToString:@""])
    {
    [dasherTextUI insertText:aString];
    flushCount += [aString length];
    }

  dasherIsModifyingText = NO;
}

- (void)unflushCallback
{
  dasherIsModifyingText = YES;

  if (flushCount > 0) {
    NSRange r = [dasherTextUI selectedRange];
    
    [dasherTextUI replaceCharactersInRange:NSMakeRange(r.location - flushCount, r.length + flushCount) withString:@""];
  }

  flushCount = 0;
  
  dasherIsModifyingText = NO;
}

- (void)deleteCallback
{
  NSRange r = [dasherTextUI selectedRange];
  if (r.location <= 0) {
    return;
  }
  
  dasherIsModifyingText = YES;
  [dasherTextUI replaceCharactersInRange:NSMakeRange(r.location - 1, 1) withString:@""];
  dasherIsModifyingText = NO;
}


- (NSString *)getNewContextCallback:(int)maxChars
{
  NSString *s = [dasherTextUI string];
  NSRange r = [dasherTextUI selectedRange];

  unsigned int q = MIN(r.location, maxChars);

  r = NSMakeRange(r.location - q, r.length + q);
  
  dasherIsModifyingText = YES;
  dasherIsModifyingText = NO;

  flushCount = 0;
  
  return r.length <= 0 ? nil : [s substringWithRange:r];
}

- clipboardCallbackWithAction:(clipboard_action)act
{
  dasherIsModifyingText = YES;

  switch( act )
    {
    case CLIPBOARD_CUT:
      [dasherTextUI cut:self];
      break;
    case CLIPBOARD_COPY:
      [dasherTextUI copy:self];
      break;
    case CLIPBOARD_PASTE:
      [dasherTextUI paste:self];
      break;
    case CLIPBOARD_COPYALL:
      [dasherTextUI selectAll:self];
      [dasherTextUI copy:self];
      break;
    case CLIPBOARD_SELECTALL:
      [dasherTextUI selectAll:self];
      break;
    case CLIPBOARD_CLEAR:
      [dasherTextUI setString:nil];
      break;
    }

  dasherIsModifyingText = NO;
}


@end
