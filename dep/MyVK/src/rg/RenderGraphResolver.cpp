#include "RenderGraphResolver.hpp"

#include <algorithm>
#include <list>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace myvk_rg::_details_ {

struct RenderGraphResolver::OriginGraph {
	enum class ExtraEdgeType : uint8_t { kNone, kWriteAfterRead, kExternalOutput, kDeleted };
	struct Edge {
		const ResourceBase *resource{};
		ResourceReference from{}, to{};
		DependencyType type{};
		ExtraEdgeType extra_type{};
	};
	struct Node {
		std::vector<Edge *> input_edges, output_edges;
		uint32_t in_degree{};
	};
	std::unordered_map<const InternalImageBase *, bool> internal_image_set;
	std::unordered_set<const ManagedBuffer *> internal_buffer_set;
	std::unordered_map<const PassBase *, Node> nodes;
	std::list<Edge> edges;

	inline void add_edge(const ResourceBase *resource, const ResourceReference &from, const ResourceReference &to,
	                     DependencyType type, ExtraEdgeType extra_type = ExtraEdgeType::kNone) {
		assert(bool(from.pass) == bool(from.p_input));
		assert(bool(to.pass) == bool(to.p_input));
		edges.push_back({resource, from, to, type, extra_type});
		auto *edge = &edges.back();
		nodes[from.pass].output_edges.push_back(edge);
		if (to.pass) {
			nodes[to.pass].input_edges.push_back(edge);
			++nodes[to.pass].in_degree;
		}
	}
	inline void add_internal_buffer(const ManagedBuffer *buffer) { internal_buffer_set.insert(buffer); }
	inline void add_internal_image(const InternalImageBase *image, bool set_has_parent = false) {
		auto it = internal_image_set.find(image);
		if (it == internal_image_set.end())
			internal_image_set[image] = set_has_parent;
		else if (set_has_parent)
			it->second = true;
	}
	inline void visit_resource_dep_passes(const ResourceBase *resource, const PassBase *pass, const Input *p_input) {
		// TODO: Handle LastFrame Images and Buffers
		const auto add_visitor_edge = [this, pass, p_input](const ResourceBase *resource, const PassBase *dep_pass,
		                                                    const Input *p_dep_input, DependencyType type) -> void {
			if (!pass || !p_input)
				return;
			assert(resource);
			add_edge(resource, {p_dep_input, dep_pass}, {p_input, pass}, type);
		};
		const auto add_edge_and_visit_dep_pass = [this, &add_visitor_edge](const ResourceBase *resource,
		                                                                   const PassBase *dep_pass,
		                                                                   const Input *p_dep_input) -> void {
			bool not_visited = nodes.find(dep_pass) == nodes.end();
			assert(dep_pass);
			if (dep_pass) { // TODO: Remove these ifs'
				assert(p_dep_input);
				add_visitor_edge(resource, dep_pass, p_dep_input, DependencyType::kDependency);
				if (not_visited) {
					// Further Traverse dep_pass's dependent Resources
					dep_pass->for_each_input([this, dep_pass](const Input *p_input) {
						visit_resource_dep_passes(p_input->GetResource(), dep_pass, p_input);
					});
				}
			}
		};
		resource->Visit([this, &add_edge_and_visit_dep_pass, &add_visitor_edge](auto *resource) -> void {
			constexpr auto kClass = ResourceVisitorTrait<decltype(resource)>::kClass;
			// For CombinedImage, further For Each its Child Images
			if constexpr (kClass == ResourceClass::kCombinedImage) {
				add_internal_image(resource);
				// Visit Each SubImage
				resource->ForEachImage([this, &add_edge_and_visit_dep_pass](auto *sub_image) -> void {
					sub_image->GetPointedResource()->Visit([this](const auto *pointed_sub_image) {
						if constexpr (ResourceVisitorTrait<decltype(pointed_sub_image)>::kIsInternal)
							add_internal_image(pointed_sub_image, true);
						else
							assert(false);
					});
					add_edge_and_visit_dep_pass(sub_image->GetPointedResource(), sub_image->GetProducerPass(),
					                            sub_image->GetProducerInput());
				});
			} else {
				if constexpr (kClass == ResourceClass::kManagedImage) {
					add_internal_image(resource);
					add_visitor_edge(resource, nullptr, nullptr, DependencyType::kValidation);
				} else if constexpr (kClass == ResourceClass::kManagedBuffer) {
					add_internal_buffer(resource);
					add_visitor_edge(resource, nullptr, nullptr, DependencyType::kValidation);
				} else if constexpr (kClass == ResourceClass::kLastFrameImage) {
					add_internal_image(resource->GetCurrentResource());
					add_visitor_edge(resource, nullptr, nullptr,
					                 DependencyType::kLastFrame); // Last Frame Edge
				} else if constexpr (kClass == ResourceClass::kLastFrameBuffer) {
					add_internal_buffer(resource->GetCurrentResource());
					add_visitor_edge(resource, nullptr, nullptr,
					                 DependencyType::kLastFrame); // Last Frame Edge
				} else if constexpr (kClass == ResourceClass::kExternalImageBase ||
				                     kClass == ResourceClass::kExternalBufferBase) {
					add_visitor_edge(resource, nullptr, nullptr, DependencyType::kExternal);
				} else if constexpr (GetResourceState(kClass) == ResourceState::kAlias) {
					add_edge_and_visit_dep_pass(resource->GetPointedResource(), resource->GetProducerPass(),
					                            resource->GetProducerInput());
				}
			}
		});
	}
	inline void insert_write_after_read_edges() {
		// Dependency Edges
		for (auto &pair : nodes) {
			std::unordered_map<const ResourceBase *, const Input *> write_inputs;
			std::unordered_map<const ResourceBase *, Edge *> write_outputs;
			const PassBase *pass = pair.first;
			auto &node = pair.second;

			for (auto *p_edge : node.input_edges) {
				if (p_edge->extra_type == ExtraEdgeType::kNone) {
					if (UsageIsReadOnly(p_edge->to.p_input->GetUsage()))
						continue;
					assert(write_inputs.find(p_edge->resource) == write_inputs.end());
					write_inputs[p_edge->resource] = p_edge->to.p_input;
				}
			}
			for (auto *p_edge : node.output_edges) {
				if (p_edge->extra_type == ExtraEdgeType::kNone) {
					write_inputs.erase(p_edge->resource);

					if (UsageIsReadOnly(p_edge->to.p_input->GetUsage()))
						continue;
					assert(write_outputs.find(p_edge->resource) ==
					       write_outputs.end()); // An output can only be written once
					write_outputs[p_edge->resource] = p_edge;
				}
			}

			// inputs contains all the input-only resources, should be their last use
			for (const auto &it : write_inputs) {
				const auto *resource = it.first;
				const auto *p_input = it.second;
				if (resource->GetState() == ResourceState::kExternal)
					add_edge(resource, {p_input, pass}, {nullptr, nullptr}, DependencyType::kExternal,
					         ExtraEdgeType::kExternalOutput);
			}

			for (const auto *p_edge : node.output_edges) {
				if (p_edge->extra_type == ExtraEdgeType::kNone && UsageIsReadOnly(p_edge->to.p_input->GetUsage())) {
					auto it = write_outputs.find(p_edge->resource);
					if (it != write_outputs.end()) {
						if (p_edge->resource->GetType() == ResourceType::kBuffer) {
							add_edge(p_edge->resource, p_edge->to, it->second->to, DependencyType::kDependency,
							         ExtraEdgeType::kWriteAfterRead);
							it->second->extra_type = ExtraEdgeType::kDeleted;
							// Delete the direct write edge if a Write-After-Read edge exists (Buffer)
						} else {
							add_edge(p_edge->resource, p_edge->to, it->second->to, DependencyType::kDependency,
							         ExtraEdgeType::kDeleted);
						}
					} else if (p_edge->resource->GetClass() == ResourceClass::kExternalBufferBase) {
						// For External Buffers, if no writes, then add external edges
						add_edge(p_edge->resource, p_edge->to, {nullptr, nullptr}, DependencyType::kExternal,
						         ExtraEdgeType::kExternalOutput);
					}
				}
			}
		}
	}
};

RenderGraphResolver::OriginGraph RenderGraphResolver::make_origin_graph(const RenderGraphBase *p_render_graph) {
	OriginGraph graph = {};
	for (auto it = p_render_graph->m_p_result_pool_data->pool.begin();
	     it != p_render_graph->m_p_result_pool_data->pool.end(); ++it)
		graph.visit_resource_dep_passes(*p_render_graph->m_p_result_pool_data->ValueGet<0, const ResourceBase *>(it),
		                                nullptr, nullptr);
	return graph;
}

void RenderGraphResolver::_initialize_combined_image(const CombinedImage *image) {
	// Visit Each Child Image, Update Size and Base Layer
	image->ForEachExpandedImage([image](auto *sub_image) -> void {
		if constexpr (ResourceVisitorTrait<decltype(sub_image)>::kIsCombinedOrManagedImage) {
			sub_image->m_resolved_info.image_id = image->m_resolved_info.image_id;
			// sub_image->m_internal_info.parent = parent;
			// Merge the Size of the Current Child Image
			if constexpr (ResourceVisitorTrait<decltype(sub_image)>::kClass == ResourceClass::kCombinedImage)
				_initialize_combined_image(sub_image); // Further Query SubImage Size
		}
	});
}

void RenderGraphResolver::extract_resources(const OriginGraph &graph) {
	{
		m_internal_buffers.clear();
		m_internal_buffers.reserve(graph.internal_buffer_set.size());
		for (auto *buffer : graph.internal_buffer_set) {
			buffer->m_resolved_info.buffer_id = m_internal_buffers.size();
			m_internal_buffers.emplace_back();
			m_internal_buffers.back().buffer = buffer;
		}
	}
	{
		m_internal_images.clear();
		m_internal_image_views.clear();
		m_internal_image_views.reserve(graph.internal_image_set.size());
		for (auto &it : graph.internal_image_set) {
			it.first->Visit([this](auto *image) -> void {
				if constexpr (ResourceVisitorTrait<decltype(image)>::kIsInternal) {
					image->m_resolved_info.image_view_id = m_internal_image_views.size();
					m_internal_image_views.emplace_back();
					m_internal_image_views.back().image = image;
				}
			});
		}
		for (auto &it : graph.internal_image_set) {
			if (!it.second)
				it.first->Visit([this](auto *image) -> void {
					if constexpr (ResourceVisitorTrait<decltype(image)>::kIsInternal) {
						image->m_resolved_info.image_id = m_internal_images.size();
						m_internal_images.emplace_back();
						m_internal_images.back().image = image;
						if constexpr (ResourceVisitorTrait<decltype(image)>::kClass == ResourceClass::kCombinedImage)
							_initialize_combined_image(image);
					}
				});
		}
	}
	// Compute Image View Relation
	// TODO: Optimize this with ApplyRelations, or REMOVE it
	/*m_image_view_contain_relation.Reset(GetIntImageViewCount(), GetIntImageViewCount());
	for (uint32_t image_view_id = 0; image_view_id < GetIntImageViewCount(); ++image_view_id) {
	    m_image_view_contain_relation.SetRelation(image_view_id, image_view_id);
	    m_internal_image_views[image_view_id].image->Visit([this, image_view_id](const auto *image) -> void {
	        if constexpr (ResourceVisitorTrait<decltype(image)>::kClass == ResourceClass::kCombinedImage)
	            image->ForAllImages([this, image_view_id](const auto *sub_image) -> void {
	                if constexpr (ResourceVisitorTrait<decltype(sub_image)>::kIsInternal)
	                    m_image_view_contain_relation.SetRelation(image_view_id, GetIntImageViewID(sub_image));
	            });
	    });
	}*/
}

void RenderGraphResolver::extract_ordered_passes_and_edges(OriginGraph &&graph) {
	graph.insert_write_after_read_edges();
	m_pass_nodes.clear();
	m_pass_nodes.reserve(graph.nodes.size());

	std::queue<const PassBase *> candidate_queue;

	assert(graph.nodes.find(nullptr) != graph.nodes.end());
	assert(graph.nodes.at(nullptr).in_degree == 0);
	for (auto *p_edge : graph.nodes.at(nullptr).output_edges) {
		uint32_t degree = --graph.nodes.at(p_edge->to.pass).in_degree;
		if (degree == 0)
			candidate_queue.push(p_edge->to.pass);
	}
	while (!candidate_queue.empty()) {
		const PassBase *pass = candidate_queue.front();
		candidate_queue.pop();

		if (!pass)
			continue;

		pass->m_resolved_info.pass_order = m_pass_nodes.size();
		m_pass_nodes.push_back({pass});

		for (auto *p_edge : graph.nodes.at(pass).output_edges) {
			uint32_t degree = --graph.nodes.at(p_edge->to.pass).in_degree;
			if (degree == 0)
				candidate_queue.push(p_edge->to.pass);
		}
	}

	m_pass_edges.clear();
	m_pass_edges.reserve(graph.edges.size());
	m_src_output_edges.clear();

	const auto add_edge = [this](const ResourceBase *resource, const ResourceReference &from,
	                             const ResourceReference &to, DependencyType type) {
		m_pass_edges.push_back({resource, from, to, type});
		const PassEdge *p_edge = &m_pass_edges.back();
		(from.pass ? m_pass_nodes[GetPassOrder(from.pass)].output_edges : m_src_output_edges).push_back(p_edge);
		if (to.pass)
			m_pass_nodes[GetPassOrder(to.pass)].input_edges.push_back(p_edge);
	};

	for (const auto &edge : graph.edges) {
		if (edge.extra_type == OriginGraph::ExtraEdgeType::kDeleted)
			continue;

		add_edge(edge.resource, edge.from, edge.to, edge.type);
	}

	// Sort Output (for RenderGraphScheduler)
	const auto sort_output = [](std::vector<const PassEdge *> *p_output_edges) {
		std::sort(p_output_edges->begin(), p_output_edges->end(), [](const PassEdge *l, const PassEdge *r) {
			uint32_t l_order = l->to.pass ? GetPassOrder(l->to.pass) : -1;
			uint32_t r_order = r->to.pass ? GetPassOrder(r->to.pass) : -1;
			return l_order < r_order;
		});
	};
	sort_output(&m_src_output_edges);
	for (auto &node : m_pass_nodes)
		sort_output(&node.output_edges);
}

void RenderGraphResolver::extract_pass_relation() {
	const uint32_t kOrderedPassCount = m_pass_nodes.size();
	m_pass_prior_relation.Reset(kOrderedPassCount, kOrderedPassCount);
	for (uint32_t i = kOrderedPassCount - 1; ~i; --i) {
		for (const auto *p_edge : m_pass_nodes[i].input_edges) {
			if (p_edge->from.pass) {
				m_pass_prior_relation.SetRelation(GetPassOrder(p_edge->from.pass), i);
				m_pass_prior_relation.ApplyRelations(i, GetPassOrder(p_edge->from.pass));
			}
		}
	}

	// The transpose of m_pass_prior_relation
	m_pass_prior_relation_transpose = m_pass_prior_relation.GetTranspose();
}

void RenderGraphResolver::extract_resource_relation() {
	const uint32_t kOrderedPassCount = m_pass_nodes.size();

	RelationMatrix pass_resource_not_prior_relation;
	{
		// Compute its transpose
		RelationMatrix pass_resource_not_prior_relation_transpose;
		pass_resource_not_prior_relation_transpose.Reset(GetIntResourceCount(), kOrderedPassCount);
		for (uint32_t pass_order = 0; pass_order < kOrderedPassCount; ++pass_order) {
			m_pass_nodes[pass_order].pass->for_each_input(
			    [this, pass_order, &pass_resource_not_prior_relation_transpose](const Input *p_input) {
				    if (p_input->GetResource()->GetState() == ResourceState::kLastFrame)
					    return;
				    uint32_t internal_resource_id = GetIntResourceID(p_input->GetResource());
				    if (~internal_resource_id) {
					    for (uint32_t i = 0; i < pass_resource_not_prior_relation_transpose.GetRowSize(); ++i) {
						    pass_resource_not_prior_relation_transpose.GetRowData(internal_resource_id)[i] |=
						        ~m_pass_prior_relation_transpose.GetRowData(pass_order)[i];
					    }
					    // If a pass uses a resource, then all the pass's non-precursor are no prior than the resource
				    }
			    });
		}
		pass_resource_not_prior_relation = pass_resource_not_prior_relation_transpose.GetTranspose();
	}

	m_resource_not_prior_relation.Reset(GetIntResourceCount(), GetIntResourceCount());
	{
		for (uint32_t pass_order = 0; pass_order < kOrderedPassCount; ++pass_order) {
			m_pass_nodes[pass_order].pass->for_each_input(
			    [this, pass_order, &pass_resource_not_prior_relation](const Input *p_input) {
				    if (p_input->GetResource()->GetState() == ResourceState::kLastFrame)
					    return;
				    uint32_t internal_resource_id = GetIntResourceID(p_input->GetResource());
				    if (~internal_resource_id)
					    m_resource_not_prior_relation.ApplyRelations(pass_resource_not_prior_relation, pass_order,
					                                                 internal_resource_id);
			    });
		}
	}

	m_resource_conflict_relation.Reset(GetIntResourceCount(), GetIntResourceCount());
	for (uint32_t resource_id_0 = 0; resource_id_0 < GetIntResourceCount(); ++resource_id_0) {
		m_resource_conflict_relation.SetRelation(resource_id_0, resource_id_0);
		for (uint32_t resource_id_1 = 0; resource_id_1 < resource_id_0; ++resource_id_1) {
			if (m_resource_not_prior_relation.GetRelation(resource_id_0, resource_id_1) &&
			    m_resource_not_prior_relation.GetRelation(resource_id_1, resource_id_0)) {
				m_resource_conflict_relation.SetRelation(resource_id_0, resource_id_1);
				m_resource_conflict_relation.SetRelation(resource_id_1, resource_id_0);
			}
		}
	}
}

void RenderGraphResolver::extract_resource_references() {
	const uint32_t kOrderedPassCount = m_pass_nodes.size();

	m_last_frame_resources.clear();
	m_last_frame_resources.reserve(GetIntResourceCount());

	RelationMatrix resource_pass_visited, resource_lf_pass_visited;
	resource_pass_visited.Reset(GetIntResourceCount(), kOrderedPassCount);
	resource_lf_pass_visited.Reset(GetIntResourceCount(), kOrderedPassCount);

	for (uint32_t order = kOrderedPassCount - 1; ~order; --order) {
		const auto &node = m_pass_nodes[order];

		const auto update_resource_reference =
		    [this, order, &node, &resource_pass_visited](const auto *resource, const Input *p_input) -> void {
			using Trait = ResourceVisitorTrait<decltype(resource)>;
			if constexpr (Trait::kIsInternal) {
				uint32_t int_res_id = GetIntResourceID(resource);
				auto &int_res_info = GetIntResourceInfo(int_res_id);

				int_res_info.references.push_back({p_input, node.pass});
				if (!resource_pass_visited.GetRelation(int_res_id, order)) {
					int_res_info.last_references.push_back({p_input, node.pass});
					if constexpr (Trait::kType == ResourceType::kImage)
						assert(resource == m_internal_images[resource->m_resolved_info.image_id].image);
				}

				// Exclude all the passes prior than last_reference
				assert(resource_pass_visited.GetRowSize() == m_pass_prior_relation_transpose.GetRowSize());
				for (uint32_t i = 0; i < resource_pass_visited.GetRowSize(); ++i)
					resource_pass_visited.GetRowData(int_res_id)[i] |=
					    m_pass_prior_relation_transpose.GetRowData(order)[i];
			}
		};

		const auto update_lf_resource_reference =
		    [this, order, &node, &resource_lf_pass_visited](const auto *resource, const Input *p_input) -> void {
			using Trait = ResourceVisitorTrait<decltype(resource)>;

			uint32_t int_res_id = GetIntResourceID(resource);
			auto &int_res_info = GetIntResourceInfo(resource->GetCurrentResource());

			if (int_res_info.p_last_frame_info == nullptr) {
				m_last_frame_resources.emplace_back();
				int_res_info.p_last_frame_info = &m_last_frame_resources.back();
			}

			int_res_info.p_last_frame_info->lf_resource = resource;
			int_res_info.p_last_frame_info->references.push_back({p_input, node.pass});
			if (!resource_lf_pass_visited.GetRelation(int_res_id, order)) {
				int_res_info.p_last_frame_info->last_references.push_back({p_input, node.pass});
				if constexpr (Trait::kType == ResourceType::kImage)
					assert(resource->GetCurrentResource() ==
					       m_internal_images[resource->GetCurrentResource()->m_resolved_info.image_id].image);
			}

			// Exclude all the passes prior than last_reference
			assert(resource_lf_pass_visited.GetRowSize() == m_pass_prior_relation_transpose.GetRowSize());
			for (uint32_t i = 0; i < resource_lf_pass_visited.GetRowSize(); ++i)
				resource_lf_pass_visited.GetRowData(int_res_id)[i] |=
				    m_pass_prior_relation_transpose.GetRowData(order)[i];
		};

		node.pass->for_each_input([this, &node, &update_resource_reference,
		                           &update_lf_resource_reference](const Input *p_input) -> void {
			const auto local_update_resource_reference = [&update_resource_reference, p_input](const auto *resource) {
				return update_resource_reference(resource, p_input);
			};
			const auto local_update_lf_resource_reference = [&update_lf_resource_reference,
			                                                 p_input](const auto *resource) {
				return update_lf_resource_reference(resource, p_input);
			};
			ResourceReference res_ref = {p_input, node.pass};
			p_input->GetResource()->Visit([this, &res_ref, &local_update_resource_reference,
			                               &local_update_lf_resource_reference](const auto *resource) {
				if constexpr (ResourceVisitorTrait<decltype(resource)>::kIsAlias) {
					resource->GetPointedResource()->Visit(local_update_resource_reference);
				} else if constexpr (ResourceVisitorTrait<decltype(resource)>::kIsLastFrame) {
					// last_frame_references
					local_update_lf_resource_reference(resource);
				} else {
					if constexpr (ResourceVisitorTrait<decltype(resource)>::kIsInternal) {
						// validation_references
						GetIntResourceInfo(resource).validation_references.push_back(res_ref);
					}
					local_update_resource_reference(resource);
				}
			});
		});
	}

	// Reverse the references so that the first reference is in the earliest pass
	for (auto &image_info : m_internal_images) {
		assert(!image_info.references.empty() && !image_info.last_references.empty());
		std::reverse(image_info.references.begin(), image_info.references.end());
		std::reverse(image_info.last_references.begin(), image_info.last_references.end());

		std::reverse(image_info.validation_references.begin(), image_info.validation_references.end());
		if (image_info.p_last_frame_info) {
			std::reverse(image_info.p_last_frame_info->references.begin(),
			             image_info.p_last_frame_info->references.end());
			std::reverse(image_info.p_last_frame_info->last_references.begin(),
			             image_info.p_last_frame_info->last_references.end());
		}
	}
	for (auto &buffer_info : m_internal_buffers) {
		assert(!buffer_info.references.empty() && !buffer_info.last_references.empty());
		std::reverse(buffer_info.references.begin(), buffer_info.references.end());
		std::reverse(buffer_info.last_references.begin(), buffer_info.last_references.end());

		std::reverse(buffer_info.validation_references.begin(), buffer_info.validation_references.end());
		if (buffer_info.p_last_frame_info) {
			std::reverse(buffer_info.p_last_frame_info->references.begin(),
			             buffer_info.p_last_frame_info->references.end());
			std::reverse(buffer_info.p_last_frame_info->last_references.begin(),
			             buffer_info.p_last_frame_info->last_references.end());
		}
	}
}

void RenderGraphResolver::Resolve(const RenderGraphBase *p_render_graph) {
	{
		OriginGraph graph = make_origin_graph(p_render_graph);
		extract_resources(graph);
		extract_ordered_passes_and_edges(std::move(graph));
	}

	extract_pass_relation();
	extract_resource_relation();
	extract_resource_references();
}

} // namespace myvk_rg::_details_
