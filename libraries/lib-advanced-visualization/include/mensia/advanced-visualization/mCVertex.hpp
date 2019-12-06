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

			explicit CVertex(const double _x = 0, const double _y = 0, const double _z = 0, const double _u = 0, const double _v = 0)
				: x(float(_x)), y(float(_y)), z(float(_z)), u(float(_u)), v(float(_v)) { }

			CVertex(const CVertex& a, const CVertex& b)
				: x(b.x - a.x), y(b.y - a.y), z(b.z - a.z), u(b.u - a.u), v(b.v - a.v) { }

			float x = 0;
			float y = 0;
			float z = 0;
			float u = 0;
			float v = 0;

			CVertex& normalize()

			{
				const float n = this->length();
				if (n != 0)
				{
					const float in = 1.F / n;
					this->x *= in;
					this->y *= in;
					this->z *= in;
				}
				return *this;
			}

			float length() const { return sqrt(this->sqrLength()); }

			float sqrLength() const { return dot(*this, *this); }

			static float dot(const CVertex& a, const CVertex& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

			static CVertex cross(const CVertex& a, const CVertex& b)
			{
				CVertex r;
				r.x = a.y * b.z - a.z * b.y;
				r.y = a.z * b.x - a.x * b.z;
				r.z = a.x * b.y - a.y * b.x;
				return r;
			}

			static CVertex cross(const CVertex& a1, const CVertex& b1, const CVertex& a2, const CVertex& b2)
			{
				const CVertex v1(a1, b1);
				const CVertex v2(a2, b2);
				return cross(v1, v2);
			}

			static bool isOnSameSide(const CVertex& p1, const CVertex& p2, const CVertex& a, const CVertex& b)
			{
				const CVertex cp1 = cross(a, b, a, p1);
				const CVertex cp2 = cross(a, b, a, p2);
				return dot(cp1, cp2) >= 0;
			}

			static bool isInTriangle(const CVertex& p, const CVertex& a, const CVertex& b, const CVertex& c)
			{
				return isOnSameSide(p, a, b, c) && isOnSameSide(p, b, c, a) && isOnSameSide(p, c, a, b);
			}
		};
	} // namespace AdvancedVisualization
} // namespace Mensia
#pragma pack()
