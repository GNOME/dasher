//
//  Queue.h
//  Dasher
//
//  Created by Alan Lawrence on 08/05/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Queue : NSObject {
  NSMutableArray *m_in, *m_out;
}

-(id)init;
-(void)push:(id)obj;
-(id)pop;
-(bool)hasItems;
-(void)clear;
@end
