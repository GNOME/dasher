//
//  DasherEdit.mm
//  Dasher
//
//  Created by Doug Dickinson on Fri May 30 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import "DasherEdit.h"
#import "DasherApp.h"
#import "DasherUtil.h"
#import "PreferencesController.h"
#include "libdasher.h"
#import "TextDocument.h"

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
  str = StdStringFromNSString(s);
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

static DasherEdit *dasherEdit = nil;


@implementation DasherEdit

+ dasherEdit
{
  if (dasherEdit == nil)
    {
    dasherEdit = [[self alloc] init];  // retain for use as singleton

    }

  return dasherEdit;
}

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
    XXXdasherEdit = self;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(textViewDidChangeSelection:) name:NSTextViewDidChangeSelectionNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(textViewDidChangeSelection:) name:NSWindowDidBecomeMainNotification object:nil];
  }

  return self;
}

- (void)awakeFromNib {

  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

  dasherIsModifyingText = YES;

  
  // TODO this should actually happen on creating a new document

  dasherIsModifyingText = NO;
}

- (NSTextView *)currentTextUI
{
  return [[[NSDocumentController sharedDocumentController] currentDocument] textUI];
}

- (void)outputCallback:(NSString *)aString
{
  dasherIsModifyingText = YES;
  [[self currentTextUI] insertText:aString];
  dasherIsModifyingText = NO;
}

- (void)flushCallback:(NSString *)aString
{
  dasherIsModifyingText = YES;

  if (aString != nil && ![aString isEqualToString:@""])
    {
    [[self currentTextUI] insertText:aString];
    flushCount += [aString length];
    }

  dasherIsModifyingText = NO;
}

- (void)unflushCallback
{
  dasherIsModifyingText = YES;

  if (flushCount > 0) {
    NSRange r = [[self currentTextUI] selectedRange];
    if (r.location <= flushCount) {
      r.location = 0;
    } else {
      r.location -= flushCount;
    }

    r.length += flushCount;

    [[self currentTextUI] replaceCharactersInRange:r withString:@""];
  }

  flushCount = 0;

  dasherIsModifyingText = NO;
}

- (void)deleteCallback
{
  NSRange r = [[self currentTextUI] selectedRange];
  if (r.location <= 0) {
    return;
  }

  dasherIsModifyingText = YES;
  [[self currentTextUI] replaceCharactersInRange:NSMakeRange(r.location - 1, 1) withString:@""];
  dasherIsModifyingText = NO;
}


- (NSString *)getNewContextCallback:(int)maxChars
{
  NSString *s = [[self currentTextUI] string];
  NSRange r = [[self currentTextUI] selectedRange];

  unsigned int q = MIN(r.location, maxChars);

  r = NSMakeRange(r.location - q, r.length + q);

  dasherIsModifyingText = YES;
  dasherIsModifyingText = NO;

  flushCount = 0;

  return r.length <= 0 ? nil : [s substringWithRange:r];
}

- clipboardCallbackWithAction:(clipboard_action)act
{
  NSRange r;
  
  dasherIsModifyingText = YES;

  switch( act )
    {
    case CLIPBOARD_CUT:
      [[self currentTextUI] cut:self];
      break;
    case CLIPBOARD_COPY:
      [[self currentTextUI] copy:self];
      break;
    case CLIPBOARD_PASTE:
      [[self currentTextUI] paste:self];
      break;
    case CLIPBOARD_COPYALL:
      r = [[self currentTextUI] selectedRange];
      [[self currentTextUI] selectAll:self];
      [[self currentTextUI] copy:self];
      [[self currentTextUI] setSelectedRange:r];
      break;
    case CLIPBOARD_SELECTALL:
      [[self currentTextUI] selectAll:self];
      break;
    case CLIPBOARD_CLEAR:
      [[self currentTextUI] setString:nil];
      break;
    }

  dasherIsModifyingText = NO;
}


@end
