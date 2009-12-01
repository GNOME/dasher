/*
 *  Vec3.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 23/04/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#ifndef __VEC3_H_

#define __VEC3_H_

typedef struct _tag_ {
	float x,y,z;
} Vec3;

Vec3 Vec3Make(float x,float y,float z);
float Vec3Dot(Vec3 a,Vec3 b);
Vec3 Vec3Cross(Vec3 a,Vec3 b);
Vec3 Vec3Sub(Vec3 a,Vec3 b);
Vec3 Vec3Mul(Vec3 v, float m);
float Vec3Len(Vec3 v);

static const Vec3 Vec3Zero = {0.0, 0.0, 0.0};
static const Vec3 Vec3X = {1.0, 0.0, 0.0};
static const Vec3 Vec3Y = {0.0, 1.0, 0.0};
static const Vec3 Vec3Z = {0.0, 0.0, 1.0};

#endif