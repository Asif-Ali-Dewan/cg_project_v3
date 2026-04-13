// Arena Dungeon – 3D OpenGL  (Code::Blocks / MinGW)
// Link: opengl32  glu32  freeglut
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include "System.h"

static const int WIN_W=800,WIN_H=600,FPS=60;
static int g_lastTime=0;

void display(){if(g_game)g_game->draw();glutSwapBuffers();}
void timerCB(int){
    int now=glutGet(GLUT_ELAPSED_TIME);
    float dt=(now-g_lastTime)/1000.f;
    g_lastTime=now;if(dt>.05f)dt=.05f;
    if(g_game)g_game->update(dt);
    glutPostRedisplay();glutTimerFunc(1000/FPS,timerCB,0);
}
void keyboard(unsigned char k,int,int){if(g_game)g_game->onKeyDown(k);}
void keyUp(unsigned char k,int,int){if(g_game)g_game->onKeyUp(k);}
void mouseMove(int x,int y){if(g_game)g_game->onMouseMove(x,y);}
void mouseClick(int b,int s,int x,int y){if(g_game)g_game->onMouseClick(b,s,x,y);}
void reshape(int w,int h){
    if(!h)h=1;glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();
    gluOrtho2D(0,WIN_W,0,WIN_H);
    glMatrixMode(GL_MODELVIEW);glLoadIdentity();
}
int main(int argc,char**argv){
    srand((unsigned)time(nullptr));
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(WIN_W,WIN_H);
    glutInitWindowPosition(100,80);
    glutCreateWindow("Arena Dungeon  –  3D");
    glClearColor(.04f,.04f,.06f,1);
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);
    reshape(WIN_W,WIN_H);
    g_game=new GameSystem();
    glutDisplayFunc(display);glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);glutKeyboardUpFunc(keyUp);
    glutPassiveMotionFunc(mouseMove);glutMotionFunc(mouseMove);
    glutMouseFunc(mouseClick);
    g_lastTime=glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(1000/FPS,timerCB,0);
    glutMainLoop();
    delete g_game;return 0;
}
