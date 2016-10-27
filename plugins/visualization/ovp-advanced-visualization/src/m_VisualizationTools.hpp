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

#ifndef __OpenViBEPlugins_Tools_H__
#define __OpenViBEPlugins_Tools_H__

#include "m_defines.hpp"

#include <mensia/advanced-visualization.h>

#include <string>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		std::string sanitize(const std::string& sValue);
		IRendererContext& getContext(void);
	};
};

#endif // __OpenViBEPlugins_Tools_H__
