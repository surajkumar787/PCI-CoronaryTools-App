// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "Matrix4x4.h"

namespace CommonPlatform {	namespace Xray {


struct Space3d
{
	Matrix4x4 toRoot;
	Matrix4x4 fromRoot;

	inline Space3d()
	{
		toRoot   = Matrix4x4::identity();
		fromRoot = Matrix4x4::identity();
	}

	inline bool operator == (const Space3d &space) const	
	{ 
		return (toRoot == space.toRoot && fromRoot == space.fromRoot);
	}

	inline bool operator != (const Space3d &space) const	
	{ 
		return (toRoot != space.toRoot || fromRoot != space.fromRoot);
	}


	inline Space3d operator * (const Space3d &space) const	
	{ 
		Space3d s;
		s.toRoot   = toRoot * space.toRoot;
		s.fromRoot = space.fromRoot * fromRoot;
		return s;
	}

	inline Vector3d mapFrom(const Space3d &space, const Vector3d &vector) const	
	{ 
		return (&space == this) ? vector : fromRoot * (space.toRoot * vector);
	}


	static Space3d translate(const Vector3d &t)
	{
		Space3d s;
		s.toRoot   = Matrix4x4::translate(t);
		s.fromRoot = Matrix4x4::translate(-t);
		return s;
	}

	static Space3d scale(const Vector3d &z)
	{
		Space3d s;
		s.toRoot   = Matrix4x4::scale(z);
		s.fromRoot = Matrix4x4::scale(z.x != 0.0 && z.y != 0.0 && z.z != 0.0 ? Vector3d(1.0)/z : Vector3d(0.0));
		return s;
	}

	static Space3d rotate(double angle, const Vector3d &axis)
	{
		Space3d s;
		s.toRoot   = Matrix4x4::rotate( angle, axis);
		s.fromRoot = Matrix4x4::rotate(-angle, axis);
		return s;
	}

	static Space3d projection(double focus)
	{
		Space3d s;
		s.toRoot   = Matrix4x4::projection( focus);
		s.fromRoot = Matrix4x4::projection(-focus);
		return s;
    }

};

}}