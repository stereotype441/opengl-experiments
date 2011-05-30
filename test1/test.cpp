#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdlib.h>
#include <iostream>

#include "lwo_parser.h"
#include "mesh.h"

extern "C" {
  extern char _binary_vertex_glsl_start;
  extern char _binary_vertex_glsl_end;
  extern char _binary_fragment_glsl_start;
  extern char _binary_fragment_glsl_end;
  extern char _binary____models_elfegab_lwo_start;
}

using namespace std;

int lr_rotation = 0;
int ud_rotation = 0;
float lr_translation = 0;
float ud_translation = 0;
float zoom = 1.0;

int current_surface = 0;
GLuint current_surface_handle;
int num_triangles;
int num_surfaces;
bool show_mobius = false;
bool one_surface_only = false;
GLuint one_surface_only_handle;
GLuint show_mobius_handle;

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

void check_program_info_log(GLuint program, GLenum pname)
{
  GLint success;
  glGetProgramiv(program, pname, &success);
  if (!success) {
    GLint info_log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
    GLchar *msg = new GLchar[info_log_length];
    glGetProgramInfoLog(program, info_log_length, NULL, msg);
    cout << "Compile failed: " << msg << endl;
    delete msg;
    exit(1);
  }
}

void check_shader_info_log(GLuint shader)
{
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint info_log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
    GLchar *msg = new GLchar[info_log_length];
    glGetShaderInfoLog(shader, info_log_length, NULL, msg);
    cout << "Compile failed: " << msg << endl;
    delete msg;
    exit(1);
  }
}

GLuint setup_shaders()
{
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  GLchar const *vertex_strings[1] = { &_binary_vertex_glsl_start };
  GLint vertex_lengths[1] = {
    &_binary_vertex_glsl_end - &_binary_vertex_glsl_start
  };
  glShaderSource(vertex_shader, 1, vertex_strings, vertex_lengths);
  glCompileShader(vertex_shader);
  check_shader_info_log(vertex_shader);

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  GLchar const *fragment_strings[1] = { &_binary_fragment_glsl_start };
  GLint fragment_lengths[1] = {
    &_binary_fragment_glsl_end - &_binary_fragment_glsl_start };
  glShaderSource(fragment_shader, 1, fragment_strings, fragment_lengths);
  glCompileShader(fragment_shader);
  check_shader_info_log(fragment_shader);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  check_program_info_log(program, GL_LINK_STATUS);
  glValidateProgram(program);
  check_program_info_log(program, GL_VALIDATE_STATUS);
  return program;
}

void setup_data(GLuint program)
{
  GLuint buffer;

  Lwo::Lwo *model = Lwo::parse(&_binary____models_elfegab_lwo_start);

  // Compute vertex, normal, and triangle vectors.
  std::map<Vector<3> const *, Vector<3> > normals;
  std::map<Mesh::V3 const *, bool> mobius_flags;
  Mesh::compute_mesh_normals(model->layers[0]->polygons[1]->polygons,
			     normals, mobius_flags);
  std::vector<Mesh::Mesh> surfaces;
  Mesh::split_mesh_pointwise(model->layers[0]->polygons[1]->polygons, surfaces);
  num_surfaces = surfaces.size();
  cout << "Found " << num_surfaces << " surfaces." << endl;
  std::map<Mesh::V3 const *, int> surface_indices;
  Mesh::compute_surface_indices(surfaces, surface_indices);
  std::vector<Vector<3> const *> triangles;
  Mesh::mesh_to_triangles(model->layers[0]->polygons[1]->polygons,
			  triangles);
  Mesh::PointSet points;
  std::vector<int> triangle_indices;
  points.translate(triangles, triangle_indices);
  std::vector<Vector<3> > normal_array;
  points.translate(normals, normal_array);
  std::vector<int> surface_index_array;
  points.translate(surface_indices, surface_index_array);
  std::vector<char> mobius_flag_array;
  points.translate(mobius_flags, mobius_flag_array);

  // Set up vertex inputs
  glGenBuffers(1, &buffer);	// Allocate GPU buffer
  glBindBuffer(GL_ARRAY_BUFFER, buffer); // Make GL_ARRAY_BUFFER point to buffer
  glBufferData(GL_ARRAY_BUFFER, points.raw().size() * 3 * sizeof(float),
	       &points.raw()[0], GL_STATIC_DRAW
	       ); // Copy data into what GL_ARRAY_BUFFER points to
  glVertexPointer(3, // Num components in each vector
		  GL_FLOAT, // Data type of each vector component
		  0, // Stride, or 0 if packed
		  0 // pointer (within GL_ARRAY_BUFFER I presume?)
		  );
  glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind for safety
  glEnableClientState(GL_VERTEX_ARRAY);

  // Set up normal inputs.
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, normal_array.size() * 3 * sizeof(float),
	       &normal_array[0], GL_STATIC_DRAW);
  glNormalPointer(GL_FLOAT, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glEnableClientState(GL_NORMAL_ARRAY);

  // Set up surface index inputs.
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, surface_indices.size() * sizeof(int),
	       &surface_index_array[0], GL_STATIC_DRAW);
  GLuint surface_index_handle = glGetAttribLocation(program, "surface_index");
  glVertexAttribIPointer(surface_index_handle, 1, GL_INT, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glEnableVertexAttribArray(surface_index_handle);

  current_surface_handle = glGetUniformLocation(program, "current_surface");

  // Set up error flag inputs.
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, surface_indices.size() * sizeof(int),
	       &mobius_flag_array[0], GL_STATIC_DRAW);
  GLuint mobius_flag_handle = glGetAttribLocation(program, "mobius_flag");
  glVertexAttribIPointer(mobius_flag_handle, 1, GL_BYTE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glEnableVertexAttribArray(mobius_flag_handle);

  show_mobius_handle = glGetUniformLocation(program, "show_mobius");
  one_surface_only_handle = glGetUniformLocation(program, "one_surface_only");

  // Set up triangle inputs.  This will be used for future invocations
  // of glDrawElements().
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_indices.size() * sizeof(int),
	       &triangle_indices[0], GL_STATIC_DRAW);
  num_triangles = triangle_indices.size();

  // Get ready to use the program
  glUseProgram(program);
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
  glDrawElements(GL_TRIANGLES, num_triangles, GL_UNSIGNED_INT, 0);
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
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  setup_data(setup_shaders());
  glutMainLoop();
}
