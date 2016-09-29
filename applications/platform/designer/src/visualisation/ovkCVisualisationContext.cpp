#include "ovkCVisualisationContext.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;

CVisualisationContext::CVisualisationContext(const IKernelContext& rKernelContext, const IVisualisationManager& rVisualisationManager, CIdentifier oVisualisationTreeIdentifier)
	: m_rKernelContext(rKernelContext)
	, m_rVisualisationManager(rVisualisationManager)
    , m_oVisualisationTreeIdentifier(oVisualisationTreeIdentifier)
{
}

CVisualisationContext::~CVisualisationContext(void)
{
}

bool CVisualisationContext::setToolbar(::GtkWidget* pToolbarWidget)
{
	CIdentifier l_oBoxIdentifier;
	/*
	m_pSimulatedBox->getBoxIdentifier(l_oBoxIdentifier);

	return m_pVisualisationManager->setToolbar(
		m_pSimulatedBox->getScenario().getVisualisationTreeIdentifier(),
		l_oBoxIdentifier,
		pToolbarWidget);
		*/
	return true;
}

bool CVisualisationContext::setWidget(::GtkWidget* pTopmostWidget)
{
	CIdentifier l_oBoxIdentifier;
//	m_pSimulatedBox->getBoxIdentifier(l_oBoxIdentifier);

//	return m_pVisualisationManager->setWidget(
//		m_pSimulatedBox->getScenario().getVisualisationTreeIdentifier(),
//		l_oBoxIdentifier,
//		pTopmostWidget);

	return true;
}
