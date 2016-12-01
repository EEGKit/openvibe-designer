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

#pragma once

#include <mensia/advanced-visualization.h>

#include <map>

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CBoxAlgorithmViz;

		class CMouse
		{
		public:

			CMouse(CBoxAlgorithmViz& rBoxAlgorithmViz);
			void mouseButton(IRendererContext& rContext, int32_t x, int32_t y, int32_t button, int status);
			void mouseMotion(IRendererContext& rContext, int32_t x, int32_t y);
			bool hasButtonPressed(void);

			CBoxAlgorithmViz& m_rBoxAlgorithmViz;
			std::map < int32_t, int > m_vButton;
			int32_t m_i32MouseX;
			int32_t m_i32MouseY;
		};
	};
};

