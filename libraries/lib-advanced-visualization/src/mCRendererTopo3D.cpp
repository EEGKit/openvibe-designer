#include "mCRendererTopo3D.hpp"

#include "content/Face.obj.hpp"
#include "content/Scalp.obj.hpp"

using namespace OpenViBE;
using namespace AdvancedVisualization;

void CRendererTopo3D::rebuild3DMeshesPre(const CRendererContext& /*ctx*/)
{
	m_face.clear();
	m_scalp.clear();

	//m_face.load(OpenViBE::Directories::getDataDir() + "/content/Face.obj");
	//m_scalp.load(OpenViBE::Directories::getDataDir() + "/content/Scalp.obj");
	m_face.load(FACE_DATA);
	m_scalp.load(SCALP_DATA);

	m_face.m_Color[0] = .8F;
	m_face.m_Color[1] = .6F;
	m_face.m_Color[2] = .5F;
}
