#ifdef MYVK_ENABLE_RG

#ifndef MYVK_RG_RENDER_GRAPH_HPP
#define MYVK_RG_RENDER_GRAPH_HPP

#include "_details_/RenderGraph.hpp"

namespace myvk_rg {

using Key = _details_::PoolKey;

using GraphicsPassBase = _details_::GraphicsPassBase;
using ComputePassBase = _details_::ComputePassBase;
using TransferPassBase = _details_::TransferPassBase;
using PassGroupBase = _details_::PassGroupBase;

using ImageBase = _details_::ImageBase;
using ImageInput = const _details_::ImageBase *;
using ImageOutput = const _details_::ImageAlias *;
using ManagedImage = _details_::ManagedImage;
using CombinedImage = _details_::CombinedImage;
using LastFrameImage = _details_::LastFrameImage;
using ExternalImageBase = _details_::ExternalImageBase;

using BufferBase = _details_::BufferBase;
using BufferInput = const _details_::BufferBase *;
using BufferOutput = const _details_::BufferAlias *;
using ManagedBuffer = _details_::ManagedBuffer;
using LastFrameBuffer = _details_::LastFrameBuffer;
using ExternalBufferBase = _details_::ExternalBufferBase;

using SubImageSize = _details_::SubImageSize;

template <typename Derived> using RenderGraph = _details_::RenderGraph<Derived>;

} // namespace myvk_rg

#endif

#endif