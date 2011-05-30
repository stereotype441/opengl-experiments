#include "mesh.h"
#include <iostream>
#include <set>

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

void split_mesh_edgewise(Mesh const &mesh, std::vector<Mesh> &surfaces)
{
  // Make a map from each edge to a list of indices of polygons that
  // touch it.  An edge is defined as a sorted pair of V3 pointers.
  typedef std::pair<V3 const *, V3 const *> Edge;
  std::map<Edge, std::vector<int> > edge_to_poly_index;
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const &poly = mesh[i];
    for (int j = 0; j < poly.size(); ++j) {
      V3 const *a = poly[j];
      V3 const *b = poly[(j+1) % poly.size()];
      if (a < b) {
	edge_to_poly_index[std::make_pair(a, b)].push_back(i);
      } else {
	edge_to_poly_index[std::make_pair(b, a)].push_back(i);
      }
    }
  }

  // Make a map from each polygon index to a list of indices of
  // polygons that touch it.
  std::vector<std::vector<int> > adjacency(mesh.size());
  for (std::map<Edge, std::vector<int> >::const_iterator iter
	 = edge_to_poly_index.begin(); iter != edge_to_poly_index.end();
       ++iter) {
    std::vector<int> const &polygon_indices = iter->second;
    for (int i = 0; i < polygon_indices.size(); ++i) {
      for (int j = 0; j < polygon_indices.size(); ++j) {
	if (i != j) {
	  int x = polygon_indices[i];
	  int y = polygon_indices[j];
	  if (x != y) {
	    adjacency[x].push_back(y);
	  }
	}
      }
    }
  }

  // Make a set to hold the indices of polygons that have not yet been
  // assigned a surface.
  std::set<int> unassigned_polygon_indices;
  for (int i = 0; i < mesh.size(); ++i) {
    unassigned_polygon_indices.insert(i);
  }

  // While there are unassigned polygons left:
  while (!unassigned_polygon_indices.empty()) {
    // Create a new surface containing only this polygon.
    surfaces.push_back(Mesh());
    Mesh &surface = surfaces.back();
    int poly_index = *unassigned_polygon_indices.begin();
    unassigned_polygon_indices.erase(unassigned_polygon_indices.begin());
    surface.push_back(mesh[poly_index]);
    std::vector<int> indices_to_traverse;
    indices_to_traverse.push_back(poly_index);

    // And then "flood fill" to all other polygons that share edges.
    while (!indices_to_traverse.empty()) {
      int poly_index = indices_to_traverse.back();
      indices_to_traverse.pop_back();
      std::vector<int> const &adjacent_polygon_indices = adjacency[poly_index];
      for (int i = 0; i < adjacent_polygon_indices.size(); ++i) {
	int adjacent_polygon_index = adjacent_polygon_indices[i];
	std::set<int>::iterator found
	  = unassigned_polygon_indices.find(adjacent_polygon_index);
	if (found != unassigned_polygon_indices.end()) {
	  unassigned_polygon_indices.erase(found);
	  surface.push_back(mesh[adjacent_polygon_index]);
	  indices_to_traverse.push_back(adjacent_polygon_index);
	}
      }
    }
  }
}

void split_mesh_pointwise(Mesh const &mesh, std::vector<Mesh> &surfaces)
{
  // Make a map from each point to a list of indices of polygons that
  // include it.
  std::map<V3 const *, std::vector<int> > point_to_poly_index;
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const &poly = mesh[i];
    for (int j = 0; j < poly.size(); ++j) {
      point_to_poly_index[poly[j]].push_back(i);
    }
  }

  // Make a set to hold the indices of polygons that have not yet been
  // assigned a surface.
  std::set<int> unassigned_polygon_indices;
  for (int i = 0; i < mesh.size(); ++i) {
    unassigned_polygon_indices.insert(i);
  }

  // While there are unassigned polygons left:
  while (!unassigned_polygon_indices.empty()) {
    // Create a new surface containing only this polygon.
    surfaces.push_back(Mesh());
    Mesh &surface = surfaces.back();
    int poly_index = *unassigned_polygon_indices.begin();
    unassigned_polygon_indices.erase(unassigned_polygon_indices.begin());
    surface.push_back(mesh[poly_index]);
    std::vector<int> indices_to_traverse;
    indices_to_traverse.push_back(poly_index);

    // And then "flood fill" to all other polygons that share edges.
    while (!indices_to_traverse.empty()) {
      int poly_index = indices_to_traverse.back();
      indices_to_traverse.pop_back();
      Polygon const &poly = mesh[poly_index];
      for (int i = 0; i < poly.size(); ++i) {
	std::vector<int> const &adjacent_poly_indices
	  = point_to_poly_index[poly[i]];
	for (int j = 0; j < adjacent_poly_indices.size(); ++j) {
	  int adjacent_poly_index = adjacent_poly_indices[j];
	  std::set<int>::iterator found
	    = unassigned_polygon_indices.find(adjacent_poly_index);
	  if (found != unassigned_polygon_indices.end()) {
	    unassigned_polygon_indices.erase(found);
	    surface.push_back(mesh[adjacent_poly_index]);
	    indices_to_traverse.push_back(adjacent_poly_index);
	  }
	}
      }
    }
  }
}

void compute_surface_indices(
    std::vector<Mesh> const &surfaces, std::map<V3 const *, int> &indices)
{
  for (int i = 0; i < surfaces.size(); ++i) {
    Mesh const &mesh = surfaces[i];
    for (int j = 0; j < mesh.size(); ++j) {
      Polygon const &poly = mesh[j];
      for (int k = 0; k < poly.size(); ++k) {
	indices[poly[k]] = i;
      }
    }
  }
}

};
