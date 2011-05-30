#pragma once

#include <vector>
#include "vectors.h"

namespace Mesh {

void polygons_to_triangles(
    std::vector<std::vector<unsigned int> > const &polygons,
    std::vector<unsigned int> &triangles);

void compute_polygon_normals(
    std::vector<Vector<3> > const &points,
    std::vector<std::vector<unsigned int> > const &polygons,
    std::vector<Vector<3> > &normals);

};
