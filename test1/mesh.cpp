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

void compute_mesh_normals(
    Mesh const &mesh, std::map<V3 const *, V3> &normals,
    std::map<V3 const *, bool> &mobius_flags)
{
  std::map<Polygon const *, int> orientations;
  assign_polygon_orientations(mesh, orientations, mobius_flags);

  // For each polygon:
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const *polygon = mesh[i];
    int orientation = orientations[polygon];
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

      // Adjust based on the orientation of this polygon
      normal *= orientation;

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

typedef std::pair<V3 const *, V3 const *> Edge;

void split_mesh_edgewise(Mesh const &mesh, std::vector<Mesh> &surfaces)
{
  // Make a map from each edge to a list of pointers to polygons that
  // touch it.  An edge is defined as a sorted pair of V3 pointers.
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
	    adjacency[y].push_back(x);
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

    // And then "flood fill" to all other polygons that share edges.
    std::vector<Polygon const *> polys_to_traverse;
    polys_to_traverse.push_back(poly);
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

    // And then "flood fill" to all other polygons that share points.
    std::vector<Polygon const *> polys_to_traverse;
    polys_to_traverse.push_back(poly);
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

struct AssignPolygonOrientationsAdjacencyData
{
  AssignPolygonOrientationsAdjacencyData(
      Polygon const *_polygon, int _join_orientation, Edge const &_edge)
    : polygon(_polygon), join_orientation(_join_orientation), edge(_edge)
  {
  }

  Polygon const *polygon;
  int join_orientation;
  Edge edge;
};

void assign_polygon_orientations(
    Mesh const &mesh, std::map<Polygon const *, int> &orientations,
    std::map<V3 const *, bool> &mobius_flags)
{
  // First construct a map from each edge to a list of pairs, where
  // the pairs consist of (polygon containing that edge,
  // orientation of that edge).  An edge is defined as a sorted pair
  // of V3 pointers.  An orientation is 1 if the sorted pair of V3
  // pointers is in the same order it appears in the polygon, -1 if
  // it's in the opposite order.
  typedef std::pair<Polygon const *, int> EdgeData;
  std::map<Edge, std::vector<EdgeData> > edge_data_map;
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const *poly = mesh[i];
    for (int j = 0; j < poly->size(); ++j) {
      V3 const *a = (*poly)[j];
      V3 const *b = (*poly)[(j+1) % poly->size()];
      if (a < b) {
	edge_data_map[std::make_pair(a, b)].push_back(make_pair(poly, 1));
      } else {
	edge_data_map[std::make_pair(b, a)].push_back(make_pair(poly, -1));
      }
    }
  }

  // Now construct a map from each polygon to a list of pairs, where
  // the pairs consist of (adjoining polygon, orientation of the join,
  // shared vertices).  The orientation of a join is 1 if the common
  // edge is contained in the two polygons in opposite orientation, -1
  // if it is contained in the two polygons in the same orientation.
  typedef AssignPolygonOrientationsAdjacencyData AdjacencyData;
  std::map<Polygon const *, std::vector<AdjacencyData> > adjacency;
  for (std::map<Edge, std::vector<EdgeData> >::const_iterator iter
	 = edge_data_map.begin(); iter != edge_data_map.end(); ++iter) {
    Edge const &edge = iter->first;
    std::vector<EdgeData> const &edge_data = iter->second;
    for (int i = 0; i < edge_data.size(); ++i) {
      for (int j = 0; j < edge_data.size(); ++j) {
	if (i != j) {
	  Polygon const *x = edge_data[i].first;
	  Polygon const *y = edge_data[j].first;
	  if (x != y) {
	    int x_orientation = edge_data[i].second;
	    int y_orientation = edge_data[j].second;
	    int join_orientation = -1 * x_orientation * y_orientation;
	    adjacency[x].push_back(AdjacencyData(y, join_orientation, edge));
	    adjacency[y].push_back(AdjacencyData(x, join_orientation, edge));
	  }
	}
      }
    }
  }

  // Make a set to hold the pointers to polygons that have not yet
  // been assigned an orientation.
  std::set<Polygon const *> unassigned_polygons;
  for (int i = 0; i < mesh.size(); ++i) {
    unassigned_polygons.insert(mesh[i]);
  }

  // While there are unassigned polygons left:
  while (!unassigned_polygons.empty()) {
    // Choose a polygon and assign an orientation arbitrarily.
    Polygon const *poly = *unassigned_polygons.begin();
    unassigned_polygons.erase(unassigned_polygons.begin());
    orientations[poly] = 1;

    // And then "flood fill" to all other polygons that share edges.
    std::vector<Polygon const *> polys_to_traverse;
    polys_to_traverse.push_back(poly);
    while (!polys_to_traverse.empty()) {
      Polygon const *poly = polys_to_traverse.back();
      polys_to_traverse.pop_back();
      std::vector<AdjacencyData> const &adjacency_data = adjacency[poly];
      for (int i = 0; i < adjacency_data.size(); ++i) {
	Polygon const *adjacent_polygon = adjacency_data[i].polygon;
	int join_orientation = adjacency_data[i].join_orientation;
	Edge const &edge = adjacency_data[i].edge;
	std::set<Polygon const *>::iterator found
	  = unassigned_polygons.find(adjacent_polygon);
	if (found != unassigned_polygons.end()) {
	  unassigned_polygons.erase(found);
	  orientations[adjacent_polygon]
	    = orientations[poly] * join_orientation;
	  polys_to_traverse.push_back(adjacent_polygon);
	} else if (orientations[adjacent_polygon]
	      != orientations[poly] * join_orientation) {
	  cout << "Warning: mobius surface" << endl;
	  mobius_flags[edge.first] = true;
	  mobius_flags[edge.second] = true;
	}
      }
    }
  }
}

Bbox compute_bbox(Mesh const &mesh)
{
  Bbox result;
  result.m_min = V3(INFINITY);
  result.m_max = V3(-INFINITY);
  for (int i = 0; i < mesh.size(); ++i) {
    Polygon const &poly = *mesh[i];
    for (int j = 0; j < poly.size(); ++j) {
      for (int k = 0; k < 3; ++k) {
        result.m_min.coords[k]
          = min(result.m_min.coords[k], poly[j]->coords[k]);
        result.m_max.coords[k]
          = max(result.m_max.coords[k], poly[j]->coords[k]);
      }
    }
  }
  return result;
}

};
