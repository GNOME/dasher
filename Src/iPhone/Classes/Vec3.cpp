/*
 *  Vec3.c
 *  Dasher
 *
 *  Created by Alan Lawrence on 23/04/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#include "Vec3.h"
#include "math.h"

Vec3 Vec3Make(float x,float y,float z)
{
	Vec3 result = {x,y,z};
	return result;
}

float Vec3Dot(Vec3 a,Vec3 b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 Vec3Cross(Vec3 a,Vec3 b)
{
	Vec3 result = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
	return result;
}

Vec3 Vec3Sub(Vec3 a,Vec3 b)
{
	Vec3 result = {a.x-b.x, a.y-b.y, a.z-b.z};
	return result;
}

Vec3 Vec3Mul(Vec3 in, float m)
{
	Vec3 result = {in.x*m, in.y*m, in.z*m};
	return result;
}

float Vec3Len(Vec3 v)
{
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}