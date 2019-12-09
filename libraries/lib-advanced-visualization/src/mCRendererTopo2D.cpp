#include "mCRendererTopo2D.hpp"
#include <cmath>

using namespace Mensia;
using namespace AdvancedVisualization;

// constexpr constexpr float OFFSET = 0.0001f; //Macro modernization, Not yet with jenkins (not the last visual 2013 which it works)
#define OFFSET 0.0001f

void CRendererTopo2D::rebuild3DMeshesPre(const CRendererContext& /*rContext*/)
{
	const size_t nVertex1      = 32;
	const size_t nVertex2      = 32;
	const size_t nCircleVertex = 128;

	{	// Scalp
		m_scalp.clear();

		std::vector<CVertex>& vertices   = m_scalp.m_Vertices;
		std::vector<uint32_t>& triangles = m_scalp.m_Triangles;

		vertices.resize(nVertex1 * nVertex2);
		for (size_t i = 0, k = 0; i < nVertex1; ++i)
		{
			for (size_t j = 0; j < nVertex2; j++, k++)
			{
				const auto a  = float(i * M_PI / (nVertex1 - 1));
				const auto b  = float(j * 1.25 * M_PI / (nVertex2 - 1) - M_PI * .2);
				vertices[k].x = cosf(a);
				vertices[k].y = sinf(a) * sinf(b) - OFFSET;
				vertices[k].z = sinf(a) * cosf(b);
				vertices[k].u = k * 200.F / (nVertex1 * nVertex2);
			}
		}

		triangles.resize((nVertex1 - 1) * (nVertex2 - 1) * 6);
		for (size_t i = 0, k = 0; i < nVertex1 - 1; ++i)
		{
			for (size_t j = 0; j < nVertex2 - 1; j++, k += 6)
			{
				triangles[k]     = (i) * nVertex2 + (j);
				triangles[k + 1] = (i + 1) * nVertex2 + (j);
				triangles[k + 2] = (i + 1) * nVertex2 + (j + 1);

				triangles[k + 3] = triangles[k];
				triangles[k + 4] = triangles[k + 2];
				triangles[k + 5] = (i) * nVertex2 + (j + 1);
			}
		}

		m_scalp.m_Color.fill(1.0);
		// m_scalp.compile();
	}

	{	// Face
		m_face.clear();

		std::vector<CVertex>& vertices   = m_face.m_Vertices;
		std::vector<uint32_t>& triangles = m_face.m_Triangles;

		// Ribbon mesh

		vertices.resize(nCircleVertex * 2/*+6*/);
		for (size_t i = 0, k = 0; i < nCircleVertex; ++i, ++k)
		{
			const auto a = float(i * 4 * M_PI / nCircleVertex);

			vertices[k].x = cosf(a);
			vertices[k].y = .01F;
			vertices[k].z = sinf(a);
			k++;
			vertices[k].x = cosf(a);
			vertices[k].y = -.01F;
			vertices[k].z = sinf(a);
		}

		// Nose mesh
		/*
		vertices[k].x=-1;
		vertices[k].y=.01;
		vertices[k].z=-.5;
		k++;
		vertices[k].x=-1;
		vertices[k].y=-.01;
		vertices[k].z=-.5;
		k++;
		vertices[k].x=0;
		vertices[k].y=.01;
		vertices[k].z=-1.5;
		k++;
		vertices[k].x=0;
		vertices[k].y=-.01;
		vertices[k].z=-1.5;
		k++;
		vertices[k].x=1;
		vertices[k].y=.01;
		vertices[k].z=-.5;
		k++;
		vertices[k].x=1;
		vertices[k].y=-.01;
		vertices[k].z=-.5;
		*/
		// Ribon mesh

		triangles.resize(nCircleVertex * 6/*+12*/);
		const size_t mod = nCircleVertex * 2;
		for (size_t i = 0, k = 0; i < nCircleVertex; ++i)
		{
			triangles[k++] = (i) % mod;
			triangles[k++] = (i + 1) % mod;
			triangles[k++] = (i + 2) % mod;

			triangles[k++] = (i + 1) % mod;
			triangles[k++] = (i + 2) % mod;
			triangles[k++] = (i + 3) % mod;
		}

		// Nose mesh
		/*
		triangles[k++] = mod;
		triangles[k++] = mod + 1;
		triangles[k++] = mod + 2;

		triangles[k++] = mod + 1;
		triangles[k++] = mod + 2;
		triangles[k++] = mod + 3;

		triangles[k++] = mod + 2;
		triangles[k++] = mod + 3;
		triangles[k++] = mod + 4;
		
		triangles[k++] = mod + 3;
		triangles[k++] = mod + 4;
		triangles[k++] = mod + 5;
		*/
		m_face.m_Color.fill(1.15F);
		// m_face.compile();
	}
}

namespace
{
	void unfold(std::vector<CVertex>& vertices, const float layer = 0)
	{
		for (auto& v : vertices)
		{
			v.y += OFFSET;
			const float phi = float(M_PI) * .5F - asinf(v.y);
			const float psi = atan2f(v.z, v.x);

			v.x = phi * cos(psi);
			v.y = layer;
			v.z = phi * sin(psi);
		}
	}
} // namespace

void CRendererTopo2D::rebuild3DMeshesPost(const CRendererContext& /*ctx*/)
{
	const float layer = 1E-3F;

	unfold(m_scalp.m_Vertices, -layer);
	unfold(m_face.m_Vertices, layer);
	unfold(m_projectedPositions);
}
