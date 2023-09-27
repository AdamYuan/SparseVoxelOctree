#ifndef MYVK_RG_MACRO_HPP
#define MYVK_RG_MACRO_HPP

#define MYVK_RG_OBJECT_FRIENDS template <typename, typename...> friend class myvk_rg::_details_::Pool;
#define MYVK_RG_RENDER_GRAPH_FRIENDS template <typename> friend class myvk_rg::_details_::RenderGraph;

#define MYVK_RG_FRIENDS MYVK_RG_OBJECT_FRIENDS MYVK_RG_RENDER_GRAPH_FRIENDS

#endif
