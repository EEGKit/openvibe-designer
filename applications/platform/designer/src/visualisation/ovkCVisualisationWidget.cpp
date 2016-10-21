#include <vector>

#include "../ovdAssert.h"

#include "ovkCVisualisationWidget.h"

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

CVisualisationWidget::CVisualisationWidget(const IKernelContext& kernelContext)
    : m_KernelContext(kernelContext)
    , m_Identifier(OV_UndefinedIdentifier)
    , m_Type(EVisualisationWidget_Undefined)
    , m_ParentIdentifier(OV_UndefinedIdentifier)
    , m_BoxIdentifier(OV_UndefinedIdentifier)
{
}

CVisualisationWidget::~CVisualisationWidget(void)
{
}

bool CVisualisationWidget::initialize(const CIdentifier& identifier, const CString& name, EVisualisationWidgetType type,
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

CIdentifier CVisualisationWidget::getIdentifier(void) const
{
	return m_Identifier;
}

const CString& CVisualisationWidget::getName(void) const
{
	return m_Name;
}

void CVisualisationWidget::setName(const CString& name)
{
	m_Name = name;
}

EVisualisationWidgetType CVisualisationWidget::getType(void) const
{
	return m_Type;
}

CIdentifier CVisualisationWidget::getParentIdentifier(void) const
{
	return m_ParentIdentifier;
}

void CVisualisationWidget::setParentIdentifier(const CIdentifier& parentIdentifier)
{
	m_ParentIdentifier = parentIdentifier;
}

CIdentifier CVisualisationWidget::getBoxIdentifier(void) const
{
	return m_BoxIdentifier;
}

uint32 CVisualisationWidget::getNbChildren(void) const
{
	return static_cast<uint32>(m_Children.size());
}

bool CVisualisationWidget::getChildIndex(const CIdentifier& identifier, uint32& index) const
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

bool CVisualisationWidget::addChild(const CIdentifier& childIdentifier)
{
	m_Children.push_back(childIdentifier);
	return true;
}

bool CVisualisationWidget::removeChild(const CIdentifier& identifier)
{
	for (unsigned int i = 0; i < m_Children.size(); i++)
	{
		if (m_Children[i] == identifier)
		{
			//remove tab from a window (variable number of children)
			if (m_Type == EVisualisationWidget_VisualisationWindow)
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

bool CVisualisationWidget::getChildIdentifier(uint32 childIndex, CIdentifier& identifier) const
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

bool CVisualisationWidget::setChildIdentifier(uint32 childIndex, const CIdentifier& identifier)
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
