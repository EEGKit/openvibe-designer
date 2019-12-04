#include <vector>

#include "../ovdAssert.h"

#include "ovdCVisualizationWidget.h"

// TODO: Remove this when SDK dependency is updated
#ifndef HAS_ImbuedOStreamWithCIdentifier
namespace OpenViBE
{
	std::ostream& operator<<(std::ostream& os, const OpenViBE::CIdentifier& identifier)
	{
		return os << identifier.str().c_str();
	}
}
#endif

using namespace std;
using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace Kernel;
using namespace OpenViBEVisualizationToolkit;

CVisualizationWidget::CVisualizationWidget(const IKernelContext& ctx)
	: m_kernelCtx(ctx), m_id(OV_UndefinedIdentifier), m_Type(VisualizationWidget_Undefined),
	  m_ParentID(OV_UndefinedIdentifier), m_BoxID(OV_UndefinedIdentifier) {}


bool CVisualizationWidget::initialize(const CIdentifier& id, const CString& name, const EVisualizationWidgetType type,
									  const CIdentifier& parentID, const CIdentifier& boxID, const size_t nChild)
{
	m_id       = id;
	m_Name             = name;
	m_Type             = type;
	m_ParentID = parentID;
	m_BoxID    = boxID;
	m_Children.resize(nChild, OV_UndefinedIdentifier);
	return true;
}

bool CVisualizationWidget::getChildIndex(const CIdentifier& identifier, size_t& index) const
{
	for (index = 0; index < m_Children.size(); ++index) { if (m_Children[index] == identifier) { return true; } }
	return false;
}

bool CVisualizationWidget::addChild(const CIdentifier& childID)
{
	m_Children.push_back(childID);
	return true;
}

bool CVisualizationWidget::removeChild(const CIdentifier& id)
{
	for (uint32_t i = 0; i < m_Children.size(); ++i)
	{
		if (m_Children[i] == id)
		{
			//remove tab from a window (variable number of children)
			if (m_Type == VisualizationWidget_VisualizationWindow) { m_Children.erase(m_Children.begin() + i); }
			else //clear identifier if ith child for a regular widget (fixed number of children)
			{
				m_Children[i] = OV_UndefinedIdentifier;
			}
			return true;
		}
	}

	OV_ERROR_DRF("Trying to remove non existing visualization widget " << id.str(), ErrorType::ResourceNotFound);
}

bool CVisualizationWidget::getChildIdentifier(const size_t index, CIdentifier& id) const
{
	if (index >= m_Children.size())
	{
		id = OV_UndefinedIdentifier;
		OV_ERROR_DRF("Child with index " << index << " not found", ErrorType::ResourceNotFound);
	}
	id = m_Children[index];
	return true;
}

bool CVisualizationWidget::setChildIdentifier(const size_t index, const CIdentifier& id)
{
	if (index >= m_Children.size()) { OV_ERROR_DRF("Child with index " << index << " not found", ErrorType::ResourceNotFound); }
	m_Children[index] = id;
	return true;
}
