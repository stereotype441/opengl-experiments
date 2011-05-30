#include "mesh.h"
#include <iostream>
#include <set>

using namespace std;

namespace Mesh {

void mesh_to_triangles(Mesh const &mesh, V3List &triangles)
{
  // For each polygon:
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const *polygon = mesh[i];

    // If the polygon has at least 3 vertices:
    if (polygon->size() < 3) {
      cout << "Warning: polygon with fewer than 3 vertices" << endl;
    } else {

      // Then arbitrarily create a triangle fan centered at the first
      // vertex.
      for (int i = 2; i < polygon->size(); ++i) {
	triangles.push_back((*polygon)[0]);
	triangles.push_back((*polygon)[i-1]);
	triangles.push_back((*polygon)[i]);
      }
    }
  }
}

void compute_mesh_normals(Mesh const &mesh, std::map<V3 const *, V3> &normals)
{
  // For each polygon:
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const *polygon = mesh[i];
    // For each point within that polygon:
    for (int j = 0; j < polygon->size(); ++j) {
      V3 const *point = (*polygon)[j];
      // Compute the vectors from the previous point to this one, and
      // from this point to the next one.
      int sz = polygon->size();
      V3 const *prev_point = (*polygon)[(j + sz - 1) % sz];
      V3 const *next_point = (*polygon)[(j + 1) % sz];
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
  // Make a map from each edge to a list of pointers to polygons that
  // touch it.  An edge is defined as a sorted pair of V3 pointers.
  typedef std::pair<V3 const *, V3 const *> Edge;
  std::map<Edge, std::vector<Polygon const *> > edge_to_poly;
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const *poly = mesh[i];
    for (int j = 0; j < poly->size(); ++j) {
      V3 const *a = (*poly)[j];
      V3 const *b = (*poly)[(j+1) % poly->size()];
      if (a < b) {
	edge_to_poly[std::make_pair(a, b)].push_back(poly);
      } else {
	edge_to_poly[std::make_pair(b, a)].push_back(poly);
      }
    }
  }

  // Make a map from each polygon pointer to a list of pointers to
  // polygons that touch it.
  std::map<Polygon const *, std::vector<Polygon const *> > adjacency;
  for (std::map<Edge, std::vector<Polygon const *> >::const_iterator iter
	 = edge_to_poly.begin(); iter != edge_to_poly.end(); ++iter) {
    std::vector<Polygon const *> const &polygons = iter->second;
    for (int i = 0; i < polygons.size(); ++i) {
      for (int j = 0; j < polygons.size(); ++j) {
	if (i != j) {
	  Polygon const *x = polygons[i];
	  Polygon const *y = polygons[j];
	  if (x != y) {
	    adjacency[x].push_back(y);
	  }
	}
      }
    }
  }

  // Make a set to hold the pointers to polygons that have not yet been
  // assigned a surface.
  std::set<Polygon const *> unassigned_polygons;
  for (int i = 0; i < mesh.size(); ++i) {
    unassigned_polygons.insert(mesh[i]);
  }

  // While there are unassigned polygons left:
  while (!unassigned_polygons.empty()) {
    // Create a new surface containing only this polygon.
    surfaces.push_back(Mesh());
    Mesh &surface = surfaces.back();
    Polygon const *poly = *unassigned_polygons.begin();
    unassigned_polygons.erase(unassigned_polygons.begin());
    surface.push_back(poly);
    std::vector<Polygon const *> polys_to_traverse;
    polys_to_traverse.push_back(poly);

    // And then "flood fill" to all other polygons that share edges.
    while (!polys_to_traverse.empty()) {
      Polygon const *poly = polys_to_traverse.back();
      polys_to_traverse.pop_back();
      std::vector<Polygon const *> const &adjacent_polygons
	= adjacency[poly];
      for (int i = 0; i < adjacent_polygons.size(); ++i) {
	Polygon const *adjacent_polygon = adjacent_polygons[i];
	std::set<Polygon const *>::iterator found
	  = unassigned_polygons.find(adjacent_polygon);
	if (found != unassigned_polygons.end()) {
	  unassigned_polygons.erase(found);
	  surface.push_back(adjacent_polygon);
	  polys_to_traverse.push_back(adjacent_polygon);
	}
      }
    }
  }
}

void split_mesh_pointwise(Mesh const &mesh, std::vector<Mesh> &surfaces)
{
  // Make a map from each point to a list of pointers to polygons that
  // include it.
  std::map<V3 const *, std::vector<Polygon const *> > point_to_poly;
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const *poly = mesh[i];
    for (int j = 0; j < poly->size(); ++j) {
      point_to_poly[(*poly)[j]].push_back(poly);
    }
  }

  // Make a set to hold the pointers to polygons that have not yet been
  // assigned a surface.
  std::set<Polygon const *> unassigned_polygons;
  for (int i = 0; i < mesh.size(); ++i) {
    unassigned_polygons.insert(mesh[i]);
  }

  // While there are unassigned polygons left:
  while (!unassigned_polygons.empty()) {
    // Create a new surface containing only this polygon.
    surfaces.push_back(Mesh());
    Mesh &surface = surfaces.back();
    Polygon const *poly = *unassigned_polygons.begin();
    unassigned_polygons.erase(unassigned_polygons.begin());
    surface.push_back(poly);
    std::vector<Polygon const *> polys_to_traverse;
    polys_to_traverse.push_back(poly);

    // And then "flood fill" to all other polygons that share edges.
    while (!polys_to_traverse.empty()) {
      Polygon const *poly = polys_to_traverse.back();
      polys_to_traverse.pop_back();
      for (int i = 0; i < poly->size(); ++i) {
	std::vector<Polygon const *> const &adjacent_polys
	  = point_to_poly[(*poly)[i]];
	for (int j = 0; j < adjacent_polys.size(); ++j) {
	  Polygon const *adjacent_poly = adjacent_polys[j];
	  std::set<Polygon const *>::iterator found
	    = unassigned_polygons.find(adjacent_poly);
	  if (found != unassigned_polygons.end()) {
	    unassigned_polygons.erase(found);
	    surface.push_back(adjacent_poly);
	    polys_to_traverse.push_back(adjacent_poly);
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
      Polygon const *poly = mesh[j];
      for (int k = 0; k < poly->size(); ++k) {
	indices[(*poly)[k]] = i;
      }
    }
  }
}

};
