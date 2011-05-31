// "Paul's model format"

#pragma once

#include <vector>
#include <string>

#include "mesh.h"

namespace Pmf {

class Data
{
public:
  Data();
  Data(char const *serialized);
  std::vector<char> *Serialize();

  void add_triangles(Mesh::V3List const &triangles);

  void add_vector_properties(
      std::string const &key,
      std::map<Mesh::V3 const *, Mesh::V3> const &vectors);

  template<class C>
  void add_scalar_properties(
      std::string const &key,
      std::map<Mesh::V3 const *, C> const &scalars)
  {
    std::vector<float> &raw = m_point_scalar_properties[key];
    for (typename std::map<Mesh::V3 const *, C>::const_iterator iter
           = scalars.begin(); iter != scalars.end(); ++iter) {
      int point = add_point(iter->first);
      if (point >= raw.size()) {
        raw.resize(m_points.size());
      }
      raw[point] = iter->second;
    }
  }

  std::vector<Mesh::V3> const &points() const
  {
    return m_points;
  }
  std::vector<Mesh::V3> const &point_vector_properties(
      std::string const &key) const
  {
    std::vector<Mesh::V3> &result = m_point_vector_properties[key];
    result.resize(m_points.size());
    return result;
  }

  std::vector<float> const &point_scalar_properties(
      std::string const &key) const
  {
    std::vector<float> &result = m_point_scalar_properties[key];
    result.resize(m_points.size());
    return result;
  }

  std::vector<int> const &triangles() const
  {
    return m_triangles;
  }

private:
  std::vector<Mesh::V3> m_points;
  mutable std::map<std::string, std::vector<Mesh::V3> > m_point_vector_properties;
  mutable std::map<std::string, std::vector<float> > m_point_scalar_properties;
  mutable std::vector<int> m_triangles;
  std::map<std::string, std::string> m_metadata;
  std::map<Mesh::V3 const *, int> m_point_indices;

  int add_point(Mesh::V3 const *point)
  {
    std::pair<std::map<Mesh::V3 const *, int>::iterator, bool> inserted
      = m_point_indices.insert(std::make_pair(point, m_points.size()));
    if (inserted.second) {
      m_points.push_back(*point);
    }
    return inserted.first->second;
  }
};

};
