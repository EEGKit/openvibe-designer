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

#if defined TARGET_HAS_ThirdPartyOpenGL

#include "mC3DMesh.hpp"

#include <system/Memory.h>

#include <cstdio>

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;


C3DMesh::C3DMesh(void)
{
	m_vColor[0]=1;
	m_vColor[1]=1;
	m_vColor[2]=1;
}

C3DMesh::C3DMesh(const char* sFilename)
{
	m_vColor[0]=1;
	m_vColor[1]=1;
	m_vColor[2]=1;

	this->load(sFilename);
}

C3DMesh::~C3DMesh(void)
{
}

void C3DMesh::clear(void)
{
	m_vColor[0]=1;
	m_vColor[1]=1;
	m_vColor[2]=1;

	m_vVertex.clear();
	m_vNormal.clear();
	m_vTriangle.clear();
}

boolean C3DMesh::load(const void* pBuffer, unsigned int uiBufferSize)
{
	const uint32* l_pBuffer=reinterpret_cast<const uint32*>(pBuffer);

	uint32 l_ui32VertexCount;
	uint32 l_ui32TriangleCount;

	System::Memory::littleEndianToHost(reinterpret_cast<const uint8*>(&l_pBuffer[0]), &l_ui32VertexCount);
	System::Memory::littleEndianToHost(reinterpret_cast<const uint8*>(&l_pBuffer[1]), &l_ui32TriangleCount);

	m_vVertex.resize(l_ui32VertexCount);
	m_vTriangle.resize(l_ui32TriangleCount*3);

	uint32 i, j=2;

	for(i=0; i<l_ui32VertexCount; i++)
	{
		System::Memory::littleEndianToHost(reinterpret_cast<const uint8*>(&l_pBuffer[j++]), &m_vVertex[i].x);
		System::Memory::littleEndianToHost(reinterpret_cast<const uint8*>(&l_pBuffer[j++]), &m_vVertex[i].y);
		System::Memory::littleEndianToHost(reinterpret_cast<const uint8*>(&l_pBuffer[j++]), &m_vVertex[i].z);
	}

	for(i=0; i<l_ui32TriangleCount*3; i++)
	{
		System::Memory::littleEndianToHost(reinterpret_cast<const uint8*>(&l_pBuffer[j++]), &m_vTriangle[i]);
	}

	this->compile();

	return true;
}

boolean C3DMesh::load(const char* sFilename)
{
	FILE* l_pFile = Mensia::Files::open(sFilename, "rb");
	if(!l_pFile) return false;

	std::vector < char > l_vBuffer;
	::fseek(l_pFile, 0, SEEK_END);
	l_vBuffer.resize(::ftell(l_pFile));
	::fseek(l_pFile, 0, SEEK_SET);
	boolean l_bResult=(::fread(&l_vBuffer[0], l_vBuffer.size(), 1, l_pFile)!=0);
	::fclose(l_pFile);

	return l_bResult && this->load(&l_vBuffer[0], l_vBuffer.size());
}

boolean C3DMesh::save(const char* sFilename) const
{
	FILE* l_pFile = Mensia::Files::open(sFilename, "wb");
	if(!l_pFile) return false;

	uint32 l_ui32VertexCount=m_vVertex.size();
	uint32 l_ui32TriangleCount=m_vTriangle.size()/3;

	std::vector < float32 > l_vVertex;
	l_vVertex.resize(l_ui32VertexCount*3);

	for(uint32 i=0; i<l_ui32VertexCount; i++)
	{
		l_vVertex[i*3  ]=m_vVertex[i].x;
		l_vVertex[i*3+1]=m_vVertex[i].y;
		l_vVertex[i*3+2]=m_vVertex[i].z;
	}

	::fwrite(&l_ui32VertexCount, 1, sizeof(uint32), l_pFile);
	::fwrite(&l_ui32TriangleCount, 1, sizeof(uint32), l_pFile);
	::fwrite(&l_vVertex[0], 1, sizeof(float32)*l_ui32VertexCount*3, l_pFile);
	::fwrite(&m_vTriangle[0], 1, sizeof(uint32)*l_ui32TriangleCount*3, l_pFile);
	::fclose(l_pFile);

	return true;
}

boolean C3DMesh::compile(void)
{
	uint32 i, i1, i2, i3;
	m_vNormal.clear();
	m_vNormal.resize(m_vVertex.size());
	for(i=0; i<m_vTriangle.size(); i+=3)
	{
		i1 = m_vTriangle[i  ];
		i2 = m_vTriangle[i+1];
		i3 = m_vTriangle[i+2];
		CVertex v1 = m_vVertex[i1];
		CVertex v2 = m_vVertex[i2];
		CVertex v3 = m_vVertex[i3];
		v2.x -= v1.x;
		v2.y -= v1.y;
		v2.z -= v1.z;
		v3.x -= v1.x;
		v3.y -= v1.y;
		v3.z -= v1.z;
		v1 = CVertex::cross(v2, v3);
		v1.normalize();
		m_vNormal[i1].x += v1.x;
		m_vNormal[i1].y += v1.y;
		m_vNormal[i1].z += v1.z;
		m_vNormal[i2].x += v1.x;
		m_vNormal[i2].y += v1.y;
		m_vNormal[i2].z += v1.z;
		m_vNormal[i3].x += v1.x;
		m_vNormal[i3].y += v1.y;
		m_vNormal[i3].z += v1.z;
	}
	for(i=0; i<m_vNormal.size(); i++)
	{
		m_vNormal[i].normalize();
	}
	return true;
}

boolean C3DMesh::project(std::vector < CVertex >& vProjectedChannelCoordinate, const std::vector < CVertex >& vChannelCoordinate)
{
	size_t i, j;

	vProjectedChannelCoordinate.resize(vChannelCoordinate.size());
	for(i=0; i<vChannelCoordinate.size(); i++)
	{
		CVertex p, q;
		p = vChannelCoordinate[i];
//		q = vChannelCoordinate[i];
		for(j=0; j<this->m_vTriangle.size(); j+=3)
		{
			uint32 i1, i2, i3;
			i1=this->m_vTriangle[j  ];
			i2=this->m_vTriangle[j+1];
			i3=this->m_vTriangle[j+2];

			CVertex v1, v2, v3;
			v1=this->m_vVertex[i1];
			v2=this->m_vVertex[i2];
			v3=this->m_vVertex[i3];

			CVertex e1(v1, v2);
			CVertex e2(v1, v3);
			CVertex n = CVertex::cross(e1, e2).normalize();

			float32 t = CVertex::dot(v1, n) / CVertex::dot(p, n);
			q.x = t * p.x;
			q.y = t * p.y;
			q.z = t * p.z;

			if(CVertex::isInTriangle(q, v1, v2, v3) && t>=0)
			{
				vProjectedChannelCoordinate[i] = q;
			}
		}
		if(q.x==0 && q.y==0 && q.z==0)
		{
//			::printf("Could not project coordinates on mesh for channel %i [%s]\n", i+1, rContext.getChannelName(i).c_str());
		}
	}
	return true;
}

#endif // TARGET_HAS_ThirdPartyOpenGL
