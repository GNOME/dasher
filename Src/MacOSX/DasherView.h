/*
 * Created by Doug Dickinson (dougd AT DressTheMonkey DOT plus DOT com), 20 April 2003
 */


#import <AppKit/AppKit.h>


@interface DasherView : NSView {
  NSImage *_offscreen;
  NSDictionary *_textAttributes;
  BOOL isPaused;
}

- (NSDictionary *)textAttributes;
- (void)setTextAttributes:(NSDictionary *)newTextAttributes;

- (NSDictionary *)textAttributes;
- (void)setTextAttributes:(NSDictionary *)newTextAttributes;

- (NSSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize;

- (NSImage *)offscreen;
- (void)setOffscreen:(NSImage *)newOffscreen;

@end

