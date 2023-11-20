#ifndef INC_3DGAME_MAIN_3DGAME_H
#define INC_3DGAME_MAIN_3DGAME_H

#include "glm.h";

typedef struct{
    GLfloat x, y, z;        // position
    GLint velocity;   // Velocity
    GLfloat direction;  // Direction
    GLint size;        // size
} Car;

typedef struct maze{
    int width;
    int height;
    int** cells;
}MAZE;

typedef struct model{
    MAZE maze;
    Car car;
    GLboolean paused;           // Game paused (GL_TRUE/GL_FALSE)

} MODEL;

void display();
void specialKeys(int key, int x, int y);
int collides_With_Wall(float nextX, float nextY);
int main_3DGame(int argc, char** argv);

#endif
