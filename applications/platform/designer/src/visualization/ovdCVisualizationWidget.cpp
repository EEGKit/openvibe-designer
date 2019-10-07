#include <vector>

#include "../ovdAssert.h"

#include "ovdCVisualizationWidget.h"

// TODO: Remove this when SDK dependency is updated
#ifndef HAS_ImbuedOStreamWithCIdentifier
namespace OpenViBE
{
	std::ostream& operator<<(std::ostream& os, const OpenViBE::CIdentifier& identifier)
	{
		return os << identifier.toString().toASCIIString();
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


bool CVisualizationWidget::initialize(const CIdentifier& identifier, const CString& name, const EVisualizationWidgetType type,
									  const CIdentifier& parentIdentifier, const CIdentifier& boxID, const uint32_t childCount)
{
	m_id       = identifier;
	m_Name             = name;
	m_Type             = type;
	m_ParentID = parentIdentifier;
	m_BoxID    = boxID;
	m_Children.resize(childCount, OV_UndefinedIdentifier);
	return true;
}

bool CVisualizationWidget::getChildIndex(const CIdentifier& identifier, uint32_t& index) const
{
	for (index = 0; index < m_Children.size(); ++index) { if (m_Children[index] == identifier) { return true; } }
	return false;
}

bool CVisualizationWidget::addChild(const CIdentifier& childIdentifier)
{
	m_Children.push_back(childIdentifier);
	return true;
}

bool CVisualizationWidget::removeChild(const CIdentifier& identifier)
{
	for (uint32_t i = 0; i < m_Children.size(); ++i)
	{
		if (m_Children[i] == identifier)
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

	OV_ERROR_DRF("Trying to remove non existing visualization widget " << identifier, ErrorType::ResourceNotFound);
}

bool CVisualizationWidget::getChildIdentifier(const uint32_t childIndex, CIdentifier& identifier) const
{
	if (childIndex >= m_Children.size())
	{
		identifier = OV_UndefinedIdentifier;
		OV_ERROR_DRF("Child with index " << childIndex << " not found", ErrorType::ResourceNotFound);
	}
	identifier = m_Children[childIndex];
	return true;
}

bool CVisualizationWidget::setChildIdentifier(const uint32_t childIndex, const CIdentifier& identifier)
{
	if (childIndex >= m_Children.size()) { OV_ERROR_DRF("Child with index " << childIndex << " not found", ErrorType::ResourceNotFound); }
	m_Children[childIndex] = identifier;
	return true;
}
