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
  }

  GpuBuffer m_vertices;
  GpuBuffer m_normals;
  GpuBuffer m_surface_indices;
  GpuBuffer m_mobius_flags;
  GpuBuffer m_triangles;
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
  }
};

int lr_rotation = 0;
int ud_rotation = 0;
float lr_translation = 0;
float ud_translation = 0;
float zoom = 1.0;

int current_surface = 0;
GLuint current_surface_handle;
GpuBuffer *triangle_buffer;
int num_surfaces;
bool show_mobius = false;
bool one_surface_only = false;
GLuint one_surface_only_handle;
GLuint show_mobius_handle;
GLuint surface_index_handle;
GLuint mobius_flag_handle;
ModelData *model_data;

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

void setup_data(GpuProgram *program)
{
  Pmf::Data pmf_data(&_binary_elfegab_pmf_start);
  pmf_data.get_typed_metadata("num_surfaces", &num_surfaces);

  model_data = new ModelData(pmf_data);

  surface_index_handle = glGetAttribLocation(program->handle(), "surface_index");
  current_surface_handle = glGetUniformLocation(program->handle(), "current_surface");
  mobius_flag_handle = glGetAttribLocation(program->handle(), "mobius_flag");
  show_mobius_handle = glGetUniformLocation(program->handle(), "show_mobius");
  one_surface_only_handle = glGetUniformLocation(program->handle(), "one_surface_only");

  // Get ready to use the program
  glUseProgram(program->handle());
}

void display()
{
  glEnable(GL_DEPTH_TEST);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //  glColor3f(0.5, 1.0, 0.2);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glScalef(1.0, 1.0, 1.0/zoom);
  glTranslatef(lr_translation, ud_translation, 0.0);
  glRotatef(ud_rotation, -1.0, 0.0, 0.0);
  glRotatef(lr_rotation, 0.0, 1.0, 0.0);
  glScalef(zoom, zoom, zoom);
  glUniform1f(current_surface_handle, current_surface);
  glUniform1f(show_mobius_handle, show_mobius);
  glUniform1f(one_surface_only_handle, one_surface_only);

  // glColor3fv(colors[i % 25]);
  set_vertices(&model_data->m_vertices, 0);
  set_normals(&model_data->m_normals, 0);
  set_scalar_vertex_attrib(
      &model_data->m_surface_indices, surface_index_handle, 0);
  set_scalar_vertex_attrib(
      &model_data->m_mobius_flags, mobius_flag_handle, 0);
  draw_elements(
      &model_data->m_triangles,
      model_data->m_triangles.size() / sizeof(unsigned int), 0);
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
    current_surface = (current_surface + 1) % num_surfaces;
    cout << "Surface " << current_surface << endl;
    break;
  case 'v':
    current_surface = (current_surface + num_surfaces - 1) % num_surfaces;
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
  case GLUT_KEY_LEFT:
    lr_translation -= 0.01/zoom;
    break;
  case GLUT_KEY_RIGHT:
    lr_translation += 0.01/zoom;
    break;
  case GLUT_KEY_DOWN:
    ud_translation -= 0.01/zoom;
    break;
  case GLUT_KEY_UP:
    ud_translation += 0.01/zoom;
    break;
  default:
    cout << "special " << key << endl;
    break;
  }
  display();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); // TODO: try double-buffering
  glutInitWindowSize(800, 800);
  glutInitWindowPosition(0, 0);
  int window = glutCreateWindow(argv[0]);
  (void) window;
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  setup_data(new MyProgram());
  glutMainLoop();
}
