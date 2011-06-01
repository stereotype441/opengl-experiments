#pragma once

#include <vector>
#include <map>
#include "vectors.h"

namespace Mesh {

typedef Vector<3> V3;
typedef std::vector<V3 const *> V3List;
typedef V3List Polygon;
typedef std::vector<Polygon const *> Mesh;

struct Bbox
{
  V3 m_min;
  V3 m_max;
};

void mesh_to_triangles(Mesh const &mesh, V3List &triangles);

void compute_mesh_normals(
    Mesh const &mesh, std::map<V3 const *, V3> &normals,
    std::map<V3 const *, bool> &mobius_flags);

void split_mesh_pointwise(Mesh const &mesh, std::vector<Mesh> &sub_meshes);

void split_mesh_edgewise(Mesh const &mesh, std::vector<Mesh> &sub_meshes);

void compute_surface_indices(
    std::vector<Mesh> const &surfaces, std::map<V3 const *, int> &indices);

void assign_polygon_orientations(
    Mesh const &mesh, std::map<Polygon const *, int> &orientations,
    std::map<V3 const *, bool> &mobius_flags);

Bbox compute_bbox(Mesh const &mesh);

};
