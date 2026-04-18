#pragma once
#include "LODBakingGraph.h"

namespace atlas::procedural {

MeshData DecimateMesh(const MeshData& input, float keepFactor);
MeshData MergeNearbyVertices(const MeshData& input, float threshold);
MeshData RecomputeNormals(const MeshData& input);
LODChain GenerateLODChain(const MeshData& baseMesh, uint32_t levelCount);

}
