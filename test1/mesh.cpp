#include "mesh.h"
#include <iostream>

using namespace std;

namespace Mesh {

void mesh_to_triangles(Mesh const &mesh, V3List &triangles)
{
  // For each polygon:
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const &polygon = mesh[i];

    // If the polygon has at least 3 vertices:
    if (polygon.size() < 3) {
      cout << "Warning: polygon with fewer than 3 vertices" << endl;
    } else {

      // Then arbitrarily create a triangle fan centered at the first
      // vertex.
      for (int i = 2; i < polygon.size(); ++i) {
	triangles.push_back(polygon[0]);
	triangles.push_back(polygon[i-1]);
	triangles.push_back(polygon[i]);
      }
    }
  }
}

void compute_mesh_normals(Mesh const &mesh, std::map<V3 const *, V3> &normals)
{
  // For each polygon:
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const &polygon = mesh[i];
    // For each point within that polygon:
    for (int j = 0; j < polygon.size(); ++j) {
      V3 const *point = polygon[j];
      // Compute the vectors from the previous point to this one, and
      // from this point to the next one.
      int sz = polygon.size();
      V3 const *prev_point = polygon[(j + sz - 1) % sz];
      V3 const *next_point = polygon[(j + 1) % sz];
      V3 incoming = *point - *prev_point;
      V3 outgoing = *next_point - *point;
      V3 normal = incoming % outgoing;

      // And sum into the "normals" map.
      normals[point] += normal;
    }
  }

  // Now re-normalize every element of the "normals" map.
  for (std::map<V3 const *, V3>::iterator normal_iter
	 = normals.begin(); normal_iter != normals.end(); ++normal_iter) {
    normal_iter->second = normal_iter->second.normalize();
  }
}

};
