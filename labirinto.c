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

#define	OBJETO_ALTURA		        0.4
#define OBJETO_VELOCIDADE	      0.1
#define OBJETO_ROTACAO		        5
#define OBJETO_RAIO		          0.12
#define SCALE_PERSONAGEM            0.5
#define EYE_ROTACAO			          1
#define CAMERA_HEIGHT_OFFSET    1.5f
#define CAMERA_DISTANCE         5.0f
#define FOV_CONSTANT 60.0f  // Human eye natural



#define NOME_TEXTURA_CUBOS        "../data/chao.ppm"
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
********** VARIÁVEIS GLOBAIS **********
**************************************/
char* gameOverMessage = "game over";

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

typedef struct maze{
    GLuint h;

} MAZE;

typedef struct camera{
    Posicao  eye;
    GLfloat  dir_long;  // longitude olhar (esq-dir)
    GLfloat  dir_lat;   // latitude olhar	(cima-baixo)
    GLfloat  fov;   /// power up
} Camera;
typedef struct player{
    GLuint points;
} PLAYER;
typedef struct {
    Camera        camera;
    GLboolean     difficulty;    //If set to 0 - normal , if set to 1 hardcore mode
    GLint         timer;
    GLint         mainWindow,topSubwindow,navigateSubwindow;
    Teclas        teclas;
    GLboolean     localViewer;
    GLuint        vista[NUM_JANELAS];
    GLuint       jogo;         // if set to 1 = on , if set to 0 = game over if set to 2 = win
} Estado;

typedef struct {
    GLuint        texID[NUM_JANELAS][NUM_TEXTURAS];
    GLuint        mapa[NUM_JANELAS];
    GLuint        chao[NUM_JANELAS];
    Objeto	      objeto;
    GLuint        xMouse;
    GLuint        yMouse;
    GLMmodel*     modelo;
    GLboolean     andar;
    GLuint        prev;
    GLuint        time_timer;
} Modelo;

Estado estado;
Modelo modelo;
Camera cam;     //camera sub-janela
PLAYER player;

GLfloat storedColor[3] = {1.0f, 0.0f, 0.0f}; // Initial color: Red

char mazedata [MAZE_HEIGHT][MAZE_WIDTH] = {
        "                      ",
        "                      ",
        "  ********    ********",
        "  *       *      *   *",
        "  * * *** * *        *",
        "  * **  * ** * *  *  *",
        "  *     *      *     *",
        "  *          *** **  *",
        "  *           *  ** **",
        "  *     * *** ****   *",
        "  ***   *   *    **  *",
        "  *   ****  *       **",
        "  ********  **** *   *",
        "  *            * *****",
        "  *     *      * *   *",
        "  ** ** *    *** **  *",
        "  *   *      *   ** -*",
        "  *  * **  **** ** - *",
        "  ***  ***  **       *",
        "  * *   *   *    **  *",
        "  *       *      *   *",
        "  ********    ********"
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

    modelo.xMouse = modelo.yMouse = -1;
    modelo.andar = GL_FALSE;

    player.points = 0;

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

void redisplayTopSubwindow(int width, int height)
{
    // glViewport(botom, left, width, height)
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
    cam.eye.x = modelo.objeto.pos.x;
    cam.eye.y = modelo.objeto.pos.y + CAMERA_HEIGHT_OFFSET; // Adjust the height if needed
    cam.eye.z = modelo.objeto.pos.z;

   /* cam->center.x = obj->pos.x;
    cam->center.y = obj->pos.y;
    cam->center.z = obj->pos.z;*/

  /*  cam->up.x = 0.0f;
    cam->up.y = 1.0f;
    cam->up.z = 0.0f;*/

    cam.dir_long = modelo.objeto.dir;
    cam.dir_lat = 0.0f;

    cam.fov = FOV_CONSTANT;
}

//3rd Person
void updateCameraPosition() {
    // Set the camera position behind and slightly above the object
    cam.eye.x = modelo.objeto.pos.x - CAMERA_DISTANCE * sin(modelo.objeto.dir);
    cam.eye.y = modelo.objeto.pos.y + CAMERA_HEIGHT_OFFSET;
    cam.eye.z = modelo.objeto.pos.z - CAMERA_DISTANCE * cos(modelo.objeto.dir);

    cam.dir_long = modelo.objeto.dir;
    cam.dir_lat = 0.0f;

    cam.fov = FOV_CONSTANT;
}



void reshapeNavigateSubwindow(int width, int height)
{
    // glViewport(botom, left, width, height)
    // Define parte da janela a ser utilizada pelo OpenGL
    glViewport(0, 0, (GLint) width, (GLint) height);

    // Matriz Projeção
    // Matriz onde se define como o mundo e apresentado na janela
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(estado.camera.fov,(GLfloat)width/height,0.1,50);

    // Matriz Modelview
    // Matriz onde são realizadas as transformações dos modelos desenhados
    glMatrixMode(GL_MODELVIEW);
}

void reshapeMainWindow(int width, int height)
{
    GLint w, h;
    w = (width - GAP * 3) * .5;
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

    n = strlen(str);
    glPushMatrix();
    glTranslated(x-glutStrokeLength(GLUT_STROKE_ROMAN,(const unsigned char*)str)*0.5*s,y-119.05*0.5*s,z);
    glScaled(s,s,s);
    for(i=0;i<n;i++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN,(int)str[i]);
    glPopMatrix();

}

GLboolean detectaColisao (GLfloat nx, GLfloat nz)
{
    int x = (int)(nx + 0.5);  // Round to the nearest integer
    int z = (int)(nz + 0.5);

    if (mazedata[z][x] == '*') {
        return GL_TRUE; // Collision detected
    }

    return GL_FALSE; // No collision
}

void desenhaPoligno (GLfloat a[], GLfloat b[], GLfloat c[], GLfloat d[], GLfloat normal[], GLfloat tx, GLfloat ty)
{
    glBegin(GL_POLYGON);
    glNormal3fv(normal);
    glTexCoord2f(tx+0,ty+0);
    glVertex3fv(a);
    glTexCoord2f(tx+0,ty+0.25);
    glVertex3fv(b);
    glTexCoord2f(tx+0.25,ty+0.25);
    glVertex3fv(c);
    glTexCoord2f(tx+0.25,ty+0);
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

    glBindTexture(GL_TEXTURE_2D, (GLuint) NULL);

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

void desenhaBussola(int width, int height)
{
    glViewport(width-60, 0, 60, 60);
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

    //desenhar bussola 2D
    glBegin(GL_TRIANGLES);
    glColor4f(0,0,0,0.2f);

        glVertex2f(0,15);
        glVertex2f(-6,0);
        glVertex2f(6,0);
        glColor4f(1.0f,1.0f,1.0f,0.2f);
        glVertex2f(6.0f,0.0f);
        glVertex2f(-6.0f,0.0f);
        glVertex2f(0.0f,-15.0f);
        glEnd();

        glLineWidth(2.0f);
        glColor3f(1,1,1);
        glDisable(GL_BLEND);
        strokeCenterString("N", 0, 20, 0, 0.1);
        strokeCenterString("S", 0, -20, 0, 0.1);

    //repor estado
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);

    //repor projeção chamando o redisplay
    reshapeNavigateSubwindow(width, height);
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

GLint isInsideMazeXBorders(){
    int x = (int)(modelo.objeto.pos.x + MAZE_WIDTH / 2);
    int x_left = (int)(modelo.objeto.pos.x - MAZE_WIDTH / 2);

    if (x_left <= -MAZE_WIDTH + 2){
        return 0;
    }

    if(x >= MAZE_WIDTH - 2){
        return -1;
    }
    return 1;
}

void toggleFog() {
    if (estado.difficulty == 1) {
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_EXP);  // Use linear fog
        glFogf(GL_FOG_START, 10.0f);     // Fog start distance
        glFogf(GL_FOG_END, 20.0f);       // Fog end distance

        // Set fog color
        GLfloat fogGreyColor[] = {0.5f, 0.5f, 0.5f, 1.0f};  // RGB: 0.5, 0.5, 0.5 (mid-grey)
        glFogfv(GL_FOG_COLOR, fogGreyColor);
    } else {
        glDisable(GL_FOG);
    }
}

GLint isInsideMazeZBorders(){
    int z = (int)(modelo.objeto.pos.z + MAZE_HEIGHT / 2);
    int z_down = (int)(modelo.objeto.pos.z - MAZE_HEIGHT / 2);

    if (z >= MAZE_HEIGHT - 2){
        return 0;
    }

    if(z_down <= -MAZE_HEIGHT + 2){
        return -1;
    }

    return 1;
}

void drawdifficulty(GLint width, GLint height) {
    // Save the current viewport settings
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Set viewport and orthographic projection for drawing difficulty
    glViewport(width - 100, height - 30, 100, 30);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 100, 0, 30);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.0f, 0.0f, 1.0f);

    // Position of the difficulty text in the top right corner
    GLfloat difficultyPosX = 5.0f, difficultyPosY = 10.0f;

    char difficultyText[100];

    sprintf(difficultyText, "WINS: %d", player.points);

    renderBitmapString(difficultyPosX, difficultyPosY, GLUT_BITMAP_HELVETICA_18, difficultyText);

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
    char difficultyText[200];

    if(estado.jogo == 0){
        sprintf(timerText, "GAME OVER!");
    }else{
        sprintf(timerText, "Tempo restante: %d s",modelo.time_timer);
    }
    renderBitmapString(timerPosX, timerPosY, GLUT_BITMAP_HELVETICA_18, timerText);
}

void changeLightsColorToGreen() {
    storedColor[0] = 0.0f;
    storedColor[1] = 1.0f;
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


void carColorMenu(int value){
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

void mainMenu(int value){
    switch (value) {
        case 0:
            changeLightsColorToRed();
            break;
        case 1:
            changeLightsColorToGreen();
            break;
        case 2:
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void createMenu() {
    int mainmenu = glutCreateMenu(mainMenu);
    glutAddMenuEntry("Car color: red", 0);
    glutAddMenuEntry("Car color: green", 1);
    glutAddMenuEntry("Car color: blue", 2);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void desenhaModeloDir(Objeto obj, int width, int height)
{
    redisplayTopSubwindow(width, height);
}

void desenhaAngVisao(Camera *cam)
{
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
        glVertex3f(5.0f * cos(RAD(cam->fov*ratio*0.5f)),0,-5.0f * sin(RAD(cam->fov*ratio*0.5)));
        glVertex3f(5.0f * cos(RAD(cam->fov*ratio*0.5f)),0,5.0f * sin(RAD(cam->fov*ratio*0.5)));
    glEnd();
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}


void desenhaLabirinto(GLuint texID){
    GLint i,j;
    glColor3f(0.8f, 0.8f, 0.8f);

    glPushMatrix();
    glTranslatef(-MAZE_HEIGHT * 0.5f, 0.5f, -MAZE_WIDTH * 0.5f);
    for(i = 0; i < MAZE_HEIGHT; i++)
        for(j = 0; j < MAZE_WIDTH; j++)
            if(mazedata[i][j] == '*')
            {
                glPushMatrix();
                glTranslated(i, 0 ,j);
                desenhaCubo((i+j) % 6, texID);
                glPopMatrix();
            }else if(mazedata[i][j] == '-'){//power up speed
                //glutSolidSphere(2.0f, 20, 20);
                desenhaCubo(10.0, texID);
            }
    glPopMatrix();
}

void desenhaChao(GLfloat dimensao, GLuint texID)
{
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

void createDisplayLists(int janelaID)
{
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
    //setLight();

    toggleFog();

    glCallList(modelo.mapa[JANELA_NAVIGATE]);
    glCallList(modelo.chao[JANELA_NAVIGATE]);


    // Draw the character and compass if not in navigation view
    if (!estado.vista[JANELA_NAVIGATE]) {
        glPushMatrix();
        glTranslatef(modelo.objeto.pos.x, modelo.objeto.pos.y + 0.3f, modelo.objeto.pos.z);
        glRotatef(GRAUS(modelo.objeto.dir), 0, 1, 0);
        glRotatef(90, 0, 1, 0);
        glScalef(SCALE_PERSONAGEM, SCALE_PERSONAGEM, SCALE_PERSONAGEM);
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

        // desenhaAngVisao(&estado.camera);

        // Disable lighting
        glDisable(GL_LIGHTING);

        glPopMatrix();
    }

    // Draw compass
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glRotatef(GRAUS(modelo.objeto.dir), 0, 1, 0);
    desenhaBussola(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    glPopMatrix();

    // Swap the front and back buffers
    glutSwapBuffers();
}


/////////////////////////////////////
//topSubwindow
void setTopSubwindowCamera(Camera *cam, Objeto obj)
{
    cam->eye.x=obj.pos.x;
    cam->eye.z=obj.pos.z;
    if(estado.vista[JANELA_TOP])
        gluLookAt(obj.pos.x,CHAO_DIMENSAO*.2,obj.pos.z,obj.pos.x,obj.pos.y,obj.pos.z,0,0,-1);
    else
        gluLookAt(obj.pos.x,CHAO_DIMENSAO*2,obj.pos.z,obj.pos.x,obj.pos.y,obj.pos.z,0,0,-1);
}

void displayTopSubwindow()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    setTopSubwindowCamera(&estado.camera,modelo.objeto);
    setLight();

    toggleFog();

    glCallList(modelo.mapa[JANELA_TOP]);
    glCallList(modelo.chao[JANELA_TOP]);


    glPushMatrix();
    glTranslatef(modelo.objeto.pos.x,modelo.objeto.pos.y,modelo.objeto.pos.z);
        glRotatef(GRAUS(modelo.objeto.dir),0,1,0);
        glRotatef(90,0,1,0);
        glScalef(SCALE_PERSONAGEM,SCALE_PERSONAGEM,SCALE_PERSONAGEM);

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

/////////////////////////////////////
//mainWindow

void redisplayAll(void){
    glutSetWindow(estado.mainWindow);
    glutPostRedisplay();
    glutSetWindow(estado.topSubwindow);
    glutPostRedisplay();
    glutSetWindow(estado.navigateSubwindow);
    glutPostRedisplay();
}

void displayMainWindow()
{
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    desenhaTimer(0, glutGet(GLUT_WINDOW_HEIGHT));
    drawdifficulty(glutGet(GLUT_WINDOW_WIDTH),glutGet(GLUT_WINDOW_HEIGHT));
    glutSwapBuffers();
}

/**************************************
******** CALLBACKS TIME/IDLE **********
**************************************/
void temporizador(int value) {

    if (modelo.time_timer > 0) {
        modelo.time_timer--;

        if (modelo.time_timer == 0) {
            estado.jogo = 0;
            modelo.objeto.vel = 0;
        }else if(modelo.time_timer == 60) {
            estado.difficulty = 1;
        }
    }

    glutTimerFunc(1000, temporizador, 0);
}

void check_level_win(){
    if(estado.difficulty == 1 && estado.jogo == 2){
    player.points +=5;
    }else if(estado.difficulty == 0 && estado.jogo == 2){
        player.points +=1;
    }
}

void change_direction(){
    if (estado.teclas.left)
    {
        // rodar camara e objeto
        modelo.objeto.dir += CAMERA_ROTATION;
        if (GRAUS(modelo.objeto.dir) >= 360)
        {
            modelo.objeto.dir = 0;
        }
    }
    if (estado.teclas.right)
    {
        // rodar camara e objeto
        modelo.objeto.dir -= CAMERA_ROTATION;
        if (GRAUS(modelo.objeto.dir) <= -360)
        {
            modelo.objeto.dir = 0;
        }
    }
}

void check_win(){
    if(estado.jogo == 0){
        player.points = 0;
    }else if(estado.jogo == 1 && estado.difficulty == 1){
        player.points +=5;
    }else if(estado.jogo == 1 && estado.difficulty == 0){
        player.points += 1;
    }
}


/* Callback de temporizador */
void timer(int value){
    GLfloat nx = 0, nz = 0;
    GLboolean andar = GL_FALSE;

    GLuint curr = glutGet(GLUT_ELAPSED_TIME);
    // Calcula velocidade baseado no tempo passado
    float velocidade = modelo.objeto.vel * (curr - modelo.prev) * 0.006;

    glutTimerFunc(estado.timer, timer, 0);

    modelo.prev = curr;

    if (estado.teclas.up || estado.teclas.down)
    {
        float forwardComponent = velocidade * cosf(RAD(GRAUS(modelo.objeto.dir)));
        float sidewaysComponent = velocidade * sinf(RAD(GRAUS(modelo.objeto.dir)));

        if (estado.teclas.down)
        {
            forwardComponent = -forwardComponent;
            sidewaysComponent = -sidewaysComponent;
        }

        nx = modelo.objeto.pos.x + forwardComponent;
        nz = modelo.objeto.pos.z - sidewaysComponent; // Ajuste aqui para subtrair a componente lateral

        if(isInsideMazeXBorders()){
            modelo.objeto.pos.x = nx;
        }
        if(isInsideMazeXBorders() == -1){
            modelo.objeto.pos.x = nx - 1; //Decrease a bit to return the car to the limits
        }
        if(isInsideMazeXBorders() == 0){
            modelo.objeto.pos.x = nx + 1; //Decrease a bit to return the car to the limits
        }
        if(isInsideMazeZBorders()){
            modelo.objeto.pos.z = nz;
        }
        if(isInsideMazeZBorders() == 0){
            modelo.objeto.pos.z = nz - 1; //Decrease a bit to return the car to the limits
        }
        if(isInsideMazeZBorders() == -1){
            modelo.objeto.pos.z = nz + 1; //Decrease a bit to return the car to the limits
        }
        andar = GL_TRUE;
    }

        //Change object and camera direction
        change_direction();
        check_win();
        const int exitX = MAZE_HEIGHT - 1;  // Assuming exit is at the last row
        const int exitZ = MAZE_WIDTH - 1;   // Assuming exit is at the last column
        if (modelo.objeto.pos.x == exitX && modelo.objeto.pos.z == exitZ) {
        // Player reached the exit, set win condition
        printf("Congratulations! You won!\n");
        estado.jogo = 2;
    }

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
    printf("******* Movimento ******* \n");
    printf("UP    - Avança (PARA IMPLEMENTAR) \n");
    printf("DOWN  - Recua (PARA IMPLEMENTAR)\n");
    printf("LEFT  - Vira para a direita (PARA IMPLEMENTAR)\n");
    printf("RIGHT - Vira para a esquerda (PARA IMPLEMENTAR)\n");
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
            estado.localViewer=!estado.localViewer;
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
            init();
            glutPostRedisplay();
            break;
        case'Z':
        case 'z':
            //activate fog if 1
            estado.difficulty = 1;
            glutPostRedisplay();
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
            if(estado.camera.fov>20)
            {
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
    unsigned char *image = NULL;
    int w, h, bpp;

    glGenTextures(NUM_TEXTURAS,texID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    image = glmReadPPM(NOME_TEXTURA_CHAO, &w, &h);
    if(image)
    {
        glBindTexture(GL_TEXTURE_2D, texID[ID_TEXTURA_CHAO]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, image);
    }else{
        printf("Textura %s não encontrada \n",NOME_TEXTURA_CHAO);
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
    init();
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