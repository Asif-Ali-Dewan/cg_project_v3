#include "UI.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glut.h>
#include <cstdio>
#include <cmath>

static const float UP=3.14159265f;
UI::UI(int w,int h):windowW(w),windowH(h){}

void UI::drawBar(float x,float y,float w,float h,float frac,
                 float fr,float fg,float fb,float br,float bg,float bb){
    glColor3f(br,bg,bb);
    glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h);glEnd();
    if(frac>0){glColor3f(fr,fg,fb);
        glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+w*frac,y);
        glVertex2f(x+w*frac,y+h);glVertex2f(x,y+h);glEnd();}
    glColor3f(.7f,.7f,.7f);glLineWidth(1.4f);
    glBegin(GL_LINE_LOOP);glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h);glEnd();
}

void UI::drawText(float x,float y,const char*t,float r,float g,float b,void*font){
    glColor3f(r,g,b);glRasterPos2f(x,y);
    void*f=font?font:GLUT_BITMAP_HELVETICA_18;
    for(const char*c=t;*c;c++)glutBitmapCharacter(f,*c);
}

void UI::drawTextA(float x,float y,const char*t,float r,float g,float b,float a,void*font){
    glColor4f(r,g,b,a);glRasterPos2f(x,y);
    void*f=font?font:GLUT_BITMAP_HELVETICA_18;
    for(const char*c=t;*c;c++)glutBitmapCharacter(f,*c);
}

void UI::drawAbilityIcon(float x,float y,float sz,int tp,float cd,const char*hk){
    glColor3f(.12f,.12f,.16f);
    glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+sz,y);glVertex2f(x+sz,y+sz);glVertex2f(x,y+sz);glEnd();
    float cx=x+sz*.5f,cy=y+sz*.5f,ir=sz*.3f;
    if(tp==0){
        // Gun icon
        glColor3f(.55f,.55f,.6f);
        glBegin(GL_QUADS);glVertex2f(cx-ir*.5f,cy-ir*.3f);glVertex2f(cx+ir,cy-ir*.3f);
            glVertex2f(cx+ir,cy+ir*.3f);glVertex2f(cx-ir*.5f,cy+ir*.3f);glEnd();
        glColor3f(.38f,.38f,.42f);
        glBegin(GL_QUADS);glVertex2f(cx-ir*.4f,cy-ir*.6f);glVertex2f(cx,cy-ir*.6f);
            glVertex2f(cx,cy-ir*.3f);glVertex2f(cx-ir*.4f,cy-ir*.3f);glEnd();
        // Muzzle flash
        glColor3f(1,.85f,.2f);
        glBegin(GL_TRIANGLE_FAN);glVertex2f(cx+ir,cy);
        for(int i=0;i<=8;i++){float a=i*2*UP/8;
            glVertex2f(cx+ir+cosf(a)*ir*.28f,cy+sinf(a)*ir*.28f);}
        glEnd();
    } else {
        glColor3f(1,.75f,0);glLineWidth(2.5f);
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<20;i++){float a=i*2*UP/20;
            glVertex2f(cx+cosf(a)*ir,cy+sinf(a)*ir);}
        glEnd();
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<20;i++){float a=i*2*UP/20;
            glVertex2f(cx+cosf(a)*ir*.5f,cy+sinf(a)*ir*.5f);}
        glEnd();
    }
    if(cd<1){glColor4f(0,0,0,.65f);
        glBegin(GL_QUADS);glVertex2f(x,y+sz*cd);glVertex2f(x+sz,y+sz*cd);
        glVertex2f(x+sz,y+sz);glVertex2f(x,y+sz);glEnd();}
    glColor3f(.7f,.7f,.7f);glLineWidth(1.4f);
    glBegin(GL_LINE_LOOP);glVertex2f(x,y);glVertex2f(x+sz,y);glVertex2f(x+sz,y+sz);glVertex2f(x,y+sz);glEnd();
    drawText(x+3,y+3,hk,1,1,.5f,GLUT_BITMAP_HELVETICA_12);
}

// Special sword charge icon
void UI::drawSwordIcon(float x,float y,float sz,int charges,int maxC,float rechargeFrac){
    // Background
    glColor3f(.08f,.12f,.15f);
    glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+sz,y);glVertex2f(x+sz,y+sz);glVertex2f(x,y+sz);glEnd();

    // Sword blade icon
    float cx=x+sz*.5f,cy=y+sz*.5f;
    glColor4f(.6f,.95f,1.0f, charges>0?0.9f:0.35f);
    // Blade
    glBegin(GL_QUADS);
    glVertex2f(cx-3,cy-sz*.35f);glVertex2f(cx+3,cy-sz*.35f);
    glVertex2f(cx+2,cy+sz*.38f);glVertex2f(cx-2,cy+sz*.38f);
    glEnd();
    // Guard (gold)
    glColor4f(.88f,.72f,.18f, charges>0?1:0.35f);
    glBegin(GL_QUADS);
    glVertex2f(cx-sz*.28f,cy-sz*.05f);glVertex2f(cx+sz*.28f,cy-sz*.05f);
    glVertex2f(cx+sz*.28f,cy+sz*.05f);glVertex2f(cx-sz*.28f,cy+sz*.05f);
    glEnd();

    // Charge dots at bottom of icon
    float dotR=4.5f;
    float totalW=(maxC*(dotR*2+3))-3;
    float startX=cx-totalW*.5f+dotR;
    for(int i=0;i<maxC;i++){
        float dx2=startX+i*(dotR*2+3);
        bool filled=(i<charges);
        if(filled) glColor4f(.4f,1.0f,1.0f,.95f);
        else       glColor4f(.25f,.35f,.38f,.8f);
        glBegin(GL_TRIANGLE_FAN);glVertex2f(dx2,y+8);
        for(int j=0;j<=12;j++){float a=j*2*UP/12;
            glVertex2f(dx2+cosf(a)*dotR,y+8+sinf(a)*dotR);}
        glEnd();
        glColor4f(.6f,.8f,.8f,.6f);glLineWidth(1.2f);
        glBegin(GL_LINE_LOOP);
        for(int j=0;j<12;j++){float a=j*2*UP/12;
            glVertex2f(dx2+cosf(a)*dotR,y+8+sinf(a)*dotR);}
        glEnd();
    }

    // Recharge bar (thin bar at very bottom)
    if(charges<maxC&&rechargeFrac>0){
        glColor4f(.18f,.25f,.28f,.9f);
        glBegin(GL_QUADS);glVertex2f(x+3,y+2);glVertex2f(x+sz-3,y+2);
        glVertex2f(x+sz-3,y+6);glVertex2f(x+3,y+6);glEnd();
        glColor4f(.3f,.9f,1.0f,.9f);
        float bw=(sz-6)*rechargeFrac;
        glBegin(GL_QUADS);glVertex2f(x+3,y+2);glVertex2f(x+3+bw,y+2);
        glVertex2f(x+3+bw,y+6);glVertex2f(x+3,y+6);glEnd();
    }

    // Border (glowing cyan if has charges)
    if(charges>0)glColor3f(.3f,.8f,1.0f);
    else         glColor3f(.4f,.4f,.4f);
    glLineWidth(1.6f);
    glBegin(GL_LINE_LOOP);glVertex2f(x,y);glVertex2f(x+sz,y);glVertex2f(x+sz,y+sz);glVertex2f(x,y+sz);glEnd();

    drawText(x+3,y+3,"F",1,1,.5f,GLUT_BITMAP_HELVETICA_12);
}

void UI::drawHUD(int hp,int maxHp,int mana,int maxMana,
                 int score,int wave,
                 float shootCDFrac,float aoeCDFrac,bool paused,
                 bool hasSword,int swordCharges,int maxCharges,
                 float rechargeTimer,float rechargeTime)
{
    char buf[80];
    // Bottom panel
    glColor4f(0,0,0,.62f);
    glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(windowW,0);
    glVertex2f(windowW,68);glVertex2f(0,68);glEnd();

    drawText(10,46,"HP",1,.3f,.3f,GLUT_BITMAP_HELVETICA_12);
    drawBar(36,39,188,18,(float)hp/maxHp,.9f,.15f,.15f,.25f,.08f,.08f);
    sprintf(buf,"%d/%d",hp,maxHp);drawText(230,46,buf,1,1,1,GLUT_BITMAP_HELVETICA_12);

    drawText(10,19,"MP",.3f,.55f,1,GLUT_BITMAP_HELVETICA_12);
    drawBar(36,12,188,18,(float)mana/maxMana,.2f,.42f,1,.08f,.12f,.28f);
    sprintf(buf,"%d/%d",mana,maxMana);drawText(230,19,buf,1,1,1,GLUT_BITMAP_HELVETICA_12);

    // Ability icons
    drawAbilityIcon(295,10,48,0,shootCDFrac,"Q");
    drawAbilityIcon(350,10,48,1,aoeCDFrac,"E");

    // Sword icon (only shown once player has sword, or hint after wave 2)
    if(hasSword){
        float rechargeFrac = (rechargeTime>0) ? (rechargeTimer/rechargeTime) : 1.0f;
        drawSwordIcon(406,10,48,swordCharges,maxCharges,rechargeFrac);
    }

    // Top bar
    glColor4f(0,0,0,.52f);
    glBegin(GL_QUADS);glVertex2f(0,windowH-32);glVertex2f(windowW,windowH-32);
    glVertex2f(windowW,windowH);glVertex2f(0,windowH);glEnd();
    sprintf(buf,"WAVE  %d",wave);
    drawText(windowW-140,windowH-22,buf,1,1,.4f);
    sprintf(buf,"SCORE: %d",score);
    drawText(10,windowH-22,buf,.82f,.82f,.82f);

    drawText(10,windowH-54,
             "WASD:Move  Q/Click:Shoot  E/RClick:Blast  F:Cleave(Sword)  P:Pause",
             .48f,.48f,.48f,GLUT_BITMAP_HELVETICA_10);

    if(paused){
        glColor4f(0,0,0,.5f);
        glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(windowW,0);
        glVertex2f(windowW,windowH);glVertex2f(0,windowH);glEnd();
        drawText(windowW/2-60,windowH/2+14,"PAUSED",1,1,.2f,GLUT_BITMAP_TIMES_ROMAN_24);
        drawText(windowW/2-100,windowH/2-18,"Press  P  to  Resume",.8f,.8f,.8f);
    }
}

void UI::drawSwordHint(){
    // Flashing "Glowing Sword appeared!" hint at center-top
    float t=(float)glutGet(GLUT_ELAPSED_TIME)/1000.0f;
    float alpha=0.7f+sinf(t*4)*0.3f;
    drawTextA(windowW/2-155,windowH/2+30,
              "⚔  Glowing Sword appeared!  Walk over it!",
              0.4f,1.0f,1.0f,alpha);
}

void UI::drawMenu(int highScore){
    glColor4f(0,0,0,.82f);
    glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(windowW,0);
    glVertex2f(windowW,windowH);glVertex2f(0,windowH);glEnd();
    drawText(windowW/2-168,windowH/2+105,"ARENA  DUNGEON",1,.78f,.15f,GLUT_BITMAP_TIMES_ROMAN_24);
    drawText(windowW/2-138,windowH/2+62,"Press  ENTER  to  Start",.92f,.92f,.92f);
    drawText(windowW/2-125,windowH/2+28,"WASD  -  Move",.72f,.72f,.72f);
    drawText(windowW/2-125,windowH/2+4,"Q / Left Click  -  Shoot  (8 MP)",.72f,.72f,.72f);
    drawText(windowW/2-125,windowH/2-20,"E / Right Click  -  AOE Blast  (40 MP)",.72f,.72f,.72f);
    drawText(windowW/2-125,windowH/2-44,"F  -  Special Sword Cleave  (pickup after Wave 2)",.72f,.72f,.72f);
    drawText(windowW/2-125,windowH/2-68,"P  -  Pause",.72f,.72f,.72f);
    char buf[48];sprintf(buf,"High Score:  %d",highScore);
    drawText(windowW/2-88,windowH/2-108,buf,.92f,.82f,.28f);
}

void UI::drawGameOver(int score,int wave,int highScore){
    glColor4f(0,0,0,.82f);
    glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(windowW,0);
    glVertex2f(windowW,windowH);glVertex2f(0,windowH);glEnd();
    drawText(windowW/2-118,windowH/2+92,"GAME  OVER",1,.18f,.18f,GLUT_BITMAP_TIMES_ROMAN_24);
    char buf[48];
    sprintf(buf,"Score:         %d",score);drawText(windowW/2-105,windowH/2+42,buf,1,1,.5f);
    sprintf(buf,"Wave Reached:  %d",wave);drawText(windowW/2-105,windowH/2+14,buf,.9f,.9f,.9f);
    sprintf(buf,"High Score:    %d",highScore);drawText(windowW/2-105,windowH/2-14,buf,.9f,.82f,.28f);
    drawText(windowW/2-138,windowH/2-58,"Press  ENTER  to  Play Again",.72f,.72f,.72f);
    drawText(windowW/2-100,windowH/2-83,"Press  ESC  to  Quit",.72f,.72f,.72f);
}

void UI::drawWaveBanner(int wave,float alpha){
    if(alpha<=0)return;
    char buf[32];sprintf(buf,"WAVE  %d",wave);
    drawTextA(windowW/2-72,windowH/2+90,buf,1,.95f,.22f,alpha,GLUT_BITMAP_TIMES_ROMAN_24);
    drawTextA(windowW/2-38,windowH/2+60,"Survive!",.9f,.9f,.9f,alpha*.8f);
}
