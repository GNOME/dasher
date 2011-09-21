//
//  DasherTextView.mm
//  Dasher
//
//  Created by Alan Lawrence on 19/05/2011.
//  Copyright 2011 Cavendish Laboratory. All rights reserved.
//

#import "DasherTextView.h"


@implementation DasherTextView

- (id)initWithFrame:(NSRect)frame {    
    if (self = [super initWithFrame:frame]) {
      self.delegate=self;
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder {
  if (self = [super initWithCoder:aDecoder]) {
    self.delegate = self;
  }
  return self;
}

- (void)keyDown:(NSEvent *)e {
  /*TODO, note that this isn't really "key down", rather it's "character entered"
   or similar - if the key is held down long enough to repeat, we get multiple keyDowns
   before a keyUp, and we just send them all along to the DasherCore code...*/
  [dasherApp handleKeyDown:e];
}

- (void)keyUp:(NSEvent *)e {
  [dasherApp handleKeyUp:e];
}

#pragma mark DasherEdit methods

- (void)outputCallback:(NSString *)aString {
  if (dasherApp.gameModeOn) return;
  //offsets are wrong after any control mode node has been executed; since _any_
  // adding of text (at least by following method) generally causes textViewDidChangeSelection
  // callbacks, we have to suppress these during text adding in all cases...
  suppressCursorEvents=YES;
  [self replaceCharactersInRange:[self selectedRange] withString:aString];
  [self scrollRangeToVisible:[self selectedRange]];
  suppressCursorEvents=NO;
  dasherApp.modified=YES;
}

- (void)deleteCallback:(NSString *)s {
  if (dasherApp.gameModeOn) return;
  suppressCursorEvents=YES; //similarly to outputCallback
  int len = [s length];
  NSRange curSelection = [self selectedRange];
  //delete current selection and preceding character(s)
  [self replaceCharactersInRange:NSMakeRange(curSelection.location-len, curSelection.length+len) withString:@""];
  [self scrollRangeToVisible:(self.selectedRange = NSMakeRange(curSelection.location-len, 0))];
  suppressCursorEvents=NO;
  dasherApp.modified=YES;
}

-(NSString *)textAtOffset:(unsigned int)iOffset Length:(unsigned int)iLength {
  //This does not hold if using control mode since it gets the offsets wrong...
  //DASHER_ASSERT(iOffset+iLength <= [allTextEntered length]);
  //Instead we shall handle all the out-of-bounds cases ourselves
  // (substringWithRange is not at all forgiving!)
  NSString *s = [self string];
  if (iOffset >= [s length]) return @"";
  if (iOffset+iLength > [s length])
    iLength = [s length]-iOffset;
  return [s substringWithRange:NSMakeRange(iOffset,iLength)];
}

-(NSString *)allContext {
  return [self string];
}

-(void)clearContext {
  [self replaceCharactersInRange:NSMakeRange(0, [[self string] length]) withString:@""];
}

-(unsigned int)currentCursorPos {
  if (dasherApp.gameModeOn) return 0;
  return [self selectedRange].location;
}

-(unsigned int)ctrlMove:(CControlManager::EditDistance)dist forwards:(BOOL)bForwards {
  suppressCursorEvents=YES;
  //According to the Key Bindings documentation, http://developer.apple.com/library/mac/#documentation/Cocoa/Conceptual/TextEditing/Concepts/KeyBindings.html%23//apple_ref/doc/uid/TP40008900-BAJCDFFI
  // keypresses such as (up arrow) are mapped to strings by consulting a dictionary, /System/Library/Frameworks/AppKit.framework/Resources/StandardKeyBinding.dict (user may provide alternative)
  // and these strings are then sent to the NSTextView's doCommandBySelector: method.
  // Hence, we will send the same strings directly, regardless of their key bindings...
  switch (dist) {
    case CControlManager::EDIT_CHAR:
      [self doCommandBySelector:(bForwards ? @selector(moveForward:) : @selector(moveBackward:))];
      break;
    case CControlManager::EDIT_WORD:
      [self doCommandBySelector:(bForwards ? @selector(moveWordForward:) : @selector(moveWordBackward:))];
      break;
    case CControlManager::EDIT_LINE: {
      NSRange curRange = [self selectedRange];
      SEL sel(bForwards ? @selector(moveToBeginningOfLine:) : @selector(moveToEndOfLine:));
      [self doCommandBySelector:sel];
      if (NSEqualRanges([self selectedRange], curRange)) {
        //already at beginning/end of line. move a character first
        [self doCommandBySelector:(bForwards ? @selector(moveForward:) : @selector(moveBackward:))];  
        //then try it again
        [self doCommandBySelector:sel];
      }
    }
      break;
    case CControlManager::EDIT_FILE:
      [self doCommandBySelector:(bForwards ? @selector(moveToBeginningOfDocument:) : @selector(moveToEndOfDocument:))];
      break;
    default:
      break;
  }
  NSRange newRange=[self selectedRange];
  DASHER_ASSERT(newRange.length==0);
  suppressCursorEvents=NO;
  return newRange.location;
}

-(unsigned int)ctrlDelete:(CControlManager::EditDistance)dist forwards:(BOOL)bForwards {
  suppressCursorEvents=YES;
  //According to the Key Bindings documentation, http://developer.apple.com/library/mac/#documentation/Cocoa/Conceptual/TextEditing/Concepts/KeyBindings.html%23//apple_ref/doc/uid/TP40008900-BAJCDFFI
  // keypresses such as (up arrow) are mapped to strings by consulting a dictionary, /System/Library/Frameworks/AppKit.framework/Resources/StandardKeyBinding.dict (user may provide alternative)
  // and these strings are then sent to the NSTextView's doCommandBySelector: method.
  // Hence, we will send the same strings directly, regardless of their key bindings...
  switch (dist) {
    case CControlManager::EDIT_CHAR:
      [self doCommandBySelector:(bForwards ? @selector(deleteForward:) : @selector(deleteBackward:))];
      break;
    case CControlManager::EDIT_WORD:
      [self doCommandBySelector:(bForwards ? @selector(deleteWordForward:) : @selector(deleteWordBackward:))];
      break;
    case CControlManager::EDIT_LINE: {
      NSRange curRange = [self selectedRange];
      SEL sel(bForwards ? @selector(deleteToBeginningOfLine:) : @selector(deleteToEndOfLine:));
      [self doCommandBySelector:sel];
      if (NSEqualRanges([self selectedRange],curRange)) {
        //already at beginning/end of line. move a character first
        [self doCommandBySelector:(bForwards ? @selector(deleteForward:) : @selector(deleteBackward:))];  
        //then try it again
        [self doCommandBySelector:sel];
      }
    }
      break;
    case CControlManager::EDIT_FILE:
      //deleteTo{Beginning,End}OfDocument: do not exist...
      self.selectedRange=NSMakeRange([self selectedRange].location, 0);
      [self doCommandBySelector:(bForwards ? @selector(moveToBeginningOfDocumentAndModifySelection:) : @selector(moveToEndOfDocumentAndModifySelection:))];
      [self replaceCharactersInRange:[self selectedRange] withString:@""];
      break;
    default:
      break;
  }
  NSRange newRange=[self selectedRange];
  DASHER_ASSERT(newRange.length==0);
  suppressCursorEvents=NO;
  dasherApp.modified=YES;
  return newRange.location;
}

#pragma mark NSTextViewDelegate methods

- (void)textView:(NSTextView *)aTextView clickedOnCell:(id < NSTextAttachmentCell >)cell inRect:(NSRect)cellFrame atIndex:(NSUInteger)charIndex {
  NSLog(@"clicked on %@ rect %f,%f,%f,%f idx %i",cell,cellFrame.origin.x,cellFrame.origin.y,cellFrame.size.width,cellFrame.size.height,charIndex);
}

-(void)textViewDidChangeSelection:(NSNotification *)notification {
  if (dasherApp.gameModeOn) return;
  if (suppressCursorEvents) return;
  //NSLog(@"DidChangeSelection %i+%i",[textView selectedRange].location,[textView selectedRange].length);
  if ([dasherApp timer]) //timer set when Realize() completed
    [dasherApp aquaDasherControl]->SetOffset([self selectedRange].location, false);
}

@end
