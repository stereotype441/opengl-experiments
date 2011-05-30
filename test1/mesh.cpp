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

};
