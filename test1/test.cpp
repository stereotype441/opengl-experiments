#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "generated.h"

void display()
{
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(0.5, 1.0, 0.2);
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

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE); // TODO: try double-buffering
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(0, 0);
  int window = glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  // TODO: try glutKeyboardFunc
  glutMainLoop();
}
