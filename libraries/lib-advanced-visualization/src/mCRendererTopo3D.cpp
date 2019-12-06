#include "mCRendererTopo3D.hpp"

#include "content/Face.obj.hpp"
#include "content/Scalp.obj.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

void CRendererTopo3D::rebuild3DMeshesPre(const IRendererContext& /*rContext*/)
{
	m_face.clear();
	m_scalp.clear();

#if 0
	m_face.load(Mensia::Directories::getDataDir() + "/content/Face.obj");
	m_scalp.load(Mensia::Directories::getDataDir() + "/content/Scalp.obj");
#else
	m_face.load(g_pFaceData, sizeof(g_pFaceData));
	m_scalp.load(g_pScalpData, sizeof(g_pScalpData));
#endif

	m_face.m_vColor[0] = .8f;
	m_face.m_vColor[1] = .6f;
	m_face.m_vColor[2] = .5f;
}
