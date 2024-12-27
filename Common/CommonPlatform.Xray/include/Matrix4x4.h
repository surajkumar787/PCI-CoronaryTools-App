// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <math.h>
#include "Vector3d.h"

namespace CommonPlatform {	namespace Xray {

struct Matrix4x4
{
	inline Matrix4x4()
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				m[y][x] = 0;
			}
		}
	}

    // Creates a 3d matrix from the given (4x4) array of length 16 doubles (defined row-by-row)
	inline Matrix4x4(double element[16])
	{
		for (int y = 0; y < 4; ++y)
		{
			for (int x = 0; x < 4; ++x)
			{
				m[y][x] = element[x + (y * 4)];
			}
		}
	}

	inline Matrix4x4 transpose() const
	{
		Matrix4x4 r;
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				r.m[y][x] = m[x][y];
			}
		}
		return r;
	}

	
	inline double determinant() const
	{
		double value;
	  
		value = m[0][3]*m[1][2]*m[2][1]*m[3][0] - m[0][2]*m[1][3]*m[2][1]*m[3][0] - m[0][3]*m[1][1]*m[2][2]*m[3][0] + m[0][1]*m[1][3]*m[2][2]*m[3][0] +
				m[0][2]*m[1][1]*m[2][3]*m[3][0] - m[0][1]*m[1][2]*m[2][3]*m[3][0] - m[0][3]*m[1][2]*m[2][0]*m[3][1] + m[0][2]*m[1][3]*m[2][0]*m[3][1] +
				m[0][3]*m[1][0]*m[2][2]*m[3][1] - m[0][0]*m[1][3]*m[2][2]*m[3][1] - m[0][2]*m[1][0]*m[2][3]*m[3][1] + m[0][0]*m[1][2]*m[2][3]*m[3][1] +
				m[0][3]*m[1][1]*m[2][0]*m[3][2] - m[0][1]*m[1][3]*m[2][0]*m[3][2] - m[0][3]*m[1][0]*m[2][1]*m[3][2] + m[0][0]*m[1][3]*m[2][1]*m[3][2] +
				m[0][1]*m[1][0]*m[2][3]*m[3][2] - m[0][0]*m[1][1]*m[2][3]*m[3][2] - m[0][2]*m[1][1]*m[2][0]*m[3][3] + m[0][1]*m[1][2]*m[2][0]*m[3][3] +
				m[0][2]*m[1][0]*m[2][1]*m[3][3] - m[0][0]*m[1][2]*m[2][1]*m[3][3] - m[0][1]*m[1][0]*m[2][2]*m[3][3] + m[0][0]*m[1][1]*m[2][2]*m[3][3] ;
	   
		return value;
	}

	Matrix4x4 invert() const
   {
		Matrix4x4 r;
		double d = determinant();
		if (d == 0) return Matrix4x4();

		r.m[0][0] = (m[1][2]*m[2][3]*m[3][1] - m[1][3]*m[2][2]*m[3][1] + m[1][3]*m[2][1]*m[3][2] - m[1][1]*m[2][3]*m[3][2] - m[1][2]*m[2][1]*m[3][3] + m[1][1]*m[2][2]*m[3][3]) / d;
		r.m[0][1] = (m[0][3]*m[2][2]*m[3][1] - m[0][2]*m[2][3]*m[3][1] - m[0][3]*m[2][1]*m[3][2] + m[0][1]*m[2][3]*m[3][2] + m[0][2]*m[2][1]*m[3][3] - m[0][1]*m[2][2]*m[3][3]) / d;
		r.m[0][2] = (m[0][2]*m[1][3]*m[3][1] - m[0][3]*m[1][2]*m[3][1] + m[0][3]*m[1][1]*m[3][2] - m[0][1]*m[1][3]*m[3][2] - m[0][2]*m[1][1]*m[3][3] + m[0][1]*m[1][2]*m[3][3]) / d;
		r.m[0][3] = (m[0][3]*m[1][2]*m[2][1] - m[0][2]*m[1][3]*m[2][1] - m[0][3]*m[1][1]*m[2][2] + m[0][1]*m[1][3]*m[2][2] + m[0][2]*m[1][1]*m[2][3] - m[0][1]*m[1][2]*m[2][3]) / d;
		r.m[1][0] = (m[1][3]*m[2][2]*m[3][0] - m[1][2]*m[2][3]*m[3][0] - m[1][3]*m[2][0]*m[3][2] + m[1][0]*m[2][3]*m[3][2] + m[1][2]*m[2][0]*m[3][3] - m[1][0]*m[2][2]*m[3][3]) / d;
		r.m[1][1] = (m[0][2]*m[2][3]*m[3][0] - m[0][3]*m[2][2]*m[3][0] + m[0][3]*m[2][0]*m[3][2] - m[0][0]*m[2][3]*m[3][2] - m[0][2]*m[2][0]*m[3][3] + m[0][0]*m[2][2]*m[3][3]) / d;
		r.m[1][2] = (m[0][3]*m[1][2]*m[3][0] - m[0][2]*m[1][3]*m[3][0] - m[0][3]*m[1][0]*m[3][2] + m[0][0]*m[1][3]*m[3][2] + m[0][2]*m[1][0]*m[3][3] - m[0][0]*m[1][2]*m[3][3]) / d;
		r.m[1][3] = (m[0][2]*m[1][3]*m[2][0] - m[0][3]*m[1][2]*m[2][0] + m[0][3]*m[1][0]*m[2][2] - m[0][0]*m[1][3]*m[2][2] - m[0][2]*m[1][0]*m[2][3] + m[0][0]*m[1][2]*m[2][3]) / d;
		r.m[2][0] = (m[1][1]*m[2][3]*m[3][0] - m[1][3]*m[2][1]*m[3][0] + m[1][3]*m[2][0]*m[3][1] - m[1][0]*m[2][3]*m[3][1] - m[1][1]*m[2][0]*m[3][3] + m[1][0]*m[2][1]*m[3][3]) / d;
		r.m[2][1] = (m[0][3]*m[2][1]*m[3][0] - m[0][1]*m[2][3]*m[3][0] - m[0][3]*m[2][0]*m[3][1] + m[0][0]*m[2][3]*m[3][1] + m[0][1]*m[2][0]*m[3][3] - m[0][0]*m[2][1]*m[3][3]) / d;
		r.m[2][2] = (m[0][1]*m[1][3]*m[3][0] - m[0][3]*m[1][1]*m[3][0] + m[0][3]*m[1][0]*m[3][1] - m[0][0]*m[1][3]*m[3][1] - m[0][1]*m[1][0]*m[3][3] + m[0][0]*m[1][1]*m[3][3]) / d;
		r.m[2][3] = (m[0][3]*m[1][1]*m[2][0] - m[0][1]*m[1][3]*m[2][0] - m[0][3]*m[1][0]*m[2][1] + m[0][0]*m[1][3]*m[2][1] + m[0][1]*m[1][0]*m[2][3] - m[0][0]*m[1][1]*m[2][3]) / d;
		r.m[3][0] = (m[1][2]*m[2][1]*m[3][0] - m[1][1]*m[2][2]*m[3][0] - m[1][2]*m[2][0]*m[3][1] + m[1][0]*m[2][2]*m[3][1] + m[1][1]*m[2][0]*m[3][2] - m[1][0]*m[2][1]*m[3][2]) / d;
		r.m[3][1] = (m[0][1]*m[2][2]*m[3][0] - m[0][2]*m[2][1]*m[3][0] + m[0][2]*m[2][0]*m[3][1] - m[0][0]*m[2][2]*m[3][1] - m[0][1]*m[2][0]*m[3][2] + m[0][0]*m[2][1]*m[3][2]) / d;
		r.m[3][2] = (m[0][2]*m[1][1]*m[3][0] - m[0][1]*m[1][2]*m[3][0] - m[0][2]*m[1][0]*m[3][1] + m[0][0]*m[1][2]*m[3][1] + m[0][1]*m[1][0]*m[3][2] - m[0][0]*m[1][1]*m[3][2]) / d;
		r.m[3][3] = (m[0][1]*m[1][2]*m[2][0] - m[0][2]*m[1][1]*m[2][0] + m[0][2]*m[1][0]*m[2][1] - m[0][0]*m[1][2]*m[2][1] - m[0][1]*m[1][0]*m[2][2] + m[0][0]*m[1][1]*m[2][2]) / d;

		return r;
	}
	
	inline bool operator == (const Matrix4x4 &mat) const	
	{ 
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				if (m[y][x] != mat.m[y][x])
				{
					return false;
				}
			}
		}
			return true;
	};

	inline bool operator != (const Matrix4x4 &mat) const	
	{ 
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				if (m[y][x] != mat.m[y][x])
				{
					return true;
				}
			}
		}
		return false;
	};
	

	inline Matrix4x4 operator * (const Matrix4x4 &mat) const	
	{ 
		Matrix4x4 r;
		
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				for (int i = 0; i < 4; i++)
				{
					r.m[y][x] += m[y][i] * mat.m[i][x];
				}
			}
		}
		return r;
	};
	
    // Overload for multiplication operator
	inline Vector3d operator * (const Vector3d &p) const
	{
		Vector3d r;

		r.x      = p.x * m[0][0] + p.y * m[0][1] + p.z * m[0][2] + m[0][3];
		r.y      = p.x * m[1][0] + p.y * m[1][1] + p.z * m[1][2] + m[1][3];
		r.z      = p.x * m[2][0] + p.y * m[2][1] + p.z * m[2][2] + m[2][3];
		double w = p.x * m[3][0] + p.y * m[3][1] + p.z * m[3][2] + m[3][3];
	
		return (w == 0.0) ? 0.0 : r / w;
	};

	static Matrix4x4 identity()
	{ 
		Matrix4x4 m;
		m.m[0][0] = 1.0; m.m[1][1] = 1.0; m.m[2][2] = 1.0;   m.m[3][3] = 1.0;	
		return m; 
	}
	
	static Matrix4x4 translate(const Vector3d &t)
	{
		Matrix4x4 m = identity();
		m.m[0][3] = t.x; m.m[1][3] = t.y; m.m[2][3] = t.z;
		return m;
	}

	static Matrix4x4 scale(const Vector3d &s)
	{
		Matrix4x4 m = identity();
		m.m[0][0] = s.x; m.m[1][1] = s.y; m.m[2][2] = s.z; 
		return m;
	}

	static Matrix4x4 rotate(double angle, const Vector3d &axis)
	{
		Matrix4x4 m = identity();
		
		const double pi  = 3.14159265358979323846;
		const double rad = angle / 180.0 * pi;
		const double s   = sin(rad);
		const double c   = cos(rad);

		m.m[0][0] = axis.x*axis.x*(1-c) + c;			m.m[0][1] = axis.x*axis.y*(1-c) - axis.z*s; 	m.m[0][2] = axis.x*axis.z*(1-c) + axis.y*s;
		m.m[1][0] = axis.y*axis.x*(1-c) + axis.z*s;		m.m[1][1] = axis.y*axis.y*(1-c) + c;			m.m[1][2] = axis.y*axis.z*(1-c) - axis.x*s;
		m.m[2][0] = axis.z*axis.x*(1-c) - axis.y*s; 	m.m[2][1] = axis.z*axis.y*(1-c) + axis.x*s;		m.m[2][2] = axis.z*axis.z*(1-c) + c;

		return m;
	}
	
	static Matrix4x4 projection(double focus)
	{
		Matrix4x4 m = Matrix4x4::identity();
		m.m[3][2] =  1.0 / focus;
		return m;
    }

	double m[4][4];

};

}}