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

#include "mIRendererContext.h"
#include "mCRendererContext.hpp"

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;

IRendererContext* IRendererContext::create(IRendererContext* pParentRendererContext)
{
	return new CRendererContext(pParentRendererContext);
}

void IRendererContext::release(IRendererContext* pRendererContext)
{
	delete pRendererContext;
}
