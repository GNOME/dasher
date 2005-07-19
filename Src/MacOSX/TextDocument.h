//
//  MyDocument.h
//  MD
//
//  Created by Doug Dickinson on Sun Jun 01 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher@DressTheMonkey.plus.com). All rights reserved.
//

#import <Cocoa/Cocoa.h>

@ interface TextDocument:NSDocument {
  IBOutlet NSTextView *textUI;

  NSData *_fileContents;
}

-(NSData *) fileContents;
-(void)setFileContents:(NSData *) newFileContents;

-(NSTextView *) textUI;

@end
