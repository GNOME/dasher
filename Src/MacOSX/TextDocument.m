//
//  TextDocument.m
//
//  Created by Doug Dickinson on Sun Jun 01 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import "TextDocument.h"
#import "PreferencesController.h"

@implementation TextDocument

- (id)init
{
  self = [super init];
  if (self) {
  }
  return self;
}

- (NSString *)windowNibName
{
  return @"TextDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  [super windowControllerDidLoadNib:aController];
  [textUI setFont:[NSFont fontWithName:[defaults stringForKey:EDIT_FONT] size:(float)[defaults integerForKey:EDIT_FONT_SIZE]]];
  [textUI setString:[[[NSString alloc] initWithData:[self fileContents] encoding:NSMacOSRomanStringEncoding] autorelease]];
  [self setFileContents:nil];
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
  return [[textUI string] dataUsingEncoding:NSMacOSRomanStringEncoding];
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
  [self setFileContents:data];
  return YES;
}

- (NSData *)fileContents {
  return _fileContents;
}

- (void)setFileContents:(NSData *)newFileContents {
  if (_fileContents != newFileContents) {
    NSData *oldValue = _fileContents;
    _fileContents = [newFileContents retain];
    [oldValue release];
  }
}

- (NSTextView *)textUI
{
  return textUI;
}

- (void)dealloc
{
  [_fileContents release];
  [super dealloc];
}

@end
