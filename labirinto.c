#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

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

#define	GAP					              25

#define MAZE_HEIGHT               22
#define MAZE_WIDTH                22

#define	OBJETO_ALTURA		        0.4
#define OBJETO_VELOCIDADE	      0.5
#define OBJETO_ROTACAO		        5
#define OBJETO_RAIO		          0.12
#define SCALE_PERSONAGEM        0.5
#define EYE_ROTACAO			          1

#define NOME_TEXTURA_CUBOS        "data/Chao.ppm"
#define NOME_TEXTURA_CHAO         "data/Chao.ppm"

#define NUM_TEXTURAS              2
#define ID_TEXTURA_CUBOS          0
#define ID_TEXTURA_CHAO           1

#define	CHAO_DIMENSAO		          10

#define NUM_JANELAS               2
#define JANELA_TOP                0
#define JANELA_NAVIGATE           1

#define STEP                      1

#define NOME_PERSONAGEM         "data/al.obj"

/**************************************
********** VARIÁVEIS GLOBAIS **********
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

typedef struct {
    Posicao  eye;  
    GLfloat  dir_long;  // longitude olhar (esq-dir)
    GLfloat  dir_lat;   // latitude olhar	(cima-baixo)
    GLfloat  fov;
} Camera;

typedef struct {
    Camera      camera;
    GLint         timer;
    GLint         mainWindow,topSubwindow,navigateSubwindow;
    Teclas        teclas;
    GLboolean     localViewer;
    GLuint        vista[NUM_JANELAS];
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
} Modelo;

Estado estado;
Modelo modelo;

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
  "  *   ****  *      ***",
  "  ********  **** *   *",
  "  *            * *****",
  "  *     *      * *   *",
  "  ** ** *    *** **  *",
  "  *   *      *   **  *",
  "  *  * **  **** **   *",
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
  GLfloat light_pos[4] = {-5.0, 20.0, -8.0, 0.0};
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

  modelo.objeto.pos.x = 0;
  modelo.objeto.pos.y = OBJETO_ALTURA * .5;
  modelo.objeto.pos.z = 0;
  modelo.objeto.dir = 0;
  modelo.objeto.vel = OBJETO_VELOCIDADE;

  modelo.xMouse = modelo.yMouse = -1;
  modelo.andar = GL_FALSE;

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
  int x = nx + MAZE_WIDTH/2.0 - 1 + 0.5;
  int z = nz + MAZE_HEIGHT/2.0 - 1 + 0.5;

  if(mazedata[z][x] == '*'){
    return GL_TRUE;
  }
  return GL_FALSE;
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

void desenhaCubo (int tipo, GLuint texID)
{
  GLfloat vertices[][3] = {{-0.5, -0.5, -0.5},
                          {0.5, -0.5, -0.5},
                          {0.5, 0.5, -0.5},
                          {-0.5, 0.5, -0.5},
                          {-0.5, -0.5, 0.5},
                          {0.5, -0.5, 0.5},
                          {0.5, 0.5, 0.5},
                          {-0.5, 0.5, 0.5}};

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
      tx=0,ty=0.25; 
      break; 
    case 2: 
      tx=0,ty=0.5; 
      break; 
    case 3: 
      tx=0,ty=0.75; 
      break; 
    case 4: 
      tx=0.25,ty=0; 
      break; 
    default: 
      tx=0.75,ty=0.75; 
    } 
    
  glBindTexture(GL_TEXTURE_2D, texID);

  desenhaPoligno(vertices[1], vertices[0], vertices[3], vertices[2], normais[0], tx, ty);
  desenhaPoligno(vertices[2], vertices[3], vertices[7], vertices[6], normais[1], tx, ty);
  desenhaPoligno(vertices[3], vertices[0], vertices[4], vertices[7], normais[2], tx, ty);
  desenhaPoligno(vertices[6], vertices[5], vertices[1], vertices[2], normais[3], tx, ty);
  desenhaPoligno(vertices[4], vertices[5], vertices[6], vertices[7], normais[4], tx, ty);
  desenhaPoligno(vertices[5], vertices[4], vertices[0], vertices[1], normais[5], tx, ty);

  glBindTexture(GL_TEXTURE_2D, NULL);

}

void desenhaPersonagem(void) { 
  
  if (!modelo.modelo) { 
    modelo.modelo = glmReadOBJ(NOME_PERSONAGEM); 
    
    if (!modelo.modelo){
      exit(0); 
    }
      
    glmUnitize(modelo.modelo); 
    glmFacetNormals(modelo.modelo); 
    glmVertexNormals(modelo.modelo, 90.0); 
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
  glRotatef(GRAUS(estado.camera.dir_long + estado.camera.dir_lat) - 90,0,0,1);

  //desenhar bussola 2D
  glBegin(GL_TRIANGLES);
    glColor4f(0,0,0,0.2);
    glVertex2f(0,15);
    glVertex2f(-6,0);
    glVertex2f(6,0);
    glColor4f(1,1,1,0.2);
    glVertex2f(6,0);
    glVertex2f(-6,0);
    glVertex2f(0,-15);
  glEnd();

  glLineWidth(2.0);
  glColor3f(1,1,1);
  glDisable(GL_BLEND);
  strokeCenterString("N", 0, 20, 0, 0.1);
  strokeCenterString("S", 0, -20, 0, 0.1);

  //repor estado
  glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);

  //repor projeºção chamando o redisplay
  reshapeNavigateSubwindow(width, height);

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
        glColor4f(0,0,1,0.2);
        glRotatef(GRAUS(cam->dir_long),0,1,0);

        glBegin(GL_TRIANGLES);
            glVertex3f(0,0,0);
            glVertex3f(5*cos(RAD(cam->fov*ratio*0.5)),0,-5*sin(RAD(cam->fov*ratio*0.5)));
            glVertex3f(5*cos(RAD(cam->fov*ratio*0.5)),0,5*sin(RAD(cam->fov*ratio*0.5)));
        glEnd();
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void desenhaModelo()
{
    glColor3f(0,1,0);
    glutSolidCube(OBJETO_ALTURA);
    glPushMatrix();
        glColor3f(1,0,0);
        glTranslatef(0,OBJETO_ALTURA*0.75,0);
        glRotatef(GRAUS(estado.camera.dir_long-modelo.objeto.dir),0,1,0);
        glutSolidCube(OBJETO_ALTURA*0.5);
    glPopMatrix();
}

void desenhaLabirinto(GLuint texID)
{
  int i,j;
  glColor3f(0.8f, 0.8f, 0.8f);

  glPushMatrix();
    glTranslatef(-MAZE_HEIGHT * 0.5, 0.5, -MAZE_WIDTH * 0.5);
    for(i = 0; i < MAZE_HEIGHT; i++)
      for(j = 0; j < MAZE_WIDTH; j++)
        if(mazedata[i][j] == '*')
        {
          glPushMatrix();
            glTranslatef(i, 0 ,j);
            desenhaCubo((i+j)%6, texID);
          glPopMatrix();  
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

void setNavigateSubwindowCamera(Camera *cam, Objeto obj)
{
  Posicao center;
  
  if(estado.vista[JANELA_NAVIGATE])
  {
    cam->eye.x = obj.pos.x;
    cam->eye.y = obj.pos.y;
    cam->eye.z = obj.pos.z;
    center.x = obj.pos.x + cos(cam->dir_long) * cos(cam->dir_lat);
    center.z = obj.pos.z + sin(-cam->dir_long) * cos(cam->dir_lat);
    center.y = cam->eye.y + sin(cam->dir_lat);
  }
  else
  {
    center.x = obj.pos.x;
    center.y = obj.pos.y + 0.5;
    center.z = obj.pos.z;
    cam->eye.x = center.x - cos(cam->dir_long);
    cam->eye.z = center.z - sin(-cam->dir_long);
    cam->eye.y = center.y + 0.5;
  }

  gluLookAt(cam->eye.x,cam->eye.y,cam->eye.z,center.x,center.y,center.z,0,1,0);
}


void displayNavigateSubwindow()
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();

	setNavigateSubwindowCamera(&estado.camera, modelo.objeto);
  //setLight();

	glCallList(modelo.mapa[JANELA_NAVIGATE]);
	glCallList(modelo.chao[JANELA_NAVIGATE]);

	if(!estado.vista[JANELA_NAVIGATE])
  {
    glPushMatrix();
        glTranslatef(modelo.objeto.pos.x,modelo.objeto.pos.y+0.3,modelo.objeto.pos.z);
        glRotatef(GRAUS(modelo.objeto.dir),0,1,0);
        glRotatef(90,0,1,0);
        glScalef(SCALE_PERSONAGEM,SCALE_PERSONAGEM,SCALE_PERSONAGEM);

        glEnable(GL_LIGHTING);
        glPushMatrix();
          GLfloat light_pos[] = { 0.0, 2.0, -1.0, 0.0 };
          glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        glPopMatrix();

        //desenhaPersonagem();      

        glDisable(GL_LIGHTING);
        
    glPopMatrix();
  }

  desenhaBussola(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
   
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

	glCallList(modelo.mapa[JANELA_TOP]);
	glCallList(modelo.chao[JANELA_TOP]);
	
    glPushMatrix();		
        glTranslatef(modelo.objeto.pos.x,modelo.objeto.pos.y,modelo.objeto.pos.z);
        glRotatef(GRAUS(modelo.objeto.dir),0,1,0);
        glRotatef(90,0,1,0);
        glScalef(SCALE_PERSONAGEM,SCALE_PERSONAGEM,SCALE_PERSONAGEM); 
        //desenhaPersonagem();
        
    glPopMatrix();

	desenhaAngVisao(&estado.camera);
	desenhaModeloDir(modelo.objeto, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	glutSwapBuffers();
}

/////////////////////////////////////
//mainWindow

void redisplayAll(void)
{
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
	glutSwapBuffers();
}


/**************************************
******** CALLBACKS TIME/IDLE **********
**************************************/

/* Callback de temporizador */
void timer(int value)
{
  GLfloat nx=0,nz=0;
  GLboolean andar=GL_FALSE;

  GLuint curr = glutGet(GLUT_ELAPSED_TIME);
  // Calcula velocidade baseado no tempo passado
	float velocidade= modelo.objeto.vel*(curr - modelo.prev )*0.002;

  glutTimerFunc(estado.timer, timer, 0);
  /* Acções do temporizador ...
     Não colocar aqui primitivas OpenGL de desenho glBegin, glEnd, etc.
   */

  modelo.prev = curr;

  if(estado.teclas.up)
  {
    nx = modelo.objeto.pos.x + (velocidade * cosf(RAD(GRAUS(modelo.objeto.dir))));
    nz = modelo.objeto.pos.z + (velocidade * sinf(RAD(GRAUS(modelo.objeto.dir))));

    if(!detectaColisao(nx,nz)) { 
      modelo.objeto.pos.x = nx; 
      modelo.objeto.pos.z = nz; 
    }
    andar=GL_TRUE;
	}
	
  if(estado.teclas.down){
    nx = modelo.objeto.pos.x - (velocidade * cosf(RAD(GRAUS(modelo.objeto.dir)))); 
    nz = modelo.objeto.pos.z - (velocidade * sinf(RAD(GRAUS(modelo.objeto.dir))));
    
    // calcula nova posição nx,nz 
    if(!detectaColisao(nx,nz)) { 
      modelo.objeto.pos.x = nx; 
      modelo.objeto.pos.z = nz; 
      } 
    andar=GL_TRUE;
	}
	
  if(estado.teclas.left){
    // rodar camara e objeto 
    modelo.objeto.dir += 0.3; 
    if (GRAUS(modelo.objeto.dir) >= 360) { 
      modelo.objeto.dir = 0; 
    }
  }
	if(estado.teclas.right){
    // rodar camara e objeto 
    modelo.objeto.dir += -0.3; 
    if (GRAUS(modelo.objeto.dir) <= -360) { 
      modelo.objeto.dir = 0; 
    }
	}

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
void specialKey(int key, int x, int y)
{
  /* Ações sobre outras teclas especiais
      GLUT_KEY_F1 ... GLUT_KEY_F12
      GLUT_KEY_UP
      GLUT_KEY_DOWN
      GLUT_KEY_LEFT
      GLUT_KEY_RIGHT
      GLUT_KEY_PAGE_UP
      GLUT_KEY_PAGE_DOWN
      GLUT_KEY_HOME
      GLUT_KEY_END
      GLUT_KEY_INSERT */

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

int main(int argc, char **argv)
{
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
  //glutMouseFunc(mouseNavigateSubwindow);

  glutTimerFunc(estado.timer, timer, 0);
  glutKeyboardFunc(key);
  glutSpecialFunc(specialKey);
  glutSpecialUpFunc(specialKeyUp);

  glutMainLoop();
  return 0;
}
