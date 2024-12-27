// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <math.h>
namespace CommonPlatform {	namespace Xray {


struct Vector3d
{
	inline Vector3d()								: x(0),		y(0),		z(0)		{};
	inline Vector3d(double value)					: x(value), y(value),	z(value)	{};
	inline Vector3d(double x, double y, double z)	: x(x),		y(y),		z(z)		{};
	
	inline Vector3d	operator +  (const Vector3d &vec) const	{ return Vector3d(x+vec.x, y+vec.y, z+vec.z);								};
	inline Vector3d	operator -  (const Vector3d &vec) const	{ return Vector3d(x-vec.x, y-vec.y, z-vec.z);								};
	inline Vector3d	operator *  (const Vector3d &vec) const	{ return Vector3d(x*vec.x, y*vec.y, z*vec.z);								};
	inline Vector3d	operator /  (const Vector3d &vec) const	{ return Vector3d(x/vec.x, y/vec.y, z/vec.z);								};
	inline Vector3d	operator -  ()					  const	{ return Vector3d(-x, -y, -z);												};
													 
	inline bool		operator == (const Vector3d &vec) const	{ return x == vec.x && y == vec.y && z == vec.z;							};
	inline bool		operator != (const Vector3d &vec) const	{ return x != vec.x || y != vec.y || z != vec.z;							};
													 
	inline void		operator += (const Vector3d &vec) 		{ x += vec.x; y += vec.y; z += vec.z;										};
	inline void		operator -= (const Vector3d &vec) 		{ x -= vec.x; y -= vec.y; z -= vec.z;										};
	inline void		operator *= (const Vector3d &vec) 		{ x *= vec.x; y *= vec.y; z *= vec.z;										};
	inline void		operator /= (const Vector3d &vec) 		{ x /= vec.x; y /= vec.y; z /= vec.z;										};
													 
	inline double   length		()					  const	{ return sqrt(x*x+y*y+z*z);													};
	inline double   dot			(const Vector3d &vec) const	{ return x*vec.x + y*vec.y + z*vec.z;										};
	inline Vector3d cross		(const Vector3d &vec) const	{ return Vector3d(y*vec.z - z*vec.y, z*vec.x - x*vec.z, x*vec.y-y*vec.x);	};
	inline Vector3d normalize	()					  const	{ return (length() > 0.0) ? *this / length() : Vector3d(0.0);				};

	static inline Vector3d minimum(const Vector3d &a, const Vector3d &b) { return Vector3d(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z);															};
	static inline Vector3d maximum(const Vector3d &a, const Vector3d &b) { return Vector3d(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z);															};

	double x; 
	double y; 
	double z; 

private:

	Vector3d(bool);
};

}}