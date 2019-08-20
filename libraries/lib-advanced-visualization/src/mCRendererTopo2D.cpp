#include "mCRendererTopo2D.hpp"
#include <cmath>

using namespace Mensia;
using namespace AdvancedVisualization;

//	constexpr constexpr float OFFSET = 0.0001f; //Macro modernization, Not yet with jenkins (not the last visual 2013 which it works)
#define OFFSET 0.0001f

void CRendererTopo2D::rebuild3DMeshesPre(const IRendererContext& /*rContext*/)
{
	uint32_t i, j, k;

	const uint32_t vertexCount1      = 32;
	const uint32_t vertexCount2      = 32;
	const uint32_t circleVertexCount = 128;

	{
		m_oScalp.clear();

		std::vector<CVertex>& l_vVertex    = m_oScalp.m_vVertex;
		std::vector<uint32_t>& l_vTriangle = m_oScalp.m_vTriangle;

		l_vVertex.resize(vertexCount1 * vertexCount2);
		for (i = 0, k = 0; i < vertexCount1; ++i)
		{
			for (j = 0; j < vertexCount2; j++, k++)
			{
				const auto a   = float(i * M_PI / (vertexCount1 - 1));
				const auto b   = float(j * 1.25 * M_PI / (vertexCount2 - 1) - M_PI * .2);
				l_vVertex[k].x = cosf(a);
				l_vVertex[k].y = sinf(a) * sinf(b) - OFFSET;
				l_vVertex[k].z = sinf(a) * cosf(b);
				l_vVertex[k].u = k * 200.f / (vertexCount1 * vertexCount2);
			}
		}

		l_vTriangle.resize((vertexCount1 - 1) * (vertexCount2 - 1) * 6);
		for (i = 0, k = 0; i < vertexCount1 - 1; ++i)
		{
			for (j = 0; j < vertexCount2 - 1; j++, k += 6)
			{
				l_vTriangle[k]     = (i) * vertexCount2 + (j);
				l_vTriangle[k + 1] = (i + 1) * vertexCount2 + (j);
				l_vTriangle[k + 2] = (i + 1) * vertexCount2 + (j + 1);

				l_vTriangle[k + 3] = l_vTriangle[k];
				l_vTriangle[k + 4] = l_vTriangle[k + 2];
				l_vTriangle[k + 5] = (i) * vertexCount2 + (j + 1);
			}
		}

		m_oScalp.m_vColor[0] = 1;
		m_oScalp.m_vColor[1] = 1;
		m_oScalp.m_vColor[2] = 1;

		//		m_oScalp.compile();
	}

	{
		m_oFace.clear();

		std::vector<CVertex>& l_vVertex    = m_oFace.m_vVertex;
		std::vector<uint32_t>& l_vTriangle = m_oFace.m_vTriangle;

		// Ribbon mesh

		l_vVertex.resize(circleVertexCount * 2/*+6*/);
		for (i = 0, k = 0; i < circleVertexCount; i++, k += 2)
		{
			const auto a = float(i * 4 * M_PI / circleVertexCount);

			l_vVertex[k].x = cosf(a);
			l_vVertex[k].y = .01f;
			l_vVertex[k].z = sinf(a);

			l_vVertex[k + 1].x = cosf(a);
			l_vVertex[k + 1].y = -.01f;
			l_vVertex[k + 1].z = sinf(a);
		}

		// Nose mesh
		/*
				l_vVertex[k  ].x=-1;
				l_vVertex[k  ].y=.01;
				l_vVertex[k  ].z=-.5;
		
				l_vVertex[k+1].x=-1;
				l_vVertex[k+1].y=-.01;
				l_vVertex[k+1].z=-.5;
		
				l_vVertex[k+2].x=0;
				l_vVertex[k+2].y=.01;
				l_vVertex[k+2].z=-1.5;
		
				l_vVertex[k+3].x=0;
				l_vVertex[k+3].y=-.01;
				l_vVertex[k+3].z=-1.5;
		
				l_vVertex[k+4].x=1;
				l_vVertex[k+4].y=.01;
				l_vVertex[k+4].z=-.5;
		
				l_vVertex[k+5].x=1;
				l_vVertex[k+5].y=-.01;
				l_vVertex[k+5].z=-.5;
		*/
		// Ribon mesh

		l_vTriangle.resize(circleVertexCount * 6/*+12*/);
		for (i = 0, k = 0; i < circleVertexCount; i++, k += 6)
		{
			l_vTriangle[k]     = (i) % (circleVertexCount * 2);
			l_vTriangle[k + 1] = (i + 1) % (circleVertexCount * 2);
			l_vTriangle[k + 2] = (i + 2) % (circleVertexCount * 2);

			l_vTriangle[k + 3] = (i + 1) % (circleVertexCount * 2);
			l_vTriangle[k + 4] = (i + 2) % (circleVertexCount * 2);
			l_vTriangle[k + 5] = (i + 3) % (circleVertexCount * 2);
		}

		// Nose mesh
		/*
				l_vTriangle[k  ]=l_ui32CircleVertexCount*2;
				l_vTriangle[k+1]=l_ui32CircleVertexCount*2+1;
				l_vTriangle[k+2]=l_ui32CircleVertexCount*2+2;
		
				l_vTriangle[k+3]=l_ui32CircleVertexCount*2+1;
				l_vTriangle[k+4]=l_ui32CircleVertexCount*2+2;
				l_vTriangle[k+5]=l_ui32CircleVertexCount*2+3;
		
				l_vTriangle[k+6]=l_ui32CircleVertexCount*2+2;
				l_vTriangle[k+7]=l_ui32CircleVertexCount*2+3;
				l_vTriangle[k+8]=l_ui32CircleVertexCount*2+4;
		
				l_vTriangle[k+9]=l_ui32CircleVertexCount*2+3;
				l_vTriangle[k+10]=l_ui32CircleVertexCount*2+4;
				l_vTriangle[k+11]=l_ui32CircleVertexCount*2+5;
		*/
		m_oFace.m_vColor[0] = 1.15f;
		m_oFace.m_vColor[1] = 1.15f;
		m_oFace.m_vColor[2] = 1.15f;

		//		m_oFace.compile();
	}
}

namespace
{
	void unfold(std::vector<CVertex>& rVertex, const float fLayer = 0)
	{
		for (std::vector<CVertex>::iterator it = rVertex.begin(); it != rVertex.end(); ++it)
		{
			CVertex& p = (*it);
			p.y += OFFSET;
			const float phi = float(M_PI) * .5f - asinf(p.y);
			const float psi = atan2f(p.z, p.x);

			p.x = phi * cos(psi);
			p.y = fLayer;
			p.z = phi * sin(psi);
		}
	}
} // namespace

void CRendererTopo2D::rebuild3DMeshesPost(const IRendererContext& /*rContext*/)
{
	const float l_f32Layer = 1E-3f;

	unfold(m_oScalp.m_vVertex, -l_f32Layer);
	unfold(m_oFace.m_vVertex, l_f32Layer);
	unfold(m_vProjectedChannelCoordinate);
}
