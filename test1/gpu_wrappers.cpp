#include "gpu_wrappers.h"

inline GLvoid const *translate_offset(size_t offset)
{
  return static_cast<char const *>(NULL) + offset;
}

void set_vertices(GpuBuffer const *buffer, size_t offset)
{
  glBindBuffer(GL_ARRAY_BUFFER, buffer->handle());
  glVertexPointer(3, // Num components in each vector
                  GL_FLOAT, // Data type of each vector component
                  0, // Stride, or 0 if packed
                  translate_offset(offset));
  glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind for safety
  glEnableClientState(GL_VERTEX_ARRAY);
}

void set_normals(GpuBuffer const *buffer, size_t offset)
{
  glBindBuffer(GL_ARRAY_BUFFER, buffer->handle());
  glNormalPointer(GL_FLOAT, // Data type of each vector component
                  0, // Stride, or 0 if packed
                  translate_offset(offset));
  glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind for safety
  glEnableClientState(GL_NORMAL_ARRAY);
}

void set_scalar_vertex_attrib(
    GpuBuffer const *buffer, GLuint program, GLuint attrib_handle,
    size_t offset)
{
  glBindBuffer(GL_ARRAY_BUFFER, buffer->handle());
  glVertexAttribPointer(attrib_handle, // Attribute to assign
                        1, // Number of vector elements
                        GL_FLOAT, // Data type of each vector component
                        GL_FALSE, // Auto-normalization flag
                        0, // Stride
                        translate_offset(offset));
  glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind for safety
  glEnableVertexAttribArray(attrib_handle);
}

void draw_elements(GpuBuffer const *buffer, size_t count, size_t offset)
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->handle());
  glDrawElements(
      GL_TRIANGLES, count, GL_UNSIGNED_INT, translate_offset(offset));
}
