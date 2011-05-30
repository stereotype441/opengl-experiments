#pragma once

#include <vector>
#include <map>
#include "vectors.h"

namespace Mesh {

typedef Vector<3> V3;
typedef std::vector<V3 const *> Polygon;

class PointSet
{
public:
  void translate(std::vector<V3 const *> const &in,
		 std::vector<int> &out)
  {
    for (int i = 0; i < in.size(); ++i) {
      std::pair<std::map<V3 const *, int>::iterator, bool> inserted
	= point_to_index.insert(std::make_pair(in[i], points.size()));
      if (inserted.second) {
	deref_points.push_back(*in[i]);
	points.push_back(in[i]);
      }
      out.push_back(inserted.first->second);
    }
  }

  void translate(std::map<V3 const *, V3> const &in,
		 std::vector<V3> &out)
  {
    for (int i = 0; i < points.size(); ++i) {
      std::map<V3 const *, V3>::const_iterator found
	= in.find(points[i]);
      if (found != in.end()) {
	out.push_back(found->second);
      } else {
	out.push_back(V3());
      }
    }
  }

  std::vector<V3> const &raw()
  {
    return deref_points;
  }

private:
  std::vector<V3 const *> points;
  std::vector<V3> deref_points;
  std::map<V3 const *, int> point_to_index;
};

void polygons_to_triangles(
    std::vector<Polygon> const &polygons,
    std::vector<V3 const *> &triangles);

void compute_polygon_normals(
    std::vector<Polygon> const &polygons,
    std::map<V3 const *, V3> &normals);

};
