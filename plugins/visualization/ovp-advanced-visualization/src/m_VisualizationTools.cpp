/*
 * MENSIA TECHNOLOGIES CONFIDENTIAL
 * ________________________________
 *
 *  [2012] - [2013] Mensia Technologies SA
 *  Copyright, All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Mensia Technologies SA.
 * The intellectual and technical concepts contained
 * herein are proprietary to Mensia Technologies SA
 * and are covered copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Mensia Technologies SA.
 */

#include "m_VisualizationTools.hpp"

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;

std::string Mensia::AdvancedVisualization::trim(const std::string& sValue)
{
	if(sValue.length()==0) return "";
	size_t i=0;
	size_t j=sValue.length()-1;
	while(i<sValue.length() && sValue[i]==' ') i++;
	while(j>i && sValue[j]==' ') j--;
	return sValue.substr(i, j-i+1);
}

IRendererContext& Mensia::AdvancedVisualization::getContext(void)
{
	static IRendererContext* l_pRendererContext=IRendererContext::create();
	return *l_pRendererContext;
}
