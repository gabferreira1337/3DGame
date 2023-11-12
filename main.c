#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "glm.h"

typedef struct
{
    GLfloat x, y;        // position
    GLint velocidade;   // Velocity
    GLfloat direction;  // Direction
    GLint size;        // size
} Car;

typedef struct{

    Car player1;
    GLboolean paused;           // Game paused (GL_TRUE/GL_FALSE)

} Model;


Model model;

/**************************************
***          OPENGL INIT             **
**************************************/

void init_game(){


}




void displayMe(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.5, 0.0, 0.0);
    glVertex3f(0.5, 0.5, 0.0);
    glVertex3f(0.0, 0.5, 0.0);
    glEnd();
    glFlush();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(300, 300);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Hello world");
    glClearColor(1.0, 1.0, 1.0, 0.0);

    glutDisplayFunc(displayMe);
    glutMainLoop();
    return 0;
}