#pragma once
#include "ProceduralMeshGraph.h"

namespace atlas::procedural {

MeshData GenerateCube(float size);
MeshData GeneratePlane(float size);
MeshData GenerateSphere(float size, int segments);
MeshData GenerateCylinder(float size, int segments);
MeshData SubdivideMesh(const MeshData& input);
MeshData ApplyNoiseDisplacement(const MeshData& input, float amplitude, uint64_t seed);

}
