#pragma once

#include <vector>
#include <map>
#include "vectors.h"

namespace Mesh {

class PointSet
{
public:
  void translate(std::vector<Vector<3> const *> const &in,
		 std::vector<int> &out)
  {
    for (int i = 0; i < in.size(); ++i) {
      std::pair<std::map<Vector<3> const *, int>::iterator, bool> inserted
	= point_to_index.insert(std::make_pair(in[i], points.size()));
      if (inserted.second) {
	deref_points.push_back(*in[i]);
	points.push_back(in[i]);
      }
      out.push_back(inserted.first->second);
    }
  }

  void translate(std::map<Vector<3> const *, Vector<3> > const &in,
		 std::vector<Vector<3> > &out)
  {
    for (int i = 0; i < points.size(); ++i) {
      std::map<Vector<3> const *, Vector<3> >::const_iterator found
	= in.find(points[i]);
      if (found != in.end()) {
	out.push_back(found->second);
      } else {
	out.push_back(Vector<3>());
      }
    }
  }

  std::vector<Vector<3> > const &raw()
  {
    return deref_points;
  }

private:
  std::vector<Vector<3> const *> points;
  std::vector<Vector<3> > deref_points;
  std::map<Vector<3> const *, int> point_to_index;
};

void polygons_to_triangles(
    std::vector<std::vector<Vector<3> const *> > const &polygons,
    std::vector<Vector<3> const *> &triangles);

void compute_polygon_normals(
    std::vector<std::vector<Vector<3> const *> > const &polygons,
    std::map<Vector<3> const *, Vector<3> > &normals);

};
