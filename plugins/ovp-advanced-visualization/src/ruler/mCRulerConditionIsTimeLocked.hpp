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

#ifndef __OpenViBEPlugins_TRulerConditionIsTimeLocked_H__
#define __OpenViBEPlugins_TRulerConditionIsTimeLocked_H__

#include "../mIRuler.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		class CRulerConditionIsTimeLocked : public IRuler
		{
		public:

			CRulerConditionIsTimeLocked(void)
				:m_pRendererContext(NULL)
				,m_pRenderer(NULL)
			{
			}

			virtual void setRendererContext(const IRendererContext* pRendererContext)
			{
				m_pRendererContext=pRendererContext;
			}

			virtual void setRenderer(const IRenderer* pRenderer)
			{
				m_pRenderer=pRenderer;
			}

			boolean operator()(void)
			{
				return m_pRendererContext->isTimeLocked();
			}

			const IRendererContext* m_pRendererContext;
			const IRenderer* m_pRenderer;
		};
	};
};

#endif // __OpenViBEPlugins_TRulerConditionIsTimeLocked_H__
