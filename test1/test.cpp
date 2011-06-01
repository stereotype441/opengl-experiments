#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdlib.h>
#include <iostream>

#include "mesh.h"
#include "pmf.h"
#include "gpu_wrappers.h"

extern "C" {
  extern char _binary_vertex_glsl_start;
  extern char _binary_vertex_glsl_end;
  extern char _binary_fragment_glsl_start;
  extern char _binary_fragment_glsl_end;
  extern char _binary_elfegab_pmf_start;
}

using namespace std;

class ModelData
{
public:
  ModelData(Pmf::Data const &data)
    : m_vertices(data.points(), GL_ARRAY_BUFFER),
      m_normals(data.point_vector_properties("normal"), GL_ARRAY_BUFFER),
      m_surface_indices(
          data.point_scalar_properties("surface_index"), GL_ARRAY_BUFFER),
      m_mobius_flags(
          data.point_scalar_properties("mobius_flag"), GL_ARRAY_BUFFER),
      m_triangles(data.triangles(), GL_ELEMENT_ARRAY_BUFFER)
  {
    data.get_typed_metadata("num_surfaces", &m_num_surfaces);
    data.get_vector_metadata("surface_starts", m_surface_starts);
    data.get_vector_metadata("surface_lengths", m_surface_lengths);
    data.get_vector_metadata("surface_bboxes", m_surface_bboxes);
  }

  GpuBuffer m_vertices;
  GpuBuffer m_normals;
  GpuBuffer m_surface_indices;
  GpuBuffer m_mobius_flags;
  GpuBuffer m_triangles;
  int m_num_surfaces;
  std::vector<int> m_surface_starts;
  std::vector<int> m_surface_lengths;
  std::vector<Mesh::Bbox> m_surface_bboxes;
};

class MyProgram : public GpuProgram
{
public:
  MyProgram()
  {
    GpuShader vertex_shader(
        GL_VERTEX_SHADER, &_binary_vertex_glsl_start,
        &_binary_vertex_glsl_end - &_binary_vertex_glsl_start);
    Attach(vertex_shader);
    GpuShader fragment_shader(
        GL_FRAGMENT_SHADER, &_binary_fragment_glsl_start,
        &_binary_fragment_glsl_end - &_binary_fragment_glsl_start);
    Attach(fragment_shader);
    Link();

    m_surface_index_handle = get_attrib_loc("surface_index");
    m_current_surface_handle = get_uniform_loc("current_surface");
    m_mobius_flag_handle = get_attrib_loc("mobius_flag");
    m_show_mobius_handle = get_uniform_loc("show_mobius");
  }

  GLuint m_current_surface_handle;
  GLuint m_show_mobius_handle;
  GLuint m_surface_index_handle;
  GLuint m_mobius_flag_handle;
};

int lr_rotation = 0;
int ud_rotation = 0;
float zoom = 1.0;

int current_surface = 0;
bool show_mobius = false;
bool one_surface_only = false;
bool center_on_surface = false;
ModelData *model_data;
MyProgram *program;

float colors[25][3] = {
  { 0.0, 0.0, 0.5 },
  { 0.0, 0.0, 1.0 },
  { 0.0, 0.5, 0.0 },
  { 0.0, 0.5, 0.5 },
  { 0.0, 0.5, 1.0 },
  { 0.0, 1.0, 0.0 },
  { 0.0, 1.0, 0.5 },
  { 0.0, 1.0, 1.0 },
  { 0.5, 0.0, 0.0 },
  { 0.5, 0.0, 0.5 },
  { 0.5, 0.0, 1.0 },
  { 0.5, 0.5, 0.0 },
  { 0.5, 0.5, 0.5 },
  { 0.5, 0.5, 1.0 },
  { 0.5, 1.0, 0.0 },
  { 0.5, 1.0, 0.5 },
  { 0.5, 1.0, 1.0 },
  { 1.0, 0.0, 0.0 },
  { 1.0, 0.0, 0.5 },
  { 1.0, 0.0, 1.0 },
  { 1.0, 0.5, 0.0 },
  { 1.0, 0.5, 0.5 },
  { 1.0, 0.5, 1.0 },
  { 1.0, 1.0, 0.0 },
  { 1.0, 1.0, 0.5 },
};

void display()
{
  Mesh::Bbox surface_bbox = model_data->m_surface_bboxes[current_surface];
  Mesh::V3 surface_bbox_center
    = 0.5 * (surface_bbox.m_min + surface_bbox.m_max);

  set_program(program);
  glEnable(GL_DEPTH_TEST);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //  glColor3f(0.5, 1.0, 0.2);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glScalef(1.0, 1.0, 1.0/zoom);
  glRotatef(ud_rotation, -1.0, 0.0, 0.0);
  glRotatef(lr_rotation, 0.0, 1.0, 0.0);
  glScalef(zoom, zoom, zoom);
  if (center_on_surface) {
    glTranslatef(-surface_bbox_center[0], -surface_bbox_center[1],
                 -surface_bbox_center[2]);
  }

  glUniform1f(program->m_current_surface_handle, current_surface);
  glUniform1f(program->m_show_mobius_handle, show_mobius);

  // glColor3fv(colors[i % 25]);
  set_vertices(&model_data->m_vertices, 0);
  set_normals(&model_data->m_normals, 0);
  set_scalar_vertex_attrib(
      &model_data->m_surface_indices, program->m_surface_index_handle, 0);
  set_scalar_vertex_attrib(
      &model_data->m_mobius_flags, program->m_mobius_flag_handle, 0);
  size_t count;
  size_t start;
  if (one_surface_only) {
    start = model_data->m_surface_starts[current_surface];
    count = model_data->m_surface_lengths[current_surface];
  } else {
    start = 0;
    count = model_data->m_triangles.size() / sizeof(unsigned int);
  }
  draw_elements(&model_data->m_triangles, count, start * sizeof(unsigned int));
  Mesh::V3 surface_bbox_size = surface_bbox.m_max - surface_bbox.m_min;
  glTranslatef(
      surface_bbox.m_min[0], surface_bbox.m_min[1], surface_bbox.m_min[2]);
  glScalef(surface_bbox_size[0], surface_bbox_size[1], surface_bbox_size[2]);
  glTranslatef(0.5, 0.5, 0.5);
  glUseProgram(0);
  glColor3f(1, 1, 1);
  glutWireCube(1.0);
  glutSwapBuffers();
}

void reshape(int w, int h)
{
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 0x03: // Ctrl-C
  case 0x1b: // ESC
    exit(0);
    break;
  case 'm':
    show_mobius = !show_mobius;
    break;
  case 'h':
    one_surface_only = !one_surface_only;
    break;
  case 'c':
    center_on_surface = !center_on_surface;
    break;
  case 'q':
    ud_rotation += 1;
    if (ud_rotation > 90) {
      ud_rotation = 90;
    }
    break;
  case 'a':
    ud_rotation -= 1;
    if (ud_rotation < -90) {
      ud_rotation = -90;
    }
    break;
  case 'x':
    lr_rotation = (lr_rotation + 1) % 360;
    break;
  case 'z':
    lr_rotation = (lr_rotation + 359) % 360;
    break;
  case '[':
    zoom *= 0.99;
    break;
  case ']':
    zoom *= 1.01;
    break;
  case 'b':
    current_surface = (current_surface + 1) % model_data->m_num_surfaces;
    cout << "Surface " << current_surface << endl;
    break;
  case 'v':
    current_surface = (current_surface + model_data->m_num_surfaces - 1)
      % model_data->m_num_surfaces;
    cout << "Surface " << current_surface << endl;
    break;
  default:
    cout << "key " << (int) key << endl;
    break;
  }
  display();
}

void special(int key, int x, int y)
{
  switch (key) {
  default:
    cout << "special " << key << endl;
    break;
  }
  display();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(800, 800);
  glutInitWindowPosition(0, 0);
  int window = glutCreateWindow(argv[0]);
  (void) window;
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  program = new MyProgram();
  model_data = new ModelData(Pmf::Data(&_binary_elfegab_pmf_start));
  glutMainLoop();
}
