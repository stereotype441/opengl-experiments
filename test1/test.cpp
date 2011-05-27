#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdlib.h>
#include <iostream>

#include "generated.h"

using namespace std;

int rotation = 0;

void display()
{
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(0.5, 1.0, 0.2);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glRotatef(rotation, 0.0, 1.0, 0.0);

  glBegin(GL_POINTS);
  for (int i = 0; i < num_points_in_layer_0; ++i) {
    glVertex3f(layer_0_points[i][0], layer_0_points[i][1],
	       layer_0_points[i][2]);
  }
  glEnd();
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
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE); // TODO: try double-buffering
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(0, 0);
  int window = glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  glutMainLoop();
}
