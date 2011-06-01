#include <vector>
#include <iostream>

#include "file_tools.h"
#include "mesh.h"
#include "pmf.h"

using namespace std;

void dump_vectors(std::vector<Mesh::V3> const &data)
{
  for (int i = 0; i < static_cast<int>(data.size()); ++i) {
    cout << "  " << i << ": (" << data[i].coords[0] << ", "
         << data[i].coords[0] << ", " << data[i].coords[0] << ")" << endl;
  }
}

template<class C>
void dump_scalars(std::vector<C> const &data)
{
  for (int i = 0; i < static_cast<int>(data.size()); ++i) {
    cout << "  " << i << ": " << data[i] << endl;
  }
}

void dump_raw_metadata(Pmf::Data const &data, std::string const &key)
{
  std::vector<unsigned char> metadata;
  data.get_vector_metadata(key, metadata);
  char hex[] = "0123456789abcdef";
  for (int i = 0; i < static_cast<int>(metadata.size()); ++i) {
    if (i != 0 && (i % 16 == 0)) {
      cout << endl;
    }
    if (i % 16 == 0) {
      cout << " ";
    }
    cout << " " << hex[metadata[i] >> 4] << hex[metadata[i] & 0xf];
  }
  cout << endl;
}

void dump_float_metadata(
    Pmf::Data const &data, std::string const &key, int group_size)
{
  std::vector<float> metadata;
  data.get_vector_metadata(key, metadata);
  for (int i = 0; i < static_cast<int>(metadata.size()); ++i) {
    if (i != 0 && (i % group_size == 0)) {
      cout << endl;
    }
    if (i % group_size == 0) {
      cout << " ";
    }
    cout << " " << metadata[i];
  }
  cout << endl;
}

void dump_int_metadata(
    Pmf::Data const &data, std::string const &key, int group_size)
{
  std::vector<int> metadata;
  data.get_vector_metadata(key, metadata);
  for (int i = 0; i < static_cast<int>(metadata.size()); ++i) {
    if (i != 0 && (i % group_size == 0)) {
      cout << endl;
    }
    if (i % group_size == 0) {
      cout << " ";
    }
    cout << " " << metadata[i];
  }
  cout << endl;
}

void dump_metadata(Pmf::Data const &data, std::string const &key)
{
  if (key == "num_surfaces") {
    dump_int_metadata(data, key, 1);
  } else if (key == "surface_bboxes") {
    dump_float_metadata(data, key, 6);
  } else if (key == "surface_lengths") {
    dump_int_metadata(data, key, 1);
  } else if (key == "surface_starts") {
    dump_int_metadata(data, key, 1);
  } else {
    dump_raw_metadata(data, key);
  }
}

int main()
{
  Pmf::Data data(static_cast<char const *>(MappedFile("elfegab.pmf").data()));
  cout << "Points:" << endl;
  dump_vectors(data.points());
  cout << "Triangles:" << endl;
  dump_scalars(data.triangles());
  for (std::map<std::string, std::vector<Mesh::V3> >::const_iterator iter
         = data.all_point_vector_properties().begin();
       iter != data.all_point_vector_properties().end(); ++iter) {
    cout << "Point vector property \"" << iter->first << "\"" << endl;
    dump_vectors(iter->second);
  }
  for (std::map<std::string, std::vector<float> >::const_iterator iter
         = data.all_point_scalar_properties().begin();
       iter != data.all_point_scalar_properties().end(); ++iter) {
    cout << "Point scalar property \"" << iter->first << "\"" << endl;
    dump_scalars(iter->second);
  }
  for (std::map<std::string, std::string>::const_iterator iter
         = data.all_metadata().begin(); iter != data.all_metadata().end();
       ++iter) {
    cout << "Metadata \"" << iter->first << "\"" << endl;
    dump_metadata(data, iter->first);
  }
}
