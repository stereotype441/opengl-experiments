#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdlib.h>
#include <iostream>

#include "generated.h"

using namespace std;

int rotation = 0;

float colors[6][3] = {
  { 0, 0, 1 },
  { 0, 1, 0 },
  { 0, 1, 1 },
  { 1, 0, 0 },
  { 1, 0, 1 },
  { 1, 1, 0 },
};

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

  for (int i = 0; i < layer_0_pols_PTCH_size; ++i) {
    glColor3fv(colors[i % 6]);
    glBegin(GL_POLYGON);
    for (int j = 0; j < layer_0_pols_PTCH_sizes[i]; ++j) {
      glVertex3fv(layer_0_points[layer_0_pols_PTCH[i][j]]);
    }
    glEnd();
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
  default:
    cout << "key " << (int) key << endl;
    break;
  }
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
  glutMainLoop();
}
