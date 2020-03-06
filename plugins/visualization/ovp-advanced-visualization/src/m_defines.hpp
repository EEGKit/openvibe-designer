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

#pragma once

// Temporal coherence definition
#define OVP_TypeId_TemporalCoherence					OpenViBE::CIdentifier(0x8F02E3F6, 0xFFB00F4B)

enum class ETemporalCoherence { TimeLocked, Independant };

#define OV_TypeId_TimeFrequency							OpenViBE::CIdentifier(0x5A90816B, 0xFF2AFF72)
