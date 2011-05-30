#include "mesh.h"
#include <iostream>

using namespace std;

namespace Mesh {

void polygons_to_triangles(
    std::vector<std::vector<unsigned int> > const &polygons,
    std::vector<unsigned int> &triangles)
{
  for (std::vector<std::vector<unsigned int> >::const_iterator poly
	 = polygons.begin(); poly < polygons.end(); ++poly) {
    size_t poly_size = poly->size();
    if (poly_size < 3) {
      cout << "Warning: polygon with fewer than 3 vertices" << endl;
    } else {
      for (int i = 2; i < poly_size; ++i) {
	triangles.push_back((*poly)[0]);
	triangles.push_back((*poly)[i-1]);
	triangles.push_back((*poly)[i]);
      }
    }
  }
}

void compute_polygon_normals(
    std::vector<Vector<3> > const &points,
    std::vector<std::vector<unsigned int> > const &polygons,
    std::vector<Vector<3> > &normals)
{
  // Start by making a map from point to the indices of the polygons
  // that contain it.
  std::vector<std::vector<std::pair<int, int> > >
    point_to_polygons(points.size());
  for (int i = 0; i < polygons.size(); ++i) {
    for (int j = 0; j < polygons[i].size(); ++j) {
      point_to_polygons[polygons[i][j]].push_back(
          std::pair<int, int>(i, j));
    }
  }

  Vector<3> zero_vector;
  for (int i = 0; i < 3; ++i) {
    zero_vector[i] = 0;
  }

  for (int vertex = 0; vertex < points.size(); ++vertex) {
    if (point_to_polygons[vertex].size() == 0) {
      normals.push_back(zero_vector);
    } else {
      Vector<3> normal_vector(zero_vector);
      for (int j = 0; j < point_to_polygons[vertex].size(); ++j) {
	std::pair<int, int> const &poly_ref = point_to_polygons[vertex][j];
	std::vector<unsigned int> const &poly = polygons[poly_ref.first];
	int poly_size = poly.size();
	unsigned int next_vertex = poly[(poly_ref.second + 1) % poly_size];
	unsigned int prev_vertex
	  = poly[(poly_ref.second + (poly_size-1)) % poly_size];
	Vector<3> a = points[vertex] - points[prev_vertex];
	Vector<3> b = points[next_vertex] - points[vertex];
	normal_vector += (a % b).normalize();
      }
      normals.push_back(normal_vector.normalize());
    }
  }
}

};
