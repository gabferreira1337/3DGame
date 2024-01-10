#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "labirinto.h"

#define GL_SILENCE_DEPRECATION

#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>

#else
#include <GL/glut.h>
#endif

#include "glm.h"

/**************************************
************* CONSTANTE PI ************
**************************************/

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

/**************************************
* AUXILIARES CONVERSÃO GRAUS-RADIANOS *
**************************************/

#define RAD(x)          (M_PI*(x)/180)
#define GRAUS(x)        (180*(x)/M_PI)

#define DEBUG 1

#define	GAP					      25

#define MAZE_HEIGHT               22
#define MAZE_WIDTH                22
#define CAMERA_ROTATION           0.1

#define EXIT_X                    10.0f
#define EXIT_Y                    1.0f
#define	OBJETO_ALTURA		      0.4f
#define OBJETO_VELOCIDADE	      0.1f
#define SCALE_PERSONAGEM            0.4f
#define EYE_ROTACAO			          1
#define CAMERA_HEIGHT_OFFSET    1.5f
#define CAMERA_DISTANCE         5.0f
#define FOV_CONSTANT 60.0f  // Human eye natural
#define POWERUP_SIZE 0.20f
#define CUBE_SIZE 0.5f
#define HARD_TIME 60 //start hardcore mode when 60 seconds left
#define VELOCITY_MULT 0.012f
#define INCREASE_TIME 5 //time to increase when car collides with power-up
#define MAX_VELOCITY 0.20f
#define MOUSE_TIME 10       //tempo do efeito ao apanhar power-up modo rato



#define NOME_TEXTURA_CUBOS        "../data/marble.ppm"
#define NOME_TEXTURA_CHAO         "../data/chao.ppm"

#define NUM_TEXTURAS              2
#define ID_TEXTURA_CUBOS          0
#define ID_TEXTURA_CHAO           1

#define	CHAO_DIMENSAO		      10

#define NUM_JANELAS               2
#define JANELA_TOP                0
#define JANELA_NAVIGATE           1

#define STEP                      1

#define NOME_PERSONAGEM         "../data/porsche.mtl"

#define GAME_DURATION             120

/**************************************
*** VARIÁVEIS GLOBAIS E ESTRUTURAS *****
**************************************/

typedef struct {
    GLboolean   up,down,left,right;
} Teclas;

typedef struct {
    GLfloat    x,y,z;
} Posicao;

typedef struct {
    Posicao   pos;
    GLfloat   dir;
    GLfloat   vel;
} Objeto;


typedef struct camera{
    Posicao  eye;
    GLfloat  dir_long;  // longitude olhar (esq-dir)
    GLfloat  dir_lat;   // latitude olhar	(cima-baixo)
    GLfloat  fov;
} Camera;
typedef struct player{
    GLuint points;
    GLuint wins;
    GLboolean powerup;  //if set to 1 timer power-up activated, if set to 2 velocity power-up activated if set to 3 mouse mode activated
} PLAYER;
typedef struct {
    Camera        camera;
    GLboolean     difficulty;    //If set to 0 - normal , if set to 1 hardcore mode
    GLint         timer;
    GLint         mainWindow,topSubwindow,navigateSubwindow;
    Teclas        teclas;
    GLboolean     localViewer;
    GLuint        vista[NUM_JANELAS];
    GLuint        jogo;         // if set to 1 = on , if set to 0 = game over if set to 2 = win
    GLuint        start;        //if set to 1 start game (timer)
    GLuint       won;
} Estado;

typedef struct {
    GLuint        texID[NUM_JANELAS][NUM_TEXTURAS];
    GLuint        mapa[NUM_JANELAS];
    GLuint        chao[NUM_JANELAS];
    Objeto	      objeto;
    GLuint        xMouse;
    GLuint        yMouse;
    GLMmodel*     modelo;
    GLboolean     andar;        //1 = anda 0 = para
    GLuint        prev;
    GLuint        time_timer;
    GLfloat       power_up_time_size;
    GLfloat       power_up_vel_size;
    GLfloat       power_up_mouse_size;    //size of wall hack power-up
    GLfloat       velocity_mult;      //velocity multiplier
    GLfloat       teapot_size;
    GLfloat         car_size;
    GLfloat         exit_dir;
} Modelo;

Estado estado;
Modelo modelo;
Camera cam;     //camera sub-janela
PLAYER player;

GLfloat storedColor[3] = {1.0f, 0.0f, 0.0f}; // Initial color: Red


// '*' = wall from maze '-' = time power-up  '~' = velocity power-up '+' = teapot  '^' = mouse mode power-up
char mazedata [MAZE_HEIGHT][MAZE_WIDTH] = {
        "                      ",
        "                      ",
        "  ********    ********",
        "  *       *      *   *",
        "  * * **    *        *",
        "  *~**  * ** * *  * -*",
        "  *     *      *     *",
        "  *        ^ *** **  *",
        "  *           *  ** **",
        "  *     * *** ****   *",
        "  ***   *   *    **  *",
        "  *   ****  *       **",
        "  ********  **** *   *",
        "  *            * *****",
        "  *     *      * *   *",
        "  **-** *    *** **  *",
        "  *   *      *   **  *",
        "  *  * **  **** ** ~ *",
        "  ***  ***  **       *",
        "  * *   *   *    **  *",
        "  *       *      *   *",
        "  ********  + ********"
};


/**************************************
******* ILUMINAÇÃO E MATERIAIS ********
**************************************/

void setLight()
{
    GLfloat light_pos[4] = {-5.0f, 20.0f, -8.0f, 0.0f};
    GLfloat light_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat light_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, estado.localViewer);
}

void setMaterial()
{
    GLfloat mat_specular[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat mat_shininess = 104;

    // Criação automática das componentes Ambiente e Difusa do material a partir das cores
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Definir de outros parâmetros dos materiais estáticamente
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
}

/**************************************
*** INICIALIZAÇÃO DO AMBIENTE OPENGL **
**************************************/

void init(void)
{
    GLfloat amb[] = {0.3f, 0.3f, 0.3f, 1.0f};

    estado.timer = 100;

    estado.camera.eye.x = 0;
    estado.camera.eye.y = OBJETO_ALTURA * 2;
    estado.camera.eye.z = 0;
    estado.camera.dir_long = 0;
    estado.camera.dir_lat = 0;
    estado.camera.fov = 60;
    estado.start = 0;
    estado.won = 0;

    estado.localViewer = 1;
    estado.vista[JANELA_TOP] = 0;
    estado.vista[JANELA_NAVIGATE] = 0;
    estado.jogo = -1;           //Game started
    estado.difficulty = 0;

    modelo.objeto.pos.x = -8.7f;
    modelo.objeto.pos.y = OBJETO_ALTURA * .5;
    modelo.objeto.pos.z = 1.0f;
    modelo.objeto.dir = 0;
    modelo.objeto.vel = OBJETO_VELOCIDADE;
    modelo.time_timer = GAME_DURATION; //2 minutes
    modelo.power_up_time_size = POWERUP_SIZE;
    modelo.power_up_vel_size = POWERUP_SIZE;
    modelo.velocity_mult = VELOCITY_MULT;
    modelo.teapot_size = POWERUP_SIZE;
    modelo.power_up_mouse_size = POWERUP_SIZE;
    modelo.car_size = SCALE_PERSONAGEM;

    modelo.xMouse = modelo.yMouse = -1;
    modelo.andar = GL_FALSE;

    player.powerup = 0;

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);

    if (glutGetWindow() == estado.mainWindow)
        glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
    else
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
}

/**************************************
***** CALL BACKS DE JANELA/DESENHO ****
**************************************/

void redisplayTopSubwindow(int width, int height){

    // Define parte da janela a ser utilizada pelo OpenGL
    glViewport(0, 0, (GLint) width, (GLint) height);

    // Matriz Projeção
    // Matriz onde se define como o mundo e apresentado na janela
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60,(GLfloat)width/height,.5,100);

    // Matriz Modelview
    // Matriz onde são realizadas as transformações dos modelos desenhados
    glMatrixMode(GL_MODELVIEW);
}

//1st person
void updateCameraPositionFirstPerson() {
    // Set the camera position
    cam.eye.x = modelo.objeto.pos.x;
    cam.eye.y = modelo.objeto.pos.y + CAMERA_HEIGHT_OFFSET; // Adjust the height if needed
    cam.eye.z = modelo.objeto.pos.z;
    //set camera direction
    cam.dir_long = modelo.objeto.dir;
    cam.dir_lat = 0.0f;
    //Set camera field of view
    cam.fov = FOV_CONSTANT;
}

//3rd Person
void updateCameraPosition() {
    // Set the camera position behind and slightly above the object
    cam.eye.x = modelo.objeto.pos.x - CAMERA_DISTANCE * sin(modelo.objeto.dir);
    cam.eye.y = modelo.objeto.pos.y + CAMERA_HEIGHT_OFFSET;
    cam.eye.z = modelo.objeto.pos.z - CAMERA_DISTANCE * cos(modelo.objeto.dir);
    //set camera direction
    cam.dir_long = modelo.objeto.dir;
    cam.dir_lat = 0.0f;
    //Set camera field of view
    cam.fov = FOV_CONSTANT;
}



void reshapeNavigateSubwindow(int width, int height){
    // glViewport(botom, left, width, height)
    // Define parte da janela a ser utilizada pelo OpenGL
    glViewport(0, 0, (GLint) width, (GLint) height);

    // Matriz Projeção
    // Matriz onde se define como o mundo e apresentado na janela
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(estado.camera.fov,(GLfloat)width/height,0.1f,50.0f);

    // Matriz Modelview
    // Matriz onde são realizadas as transformações dos modelos desenhados
    glMatrixMode(GL_MODELVIEW);
}

void reshapeMainWindow(int width, int height)
{
    GLint w, h;
    w = (width - GAP * 3.0f) * .5;
    h = (height - GAP * 2);
    glutSetWindow(estado.topSubwindow);
    glutPositionWindow(GAP, GAP);
    glutReshapeWindow(w, h);
    glutSetWindow(estado.navigateSubwindow);
    glutPositionWindow(GAP + w + GAP, GAP);
    glutReshapeWindow(w, h);
}

/**************************************
** ESPAÇO PARA DEFINIÇÃO DAS ROTINAS **
****** AUXILIARES DE DESENHO ... ******
**************************************/

void strokeCenterString(char *str,double x, double y, double z, double s)
{
    int i,n;

    n = (int) strlen(str);
    glPushMatrix();
    glTranslated(x-glutStrokeLength(GLUT_STROKE_ROMAN,(const unsigned char*)str)*0.5*s,y-119.05*0.5*s,z);
    glScaled(s,s,s);
    for(i=0;i<n;i++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN,(int)str[i]);
    glPopMatrix();
}


void renderBitmapString(float x, float y, void *font, char *string) {
    char *c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

GLboolean isInsideMazeBorders(){
    int x = (int)(modelo.objeto.pos.x + MAZE_WIDTH / 2);
    int z = (int)(modelo.objeto.pos.z + MAZE_HEIGHT / 2);

    int x_left = (int)(modelo.objeto.pos.x - MAZE_WIDTH / 2);
    int z_down = (int)(modelo.objeto.pos.z - MAZE_HEIGHT / 2);

    if (x_left <= -MAZE_WIDTH + 2 || x >= MAZE_WIDTH || z_down <= -MAZE_HEIGHT + 2 || z >= MAZE_HEIGHT) {

        return GL_FALSE;
    }
    return GL_TRUE;
}


void toggleFog() {
    if (estado.difficulty == 1) {
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_EXP);        // Use exponential fog
        glFogf(GL_FOG_START, 10.0f);     // Fog start distance
        glFogf(GL_FOG_END, 20.0f);       // Fog end distance

        // Set fog color
        GLfloat fogGreyColor[] = {0.5f, 0.5f, 0.5f, 1.0f};  // RGB: 0.5, 0.5, 0.5 (mid-grey)
        glFogfv(GL_FOG_COLOR, fogGreyColor);
    } else {
        glDisable(GL_FOG);
    }
}


/******************************
**** Funções de desenho ******
******************************/

void desenhaPoligno (GLfloat a[], GLfloat b[], GLfloat c[], GLfloat d[], GLfloat normal[], GLfloat tx, GLfloat ty)
{
    glBegin(GL_POLYGON);
    glNormal3fv(normal);
    glTexCoord2f(tx+0,ty+0);
    glVertex3fv(a);
    glTexCoord2f(tx+0,ty+0.25f);
    glVertex3fv(b);
    glTexCoord2f(tx+0.25f,ty+0.25f);
    glVertex3fv(c);
    glTexCoord2f(tx+0.25f,ty+0);
    glVertex3fv(d);
    glEnd();
}

void desenhaCubo (int tipo, GLuint texID){
    GLfloat vertices[][3] = {{-0.5f, -0.5f, -0.5f},
                             {0.5f, -0.5f, -0.5f},
                             {0.5f, 0.5f, -0.5f},
                             {-0.5f, 0.5f, -0.5f},
                             {-0.5f, -0.5f, 0.5f},
                             {0.5f, -0.5f, 0.5f},
                             {0.5f, 0.5f, 0.5f},
                             {-0.5f, 0.5f, 0.5f}};

    GLfloat normais[][3] = {{0, 0, -1},
                            {0, 1, 0},
                            {-1, 0, 0},
                            {1, 0, 0},
                            {0, 0, 1},
                            {0, -1, 0}};
    GLfloat tx,ty;

    switch(tipo)
    {
        case 0:
            tx=0,ty=0;
            break;
        case 1:
            tx=0,ty=0.25f;
            break;
        case 2:
            tx=0,ty=0.5f;
            break;
        case 3:
            tx=0,ty=0.75f;
            break;
        case 4:
            tx=0.25f,ty=0;
            break;
        default:
            tx=0.75f,ty=0.75f;
    }

    glBindTexture(GL_TEXTURE_2D, texID);

    desenhaPoligno(vertices[1], vertices[0], vertices[3], vertices[2], normais[0], tx, ty);
    desenhaPoligno(vertices[2], vertices[3], vertices[7], vertices[6], normais[1], tx, ty);
    desenhaPoligno(vertices[3], vertices[0], vertices[4], vertices[7], normais[2], tx, ty);
    desenhaPoligno(vertices[6], vertices[5], vertices[1], vertices[2], normais[3], tx, ty);
    desenhaPoligno(vertices[4], vertices[5], vertices[6], vertices[7], normais[4], tx, ty);
    desenhaPoligno(vertices[5], vertices[4], vertices[0], vertices[1], normais[5], tx, ty);

    glBindTexture(GL_TEXTURE_2D, 0);

}

void desenhaPersonagem(void) {
    if (!modelo.modelo) {
        modelo.modelo = glmReadOBJ(NOME_PERSONAGEM);

        if (!modelo.modelo){
            exit(0);
        }

        glmUnitize(modelo.modelo);
        glmFacetNormals(modelo.modelo);
        glmVertexNormals(modelo.modelo, 90.0f);
    }

    glmDraw(modelo.modelo, GLM_SMOOTH | GLM_MATERIAL);
}

void desenhaBussola(int width, int height){
    glViewport(width - 60, 0, 60, 60);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(-30, 30, -30, 30);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);

    glPushMatrix();  // Save the current modelview matrix
    glRotatef(modelo.exit_dir, 0, 0, 1);  // Rotate based on modelo.exit_dir

    // Draw the compass as before
    glBegin(GL_TRIANGLES);
    glColor4f(1, 0, 0, 0.1f);
    glVertex2f(0, 15);
    glVertex2f(-6, 0);
    glVertex2f(6, 0);
    glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
    glVertex2f(6.0f, 0.0f);
    glVertex2f(-6.0f, 0.0f);
    glVertex2f(0.0f, -15.0f);
    glEnd();

    glLineWidth(2.0f);
    glColor3f(0.898f, 1.0f, 0.855f);
    glDisable(GL_BLEND);
    strokeCenterString("N", 0, 20, 0, 0.1);
    strokeCenterString("S", 0, -20, 0, 0.1);

    glPopMatrix();  // Restore the original modelview matrix

    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);

    reshapeNavigateSubwindow(width, height);
}


void draw_win_msg(GLint width, GLint height) {
    // Save the current viewport settings
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glViewport(width - 178, height - 30, 100, 30);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 100, 0, 30);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.0f, 0.0f, 1.0f);

    GLfloat difficultyPosX = 0.2f, difficultyPosY = 10.0f;

    char difficultyText[100];
    char winsText[100];

    sprintf(difficultyText, "POINTS: %d", player.points);

    renderBitmapString(difficultyPosX, difficultyPosY, GLUT_BITMAP_HELVETICA_12, difficultyText);

    // Calculate the width of the "POINTS" text
    int difficultyTextLength = strlen(difficultyText);
    GLfloat winsPosX = difficultyPosX + (difficultyTextLength * 7.0f) ;

    sprintf(winsText, "WINS: %d", player.wins);

    renderBitmapString(winsPosX, difficultyPosY, GLUT_BITMAP_HELVETICA_18, winsText);

    // Restore the original viewport settings
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}



void desenhaTimer(int width, int height) {
    glViewport(0, height - 30, 100, 30);
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    gluOrtho2D(0, 100, 0, 30);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.0f, 0.0f, 1.0f);

    // Posição do timer
    GLfloat timerPosX = 10;
    GLfloat timerPosY = 10;

    char timerText[200];

    if(estado.jogo == 0){
        sprintf(timerText, "GAME OVER! PRESS R TO RESTART");
    }else if(estado.difficulty == 1){
        sprintf(timerText, "Tempo restante: %d s HARDCORE MODE!!!",modelo.time_timer);
    }else if(player.powerup == 1  && modelo.power_up_time_size != 0){
        sprintf(timerText, "Tempo restante: %d s POWER-UP +5s",modelo.time_timer);
    }else if(player.powerup == 2 && modelo.power_up_vel_size != 0){
        sprintf(timerText, "Tempo restante: %d Da-lhe gas",modelo.time_timer);
    }else if(estado.start == 0){
        sprintf(timerText, "Press S to start! Or Z for a surprise :)", modelo.time_timer);
    }else if(estado.difficulty == 0 && estado.won == 1){
        sprintf(timerText, "You won! :)", modelo.time_timer);
    }
    else if(estado.difficulty == 1 && estado.won == 1){
        sprintf(timerText, "You are a super player! :)", modelo.time_timer);
    }
    else{
        sprintf(timerText, "Tempo restante: %d s",modelo.time_timer);
    }
    renderBitmapString(timerPosX, timerPosY, GLUT_BITMAP_HELVETICA_18, timerText);
}

void desenhaModeloDir(Objeto obj, int width, int height)
{
    redisplayTopSubwindow(width, height);
}

void desenhaAngVisao(Camera *cam){
    GLfloat ratio;
    ratio=(GLfloat)glutGet(GLUT_WINDOW_WIDTH)/glutGet(GLUT_WINDOW_HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPushMatrix();
    glTranslatef(cam->eye.x,OBJETO_ALTURA,cam->eye.z);
    glColor4f(0.0f,0.0f,1.0f,0.2f);
    glRotatef(GRAUS(cam->dir_long),0,1,0);
    float angulo = GRAUS(modelo.objeto.dir);
    glRotatef(angulo, 0, 1, 0);

    glBegin(GL_TRIANGLES);
    glVertex3f(0,0,0);
    glVertex3f(5.0f * cos(RAD(cam->fov*ratio*0.5f)),0,-5.0f * sin(RAD(cam->fov*ratio*0.5f)));
    glVertex3f(5.0f * cos(RAD(cam->fov*ratio*0.5f)),0,5.0f * sin(RAD(cam->fov*ratio*0.5f)));
    glEnd();
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void desenha_power_up(){
    GLint i,j;
    glColor3f(0.0f, 1.0f, 0.0f);

    glPushMatrix();
    glTranslatef(-MAZE_HEIGHT * 0.5f, 0.5f, -MAZE_WIDTH * 0.5f);
    for(i = 0; i < MAZE_HEIGHT; i++)
        for(j = 0; j < MAZE_WIDTH; j++)
            if(mazedata[i][j] == '-') {
                //if didn't catch any time power-up draw all
                if (modelo.power_up_time_size != 0) {
                    glPushMatrix();
                    glColor3f(0.0f, 1.0f, 0.0f); //green
                    glTranslated(i, 0, j);
                    glutSolidSphere(modelo.power_up_time_size, 20, 20);
                    glPopMatrix();
                }
                //teapot
            }else if(mazedata[i][j] == '+'){
                glPushMatrix();
                glTranslated(i, 0, j);
                glColor3f(1.0f, 0.0f, 0.0f);
                glutWireTeapot(modelo.teapot_size);
                glPopMatrix();
                //velocity power-up
            }else if(mazedata[i][j] == '~'){
                //if didn't catch any velocity power-up draw all
                if (modelo.power_up_vel_size != 0) {
                    glPushMatrix();
                    glColor3f(0.0f, 0.0f, 1.0f); //blue
                    glTranslated(i, 0, j);
                    glutSolidSphere(modelo.power_up_vel_size, 20, 20);
                    glPopMatrix();
                }
            }else if(mazedata[i][j] == '^'){
                //wall hack power-up
                if (modelo.power_up_mouse_size != 0) {
                    glPushMatrix();
                    glColor3f(0.58f, 0.341f, 0.941f); //purple
                    glTranslated(i, 0, j);
                    glutSolidSphere(modelo.power_up_mouse_size, 20, 20);
                    glPopMatrix();
                }
            }
    glPopMatrix();
}


void desenhaLabirinto(GLuint texID){
    GLint i,j;
    glColor3f(0.8f, 0.8f, 0.8f);

    glPushMatrix();
    glTranslatef(-MAZE_HEIGHT * 0.5f, 0.5f, -MAZE_WIDTH * 0.5f);
    for(i = 0; i < MAZE_HEIGHT; i++)
        for(j = 0; j < MAZE_WIDTH; j++)
            if(mazedata[i][j] == '*'){
                glPushMatrix();
                glTranslated(i, 0 ,j);
                desenhaCubo((i+j) % 6, texID);
                glPopMatrix();
            }
    glPopMatrix();

}


void desenhaChao(GLfloat dimensao, GLuint texID){
    GLfloat i,j;
    glBindTexture(GL_TEXTURE_2D, texID);

    glColor3f(0.5f,0.5f,0.5f);
    for(i=-dimensao;i<=dimensao;i+=STEP)
    {
        for(j=-dimensao;j<=dimensao;j+=STEP)
        {
            glBegin(GL_POLYGON);
            glNormal3f(0,1,0);
            glTexCoord2f(1,1);
            glVertex3f(i+STEP,0,j+STEP);
            glTexCoord2f(0,1);
            glVertex3f(i,0,j+STEP);
            glTexCoord2f(0,0);
            glVertex3f(i,0,j);
            glTexCoord2f(1,0);
            glVertex3f(i+STEP,0,j);
            glEnd();
        }
    }

}


void changeLightsColorToGreen() {
    storedColor[0] = 0.0f;
    storedColor[1] = 1.f;
    storedColor[2] = 0.0f;
}

void changeLightsColorToBlue() {

    storedColor[0] = 0.0f;
    storedColor[1] = 0.0f;
    storedColor[2] = 1.0f;
}

void changeLightsColorToRed() {
    storedColor[0] = 1.0f;
    storedColor[1] = 0.0f;
    storedColor[2] = 0.0f;
}

void changeLightsLabToGreen(){
    GLfloat green_light_pos[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // Set the position of the light source
    GLfloat green_light_diffuse[] = {0.0f, 1.0f, 0.0f, 1.0f}; // Set the light to green

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT1);

    glLightfv(GL_LIGHT1, GL_POSITION, green_light_pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, green_light_diffuse);
}
/**************************************
******** MENUS **********
**************************************/


void mainMenu(int value){
    switch (value) {
        case 0:
            changeLightsColorToRed();
            break;
        case 1:
            changeLightsColorToGreen();
            break;
        case 2:
            changeLightsColorToBlue();
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void createMenu() {
    glutCreateMenu(mainMenu);
    glutAddMenuEntry("Car color: red", 0);
    glutAddMenuEntry("Car color: green", 1);
    glutAddMenuEntry("Car color: blue", 2);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}



void createDisplayLists(int janelaID){
    modelo.mapa[janelaID]=glGenLists(2);

    glNewList(modelo.mapa[janelaID], GL_COMPILE);
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    desenhaLabirinto(modelo.texID[janelaID][ID_TEXTURA_CUBOS]);
    glPopAttrib();
    glEndList();

    modelo.chao[janelaID]=modelo.mapa[janelaID] + 1;
    glNewList(modelo.chao[janelaID], GL_COMPILE);
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT );
    desenhaChao(CHAO_DIMENSAO,modelo.texID[janelaID][ID_TEXTURA_CHAO]);
    glPopAttrib();
    glEndList();
}

/////////////////////////////////////
//navigateSubwindow
void motionNavigateSubwindow(int x, int y)
{
    int dif;
    dif = y - modelo.yMouse;
    //olhar para baixo
    if(dif > 0) {
        estado.camera.dir_lat -= dif * RAD(EYE_ROTACAO);
        if(estado.camera.dir_lat <- RAD(45))
            estado.camera.dir_lat =- RAD(45);
    }

    //olhar para cima
    if(dif < 0){
        estado.camera.dir_lat += abs(dif) * RAD(EYE_ROTACAO);
        if(estado.camera.dir_lat > RAD(45))
            estado.camera.dir_lat = RAD(45);
    }

    dif = x - modelo.xMouse;
    //olhar para a direita
    if(dif > 0){
        estado.camera.dir_long -= dif * RAD(EYE_ROTACAO);
    }

    //olhar para a esquerda
    if(dif < 0){
        estado.camera.dir_long += abs(dif) * RAD(EYE_ROTACAO);
    }

    modelo.xMouse = x;
    modelo.yMouse = y;

}

void mouseNavigateSubwindow(int button, int state, int x, int y)
{
    if(button==GLUT_RIGHT_BUTTON) {
        if(state==GLUT_DOWN) {
            modelo.xMouse=x;
            modelo.yMouse=y;
            glutMotionFunc(motionNavigateSubwindow);
        }
        else
            glutMotionFunc(NULL);
    }
}

void setNavigateSubwindowCamera(Camera *cam, Objeto obj) {
    Posicao center;

    if (estado.vista[JANELA_NAVIGATE]) {
        // If navigation view is active, set camera position to object's position
        cam->eye.x = obj.pos.x;
        cam->eye.y = obj.pos.y;
        cam->eye.z = obj.pos.z;
        // Calculate the center based on the object's orientation
        center.x = obj.pos.x + cos(obj.dir) * cos(cam->dir_lat);
        center.z = obj.pos.z + sin(-obj.dir) * cos(cam->dir_lat);
        center.y = cam->eye.y + sin(cam->dir_lat);
    } else {
        // If navigation view is not active
        center.x = obj.pos.x;
        center.y = obj.pos.y + 0.5;
        center.z = obj.pos.z;
        // Calculate the eye position based on the object's orientation
        cam->eye.x = center.x - cos(obj.dir);
        cam->eye.z = center.z - sin(-obj.dir);
        cam->eye.y = center.y + 0.5;
        // Set up the camera view using gluLookAt
    }
    gluLookAt(cam->eye.x, cam->eye.y, cam->eye.z, center.x, center.y, center.z, 0, 1, 0);
}



void displayNavigateSubwindow(){
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    setNavigateSubwindowCamera(&estado.camera, modelo.objeto);

    toggleFog();

    glCallList(modelo.mapa[JANELA_NAVIGATE]);
    glCallList(modelo.chao[JANELA_NAVIGATE]);
    desenha_power_up();

    // Draw the character, compass and power-ups if not in navigation view
    if (!estado.vista[JANELA_NAVIGATE]) {
        glPushMatrix();
        glTranslatef(modelo.objeto.pos.x, modelo.objeto.pos.y + 0.3f, modelo.objeto.pos.z);
        glRotatef(GRAUS(modelo.objeto.dir), 0, 1, 0);
        glRotatef(90, 0, 1, 0);
        glScalef(modelo.car_size, modelo.car_size, modelo.car_size);
        // Enable color material to track material properties with current color

        glDisable(GL_LIGHTING);

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        glColor3fv(storedColor);
             desenhaPersonagem();
        glDisable(GL_COLOR_MATERIAL);

        // Update camera position to follow car direction
        updateCameraPosition();

        // Enable lighting
        glEnable(GL_LIGHTING);
        glPushMatrix();
        GLfloat light_pos[] = { 0.0f, 2.0f, -1.0f, 0.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        glPopMatrix();

        // Disable lighting
        glDisable(GL_LIGHTING);

        glPopMatrix();
    }

    // Draw compass
    desenhaBussola(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    // Swap the front and back buffers
    glutSwapBuffers();
}



void setTopSubwindowCamera(Camera *cam, Objeto obj){
    cam->eye.x=obj.pos.x;
    cam->eye.z=obj.pos.z;
    if(estado.vista[JANELA_TOP])
        gluLookAt(obj.pos.x,CHAO_DIMENSAO*.2,obj.pos.z,obj.pos.x,obj.pos.y,obj.pos.z,0,0,-1);
    else
        gluLookAt(obj.pos.x,CHAO_DIMENSAO*2,obj.pos.z,obj.pos.x,obj.pos.y,obj.pos.z,0,0,-1);
}

void displayTopSubwindow(){
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    setTopSubwindowCamera(&estado.camera,modelo.objeto);
    setLight();

    toggleFog();

    glCallList(modelo.mapa[JANELA_TOP]);
    glCallList(modelo.chao[JANELA_TOP]);

    desenha_power_up();

    glPushMatrix();
    glTranslatef(modelo.objeto.pos.x,modelo.objeto.pos.y,modelo.objeto.pos.z);
        glRotatef(GRAUS(modelo.objeto.dir),0,1,0);
        glRotatef(90,0,1,0);
        glScalef(modelo.car_size,modelo.car_size,modelo.car_size);

        glDisable(GL_LIGHTING);

        // Enable color material to track material properties with current color
        glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
            glColor3fv(storedColor);
            desenhaPersonagem();
        glDisable(GL_COLOR_MATERIAL);
    glPopMatrix();

    desenhaAngVisao(&estado.camera);
    desenhaModeloDir(modelo.objeto, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    glutSwapBuffers();
}


void redisplayAll(void){
    glutSetWindow(estado.mainWindow);
    glutPostRedisplay();
    glutSetWindow(estado.topSubwindow);
    glutPostRedisplay();
    glutSetWindow(estado.navigateSubwindow);
    glutPostRedisplay();
}

void displayMainWindow(){
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    desenhaTimer(0, glutGet(GLUT_WINDOW_HEIGHT));
    draw_win_msg(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    glutSwapBuffers();
}

void check_win(){

     if (estado.difficulty == 1 && estado.jogo == 2 && modelo.teapot_size == 0) {
         player.points = 1337;
         player.wins++;
     } else if (estado.difficulty == 0 && estado.jogo == 2 && modelo.teapot_size == 0) {
         if(player.points < 1337){
             player.points++;
             player.wins++;
         }
         if(player.points >= 1337){
             player.wins = 0;
             player.points = 0;
         }
     }
    /*else{
     player.wins = 0;
     player.points = 0;
 }*/
}



/**************************************
******** CALLBACKS TIME/IDLE **********
**************************************/
void temporizador(int value) {
    static GLint64 mouse_mode_time = 0;  // Declare as static to retain its value between function calls
    //when player starts the game by pressing 's' and if velocity is not set
    if(estado.start == 1 && !modelo.velocity_mult){
        modelo.velocity_mult = VELOCITY_MULT;
    }

    if (modelo.time_timer > 0 && estado.start == 1) {
        modelo.time_timer--;

        // If time ended
        if (modelo.time_timer == 0) {
            // Change flag to 0 meaning game has ended
            estado.jogo = 0;
            // Change velocity of car to 0
            modelo.objeto.vel = 0;
            // Reset points to 0
            player.points = 0;
        } /*else if (modelo.time_timer == HARD_TIME) {
            estado.difficulty = 1;
        }*/
        // If player catches the time power-up, increase time
        if (player.powerup == 1 && modelo.time_timer <= GAME_DURATION - INCREASE_TIME && modelo.power_up_time_size != 0) {
            // Disappear power-up
            modelo.power_up_time_size = 0;
            modelo.time_timer += INCREASE_TIME;
            modelo.velocity_mult = VELOCITY_MULT;
            player.powerup = 0;
        }
        // If player catches the velocity power-up, increase velocity of car
        if (player.powerup == 2 && modelo.velocity_mult <= MAX_VELOCITY && modelo.power_up_vel_size != 0) {
            // Disappear power-up
            modelo.power_up_vel_size = 0;
            modelo.velocity_mult += 0.01f;
            player.powerup = 0;
        }
        // If player catches the mouse mode power-up, change car size and decrease velocity
        if (player.powerup == 3 && modelo.power_up_mouse_size != 0 && mouse_mode_time == 0) {
            // Disappear power-up
            modelo.power_up_mouse_size = 0;
            modelo.car_size = 0.1f;  // Change size of car
            modelo.velocity_mult -= 0.01f;
            mouse_mode_time = time(NULL);
            //printf("Mouse Mode Time Start: %d\n", mouse_mode_time);
        }
        // If mouse mode is active and time has passed, revert changes
        if (mouse_mode_time > 0 && difftime(time(NULL), mouse_mode_time) >= MOUSE_TIME) {
            modelo.car_size = SCALE_PERSONAGEM;
            modelo.velocity_mult = VELOCITY_MULT;
            player.powerup = 0;
            mouse_mode_time = 0;  // Reset the mouse_mode_time
        }
    }else{
        modelo.velocity_mult =0 ;
    }
    check_win();
    glutTimerFunc(1000, temporizador, 0);
    redisplayAll();
}



void change_direction(){
    if (estado.teclas.left){
        // rodar camara e objeto
        modelo.objeto.dir += CAMERA_ROTATION;
        if (GRAUS(modelo.objeto.dir) >= 360){
            modelo.objeto.dir = 0;
        }
    }
    if (estado.teclas.right){
        // rodar camara e objeto
        modelo.objeto.dir -= CAMERA_ROTATION;
        if (GRAUS(modelo.objeto.dir) <= -360){
            modelo.objeto.dir = 0;
        }
    }
}



int checkCollision_powerup(float carX, float carZ, float objectX, float objectZ, float radius) {
    // Calculate the squared distance between the car and the power-up object (usando teorema de pitágoras para calcular
    //o tamanho do vector entre 2 pontos)
    float distanceSquared = pow(carX - objectX, 2) + pow(carZ - objectZ, 2);
    //Calculate the maximum squared distance at which the two car and power-up can be considered colliding
    float combinedRadiusSquared = pow(radius, 2);

    return distanceSquared <= combinedRadiusSquared;
}

int checkInsideBorders(float carX, float carZ) {
    //check if car is entirely within a rectangular region defined by CHAO_DIMENSAO
    if (carX - modelo.car_size >= -CHAO_DIMENSAO -modelo.car_size &&
        carX + modelo.car_size <= CHAO_DIMENSAO + modelo.car_size &&
        carZ - modelo.car_size >= -CHAO_DIMENSAO -modelo.car_size &&
        carZ + modelo.car_size <= CHAO_DIMENSAO + modelo.car_size){
        // The car is inside the borders
        return 1;
    }
    // The car is outside the borders
    return 0;
}


///Return 1 if no collision, return 3 if collides with velocity power ups, return 2 if collides with timer power ups, return 0 if collides with walls
///Return 4 if collides with teapo , and return 5 if collides wit mouse power_up
int checkCollision(float carX, float carZ) {
    GLfloat cubeSize = CUBE_SIZE; // Cube size
    GLfloat halfCubeSize = cubeSize / 2.0f; // Half of the cube size

    // Loop through each cube in the maze
    for (int i = 0; i < MAZE_HEIGHT; ++i) {
        for (int j = 0; j < MAZE_WIDTH; ++j) {
            if (mazedata[i][j] == '*') {
                GLfloat cubeX = i - modelo.car_size - MAZE_HEIGHT * cubeSize; // Cube's face X
                GLfloat cubeZ = j - modelo.car_size - MAZE_WIDTH * cubeSize; // Cube's face Z

                // Check collision between car and cube
                if (carX + halfCubeSize >= cubeX - halfCubeSize &&
                    carX - halfCubeSize <= cubeX + halfCubeSize &&
                    carZ + halfCubeSize >= cubeZ - halfCubeSize &&
                    carZ - halfCubeSize <= cubeZ + halfCubeSize || checkInsideBorders(carX, carZ) ==0) {
                    return 1; // Collision detected
                }
            }else if(mazedata[i][j] == '-' || mazedata[i][j] == '~' || mazedata[i][j] == '+' || mazedata[i][j] == '^') {
                //Check if collided with power-up
                GLfloat pupX = i - MAZE_HEIGHT * CUBE_SIZE; // Power-up X
                GLfloat pupZ = j - MAZE_WIDTH * CUBE_SIZE; // Power-up Z
                if (checkCollision_powerup(carX, carZ, pupX, pupZ, POWERUP_SIZE)) {
                    if(mazedata[i][j] == '~'){
                        return 3; // Collision detected power-up velocity
                    }else if(mazedata[i][j] == '-'){
                        return 2; // Collision detected power-up timer
                    }else if(mazedata[i][j] == '+'){
                        //printf("exit coord: x = %f y = %f\n", pupX, pupZ);
                        return 4; //Collision detected teapot
                    }else{
                        return 5; //collision with mouse power-up
                    }
                }
            }
        }
    }
    return 0;
}

void someFunction(int nothing){
    check_win();
    init();
    glutPostRedisplay();
}



/* Callback de temporizador */
void timer(){
    GLfloat nx = 0, nz = 0;
    GLuint curr = glutGet(GLUT_ELAPSED_TIME);
    // Calcula velocidade baseado no tempo passado
    float velocidade = modelo.objeto.vel * (curr - modelo.prev) * modelo.velocity_mult;

    glutTimerFunc(estado.timer, timer, 0);
    modelo.prev = curr;

    if (estado.teclas.up || estado.teclas.down) {
        float forwardComponent = velocidade * cosf(RAD(GRAUS(modelo.objeto.dir)));
        float sidewaysComponent = velocidade * sinf(RAD(GRAUS(modelo.objeto.dir)));

        if (estado.teclas.down) {
            forwardComponent = -forwardComponent;
            sidewaysComponent = -sidewaysComponent;
        }
        nx = modelo.objeto.pos.x + forwardComponent;
        nz = modelo.objeto.pos.z - sidewaysComponent;

        //printf("nx = %f, nz = %f\n", nx, nz );

        int collisionResult = checkCollision(nx, nz);
        if (collisionResult == 0) {
            // No collision
            modelo.objeto.pos.x = nx;
            modelo.objeto.pos.z = nz;
            //collides with power-ups or teapot
        } else if (collisionResult == 2 || collisionResult == 3 || collisionResult == 4 || collisionResult == 5) {
            // Collision with power-up
            modelo.objeto.pos.x = nx;
            modelo.objeto.pos.z = nz;
            if(collisionResult == 3){
                player.powerup = 2;     //set flag to 2 when player hits velocity power-up
            }else if(collisionResult == 2){
                player.powerup = 1;     //set flag to 1 when player hits timer power-up
                //if player hits mouse mode power-up
            }else if(collisionResult == 5){
                player.powerup = 3;
            }else{
                //if player hits teapot
                estado.jogo = 2;
                estado.won = 1;
                modelo.teapot_size = 0;
                glutTimerFunc(3000, someFunction, 0);
            }
            //printf("power up !!!\n");
        } else {
            // Collision with walls
           // printf("Collision!\n");
        }
    }
    //change compass rotation angle
    float exitDirection = atan2(EXIT_Y- modelo.objeto.pos.z, EXIT_X - modelo.objeto.pos.x);
    modelo.exit_dir = -GRAUS(exitDirection);

    //Change object and camera direction
    change_direction();

    motionNavigateSubwindow(modelo.objeto.pos.x, modelo.objeto.pos.y);
    redisplayAll();
}


/**************************************
*********** FUNÇÃO AJUDA **************
**************************************/

void imprime_ajuda(void)
{
    printf("\n\nProjeto MUL1\n");
    printf("h,H   - Ajuda \n");
    printf("******* Diversos ******* \n");
    printf("l,L   - Alterna o calculo luz entre Z e eye (GL_LIGHT_MODEL_LOCAL_VIEWER)\n");
    printf("w,W   - Wireframe \n");
    printf("f,F   - Fill \n");
    printf("r,R   - Restart Game \n");
    printf("z,Z   - Hardcore Mode \n");
    printf("******* Movimento ******* \n");
    printf("UP    - Avança  \n");
    printf("DOWN  - Recua \n");
    printf("LEFT  - Vira para a esquerda\n");
    printf("RIGHT - Vira para a direita\n");
    printf("******* Camara ******* \n");
    printf("F1    - Alterna camara da janela da Esquerda \n");
    printf("F2    - Alterna camara da janela da Direita \n");
    printf("PAGE_UP, PAGE_DOWN - Altera abertura da camara \n");
    printf("ESC - Sair\n");
}

/**************************************
********* CALLBACKS TECLADO ***********
**************************************/

/* Callback para interação via teclado (carregar na tecla) */
void key(unsigned char key, int x, int y)
{
    switch (key) {
        case 27:
            exit(1);
            break;
        case 'h' :
        case 'H' :
            imprime_ajuda();
            break;
        case 'l':
        case 'L':
            estado.localViewer = !estado.localViewer;
            break;
        case 'w':
        case 'W':
            glutSetWindow(estado.navigateSubwindow);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_TEXTURE_2D);
            glutSetWindow(estado.topSubwindow);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_TEXTURE_2D);
            break;
        case 'f':
        case 'F':
            glutSetWindow(estado.navigateSubwindow);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_TEXTURE_2D);
            glutSetWindow(estado.topSubwindow);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_TEXTURE_2D);
            break;
            //Restart game if click on R key
        case 'r':
        case 'R':
            if(estado.difficulty != 1){
                init();
                player.points = 0;
            }
            break;
        case 'z':
        case 'Z':
            //activate fog if 1
            if (modelo.time_timer >= 100) {
            estado.difficulty = 1;
            estado.start = 1;
            }
            break;
        case'x':
        case'X':
            estado.difficulty = 0;
            break;
        case's':
        case'S':
            estado.start = 1;
            break;
    }

    if (DEBUG)
        printf("Carregou na tecla %c\n", key);
}

/* Callback para interação via teclado (largar a tecla) */
void keyUp(unsigned char key, int x, int y)
{
    if (DEBUG)
        printf("Largou a tecla %c\n", key);
}

/* Callback para interacção via teclas especiais (carregar na tecla) */
void specialKey(int key, int x, int y){
    switch (key) {
        case GLUT_KEY_UP:
            estado.teclas.up =GL_TRUE;
            break;
        case GLUT_KEY_DOWN:
            estado.teclas.down =GL_TRUE;
            break;
        case GLUT_KEY_LEFT:
            estado.teclas.left =GL_TRUE;
            break;
        case GLUT_KEY_RIGHT:
            estado.teclas.right =GL_TRUE;
            break;
        case GLUT_KEY_F1:
            estado.vista[JANELA_TOP]=!estado.vista[JANELA_TOP];
            break;
        case GLUT_KEY_F2:
            estado.vista[JANELA_NAVIGATE]=!estado.vista[JANELA_NAVIGATE];
            break;
        case GLUT_KEY_PAGE_UP:
            if(estado.camera.fov>20){
                estado.camera.fov--;
                glutSetWindow(estado.navigateSubwindow);
                reshapeNavigateSubwindow(glutGet(GLUT_WINDOW_WIDTH),glutGet(GLUT_WINDOW_HEIGHT));
                redisplayAll();
            }
            break;
        case GLUT_KEY_PAGE_DOWN:
            if(estado.camera.fov<130)
            {
                estado.camera.fov++;
                glutSetWindow(estado.navigateSubwindow);
                reshapeNavigateSubwindow(glutGet(GLUT_WINDOW_WIDTH),glutGet(GLUT_WINDOW_HEIGHT));
                redisplayAll();
            }
            break;
    }

    if (DEBUG)
        printf("Carregou na tecla especial %d\n", key);
}

/* Callback para interação via teclas especiais (largar a tecla) */
void specialKeyUp(int key, int x, int y)
{
    switch (key) {
        case GLUT_KEY_UP:
            estado.teclas.up =GL_FALSE;
            break;
        case GLUT_KEY_DOWN:
            estado.teclas.down =GL_FALSE;
            break;
        case GLUT_KEY_LEFT:
            estado.teclas.left =GL_FALSE;
            break;
        case GLUT_KEY_RIGHT:
            estado.teclas.right =GL_FALSE;
            break;
    }

    if (DEBUG)
        printf("Largou a tecla especial %d\n", key);
}


/**************************************
************** TEXTURAS ***************
**************************************/

void createTextures(GLuint texID[])
{
    unsigned char *image_chao = NULL, *image_cubos = NULL;
    int w, h;

    glGenTextures(NUM_TEXTURAS,texID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    image_chao = glmReadPPM(NOME_TEXTURA_CHAO, &w, &h);
    image_cubos = glmReadPPM(NOME_TEXTURA_CUBOS, &w, &h);

    if (image_chao && image_cubos) {
        glBindTexture(GL_TEXTURE_2D, texID[ID_TEXTURA_CHAO]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, image_chao);

        glBindTexture(GL_TEXTURE_2D, texID[ID_TEXTURA_CUBOS]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, image_cubos);
    } else {
        printf("Alguma textura não encontrada \n");
        exit(0);
    }
}

/**************************************
************ FUNÇÃO MAIN **************
**************************************/

int main_3Dgame(int argc, char **argv){
    glutInit(&argc, argv);
    glutInitWindowPosition(10, 10);
    glutInitWindowSize(800 + GAP * 3, 400 + GAP * 2);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    if ((estado.mainWindow = glutCreateWindow("Labirinto")) == GL_FALSE)
        exit(1);

    imprime_ajuda();

    // Registar callbacks do GLUT da janela principal
    glutReshapeFunc(reshapeMainWindow);
    glutDisplayFunc(displayMainWindow);

    glutTimerFunc(estado.timer, timer, 0);
    glutKeyboardFunc(key);
    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);

    // criar a sub window topSubwindow
    estado.topSubwindow = glutCreateSubWindow(estado.mainWindow, GAP, GAP, 400, 400);
    init();
    setLight();
    setMaterial();
    createTextures(modelo.texID[JANELA_TOP]);
    createDisplayLists(JANELA_TOP);

    createMenu();
    glutReshapeFunc(redisplayTopSubwindow);
    glutDisplayFunc(displayTopSubwindow);

    glutTimerFunc(estado.timer, timer, 0);
    glutKeyboardFunc(key);
    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);

    // criar a sub window navigateSubwindow
    estado.navigateSubwindow = glutCreateSubWindow(estado.mainWindow, 400 + GAP, GAP, 400, 800);
    init();
    setLight();
    setMaterial();

    createTextures(modelo.texID[JANELA_NAVIGATE]);
    createDisplayLists(JANELA_NAVIGATE);

    glutReshapeFunc(reshapeNavigateSubwindow);
    glutDisplayFunc(displayNavigateSubwindow);

    glutTimerFunc(1000, temporizador, 0);
    //glutMouseFunc(mouseNavigateSubwindow);

    glutTimerFunc(estado.timer, timer, 0);
    glutKeyboardFunc(key);
    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);

    glutMainLoop();
    return 0;
}