/*********************************************************************
 * Software License Agreement (AGPL-3 License)
 *
 * OpenViBE Designer
 * Based on OpenViBE V1.1.0, Copyright (C) Inria, 2006-2015
 * Copyright (C) Inria, 2015-2017,V1.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "m_VisualizationTools.hpp"

using namespace Mensia;
using namespace AdvancedVisualization;

std::string AdvancedVisualization::trim(const std::string& sValue)
{
	if (sValue.length() == 0) { return ""; }
	size_t i = 0;
	size_t j = sValue.length() - 1;
	while (i < sValue.length() && sValue[i] == ' ') { i++; }
	while (j > i && sValue[j] == ' ') { j--; }
	return sValue.substr(i, j - i + 1);
}

CRendererContext& AdvancedVisualization::getContext() 
{
	static CRendererContext* ctx = new CRendererContext();
	return *ctx;
 }
