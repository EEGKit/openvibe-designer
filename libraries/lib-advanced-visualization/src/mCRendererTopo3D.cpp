#include "mCRendererTopo3D.hpp"

#include "content/Face.obj.hpp"
#include "content/Scalp.obj.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

void CRendererTopo3D::rebuild3DMeshesPre(const IRendererContext& rContext)
{
	m_oFace.clear();
	m_oScalp.clear();

 #if 0
	m_oFace.load(Mensia::Directories::getDataDir() + "/content/Face.obj");
	m_oScalp.load(Mensia::Directories::getDataDir() + "/content/Scalp.obj");
 #else
	m_oFace.load(g_pFaceData, sizeof(g_pFaceData));
	m_oScalp.load(g_pScalpData, sizeof(g_pScalpData));
 #endif

	m_oFace.m_vColor[0]=.8f;
	m_oFace.m_vColor[1]=.6f;
	m_oFace.m_vColor[2]=.5f;
}

void CRendererTopo3D::rebuild3DMeshesPost(const IRendererContext& rContext) { }
