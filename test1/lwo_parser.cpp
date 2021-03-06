#include "lwo_parser.h"
#include <cassert>
#include <iostream>

using namespace std;

namespace Lwo {

class FileIterator
{
public:
  FileIterator(char const *_p, char const *_end)
    : p(_p), end(_end)
  {
  }

  bool remaining() const
  {
    return p < end;
  }

  char const *pointer() const
  {
    return p;
  }

  void advance(ptrdiff_t amount)
  {
    p += amount;
  }

  unsigned int get_uint(int num_bytes)
  {
    unsigned int result = 0;
    for (int i = 0; i < num_bytes; ++i) {
      result <<= 8;
      result |= static_cast<unsigned char>(*p++);
    }
    return result;
  }

  unsigned int get_vx()
  {
    if (static_cast<unsigned char>(*p) == 0xff) {
      ++p;
      return get_uint(3);
    } else {
      return get_uint(2);
    }
  }

  float get_float()
  {
    union {
      float f;
      unsigned int i;
    } u;
    u.i = get_uint(4);
    return u.f;
  }

  std::string get_fixed_string(int len)
  {
    char const *start = p;
    p += len;
    return std::string(start, p);
  }

private:
  char const *p;
  char const *end;
};

void handle_pnts(FileIterator iter, Mesh::V3List &points)
{
  while (iter.remaining()) {
    Vector<3> *point = new Vector<3>();
    for (int i = 0; i < 3; ++i) {
      (*point)[i] = iter.get_float();
    }
    points.push_back(point);
  }
}

Pols *handle_pols(FileIterator iter, Mesh::V3List const &points)
{
  Pols *pols = new Pols();
  pols->type = iter.get_fixed_string(4);
  while (iter.remaining()) {
    unsigned short num_vertices_and_flags = iter.get_uint(2);
    int num_vertices = num_vertices_and_flags & 0x3ff;
    // TODO: ignoring flags for now.
    Mesh::Polygon *vertex_pointers = new Mesh::Polygon();
    for (int i = 0; i < num_vertices; ++i) {
      vertex_pointers->push_back(points[iter.get_vx()]);
    }
    pols->polygons.push_back(vertex_pointers);
  }
  return pols;
}

Lwo *parse(void const *lwo_data)
{
  FileIterator file_iter(static_cast<char const *>(lwo_data), NULL);
  Lwo *result = new Lwo();
  Layer *current_layer = NULL;
  Mesh::V3List current_points;
  std::string container_type = file_iter.get_fixed_string(4);
  assert (container_type == "FORM");
  unsigned int form_length = file_iter.get_uint(4);
  FileIterator container_iter(file_iter.pointer(),
			      file_iter.pointer() + form_length);
  std::string form_type = container_iter.get_fixed_string(4);
  assert (form_type == "LWO2");
  while (container_iter.remaining()) {
    std::string chunk_type = container_iter.get_fixed_string(4);
    unsigned int chunk_size = container_iter.get_uint(4);
    FileIterator chunk_contents_iter(container_iter.pointer(),
				     container_iter.pointer() + chunk_size);
    container_iter.advance(chunk_size);
    if (chunk_type == "LAYR") {
      current_layer = new Layer();
      current_points.clear();
      result->layers.push_back(current_layer);
    } else if (chunk_type == "PNTS") {
      if (!current_points.empty()) {
	cout << "Warning: multiple PNTS declarations in one layer" << endl;
	current_points.clear();
      }
      handle_pnts(chunk_contents_iter, current_points);
    } else if (chunk_type == "POLS") {
      current_layer->polygons.push_back(
          handle_pols(chunk_contents_iter, current_points));
    }
  }
  return result;
}

};
