#include <vector>

#include "../ovdAssert.h"

#include "ovdCVisualizationWidget.h"

// TODO: Remove this when SDK dependency is updated
#ifndef HAS_IMBUED_OSTREAM_WITH_C_IDENTIFIER
namespace OpenViBE
{
	std::ostream& operator<<(std::ostream& os, const CIdentifier& id) { return os << id.str(); }
}
#endif

using namespace std;
using namespace OpenViBE;
using namespace /*OpenViBE::*/Designer;
using namespace /*OpenViBE::*/Kernel;
using namespace /*OpenViBE::*/VisualizationToolkit;

bool CVisualizationWidget::initialize(const CIdentifier& id, const CString& name, const EVisualizationWidgetType type, const CIdentifier& parentID,
									  const CIdentifier& boxID, const size_t nChild)
{
	m_id       = id;
	m_name     = name;
	m_type     = type;
	m_parentID = parentID;
	m_boxID    = boxID;
	m_childrens.resize(nChild, OV_UndefinedIdentifier);
	return true;
}

bool CVisualizationWidget::getChildIndex(const CIdentifier& id, size_t& index) const
{
	for (index = 0; index < m_childrens.size(); ++index) { if (m_childrens[index] == id) { return true; } }
	return false;
}

bool CVisualizationWidget::addChild(const CIdentifier& childID)
{
	m_childrens.push_back(childID);
	return true;
}

bool CVisualizationWidget::removeChild(const CIdentifier& id)
{
	for (size_t i = 0; i < m_childrens.size(); ++i)
	{
		if (m_childrens[i] == id)
		{
			//remove tab from a window (variable number of children)
			if (m_type == VisualizationWidget_VisualizationWindow) { m_childrens.erase(m_childrens.begin() + i); }
			else //clear identifier if ith child for a regular widget (fixed number of children)
			{
				m_childrens[i] = OV_UndefinedIdentifier;
			}
			return true;
		}
	}

	OV_ERROR_DRF("Trying to remove non existing visualization widget " << id.str(), ErrorType::ResourceNotFound);
}

bool CVisualizationWidget::getChildIdentifier(const size_t index, CIdentifier& id) const
{
	if (index >= m_childrens.size())
	{
		id = OV_UndefinedIdentifier;
		OV_ERROR_DRF("Child with index " << index << " not found", ErrorType::ResourceNotFound);
	}
	id = m_childrens[index];
	return true;
}

bool CVisualizationWidget::setChildIdentifier(const size_t index, const CIdentifier& id)
{
	if (index >= m_childrens.size()) { OV_ERROR_DRF("Child with index " << index << " not found", ErrorType::ResourceNotFound); }
	m_childrens[index] = id;
	return true;
}
