/*
 * Created by Doug Dickinson (dougd AT DressTheMonkey DOT plus DOT com), 20 April 2003
 */


#import "DasherView.h"
#import "DasherApp.h"
#import "PreferencesController.h"

#include "libdasher.h"

@implementation DasherView

- (id)initWithFrame:(NSRect)frame {
  self = [super initWithFrame:frame];
  if (self) {
    [self setupOffscreenImage];
    [self setTextAttributes:[NSDictionary dictionaryWithObject:[NSColor blackColor] forKey:NSForegroundColorAttributeName]];
    isPaused = NO;
  }
  return self;
}

- (void)setFrameSize:(NSSize)newSize
{
  // if the window gets too small, the error "Can't cache image' is given
  // and the whole thing blows up
  // current workaround is to set a min window size in IB
  [super setFrameSize:newSize];

  dasher_resize_canvas((int)newSize.width, (int)newSize.height);
  [self setupOffscreenImage];
  dasher_redraw();
}

- (void)setupOffscreenImage
{
  // could optimise here by only creating the image if the offscreen image is too small
  // ie, if they had a big window and made it smaller, can still use the big image to store the small data
  [self setOffscreen:[[[NSImage alloc] initWithSize:[self bounds].size] autorelease]];
  [[self offscreen] setFlipped:YES];
}

- (BOOL)isFlipped
{
  return YES;
}

- (BOOL)isOpaque
{
  return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
  return YES;
}

- (BOOL)acceptsFirstResponder
{
  // TODO should the view should do something to indicate it has focus
  // is drawing a blue rectangle around the whole thing acceptable?
  return YES;
}

- (BOOL)becomeFirstResponder
{
  [[NSFontManager sharedFontManager] setSelectedFont:[self fontWithTextSize:10] isMultiple:NO];
}

- (BOOL)resignFirstResponder
{
}


- (void)drawRect:(NSRect)rect {
  [[self offscreen] compositeToPoint:NSMakePoint(0,[self bounds].size.height) fromRect:rect operation:NSCompositeCopy];
}

- (void)mouseDown:(NSEvent *)e
{
  if ([[NSUserDefaults standardUserDefaults] boolForKey:START_MOUSE])
    {
    [[[NSApplication sharedApplication] delegate] toggleDashing];
    }
}

- (void)keyDown:(NSEvent *)e
{
  if ([[NSUserDefaults standardUserDefaults] boolForKey:START_SPACE] && [[e characters] isEqualToString:@" "])
    {
    [[[NSApplication sharedApplication] delegate] toggleDashing];
    }
}



- (void)blankCallback
{
  NSRect r = [self bounds];
  [[self offscreen] lockFocus];
  [[NSColor whiteColor] set];
  NSRectFill(r);
  [[self offscreen] unlockFocus];
}

- (void)displayCallback
{
  [self setNeedsDisplay:YES];
}



- (void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 color:(NSColor *)aColor
{
  int x = x1;
  int y = y1;
  int width = x2 - x1;
  int height = y2 - y1;
  NSRect r;

  if (x1 > x2)
    {
    x = x2;
    width = x1 - x2;
    }
  if (y1 > y2)
    {
    y = y2;
    height = y1 - y2;
    }

  r = NSMakeRect(x1, y1, width, height);
  [[self offscreen] lockFocus];
  [aColor set];
  NSRectFill(r);
  [[self offscreen] unlockFocus];

}

- (NSSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize
{
  return [aString sizeWithAttributes:[self textAttributesWithTextSize:aSize]];
}

- (void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize
{
  [[self offscreen] lockFocus];
  [aString drawAtPoint:NSMakePoint(x1, y1) withAttributes:[self textAttributesWithTextSize:aSize]];
  [[self offscreen] unlockFocus];
}

- (void)polylineCallbackPoints:(NSArray *)points
{
  NSBezierPath *bp;
  int len = [points count];
  int i;

  if (len < 1)
    {
    return;
    }

  [[self offscreen] lockFocus];
  [[NSColor blackColor] set];
  bp = [NSBezierPath bezierPath];
  [bp moveToPoint:[[points objectAtIndex:0] pointValue]];

  for (i = 1; i < len; i++)
    {
    [bp lineToPoint:[[points objectAtIndex:i] pointValue]];
    }

  [bp closePath];
  [bp stroke];

  [[self offscreen] unlockFocus];

}

- (IBAction)changeFont:(id)sender
{
  [[PreferencesController preferencesController] changeDasherFont:sender];
}

- (NSFont *)fontWithTextSize:(int)aSize
{
  return [NSFont fontWithName:[[NSUserDefaults standardUserDefaults] objectForKey:DASHER_FONT] size:(float)aSize];
}

- (NSDictionary *)textAttributesWithTextSize:(int)aSize
{
  // some caching could take place here, we must be asking for the same textAttrib's thousands of times
  NSMutableDictionary *result = [[self textAttributes] mutableCopy];
  [result setObject:[self fontWithTextSize:aSize] forKey:NSFontAttributeName];
  return result;
}

- (NSDictionary *)textAttributes {
  return _textAttributes;
}

- (void)setTextAttributes:(NSDictionary *)newTextAttributes {
  if (_textAttributes != newTextAttributes) {
    NSDictionary *oldValue = _textAttributes;
    _textAttributes = [newTextAttributes retain];
    [oldValue release];
  }
}

- (NSImage *)offscreen {
  return _offscreen;
}

- (void)setOffscreen:(NSImage *)newOffscreen {
  if (_offscreen != newOffscreen) {
    NSImage *oldValue = _offscreen;
    _offscreen = [newOffscreen retain];
    [oldValue release];
  }
}


- (void)dealloc
{
  [_offscreen release];
  [_textAttributes release];
  [super dealloc];
}

@end
