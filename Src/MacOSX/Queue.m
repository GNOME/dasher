//
//  Queue.m
//  Dasher
//
//  Created by Alan Lawrence on 08/05/2010.
//  Copyright 2010 Cavendish Laboratory. All rights reserved.
//

#import "Queue.h"


@implementation Queue

-(id)init {
  if (self = [super init]) {
    m_in = [NSMutableArray arrayWithCapacity:5];
    [m_in retain];
    m_out = [NSMutableArray arrayWithCapacity:5];
    [m_out retain];
  }
  return self;
}

-(void)clear {
  [m_in removeAllObjects];
  [m_out removeAllObjects];
  NSLog(@"Cleared\n");
}

-(void)push:(id)obj {
  [m_in addObject:obj];
  NSLog(@"Push %@, now %i items\n",obj,[m_in count]+[m_out count]);
}

-(id)pop {
  NSLog(@"Pop - %i+%i items...",[m_in count],[m_out count]);
  if ([m_out count]==0) {
    [m_out addObjectsFromArray:[[m_in reverseObjectEnumerator] allObjects]];
    [m_in removeAllObjects];
  }
  id ret= [m_out lastObject];
  [m_out removeLastObject];
  NSLog(@"now %i + %i\n",[m_in count],[m_out count]);
  return ret;
}

-(bool)hasItems {
  return [m_in count]>0 || [m_out count]>0;
}

-(void)dealloc {
  [m_in release];
  [m_out release];
  [super dealloc];
}

@end
