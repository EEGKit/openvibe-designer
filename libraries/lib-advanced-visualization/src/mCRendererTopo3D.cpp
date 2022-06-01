#include "mCRendererTopo3D.hpp"

#include "content/Face.obj.hpp"
#include "content/Scalp.obj.hpp"

namespace OpenViBE {
namespace AdvancedVisualization {

void CRendererTopo3D::rebuild3DMeshesPre(const CRendererContext& /*ctx*/)
{
	m_face.clear();
	m_scalp.clear();

	//m_face.load(OpenViBE::Directories::getDataDir() + "/content/Face.obj");
	//m_scalp.load(OpenViBE::Directories::getDataDir() + "/content/Scalp.obj");
	m_face.load(FACE_DATA.data());
	m_scalp.load(SCALP_DATA.data());

	m_face.m_Color[0] = 0.8F;
	m_face.m_Color[1] = 0.6F;
	m_face.m_Color[2] = 0.5F;
}

}  // namespace AdvancedVisualization
}  // namespace OpenViBE
