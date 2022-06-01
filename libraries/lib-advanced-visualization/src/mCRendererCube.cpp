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
#include "mCRendererCube.hpp"
#include "m_RendererTools.hpp"

#include <cmath>

namespace OpenViBE {
namespace AdvancedVisualization {

CRendererCube::CRendererCube() = default;

void CRendererCube::rebuild(const CRendererContext& ctx)
{
	IRenderer::rebuild(ctx);

	m_vertices.clear();
	m_vertices.resize(ctx.getChannelCount());

	m_historyIdx = 0;
}

void CRendererCube::refresh(const CRendererContext& ctx)
{
	IRenderer::refresh(ctx);

	if (!m_nHistory) { return; }

	const float indexERP   = (m_erpFraction * float(m_nSample - 1));
	const float alpha      = indexERP - std::floor(indexERP);
	const size_t indexERP1 = size_t(indexERP) % m_nSample;
	const size_t indexERP2 = size_t(indexERP + 1) % m_nSample;

	for (size_t i = 0; i < m_vertices.size(); ++i) {
		m_vertices[i].u = m_history[i][m_nHistory - m_nSample + indexERP1] * (1 - alpha) + m_history[i][m_nHistory - m_nSample + indexERP2] * (alpha);
	}

	m_historyIdx = m_nHistory;
}

bool CRendererCube::render(const CRendererContext& ctx)
{
	if (!ctx.getSelectedCount() || m_vertices.empty() || !m_nHistory) { return false; }

	const float d = 3.5;

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(60, double(ctx.getAspect()), .01, 100);
	glTranslatef(0, 0, -d);
	glRotatef(ctx.getRotationX() * 10, 1, 0, 0);
	glRotatef(ctx.getRotationY() * 10, 0, 1, 0);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glScalef(ctx.getScale(), 1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glScalef(ctx.getZoom(), ctx.getZoom(), ctx.getZoom());

	glPushMatrix();
	glRotatef(19, 1, 0, 0);
	for (size_t j = 0; j < ctx.getSelectedCount(); ++j) {
		CVertex v;
		const size_t k = ctx.getSelected(j);
		ctx.getChannelLocalisation(k, v.x, v.y, v.z);
		const float scale = 0.1F * (0.25F + fabs(m_vertices[k].u * ctx.getScale()));

		glPushMatrix();
		glTranslatef(v.x, v.y, v.z);
		glTexCoord1f(m_vertices[k].u);
		glScalef(scale, scale, scale);

		glColor3f(1, 1, 1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		cube();

		glColor3f(0, 0, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		cube();

		glPopMatrix();
	}
	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (ctx.getCheckBoardVisibility()) { this->drawCoordinateSystem(); }

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	return true;
}

}  // namespace AdvancedVisualization
}  // namespace OpenViBE
