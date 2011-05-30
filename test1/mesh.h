#pragma once

#include <vector>

namespace Mesh {

void polygons_to_triangles(
    std::vector<std::vector<unsigned int> > const &polygons,
    std::vector<unsigned int> &triangles);

};
