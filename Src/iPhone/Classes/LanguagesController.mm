//
//  LanguagesController.mm
//  Dasher
//
//  Created by Alan Lawrence on 20/08/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import "LanguagesController.h"
#import "Parameters.h"
#import "DasherAppDelegate.h"

@implementation LanguagesController

-(id)init {
  if (self = [super initWithTitle:@"Languages" image:[UIImage imageNamed:@"globe.png"] settingParam:SP_ALPHABET_ID]) {
    //all ok
  }
  return self;
}

- (void)doSet:(NSString *)lang {
    [self doAsyncLocked:[NSString stringWithFormat:@"Training %@",lang] target:self selector:@selector(asyncSet:) param:lang];
}

- (void)asyncSet:(NSString *)lang {
	[super doSet:lang];
}

@end