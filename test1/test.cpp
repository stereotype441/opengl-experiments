#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdlib.h>
#include <iostream>

#include "generated.h"
#include "vertex.glsl.g.h"

using namespace std;

int rotation = 0;

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

bool show_polys = true;

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

void setup_shaders()
{
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &shader_vertex, NULL);
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

  if (show_polys) {
    for (int i = 0; i < layer_0_pols_PTCH_size; ++i) {
      glColor3fv(colors[i % 25]);
      glNormal3fv(layer_0_normals_PTCH[i]);
      glBegin(GL_POLYGON);
      for (int j = 0; j < layer_0_pols_PTCH_sizes[i]; ++j) {
	glVertex3fv(layer_0_points[layer_0_pols_PTCH[i][j]]);
      }
      glEnd();
    }
  } else {
    for (int i = 0; i < layer_0_tris_PTCH_size; ++i) {
      glColor3fv(colors[i % 25]);
      glBegin(GL_POLYGON);
      for (int j = 0; j < 3; ++j) {
	glVertex3fv(layer_0_points[layer_0_tris_PTCH[i][j]]);
      }
      glEnd();
    }
  }
  glFlush();
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
  case 'p':
    show_polys = !show_polys;
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
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH); // TODO: try double-buffering
  glutInitWindowSize(800, 800);
  glutInitWindowPosition(0, 0);
  int window = glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  setup_shaders();
  glutMainLoop();
}
