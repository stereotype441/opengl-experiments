#include "gpu_wrappers.h"

#include <iostream>

using namespace std;

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
    GpuBuffer const *buffer, GLuint attrib_handle, size_t offset)
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

GpuShader::GpuShader(GLenum type, char const *source, size_t length)
  : m_handle(glCreateShader(type))
{
  GLchar const *source_strings[1] = { source };
  GLint source_lengths[1] = { length };
  glShaderSource(m_handle, 1, source_strings, source_lengths);
  glCompileShader(m_handle);
  GLint success;
  glGetShaderiv(m_handle, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint info_log_length;
    glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &info_log_length);
    GLchar *msg = new GLchar[info_log_length];
    glGetShaderInfoLog(m_handle, info_log_length, NULL, msg);
    cout << "Compile failed: " << msg << endl;
    delete msg;
    throw GpuException();
  }
}

GpuProgram::GpuProgram()
  : m_handle(glCreateProgram())
{
}

void GpuProgram::Attach(GpuShader const &shader)
{
  glAttachShader(m_handle, shader.handle());
}

void GpuProgram::Link()
{
  glLinkProgram(m_handle);
  GLint success;
  glGetProgramiv(m_handle, GL_LINK_STATUS, &success);
  if (!success) {
    GLint info_log_length;
    glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &info_log_length);
    GLchar *msg = new GLchar[info_log_length];
    glGetProgramInfoLog(m_handle, info_log_length, NULL, msg);
    cout << "Compile failed: " << msg << endl;
    delete msg;
    throw GpuException();
  }
}

GLint GpuProgram::get_attrib_loc(char const *name) const
{
  return glGetAttribLocation(m_handle, name);
}

GLint GpuProgram::get_uniform_loc(char const *name) const
{
  return glGetUniformLocation(m_handle, name);
}

void set_program(GpuProgram const *program)
{
  glUseProgram(program->handle());
}
