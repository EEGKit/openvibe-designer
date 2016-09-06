/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
 */

#ifndef __Mensia_AdvancedVisualization_CVertex_H__
#define __Mensia_AdvancedVisualization_CVertex_H__

#include <mensia/base.h>

#include <cmath>

#pragma pack(1)
namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CVertex
		{
		public:

			CVertex(float64 _x=0, float64 _y=0, float64 _z=0, float64 _u=0, float64 _v=0)
				:x(float32(_x))
				,y(float32(_y))
				,z(float32(_z))
				,u(float32(_u))
				,v(float32(_v))
			{
			}

			CVertex(const CVertex& a, const CVertex& b)
				:x(b.x - a.x)
				,y(b.y - a.y)
				,z(b.z - a.z)
				,u(b.u - a.u)
				,v(b.v - a.v)
			{
			}

			float32 x;
			float32 y;
			float32 z;
			float32 u;
			float32 v;

			CVertex& normalize(void)
			{
				float32 n=this->length();
				if(n!=0)
				{
					float32 in=1.f/n;
					this->x*=in;
					this->y*=in;
					this->z*=in;
				}
				return *this;
			}

			float32 length(void) const
			{
				return ::sqrt(this->sqr_length());
			}

			float32 sqr_length(void) const
			{
				return CVertex::dot(*this, *this);
			}

			static float32 dot(const CVertex& v1, const CVertex& v2)
			{
				return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
			}

			static CVertex cross(const CVertex& v1, const CVertex& v2)
			{
				CVertex r;
				r.x = v1.y*v2.z - v1.z*v2.y;
				r.y = v1.z*v2.x - v1.x*v2.z;
				r.z = v1.x*v2.y - v1.y*v2.x;
				return r;
			}

			static CVertex cross(const CVertex& v1a, const CVertex& v1b, const CVertex& v2a, const CVertex& v2b)
			{
				CVertex v1(v1a, v1b);
				CVertex v2(v2a, v2b);
				return CVertex::cross(v1, v2);
			}

			static boolean isOnSameSide(const CVertex& p1, const CVertex& p2, const CVertex& a, const CVertex& b)
			{
				CVertex cp1=CVertex::cross(a, b, a, p1);
				CVertex cp2=CVertex::cross(a, b, a, p2);
				return CVertex::dot(cp1, cp2) >= 0;
			}

			static boolean isInTriangle(const CVertex& p, const CVertex& a, const CVertex& b, const CVertex& c)
			{
				return CVertex::isOnSameSide(p, a, b, c) && CVertex::isOnSameSide(p, b, c, a) && CVertex::isOnSameSide(p, c, a, b);
			}
		};
	};
};
#pragma pack()

#endif // __Mensia_AdvancedVisualization_CVertex_H__
