#pragma once

#include <vector>
#include <map>
#include "vectors.h"

namespace Mesh {

typedef Vector<3> V3;
typedef std::vector<V3 const *> V3List;
typedef V3List Polygon;
typedef std::vector<Polygon const *> Mesh;

class PointSet
{
public:
  void translate(V3List const &in,
		 std::vector<int> &out)
  {
    for (int i = 0; i < in.size(); ++i) {
      std::pair<std::map<V3 const *, int>::iterator, bool> inserted
	= point_to_index.insert(std::make_pair(in[i], points.size()));
      if (inserted.second) {
	deref_points.push_back(*in[i]);
	points.push_back(in[i]);
      }
      out.push_back(inserted.first->second);
    }
  }

  template<class C>
  void translate(std::map<V3 const *, C> const &in,
		 std::vector<C> &out)
  {
    for (int i = 0; i < points.size(); ++i) {
      typename std::map<V3 const *, C>::const_iterator found
	= in.find(points[i]);
      if (found != in.end()) {
	out.push_back(found->second);
      } else {
	out.push_back(C());
      }
    }
  }

  std::vector<V3> const &raw()
  {
    return deref_points;
  }

private:
  V3List points;
  std::vector<V3> deref_points;
  std::map<V3 const *, int> point_to_index;
};

void mesh_to_triangles(Mesh const &mesh, V3List &triangles);

void compute_mesh_normals(Mesh const &mesh, std::map<V3 const *, V3> &normals);

void split_mesh_pointwise(Mesh const &mesh, std::vector<Mesh> &sub_meshes);

void split_mesh_edgewise(Mesh const &mesh, std::vector<Mesh> &sub_meshes);

void compute_surface_indices(
    std::vector<Mesh> const &surfaces, std::map<V3 const *, int> &indices);

};
