#pragma once

#include "mesh.h"
#include <vector>
#include <string>

namespace Lwo {

struct Pols
{
  std::string type;
  Mesh::Mesh polygons;
};

struct Layer
{
  std::vector<Pols *> polygons;
};

struct Lwo
{
  std::vector<Layer *> layers;
};

Lwo *parse(void const *lwo_data);

};
