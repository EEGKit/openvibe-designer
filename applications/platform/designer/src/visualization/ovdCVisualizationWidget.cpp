#include <vector>

#include "../ovdAssert.h"

#include "ovdCVisualizationWidget.h"

namespace OpenViBE
{
	std::ostream& operator<<(std::ostream& os, const OpenViBE::CIdentifier& identifier)
	{
		return os << identifier.toString().toASCIIString();
	}
}


using namespace std;
using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace OpenViBE::Kernel;
using namespace OpenViBEVisualizationToolkit;

CVisualizationWidget::CVisualizationWidget(const IKernelContext& kernelContext)
    : m_KernelContext(kernelContext)
    , m_Identifier(OV_UndefinedIdentifier)
    , m_Type(EVisualizationWidget_Undefined)
    , m_ParentIdentifier(OV_UndefinedIdentifier)
    , m_BoxIdentifier(OV_UndefinedIdentifier)
{
}

CVisualizationWidget::~CVisualizationWidget(void)
{
}

bool CVisualizationWidget::initialize(const CIdentifier& identifier, const CString& name, EVisualizationWidgetType type,
	const CIdentifier& parentIdentifier, const CIdentifier& boxIdentifier, uint32 childCount)
{
	m_Identifier = identifier;
	m_Name = name;
	m_Type = type;
	m_ParentIdentifier = parentIdentifier;
	m_BoxIdentifier = boxIdentifier;
	m_Children.resize(childCount, OV_UndefinedIdentifier);
	return true;
}

CIdentifier CVisualizationWidget::getIdentifier(void) const
{
	return m_Identifier;
}

const CString& CVisualizationWidget::getName(void) const
{
	return m_Name;
}

void CVisualizationWidget::setName(const CString& name)
{
	m_Name = name;
}

EVisualizationWidgetType CVisualizationWidget::getType(void) const
{
	return m_Type;
}

CIdentifier CVisualizationWidget::getParentIdentifier(void) const
{
	return m_ParentIdentifier;
}

void CVisualizationWidget::setParentIdentifier(const CIdentifier& parentIdentifier)
{
	m_ParentIdentifier = parentIdentifier;
}

CIdentifier CVisualizationWidget::getBoxIdentifier(void) const
{
	return m_BoxIdentifier;
}

uint32 CVisualizationWidget::getNbChildren(void) const
{
	return static_cast<uint32>(m_Children.size());
}

bool CVisualizationWidget::getChildIndex(const CIdentifier& identifier, uint32& index) const
{
	for (index = 0; index < m_Children.size(); index++)
	{
		if (m_Children[index] == identifier)
		{
			return true;
		}
	}
	return false;
}

bool CVisualizationWidget::addChild(const CIdentifier& childIdentifier)
{
	m_Children.push_back(childIdentifier);
	return true;
}

bool CVisualizationWidget::removeChild(const CIdentifier& identifier)
{
	for (unsigned int i = 0; i < m_Children.size(); i++)
	{
		if (m_Children[i] == identifier)
		{
			//remove tab from a window (variable number of children)
			if (m_Type == EVisualizationWidget_VisualizationWindow)
			{
				m_Children.erase(m_Children.begin() + i);
			}
			else //clear identifier if ith child for a regular widget (fixed number of children)
			{
				m_Children[i] = OV_UndefinedIdentifier;
			}
			return true;
		}
	}

	OV_ERROR_DRF("Trying to remove non existing visualization widget " << identifier,
	             ErrorType::ResourceNotFound);
}

bool CVisualizationWidget::getChildIdentifier(uint32 childIndex, CIdentifier& identifier) const
{
	if (childIndex >= m_Children.size())
	{
		identifier = OV_UndefinedIdentifier;
		OV_ERROR_DRF("Child with index " << childIndex << " not found",
		             ErrorType::ResourceNotFound);
	}
	else
	{
		identifier = m_Children[childIndex];
		return true;
	}
}

bool CVisualizationWidget::setChildIdentifier(uint32 childIndex, const CIdentifier& identifier)
{
	if (childIndex >= m_Children.size())
	{
		OV_ERROR_DRF("Child with index " << childIndex << " not found",
		             ErrorType::ResourceNotFound);
	}
	else
	{
		m_Children[childIndex] = identifier;
		return true;
	}
}
