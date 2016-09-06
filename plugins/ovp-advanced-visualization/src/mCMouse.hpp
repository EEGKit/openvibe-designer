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

#ifndef __OpenViBEPlugins_CMouse_H__
#define __OpenViBEPlugins_CMouse_H__

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
			void mouseButton(IRendererContext& rContext, int32 x, int32 y, int32 button, int status);
			void mouseMotion(IRendererContext& rContext, int32 x, int32 y);
			boolean hasButtonPressed(void);

			CBoxAlgorithmViz& m_rBoxAlgorithmViz;
			std::map < int32, int > m_vButton;
			int32 m_i32MouseX;
			int32 m_i32MouseY;
		};
	};
};

#endif // __OpenViBEPlugins_CMouse_H__
