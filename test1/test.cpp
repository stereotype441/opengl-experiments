#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

void display()
{
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(0.5, 1.0, 0.2);
  glBegin(GL_LINES);
  glVertex2f(0, 0);
  glVertex2f(100, 100);
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
  glutInitWindowSize(100, 100);
  glutInitWindowPosition(0, 0);
  int window = glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  // TODO: try glutKeyboardFunc
  glutMainLoop();
}
