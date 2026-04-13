#include "Enemy.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>

static const float EP=3.14159265f;
struct GhostColor{float r,g,b;};
static GhostColor ghostColors[3]={{0.72f,0.88f,1.00f},{0.85f,0.60f,1.00f},{0.40f,0.90f,0.62f}};

static void disc(float y,float r,int segs,float cr,float cg,float cb,float ca){
    glColor4f(cr,cg,cb,ca);
    glBegin(GL_TRIANGLE_FAN);glVertex3f(0,y,0);
    for(int i=0;i<=segs;i++){float a=i*2*EP/segs;
        glVertex3f(cosf(a)*r,y,sinf(a)*r);}
    glEnd();
}

static void drawGhostBody(float cr,float cg,float cb,float wispPhase,int type){
    struct Ring{float y,r;};
    Ring rings[]={{-28,0},{-20,6},{-8,14},{0,20},{10,24},{20,22},
                  {30,18},{40,14},{50,18},{62,16},{70,8},{74,0}};
    int rCount=12;
    for(int i=0;i<rCount-1;i++){
        float y0=rings[i].y,r0=rings[i].r;
        float y1=rings[i+1].y,r1=rings[i+1].r;
        float a0=(i==0)?0:(i==1)?.25f:(i==2)?.45f:.62f;
        float a1=(i+1==0)?0:(i+1==1)?.25f:(i+1==2)?.45f:.62f;
        float waveMag=(i<4)?(4-i)*4.0f:0;
        int segs=18;
        glBegin(GL_TRIANGLE_STRIP);
        for(int j=0;j<=segs;j++){
            float ang=j*2*EP/segs;
            float w0=sinf(ang*2+wispPhase+i*.8f)*waveMag;
            float w1=sinf(ang*2+wispPhase+(i+1)*.8f)*waveMag;
            glColor4f(cr,cg,cb,a0);
            glVertex3f((r0+w0)*cosf(ang),y0,(r0+w0)*sinf(ang));
            glColor4f(cr,cg,cb,a1);
            glVertex3f((r1+w1)*cosf(ang),y1,(r1+w1)*sinf(ang));
        }
        glEnd();
    }
    float eyeY=55,eyeR=(type==2)?4.5f:3.5f,eyeZ=17,eyeX=7;
    glColor4f(1,.95f,.2f,.5f);
    for(int side=-1;side<=1;side+=2){
        glBegin(GL_TRIANGLE_FAN);glVertex3f(side*eyeX,eyeY,eyeZ);
        for(int i=0;i<=14;i++){float a=i*2*EP/14;
            glVertex3f(side*eyeX+cosf(a)*eyeR*2.2f,eyeY+sinf(a)*eyeR*2.2f,eyeZ);}
        glEnd();
    }
    float pc[3][3]={{.9f,.8f,.05f},{1,.5f,.05f},{.25f,1,.3f}};
    glColor4f(pc[type][0],pc[type][1],pc[type][2],.98f);
    for(int side=-1;side<=1;side+=2){
        glBegin(GL_TRIANGLE_FAN);glVertex3f(side*eyeX,eyeY,eyeZ);
        for(int i=0;i<=12;i++){float a=i*2*EP/12;
            glVertex3f(side*eyeX+cosf(a)*eyeR,eyeY+sinf(a)*eyeR,eyeZ+.5f);}
        glEnd();
    }
    glColor4f(0,0,0,.8f);
    float mouthY=44;
    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<=10;i++){float t=i/10.0f-.5f;
        glVertex3f(t*18,mouthY+sinf(t*EP*2+wispPhase*.5f)*2.5f,eyeZ+.5f);}
    glEnd();
}

Enemy::Enemy():x(0),y(0),speed(0),hp(0),maxHp(0),radius(0),
    alive(false),damage(0),attackCooldown(1),attackTimer(0),
    type(0),flashTimer(0),floatTimer(0),wispPhase(0),faceAngle(0),hoverY(0){}

Enemy::Enemy(float _x,float _y,int _type){
    x=_x;y=_y;type=_type;alive=true;attackTimer=0;flashTimer=0;
    floatTimer=(float)(rand()%628)/100.0f;
    wispPhase=(float)(rand()%314)/100.0f;
    faceAngle=0;hoverY=0;
    switch(type){
        case 0:speed=90;hp=maxHp=30;radius=18;damage=10;attackCooldown=1.0f;break;
        case 1:speed=165;hp=maxHp=15;radius=12;damage=7;attackCooldown=.6f;break;
        case 2:speed=55;hp=maxHp=80;radius=25;damage=22;attackCooldown=1.6f;break;
        default:speed=90;hp=maxHp=30;radius=18;damage=10;attackCooldown=1.0f;
    }
}

void Enemy::update(float dt,float px,float py){
    if(!alive)return;
    if(flashTimer>0)flashTimer-=dt;
    if(attackTimer>0)attackTimer-=dt;
    floatTimer+=dt;wispPhase+=dt*(type==1?3.2f:1.8f);
    hoverY=12.0f+sinf(floatTimer*2.2f)*6.0f;
    if(type==2)hoverY+=5;
    float dx=px-x,dy=py-y,dist=sqrtf(dx*dx+dy*dy);
    if(dist>.1f){x+=(dx/dist)*speed*dt;y+=(dy/dist)*speed*dt;}
    faceAngle=atan2f(dx,-dy)*180.0f/EP;
}

void Enemy::draw(){
    if(!alive)return;
    GhostColor gc=ghostColors[type];bool flash=(flashTimer>0);
    glPushMatrix();
    glTranslatef(x,hoverY,-y);
    glRotatef(faceAngle,0,1,0);
    glDepthMask(GL_FALSE);
    // Shadow
    glPushMatrix();glTranslatef(0,-hoverY+0.3f,0);
    float sr=radius*(0.6f+0.4f*(1.0f-hoverY/30.0f));
    glColor4f(0,0,0,.28f);
    glBegin(GL_TRIANGLE_FAN);glVertex3f(0,0,0);
    for(int i=0;i<=18;i++){float a=i*2*EP/18;glVertex3f(cosf(a)*sr,0,sinf(a)*sr);}
    glEnd();
    glPopMatrix();
    // Glow
    float gr=(float)(type+1)*10+25;
    glColor4f(gc.r,gc.g,gc.b,flash?.30f:.12f);
    glBegin(GL_TRIANGLE_FAN);glVertex3f(0,30,0);
    for(int i=0;i<=20;i++){float a=i*2*EP/20;
        glVertex3f(cosf(a)*gr,30+sinf(a)*gr*.7f,sinf(a)*gr);}
    glEnd();
    float cr=flash?1.0f:gc.r,cg=flash?1.0f:gc.g,cb2=flash?1.0f:gc.b;
    drawGhostBody(cr,cg,cb2,wispPhase,type);
    // HP bar
    {float bw=40,bh=4,by=82;
    glColor4f(.25f,.08f,.08f,.9f);
    glBegin(GL_QUADS);
    glVertex3f(-bw*.5f,by-bh*.5f,-1);glVertex3f(bw*.5f,by-bh*.5f,-1);
    glVertex3f(bw*.5f,by+bh*.5f,-1);glVertex3f(-bw*.5f,by+bh*.5f,-1);glEnd();
    float fr=(float)hp/maxHp;
    glColor4f(.1f+.8f*(1-fr),.7f*fr,.1f,.95f);
    glBegin(GL_QUADS);
    glVertex3f(-bw*.5f,by-bh*.5f,0);glVertex3f(-bw*.5f+bw*fr,by-bh*.5f,0);
    glVertex3f(-bw*.5f+bw*fr,by+bh*.5f,0);glVertex3f(-bw*.5f,by+bh*.5f,0);glEnd();}
    glDepthMask(GL_TRUE);
    glPopMatrix();
}

void Enemy::takeDamage(int dmg){
    hp-=dmg;flashTimer=0.12f;if(hp<=0){hp=0;alive=false;}
}
bool Enemy::overlapsCircle(float cx,float cy,float cr)const{
    float dx=cx-x,dy=cy-y;return sqrtf(dx*dx+dy*dy)<(radius+cr);
}
