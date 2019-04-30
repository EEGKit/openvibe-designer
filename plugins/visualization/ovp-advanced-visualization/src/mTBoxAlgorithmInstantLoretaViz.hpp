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

#include "mTBoxAlgorithmInstantViz.hpp"

namespace Mensia
{
	namespace AdvancedVisualization
	{
		template <class TRendererFactoryClass, class TRulerClass>
		class TBoxAlgorithmInstantLoretaViz : public TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>
		{
		public:

			TBoxAlgorithmInstantLoretaViz(const OpenViBE::CIdentifier& rClassId, const std::vector < int >& vParameter)
				:TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>(rClassId, vParameter) { }

			static void callback(GtkTreeSelection* pTreeSelection, TBoxAlgorithmInstantLoretaViz<TRendererFactoryClass, TRulerClass>* pBox)
			{
				pBox->region_selection_changed(pTreeSelection);
			}

			void region_selection_changed(GtkTreeSelection* pTreeSelection)
			{
				m_pRenderer->clearRegionSelection();

				GtkTreeIter l_oIter;

				char* l_sValue=nullptr;
				for(uint32_t i=0; i<3; i++)
				{
					GtkTreeSelection* l_pTreeSelection=gtk_tree_view_get_selection(m_pLookupTreeView[i]);

					if(l_pTreeSelection==pTreeSelection)
					{
						if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_pLookupListStore[i]), &l_oIter))
						{
							do
							{
								if(gtk_tree_selection_iter_is_selected(l_pTreeSelection, &l_oIter))
								{
									gtk_tree_model_get(GTK_TREE_MODEL(m_pLookupListStore[i]), &l_oIter, 0, &l_sValue, -1);
									m_pRenderer->selectRegion(i, l_sValue);
								}
							}
							while(gtk_tree_model_iter_next(GTK_TREE_MODEL(m_pLookupListStore[i]), &l_oIter));
						}
					}
					else
					{
						gtk_tree_selection_unselect_all(l_pTreeSelection);
					}
				}
			}

			void fill_region(GtkListStore* pListStore, GtkTreeView* pTreeView, IRenderer* pRenderer, uint32_t ui32RegionCategory)
			{
				GtkTreeIter l_oGtkTreeIterator;

				gtk_list_store_clear(pListStore);
				gtk_tree_selection_set_mode(gtk_tree_view_get_selection(pTreeView), GTK_SELECTION_MULTIPLE);
				for(uint32_t i=0; i<pRenderer->getRegionCount(ui32RegionCategory); i++)
				{
					const char* l_sName=m_pRenderer->getRegionName(ui32RegionCategory, i);
					gtk_list_store_append((pListStore), &l_oGtkTreeIterator);
					gtk_list_store_set(pListStore, &l_oGtkTreeIterator, 0, l_sName?l_sName:"", -1);
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(pTreeView), &l_oGtkTreeIterator);
				}
			}

			using TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::m_vRenderer;
			using TBoxAlgorithmInstantViz<TRendererFactoryClass, TRulerClass>::m_pBuilder;

			bool initialize()

			{
				TBoxAlgorithmInstantViz < TRendererFactoryClass, TRulerClass >::initialize();

				m_pRenderer=m_vRenderer[0];

				gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "expander_select")));
				gtk_widget_show(GTK_WIDGET(::gtk_builder_get_object(m_pBuilder, "expander_select_sLORETA")));

				m_pLookupTreeView[0]=GTK_TREE_VIEW(::gtk_builder_get_object(m_pBuilder, "expander_select_broadmann_treeview"));
				m_pLookupTreeView[1]=GTK_TREE_VIEW(::gtk_builder_get_object(m_pBuilder, "expander_select_neuro_1_treeview"));
				m_pLookupTreeView[2]=GTK_TREE_VIEW(::gtk_builder_get_object(m_pBuilder, "expander_select_neuro_2_treeview"));
				m_pLookupListStore[0]=GTK_LIST_STORE(::gtk_builder_get_object(m_pBuilder, "liststore_select_broadmann"));
				m_pLookupListStore[1]=GTK_LIST_STORE(::gtk_builder_get_object(m_pBuilder, "liststore_select_neuro_1"));
				m_pLookupListStore[2]=GTK_LIST_STORE(::gtk_builder_get_object(m_pBuilder, "liststore_select_neuro_2"));

				for(uint32_t i=0; i<m_pRenderer->getRegionCategoryCount() && i<3; i++)
				{
					this->fill_region(m_pLookupListStore[i], m_pLookupTreeView[i], m_pRenderer, i);
				}

				g_signal_connect(::gtk_tree_view_get_selection(m_pLookupTreeView[0]), "changed", G_CALLBACK(callback), this);
				g_signal_connect(::gtk_tree_view_get_selection(m_pLookupTreeView[1]), "changed", G_CALLBACK(callback), this);
				g_signal_connect(::gtk_tree_view_get_selection(m_pLookupTreeView[2]), "changed", G_CALLBACK(callback), this);

				return true;
			}

			IRenderer* m_pRenderer;
			GtkTreeView* m_pLookupTreeView[3];
			GtkListStore* m_pLookupListStore[3];
		};
	};
};

