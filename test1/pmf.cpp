#include "pmf.h"

#include <cstring>

namespace Pmf {

Pmf::Data::Data()
{
}

template<class C>
inline void get_raw(char const *&serialized, C *out, int count)
{
  size_t num_bytes = count * sizeof(C);
  memcpy(out, serialized, num_bytes);
  serialized += num_bytes;
}

inline int get_int(char const *&serialized)
{
  int value;
  get_raw(serialized, &value, 1);
  return value;
}

template<class C>
inline void get_thing(char const *&serialized, std::vector<C> &result)
{
  result.resize(get_int(serialized));
  get_raw(serialized, &result[0], result.size());
}

inline void get_thing(char const *&serialized, std::string &result)
{
  result.resize(get_int(serialized));
  get_raw(serialized, &result[0], result.size());
}

inline std::string get_string(char const *&serialized)
{
  std::string result;
  get_thing(serialized, result);
  return result;
}

template<class C>
inline void get_thing(
    char const *&serialized, std::map<std::string, C> &result)
{
  int size = get_int(serialized);
  while (size-- > 0) {
    std::string key = get_string(serialized);
    get_thing(serialized, result[key]);
  }
}

Pmf::Data::Data(char const *serialized)
{
  get_thing(serialized, m_points);
  get_thing(serialized, m_point_vector_properties);
  get_thing(serialized, m_point_scalar_properties);
  get_thing(serialized, m_triangles);
  get_thing(serialized, m_metadata);
}

template<class C>
inline void put_raw(std::vector<char> &out, C const *in, int count)
{
  out.insert(out.end(), reinterpret_cast<char const *>(in),
             reinterpret_cast<char const *>(in + count));
}

inline void put_int(std::vector<char> &out, int in)
{
  put_raw(out, &in, 1);
}

template<class C>
inline void put_thing(std::vector<char> &out, std::vector<C> const &in)
{
  put_int(out, in.size());
  put_raw(out, &in[0], in.size());
}

inline void put_thing(std::vector<char> &out, std::string const &in)
{
  put_int(out, in.size());
  put_raw(out, &in[0], in.size());
}

template<class C>
inline void put_thing(
    std::vector<char> &out, std::map<std::string, C> const &in)
{
  put_int(out, in.size());
  for (typename std::map<std::string, C>::const_iterator iter
         = in.begin(); iter != in.end(); ++iter) {
    put_thing(out, iter->first);
    put_thing(out, iter->second);
  }
}

std::vector<char> *Pmf::Data::Serialize() const
{
  std::vector<char> *result = new std::vector<char>();
  put_thing(*result, m_points);
  put_thing(*result, m_point_vector_properties);
  put_thing(*result, m_point_scalar_properties);
  put_thing(*result, m_triangles);
  put_thing(*result, m_metadata);
  return result;
}

void Pmf::Data::add_triangles(Mesh::V3List const &triangles)
{
  int triangles_offset = m_triangles.size();
  m_triangles.resize(triangles_offset + triangles.size());
  for (int i = 0; i < triangles.size(); ++i) {
    m_triangles[triangles_offset + i] = add_point(triangles[i]);
  }
}

void Pmf::Data::add_vector_properties(
    std::string const &key,
    std::map<Mesh::V3 const *, Mesh::V3> const &vectors)
{
  std::vector<Mesh::V3> &raw = m_point_vector_properties[key];
  for (std::map<Mesh::V3 const *, Mesh::V3>::const_iterator iter
         = vectors.begin(); iter != vectors.end(); ++iter) {
    int point = add_point(iter->first);
    if (point >= raw.size()) {
      raw.resize(m_points.size());
    }
    raw[point] = iter->second;
  }
}

};
