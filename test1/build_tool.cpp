#include <cstdlib>
#include <iostream>

#include "lwo_parser.h"
#include "pmf.h"
#include "file_tools.h"

using namespace std;

Lwo::Lwo *parse_lwo_file(char const *filename)
{
  MappedFile file(filename);
  return Lwo::parse(file.data());
}

void write_pmf_file(char const *filename, Pmf::Data const &data)
{
  std::vector<char> *serialized_data = data.Serialize();
  int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    exit(1);
  }
  ssize_t written = write(fd, &(*serialized_data)[0], serialized_data->size());
  if (static_cast<size_t>(written) != serialized_data->size()) {
    exit(1);
  }
  if (close(fd) == -1) {
    exit(1);
  }
  delete serialized_data;
}

int main()
{
  Lwo::Lwo *model = parse_lwo_file("../models/elfegab.lwo");

  // Compute vertex, normal, and triangle vectors.
  std::map<Vector<3> const *, Vector<3> > normals;
  std::map<Mesh::V3 const *, bool> mobius_flags;
  Mesh::compute_mesh_normals(model->layers[0]->polygons[1]->polygons,
			     normals, mobius_flags);
  std::vector<Mesh::Mesh> surfaces;
  Mesh::split_mesh_pointwise(model->layers[0]->polygons[1]->polygons, surfaces);
  int num_surfaces = surfaces.size();
  cout << "Found " << num_surfaces << " surfaces." << endl;
  std::map<Mesh::V3 const *, int> surface_indices;
  Mesh::compute_surface_indices(surfaces, surface_indices);

  Pmf::Data pmf_data;
  std::vector<int> surface_starts;
  std::vector<int> surface_lengths;
  std::vector<Mesh::Bbox> surface_bboxes;
  for (int i = 0; i < static_cast<int>(surfaces.size()); ++i) {
    std::vector<Vector<3> const *> triangles;
    surface_bboxes.push_back(Mesh::compute_bbox(surfaces[i]));
    Mesh::mesh_to_triangles(surfaces[i], triangles);
    int start = pmf_data.triangles().size();
    surface_starts.push_back(start);
    pmf_data.add_triangles(triangles);
    surface_lengths.push_back(pmf_data.triangles().size() - start);
  }
  pmf_data.add_vector_properties("normal", normals);
  pmf_data.add_scalar_properties("surface_index", surface_indices);
  pmf_data.add_scalar_properties("mobius_flag", mobius_flags);
  pmf_data.add_typed_metadata("num_surfaces", &num_surfaces);
  pmf_data.add_vector_metadata("surface_starts", surface_starts);
  pmf_data.add_vector_metadata("surface_lengths", surface_lengths);
  pmf_data.add_vector_metadata("surface_bboxes", surface_bboxes);

  write_pmf_file("elfegab.pmf", pmf_data);
}
