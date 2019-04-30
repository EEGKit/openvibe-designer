#include "mCRendererTopo2D.hpp"
#include <cmath>

using namespace Mensia;
using namespace AdvancedVisualization;

#define OFFSET .0001f

void CRendererTopo2D::rebuild3DMeshesPre(const IRendererContext& rContext)
{
	uint32_t i, j, k;

	const uint32_t l_ui32VertexCount1 = 32;
	const uint32_t l_ui32VertexCount2 = 32;
	const uint32_t l_ui32CircleVertexCount = 128;

	{
		m_oScalp.clear();

		std::vector < CVertex >& l_vVertex=m_oScalp.m_vVertex;
		std::vector < uint32_t >& l_vTriangle=m_oScalp.m_vTriangle;

		l_vVertex.resize(l_ui32VertexCount1*l_ui32VertexCount2);
		for(i=0, k=0; i<l_ui32VertexCount1; i++)
		{
			for(j=0; j<l_ui32VertexCount2; j++, k++)
			{
				float a=float(i*M_PI/(l_ui32VertexCount1-1));
				float b=float(j*1.25*M_PI/(l_ui32VertexCount2-1)-M_PI*.2);
				l_vVertex[k].x=cosf(a);
				l_vVertex[k].y=sinf(a)*sinf(b)-OFFSET;
				l_vVertex[k].z=sinf(a)*cosf(b);
				l_vVertex[k].u=k*200.f/(l_ui32VertexCount1*l_ui32VertexCount2);
			}
		}

		l_vTriangle.resize((l_ui32VertexCount1-1)*(l_ui32VertexCount2-1)*6);
		for(i=0, k=0; i<l_ui32VertexCount1-1; i++)
		{
			for(j=0; j<l_ui32VertexCount2-1; j++, k+=6)
			{
				l_vTriangle[k  ]=(i  )*l_ui32VertexCount2+(j  );
				l_vTriangle[k+1]=(i+1)*l_ui32VertexCount2+(j  );
				l_vTriangle[k+2]=(i+1)*l_ui32VertexCount2+(j+1);

				l_vTriangle[k+3]=l_vTriangle[k  ];
				l_vTriangle[k+4]=l_vTriangle[k+2];
				l_vTriangle[k+5]=(i  )*l_ui32VertexCount2+(j+1);
			}
		}

		m_oScalp.m_vColor[0]=1;
		m_oScalp.m_vColor[1]=1;
		m_oScalp.m_vColor[2]=1;

//		m_oScalp.compile();
	}

	{
		m_oFace.clear();

		std::vector < CVertex >& l_vVertex=m_oFace.m_vVertex;
		std::vector < uint32_t >& l_vTriangle=m_oFace.m_vTriangle;

		// Ribbon mesh

		l_vVertex.resize(l_ui32CircleVertexCount*2/*+6*/);
		for(i=0, k=0; i<l_ui32CircleVertexCount; i++, k+=2)
		{
			float a=float(i*4*M_PI/l_ui32CircleVertexCount);

			l_vVertex[k  ].x=cosf(a);
			l_vVertex[k  ].y=.01f;
			l_vVertex[k  ].z=sinf(a);

			l_vVertex[k+1].x=cosf(a);
			l_vVertex[k+1].y=-.01f;
			l_vVertex[k+1].z=sinf(a);
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

		l_vTriangle.resize(l_ui32CircleVertexCount*6/*+12*/);
		for(i=0, k=0; i<l_ui32CircleVertexCount; i++, k+=6)
		{
			l_vTriangle[k  ]=(i  )%(l_ui32CircleVertexCount*2);
			l_vTriangle[k+1]=(i+1)%(l_ui32CircleVertexCount*2);
			l_vTriangle[k+2]=(i+2)%(l_ui32CircleVertexCount*2);

			l_vTriangle[k+3]=(i+1)%(l_ui32CircleVertexCount*2);
			l_vTriangle[k+4]=(i+2)%(l_ui32CircleVertexCount*2);
			l_vTriangle[k+5]=(i+3)%(l_ui32CircleVertexCount*2);
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
		m_oFace.m_vColor[0]=1.15f;
		m_oFace.m_vColor[1]=1.15f;
		m_oFace.m_vColor[2]=1.15f;

//		m_oFace.compile();
	}
}

namespace
{
	void unfold(std::vector < CVertex >& rVertex, float fLayer=0)
	{
		std::vector < CVertex >::iterator it;
		for(it=rVertex.begin(); it!=rVertex.end(); it++)
		{
			CVertex& p=(*it);
			p.y += OFFSET;
			float phi=static_cast<float>(M_PI)*.5f - asinf(p.y);
			float psi=atan2f(p.z, p.x);

			p.x = phi*cos(psi);
			p.y = fLayer;
			p.z = phi*sin(psi);
		}
	}
}

void CRendererTopo2D::rebuild3DMeshesPost(const IRendererContext& rContext)
{
	const float l_f32Layer=1E-3f;

	unfold(m_oScalp.m_vVertex, -l_f32Layer);
	unfold(m_oFace.m_vVertex, l_f32Layer);
	unfold(m_vProjectedChannelCoordinate);
}
