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
  extern char _binary____models_elfegab_lwo_start;
}

using namespace std;

int rotation = 0;
int num_triangles;

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

GLuint setup_shaders()
{
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  GLchar const *strings[1] = { &_binary_vertex_glsl_start };
  GLint lengths[1] = { &_binary_vertex_glsl_end - &_binary_vertex_glsl_start };
  glShaderSource(vertex_shader, 1, strings, lengths);
  glCompileShader(vertex_shader);
  GLint success;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint info_log_length;
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &info_log_length);
    GLchar *msg = new GLchar[info_log_length];
    glGetShaderInfoLog(vertex_shader, info_log_length, NULL, msg);
    cout << "Compile failed: " << msg << endl;
    delete msg;
    exit(1);
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
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
  Mesh::compute_polygon_normals(model->layers[0]->polygons[1]->polygons,
				normals);
  std::vector<Vector<3> const *> triangles;
  Mesh::polygons_to_triangles(model->layers[0]->polygons[1]->polygons,
			      triangles);
  Mesh::PointSet points;
  std::vector<int> triangle_indices;
  points.translate(triangles, triangle_indices);
  std::vector<Vector<3> > normal_array;
  points.translate(normals, normal_array);

  // Set up vertex inputs
  glGenBuffers(1, &buffer);	// Allocate GLU buffer
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
  glRotatef(rotation, 0.0, 1.0, 0.0);
  float scale_amount = 1.8;
  glScalef(scale_amount, scale_amount, scale_amount);

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
  case 'f':
    while (true) {
      rotation += 1;
      display();
    }
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
    rotation += 1;
    break;
  case GLUT_KEY_RIGHT:
    rotation -= 1;
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
