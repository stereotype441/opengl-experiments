#pragma once

#include <vector>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

class GpuBuffer
{
public:
  GpuBuffer()
  {
    glGenBuffers(1, &m_handle);
  }

  template<class C>
  GpuBuffer(std::vector<C> const &data, GLenum target)
  {
    glGenBuffers(1, &m_handle);
    Fill(data, target);
  }

  template<class C>
  void Fill(std::vector<C> const &data, GLenum target)
  {
    glBindBuffer(target, m_handle);
    m_size = data.size() * sizeof(C);
    glBufferData(target, m_size, &data[0], GL_STATIC_DRAW);
    glBindBuffer(target, 0);
  }

  size_t size() const
  {
    return m_size;
  }

  GLuint handle() const
  {
    return m_handle;
  }

private:
  GLuint m_handle;
  size_t m_size;
};

void set_vertices(GpuBuffer const *buffer, size_t offset);

void set_normals(GpuBuffer const *buffer, size_t offset);

void set_scalar_vertex_attrib(
    GpuBuffer const *buffer, GLuint program, GLuint attrib_handle,
    size_t offset);

void draw_elements(GpuBuffer const *buffer, size_t count, size_t offset);
