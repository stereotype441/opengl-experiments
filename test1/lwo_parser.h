#include "vectors.h"
#include <vector>
#include <string>

namespace Lwo {

struct Pols
{
  std::string type;
  std::vector<std::vector<unsigned int> > polygons;
};

struct Layer
{
  std::vector<Vector<3> > points;
  std::vector<Pols *> polygons;
};

struct Lwo
{
  std::vector<Layer *> layers;
};

Lwo *parse(void const *lwo_data);

};
