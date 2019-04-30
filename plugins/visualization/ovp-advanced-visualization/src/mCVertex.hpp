/*********************************************************************
 * Software License Agreement (AGPL-3 License)
 *
 * OpenViBE Designer
 * Based on OpenViBE V1.1.0, Copyright (C) Inria, 2006-2015
 * Copyright (C) Inria, 2015-2017,V1.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <cmath>

#pragma pack(1)
namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CVertex
		{
		public:

			CVertex(double _x=0, double _y=0, double _z=0, double _u=0, double _v=0)
				:x(float(_x))
				,y(float(_y))
				,z(float(_z))
				,u(float(_u))
				,v(float(_v)) { }

			CVertex(const CVertex& a, const CVertex& b)
				:x(b.x - a.x)
				,y(b.y - a.y)
				,z(b.z - a.z)
				,u(b.u - a.u)
				,v(b.v - a.v) { }

			float x;
			float y;
			float z;
			float u;
			float v;

			CVertex& normalize()

			{
				float n=this->length();
				if(n!=0)
				{
					float in=1.f/n;
					this->x*=in;
					this->y*=in;
					this->z*=in;
				}
				return *this;
			}

			float length() const
			{
				return sqrt(this->sqr_length());
			}

			float sqr_length() const
			{
				return dot(*this, *this);
			}

			static float dot(const CVertex& v1, const CVertex& v2)
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
				return cross(v1, v2);
			}

			static bool isOnSameSide(const CVertex& p1, const CVertex& p2, const CVertex& a, const CVertex& b)
			{
				CVertex cp1=cross(a, b, a, p1);
				CVertex cp2=cross(a, b, a, p2);
				return dot(cp1, cp2) >= 0;
			}

			static bool isInTriangle(const CVertex& p, const CVertex& a, const CVertex& b, const CVertex& c)
			{
				return isOnSameSide(p, a, b, c) && isOnSameSide(p, b, c, a) && isOnSameSide(p, c, a, b);
			}
		};
	};
};
#pragma pack()

