#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include "main_3dgame.h"
#include "glm.h"

#define CUBE_SIZE 1.0
#define CYLINDER_BASE 0.25
#define CYLINDER_TOP 0.25
#define CYLINDER_HEIGHT 0.25
#define CYLINDER_SLICES 20
#define CYLINDER_STACKS 20
#define X_WHEEL 0.75
#define Y_WHEEL 0.75
#define Z_WHEEL 0.75
#define RIGHT_WHEELS_ADJUSTMENTS 0.25
#define X_COMPONENT 0
#define Z_COMPONENT 1

//Camera parameters
float camera_distance = 5.0;
float camera_rotation_x = 0.0;
float camera_rotation_y = 0.0;
int last_mouse_x = 0;
int last_mouse_y = 0;
bool mouse_pressed = false;
bool rotating_left = false;
bool rotating_right = false;


/**************************************
***          GLOBAL VARIABLES        **
**************************************/
float car_color[] = {1.0, 0.0, 0.0};
float wheel_color[] = {0.0, 0.0, 1.0};

MODEL model;

typedef struct movement{
    float car_speed;
    float car_speed_components[2];
    float car_rotation_angle;
    float deceleration_rate;
    float load_balance_angle;
}Movement;

Movement mov;


/**************************************
***          OPENGL INIT             **
**************************************/

void init_game(){

    model.car.x = 0.0f;
    model.car.y = 0.0f;
    model.car.z = 0.0f;
    mov.car_speed = 0.0f;
    mov.car_rotation_angle = 90.0f;
    mov.deceleration_rate = 0.0f;
    mov.load_balance_angle = 0.0f;
}

/**************************************
***          CREATE MENU            **
**************************************/

void menu(int value) {

    switch (value) {
        case 0:
            car_color[0] = 1.0;
            car_color[1] = 0.0;
            car_color[2] = 0.0;
            break;
        case 1:
            car_color[0] = 0.0;
            car_color[1] = 1.0;
            car_color[2] = 0.0;
            break;
        case 2:
            car_color[0] = 0.0;
            car_color[1] = 0.0;
            car_color[2] = 1.0;; break;
        case 3:
            wheel_color[0] = 1.0;
            wheel_color[1] = 0.0;
            wheel_color[2] = 0.0;
            break;
        case 4:
            wheel_color[0] = 0.0;
            wheel_color[1] = 1.0;
            wheel_color[2] = 0.0;
            break;
        case 5:
            wheel_color[0] = 0.0;
            wheel_color[1] = 0.0;
            wheel_color[2] = 1.0;
            break;
        case 6:
            car_color[0] = 1.0;
            car_color[1] = 0.0;
            car_color[2] = 0.0;

            wheel_color[0] = 0.0;
            wheel_color[1] = 0.0;
            wheel_color[2] = 1.0;
            break;
        case 7:
            camera_rotation_x = 0.0;
            camera_rotation_y = 0.0;
            break;
    }
    glutPostRedisplay();
}

void createMenu() {
    int rotationMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Car color: red", 0);
    glutAddMenuEntry("Car color: green", 1);
    glutAddMenuEntry("Car color: blue", 2);
    glutAddMenuEntry("Wheel color: red", 3);
    glutAddMenuEntry("Wheel color: green", 4);
    glutAddMenuEntry("Wheel color: blue", 5);
    glutAddMenuEntry("Default colors", 6);
    glutAddMenuEntry("Reset camera", 7);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void drawText(float x, float y, const char *text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *text);
        ++text;
    }
}

/**************************************
***          CAMERA FUNC            **
**************************************/

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            last_mouse_x = x;
            last_mouse_y = y;
            mouse_pressed = true;
        }
        else if (state == GLUT_UP) {
            mouse_pressed = false;
        }
    }
}

void mouse_motion(int x, int y) {
    if (mouse_pressed) {
        int delta_x = x - last_mouse_x;
        int delta_y = y - last_mouse_y;
        last_mouse_x = x;
        last_mouse_y = y;

        camera_rotation_x += (float)delta_y;
        camera_rotation_y += (float)delta_x;

        glutPostRedisplay();
    }
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

/**************************************
***          DRAW FUNC             **
**************************************/

void drawCube() {
    glutSolidCube(CUBE_SIZE);
}

void drawCylinder() {
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, CYLINDER_BASE, CYLINDER_TOP, CYLINDER_HEIGHT, CYLINDER_SLICES, CYLINDER_STACKS);

    // Draw top disk
    glPushMatrix();
    glTranslatef((float)0.0, (float)0.0, CYLINDER_HEIGHT);
    gluDisk(quad, 0.0, CYLINDER_TOP, CYLINDER_SLICES, 1);
    glPopMatrix();

    // Invert the normals for the bottom disk
    gluQuadricOrientation(quad, GLU_INSIDE);
    glPushMatrix();
    gluDisk(quad, 0.0, CYLINDER_BASE, CYLINDER_SLICES, 1);
    glPopMatrix();

    gluDeleteQuadric(quad);
}

void draw_car() {

    glPushMatrix();
        // Draw the car body (disproportional scale to simulate the body of a car)
        glScalef(1.5, 1.0, 0.75);


        if(rotating_right){
            glRotatef(mov.load_balance_angle, 0.0, 0.0, 1.0);
        }
        else {
            glRotatef(0.0, 0.0, 0.0, 1.0); // No load balance angle when not rotating
        }

    glColor3fv(car_color);
    drawCube();

    glPopMatrix();

    // Draw four wheels
    glColor3fv(wheel_color);

    // Angle for wheel rotation
    static float wheelRotation = 0.0;

    // Front left wheel
    glPushMatrix();
        glTranslatef((float)-X_WHEEL , (float)-Y_WHEEL , (float)-Z_WHEEL );
        drawCylinder();
    glPopMatrix();

    // Front right wheel
    glPushMatrix();
        glTranslatef((float)-X_WHEEL, (float)-Y_WHEEL, (float)(Z_WHEEL - RIGHT_WHEELS_ADJUSTMENTS));
        drawCylinder();
    glPopMatrix();

    // Rear left wheel
    glPushMatrix();
        glTranslatef((float)X_WHEEL, (float)-Y_WHEEL, (float)-Z_WHEEL);
        drawCylinder();
    glPopMatrix();

    // Rear right wheel
    glPushMatrix();
        glTranslatef((float)X_WHEEL, (float)-Y_WHEEL, (float)(Z_WHEEL - RIGHT_WHEELS_ADJUSTMENTS));
        drawCylinder();
    glPopMatrix();

    wheelRotation += 5.0;
    if (wheelRotation >= 360.0) {
        wheelRotation -= 360.0;
    }
}


/**************************************
***          DISPLAY FUNC             **
**************************************/

void display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    //Player's point of view
    gluLookAt(0.0, 10.0, camera_distance, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glPushMatrix();

        glTranslatef(model.car.x, model.car.y, model.car.z);

        glRotatef(camera_rotation_x, 1.0, 0.0, 0.0); // Apply rotation to the car
        glRotatef(camera_rotation_y, 0.0, 1.0, 0.0); // Apply rotation to the car

        // Rotate the car around the Y-axis by 90 degrees to position the smaller side to the front
        glRotatef(mov.car_rotation_angle, 0.0, 1.0, 0.0);

        //glutWireCube(1.0);

        draw_car();

    glPushMatrix();
    glTranslatef(0.0, 1.0, 0.0); // Translate to the top of the car
    glRotatef(-mov.car_rotation_angle, 0.0, 1.0, 0.0); // Apply the opposite rotation to the text
    glColor3f(0.0, 1.0, 0.0);
    drawText(0.0, 0.0, "1");
    glPopMatrix();

    glPopMatrix(); // Restore the previous matrix state


    glutSwapBuffers();
}

/**************************************
***          KEYBOARD FUNC             **
**************************************/

float speed_components(){
    float speedX = mov.car_speed * cosf(mov.car_rotation_angle * M_PI / 180.0);
    float speedZ = mov.car_speed * sinf(mov.car_rotation_angle * M_PI / 180.0);

    mov.car_speed_components[X_COMPONENT] = speedX;
    mov.car_speed_components[Z_COMPONENT] = speedZ;
}

void specialKeys(int key, int x, int y) {
    // Store the current position as the previous position
    float prevX = model.car.x;
    float prevY = model.car.y;
    mov.load_balance_angle = 0.0f;
    rotating_right = false;

    switch (key) {
        case GLUT_KEY_UP:
            //Since the car can change the direction of its movements, we need to decompose the speed
            speed_components();
            model.car.x += mov.car_speed_components[X_COMPONENT];
            model.car.z -= mov.car_speed_components[Z_COMPONENT];
            mov.car_speed += 0.1f;
            break;
        case GLUT_KEY_DOWN:
            speed_components();
            model.car.x -= mov.car_speed_components[X_COMPONENT];
            model.car.z += mov.car_speed_components[Z_COMPONENT];
            mov.car_speed += 0.1f;
            break;
        case GLUT_KEY_LEFT:
            mov.car_rotation_angle += 5.0f;
            break;
        case GLUT_KEY_RIGHT:
            mov.car_rotation_angle -= 5.0f;
            rotating_right = true;
            mov.load_balance_angle = 10.0f * mov.car_speed;
            break;
        case '+':
            camera_distance -= 0.1;
            break;
        case '-':
            camera_distance += 0.1;
            break;
        default:
            // Calculate deceleration rate proportional to the current speed
            mov.deceleration_rate = 0.01f * mov.car_speed;
            if(mov.car_speed > 0.0){
                mov.car_speed -= mov.deceleration_rate;
                speed_components();
                model.car.x += mov.car_speed_components[X_COMPONENT];
                model.car.z -= mov.car_speed_components[Z_COMPONENT];
            }
            break;
    }

    // Check for collisions and revert to the previous position if there is a collision
    if (collides_With_Wall(model.car.x, model.car.y)) {
        model.car.x = prevX;
        model.car.y = prevY;
    }

    if (mov.car_rotation_angle >= 360.0) {
        mov.car_rotation_angle -= 360.0;
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

    glClearColor(0.0, 0.0, 0.0, 1.0);

    init_game();
    //create menu
    createMenu();

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(display);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(mouse_motion);
    glutReshapeFunc(reshape);

    glEnable(GL_DEPTH_TEST);
    glutMainLoop();


    return 0;
}
