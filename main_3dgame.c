#include <stdio.h>
#include "main_3dgame.h"
#include "glm.h"



MODEL model;

/**************************************
***          OPENGL INIT             **
**************************************/

void init_game(){

    model.car.x = 0.0f;
    model.car.y = 0.0f;
    model.car.z = 0.0f;

}




/**************************************
***          DISPLAY FUNC             **
**************************************/



void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set the camera position
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glColor3f(1.0, 0.0, 1.0);

    glTranslatef(model.car.x, model.car.y, model.car.z);
    glutWireCube(1.0);

    glutSwapBuffers();
}

/**************************************
***          KEYBOARD FUNC             **
**************************************/

void specialKeys(int key, int x, int y) {
    // Store the current position as the previous position
    float prevX = model.car.x;
    float prevY = model.car.y;

    switch (key) {
        case GLUT_KEY_UP:
            model.car.y += 0.1f;
            break;
        case GLUT_KEY_DOWN:
            model.car.y -= 0.1f;
            break;
        case GLUT_KEY_LEFT:
            model.car.x -= 0.1f;
            break;
        case GLUT_KEY_RIGHT:
            model.car.x += 0.1f;
            break;
    }

    // Check for collisions and revert to the previous position if there is a collision
    if (collides_With_Wall(model.car.x, model.car.y)) {
        model.car.x = prevX;
        model.car.y = prevY;
    }

    glutPostRedisplay();
}

int collides_With_Wall(float nextX, float nextY) {
    // Convert float positions to array indices
    int i = (int)nextX;
    int j = (int)nextY;

    // Check if the next position is within the labyrinth bounds and is a wall
    return (i >= 0 && i < model.maze.width && j >= 0 && j < model.maze.height && model.maze.cells[i][j] == 1);
}

int main_3DGame(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("MAZE RUNNER");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(display);
    glutSpecialFunc(specialKeys);

    glutMainLoop();


    return 0;
}
