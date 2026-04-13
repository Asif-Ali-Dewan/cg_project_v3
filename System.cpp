#include "System.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>

static const float SP=3.14159265f;
GameSystem* g_game=nullptr;
static float randf(){return(float)rand()/(float)RAND_MAX;}

GameSystem::GameSystem()
    :ui(WORLD_W,WORLD_H),state(STATE_MENU),
     score(0),wave(0),highScore(0),paused(false),
     waveBannerTimer(0),betweenWaveTimer(0),waveClearPending(false),
     mouseRawX(400),mouseRawY(300),
     mouseWorldX(400),mouseWorldY(300)
{
    swordPickup.active=false;
}

// ── camera ───────────────────────────────────────────────────────────────────

void GameSystem::setup3DCamera(){
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(52.0,(double)WORLD_W/WORLD_H,1.0,3000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(400,500,200,  400,0,-300,  0,1,0);

    // Capture matrices for mouse unprojection
    glGetDoublev(GL_MODELVIEW_MATRIX,  mvMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
}

void GameSystem::begin2D(){
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();
    gluOrtho2D(0,WORLD_W,0,WORLD_H);
    glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
}
void GameSystem::end2D(){
    glMatrixMode(GL_PROJECTION);glPopMatrix();
    glMatrixMode(GL_MODELVIEW);glPopMatrix();
}

// ── mouse unprojection ───────────────────────────────────────────────────────
// Cast a ray from the screen pixel through the 3D scene and intersect y=0 plane.
void GameSystem::unprojectMouse(){
    // Convert to OpenGL screen coords (y flipped)
    double sx=(double)mouseRawX;
    double sy=(double)(viewport[3]-mouseRawY);

    double wx0,wy0,wz0,wx1,wy1,wz1;
    gluUnProject(sx,sy,0.0,mvMatrix,projMatrix,viewport,&wx0,&wy0,&wz0);
    gluUnProject(sx,sy,1.0,mvMatrix,projMatrix,viewport,&wx1,&wy1,&wz1);

    // Ray direction
    double ddy=wy1-wy0;
    if(fabs(ddy)<0.0001)return;  // ray parallel to ground, skip

    // Intersect with y=0 plane
    double t=-wy0/ddy;
    double gx=wx0+t*(wx1-wx0);
    double gz=wz0+t*(wz1-wz0);

    // 3D world: x=gameX, z=-gameY
    mouseWorldX=(float)gx;
    mouseWorldY=-(float)gz;

    // Clamp to arena
    if(mouseWorldX<0)mouseWorldX=0;
    if(mouseWorldX>WORLD_W)mouseWorldX=WORLD_W;
    if(mouseWorldY<0)mouseWorldY=0;
    if(mouseWorldY>WORLD_H)mouseWorldY=WORLD_H;
}

// ── lifecycle ─────────────────────────────────────────────────────────────────

void GameSystem::startGame(){
    player.reset();abilities.reset();
    enemies.clear();sparks.clear();
    score=wave=0;paused=false;
    waveClearPending=false;betweenWaveTimer=0;waveBannerTimer=0;
    swordPickup.active=false;
    state=STATE_PLAYING;
    spawnWave(1);
}

void GameSystem::spawnWave(int waveNum){
    wave=waveNum;waveBannerTimer=2.5f;enemies.clear();
    int nc=2+waveNum*2,fc=(waveNum>=2)?waveNum-1:0,tc=(waveNum>=3)?(waveNum-2)/2+1:0;
    if(nc>14)nc=14;if(fc>8)fc=8;if(tc>4)tc=4;
    int total=nc+fc+tc;
    for(int i=0;i<total;i++){
        int side=rand()%4;float m=45,ex,ey;
        switch(side){
            case 0:ex=m+randf()*(WORLD_W-2*m);ey=WORLD_H-m;break;
            case 1:ex=m+randf()*(WORLD_W-2*m);ey=m;        break;
            case 2:ex=m;ey=m+randf()*(WORLD_H-2*m);        break;
            default:ex=WORLD_W-m;ey=m+randf()*(WORLD_H-2*m);
        }
        enemies.push_back(Enemy(ex,ey,(i<nc)?0:(i<nc+fc)?1:2));
    }
    waveClearPending=false;
}

// ── sword pickup ─────────────────────────────────────────────────────────────

void GameSystem::updateSwordPickup(float dt){
    if(!swordPickup.active)return;
    swordPickup.rotAngle+=dt*120;
    swordPickup.bobTimer+=dt;

    // Check if player walks over it
    float dx=player.x-swordPickup.x, dy=player.y-swordPickup.y;
    if(sqrtf(dx*dx+dy*dy)<28.0f){
        player.hasSword=true;
        player.swordCharges=Player::MAX_CHARGES;
        player.swordRechargeTimer=0;
        swordPickup.active=false;
    }
}

void GameSystem::drawSwordPickup(){
    if(!swordPickup.active)return;
    float bobY=8.0f+sinf(swordPickup.bobTimer*3.0f)*6.0f;

    glDepthMask(GL_FALSE);
    glPushMatrix();
    glTranslatef(swordPickup.x, bobY, -swordPickup.y);
    glRotatef(swordPickup.rotAngle, 0,1,0);

    // Ground glow
    glPushMatrix();glTranslatef(0,-bobY+1,0);
    glColor4f(0.25f,0.85f,1.0f,0.22f);
    glBegin(GL_TRIANGLE_FAN);glVertex3f(0,0,0);
    for(int i=0;i<=20;i++){float a=i*2*SP/20;
        glVertex3f(cosf(a)*32,0,sinf(a)*32);}
    glEnd();
    glPopMatrix();

    // Outer sparkle ring
    glColor4f(0.5f,1.0f,1.0f, 0.5f+sinf(swordPickup.bobTimer*4)*0.3f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<16;i++){float a=i*2*SP/16;
        glVertex3f(cosf(a)*28,0,sinf(a)*28);}
    glEnd();

    // Sword blade (horizontal, glowing)
    // Handle
    glColor3f(0.55f,0.32f,0.08f);
    glBegin(GL_QUADS);
    glVertex3f(-3,-4,0);glVertex3f(3,-4,0);
    glVertex3f(3,4,0);glVertex3f(-3,4,0);
    glEnd();
    // Guard
    glColor3f(0.88f,0.72f,0.18f);
    glBegin(GL_QUADS);
    glVertex3f(-12,-2.5f,0);glVertex3f(12,-2.5f,0);
    glVertex3f(12,2.5f,0);glVertex3f(-12,2.5f,0);
    glEnd();
    // Blade
    glColor4f(0.7f,0.95f,1.0f,0.92f);
    glBegin(GL_QUADS);
    glVertex3f(12,-2,0);glVertex3f(48,-1,0);
    glVertex3f(48,1,0);glVertex3f(12,2,0);
    glEnd();
    glColor4f(0.4f,0.9f,1.0f,0.55f);
    glBegin(GL_QUADS);
    glVertex3f(12,-5,0);glVertex3f(48,-2,0);
    glVertex3f(48,2,0);glVertex3f(12,5,0);
    glEnd();
    // Tip
    glColor4f(1,1,1,0.9f);
    glBegin(GL_TRIANGLES);
    glVertex3f(48,0,0);glVertex3f(43,-2,0);glVertex3f(43,2,0);
    glEnd();

    glPopMatrix();
    glDepthMask(GL_TRUE);
}

// ── update ───────────────────────────────────────────────────────────────────

void GameSystem::update(float dt){
    if(state==STATE_MENU||state==STATE_GAME_OVER)return;
    if(paused)return;
    if(waveBannerTimer>0)waveBannerTimer-=dt;

    // Update mouse world pos from stored matrices
    unprojectMouse();

    // Player faces toward actual world mouse
    float dx=mouseWorldX-player.x, dy=mouseWorldY-player.y;
    player.faceAngle=atan2f(dx,-dy)*180.0f/SP;

    player.update(dt,(float)WORLD_W,(float)WORLD_H);
    abilities.update(dt);
    for(auto&e:enemies)e.update(dt,player.x,player.y);
    checkCollisions();
    updateSparks(dt);
    updateSwordPickup(dt);

    bool anyAlive=false;
    for(auto&e:enemies)if(e.alive){anyAlive=true;break;}
    if(!anyAlive&&!waveClearPending){
        waveClearPending=true;betweenWaveTimer=2.8f;
        // Spawn special sword after wave 2 clears
        if(wave==2&&!player.hasSword){
            swordPickup.x=WORLD_W*.5f;
            swordPickup.y=WORLD_H*.5f;
            swordPickup.rotAngle=0;swordPickup.bobTimer=0;
            swordPickup.active=true;
        }
    }
    if(waveClearPending&&betweenWaveTimer>0){
        betweenWaveTimer-=dt;
        if(betweenWaveTimer<=0)spawnWave(wave+1);
    }
    if(!player.alive){if(score>highScore)highScore=score;state=STATE_GAME_OVER;}
}

// ── collisions ───────────────────────────────────────────────────────────────

void GameSystem::checkCollisions(){
    for(auto&proj:abilities.projectiles){
        if(!proj.active)continue;
        for(auto&e:enemies){
            if(!e.alive)continue;
            bool hit=false;
            if(proj.isAOE){float dx=proj.x-e.x,dy=proj.y-e.y;
                hit=sqrtf(dx*dx+dy*dy)<proj.aoeCurRadius+e.radius;}
            else hit=e.overlapsCircle(proj.x,proj.y,proj.radius);
            if(hit){
                bool wd=(e.hp<=0);e.takeDamage(proj.damage);
                spawnSpark(e.x,e.y,1,.6f,.1f);
                if(!wd&&!e.alive)score+=(e.type==2?30:e.type==1?15:10)*wave;
                if(!proj.isAOE)proj.active=false;
            }
        }
    }
    for(auto&e:enemies){
        if(!e.alive)continue;
        if(e.overlapsCircle(player.x,player.y,player.radius)&&e.canAttack()){
            player.takeDamage(e.damage);e.resetAttack();
            spawnSpark(player.x,player.y,1,.2f,.2f);
        }
    }
}

// Cleave: hit up to 4 enemies in 100° arc in front of player
void GameSystem::doCleave(){
    float faceRad=player.faceAngle*SP/180.0f;
    // Forward direction in game coords
    float fx=sinf(faceRad), fy=-cosf(faceRad);
    float arcHalfDeg=50.0f; // ±50° = 100° wide
    float arcHalfRad=arcHalfDeg*SP/180.0f;
    float range=120.0f;
    int hitCount=0;

    for(auto&e:enemies){
        if(!e.alive)continue;
        if(hitCount>=4)break;
        float dx=e.x-player.x, dy=e.y-player.y;
        float dist=sqrtf(dx*dx+dy*dy);
        if(dist>range+e.radius)continue;
        // Angle between forward and enemy direction
        float ex2=dx/dist, ey2=dy/dist;
        float dot=fx*ex2+fy*ey2;
        float ang=acosf(dot<-1?-1:(dot>1?1:dot));
        if(ang<=arcHalfRad){
            bool wd=(e.hp<=0);
            e.takeDamage(abilities.cleaveDamage);
            spawnSpark(e.x,e.y,0.3f,0.9f,1.0f);
            if(!wd&&!e.alive)score+=(e.type==2?30:e.type==1?15:10)*wave;
            hitCount++;
        }
    }

    // Spawn visual arc effect
    abilities.spawnCleaveFX(player.x,player.y,player.faceAngle*SP/180.0f);
    player.startCleaveSwing();
}

// ── sparks ───────────────────────────────────────────────────────────────────

void GameSystem::spawnSpark(float x,float y,float r,float g,float b){
    for(int i=0;i<7;i++){
        HitSpark sp;
        sp.x=x+(randf()-.5f)*22;sp.y=y+(randf()-.5f)*22;
        sp.vy3d=40+randf()*60;sp.life=sp.maxLife=.4f+randf()*.3f;
        sp.r=r;sp.g=g;sp.b=b;sparks.push_back(sp);
    }
}
void GameSystem::updateSparks(float dt){
    for(auto&sp:sparks){sp.life-=dt;sp.vy3d-=120*dt;}
    for(int i=(int)sparks.size()-1;i>=0;--i)
        if(sparks[i].life<=0)sparks.erase(sparks.begin()+i);
}
void GameSystem::drawSparks(){
    glDepthMask(GL_FALSE);glPointSize(5.5f);
    for(auto&sp:sparks){
        float a=sp.life/sp.maxLife,el=sp.maxLife-sp.life;
        float sy=sp.vy3d*el-60*el*el;if(sy<0)sy=0;
        glColor4f(sp.r,sp.g,sp.b,a);
        glBegin(GL_POINTS);glVertex3f(sp.x,sy+2,-sp.y);glEnd();
        glColor4f(sp.r*.5f,sp.g*.5f,sp.b*.5f,a*.4f);
        glBegin(GL_POINTS);glVertex3f(sp.x,1,-sp.y);glEnd();
    }
    glPointSize(1);glDepthMask(GL_TRUE);
}

// ── arena ─────────────────────────────────────────────────────────────────────
void GameSystem::drawArena(){
    float W=WORLD_W,H=WORLD_H;
    // Tiled floor
    for(int tx=0;tx<WORLD_W/50;tx++)for(int ty=0;ty<WORLD_H/50;ty++){
        float bx=tx*50.f,by=ty*50.f;
        bool dk=((tx+ty)%2==0);
        glColor3f(dk?.09f:.11f,dk?.09f:.11f,dk?.12f:.15f);
        glBegin(GL_QUADS);
        glVertex3f(bx,0,-by);glVertex3f(bx+50,0,-by);
        glVertex3f(bx+50,0,-(by+50));glVertex3f(bx,0,-(by+50));
        glEnd();
    }
    // Grid
    glColor3f(.18f,.18f,.22f);glLineWidth(.8f);
    for(int gx=0;gx<=800;gx+=50){
        glBegin(GL_LINES);glVertex3f((float)gx,.5f,0);glVertex3f((float)gx,.5f,-H);glEnd();}
    for(int gy=0;gy<=600;gy+=50){
        glBegin(GL_LINES);glVertex3f(0,.5f,-(float)gy);glVertex3f(W,.5f,-(float)gy);glEnd();}
    // Walls
    float wH=40;
    glColor3f(.22f,.22f,.30f);
    // N
    glBegin(GL_QUADS);glVertex3f(0,0,-H);glVertex3f(W,0,-H);glVertex3f(W,wH,-H);glVertex3f(0,wH,-H);glEnd();
    // S
    glBegin(GL_QUADS);glVertex3f(0,0,0);glVertex3f(W,0,0);glVertex3f(W,wH,0);glVertex3f(0,wH,0);glEnd();
    // W
    glBegin(GL_QUADS);glVertex3f(0,0,0);glVertex3f(0,0,-H);glVertex3f(0,wH,-H);glVertex3f(0,wH,0);glEnd();
    // E
    glBegin(GL_QUADS);glVertex3f(W,0,0);glVertex3f(W,0,-H);glVertex3f(W,wH,-H);glVertex3f(W,wH,0);glEnd();
    glColor3f(.35f,.35f,.45f);glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(0,wH,0);glVertex3f(W,wH,0);glVertex3f(W,wH,-H);glVertex3f(0,wH,-H);glEnd();
    // Corner torches
    float cs[4][2]={{0,0},{W,0},{W,-H},{0,-H}};
    for(int k=0;k<4;k++){
        float px=cs[k][0],pz=cs[k][1];
        glColor3f(.28f,.28f,.35f);
        glBegin(GL_QUADS);
        glVertex3f(px-7,0,pz+7);glVertex3f(px+7,0,pz+7);
        glVertex3f(px+7,55,pz+7);glVertex3f(px-7,55,pz+7);glEnd();
        // Torch
        glDepthMask(GL_FALSE);
        float fy=65;
        glColor4f(1,.5f,0,.2f);
        glBegin(GL_TRIANGLE_FAN);glVertex3f(px,fy,pz);
        for(int i=0;i<=16;i++){float a=i*2*SP/16;glVertex3f(px+cosf(a)*18,fy,pz+sinf(a)*18);}
        glEnd();
        glColor4f(1,.7f,.1f,.85f);
        glBegin(GL_TRIANGLE_FAN);glVertex3f(px,fy+16,pz);
        for(int i=0;i<=10;i++){float a=i*2*SP/10;glVertex3f(px+cosf(a)*6,fy,pz+sinf(a)*6);}
        glEnd();
        glColor4f(1,1,.4f,.9f);
        glBegin(GL_TRIANGLE_FAN);glVertex3f(px,fy+10,pz);
        for(int i=0;i<=8;i++){float a=i*2*SP/8;glVertex3f(px+cosf(a)*3,fy,pz+sinf(a)*3);}
        glEnd();
        glDepthMask(GL_TRUE);
    }
}

// ── draw ─────────────────────────────────────────────────────────────────────
void GameSystem::draw(){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    if(state==STATE_MENU){
        begin2D();
        glColor3f(.05f,.05f,.08f);
        glBegin(GL_QUADS);glVertex2f(0,0);glVertex2f(WORLD_W,0);
        glVertex2f(WORLD_W,WORLD_H);glVertex2f(0,WORLD_H);glEnd();
        ui.drawMenu(highScore);end2D();return;
    }

    // 3D scene
    setup3DCamera();
    drawArena();
    drawSwordPickup();
    glEnable(GL_DEPTH_TEST);
    player.draw();
    glDepthMask(GL_FALSE);
    for(auto&e:enemies)e.draw();
    glDepthMask(GL_TRUE);
    abilities.draw();
    drawSparks();

    // Crosshair on ground plane at actual world position
    {
        float cx=mouseWorldX,cy=mouseWorldY;
        glDepthMask(GL_FALSE);
        glColor4f(1,1,1,.6f);glLineWidth(1.5f);
        float cs=12;
        glBegin(GL_LINES);
        glVertex3f(cx-cs,2,-cy);glVertex3f(cx+cs,2,-cy);
        glVertex3f(cx,2,-cy-cs);glVertex3f(cx,2,-cy+cs);glEnd();
        glColor4f(1,1,.4f,.5f);
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<16;i++){float a=i*2*SP/16;
            glVertex3f(cx+cosf(a)*7,2,-cy+sinf(a)*7);}
        glEnd();
        glDepthMask(GL_TRUE);
    }

    // 2D HUD
    begin2D();
    ui.drawHUD(player.hp,player.maxHp,player.mana,player.maxMana,
               score,wave,
               abilities.shootCDFrac(),abilities.aoeCDFrac(),
               paused,
               player.hasSword,player.swordCharges,
               Player::MAX_CHARGES,player.swordRechargeTimer,Player::SWORD_RECHARGE_TIME);
    ui.drawWaveBanner(wave,waveBannerTimer/2.5f);
    if(swordPickup.active&&wave==2)
        ui.drawSwordHint();
    if(state==STATE_GAME_OVER)ui.drawGameOver(score,wave,highScore);
    end2D();
}

// ── input ─────────────────────────────────────────────────────────────────────
void GameSystem::onKeyDown(unsigned char key){
    switch(key){
        case'w':case'W':player.moveUp=true;    break;
        case's':case'S':player.moveDown=true;  break;
        case'a':case'A':player.moveLeft=true;  break;
        case'd':case'D':player.moveRight=true; break;
        case'q':case'Q':
            if(state==STATE_PLAYING&&!paused&&player.alive)
                if(player.useMana(abilities.shootManaCost)){
                    abilities.shoot(player.x,player.y,mouseWorldX,mouseWorldY);
                    player.startShootAnim();
                }
            break;
        case'f':case'F':    // Cleave with special sword
            if(state==STATE_PLAYING&&!paused&&player.alive&&player.hasSword&&player.swordCharges>0&&!player.isSwinging){
                player.swordCharges--;
                if(player.swordCharges==Player::MAX_CHARGES-1)
                    player.swordRechargeTimer=0;
                doCleave();
            }
            break;
        case'e':case'E':
            if(state==STATE_PLAYING&&!paused&&player.alive)
                if(player.useMana(abilities.aoeManaCost))
                    abilities.castAOE(player.x,player.y);
            break;
        case'p':case'P':if(state==STATE_PLAYING)paused=!paused;break;
        case'\r':case'\n':if(state==STATE_MENU||state==STATE_GAME_OVER)startGame();break;
        case 27:if(state==STATE_GAME_OVER)exit(0);if(state==STATE_PLAYING)state=STATE_MENU;break;
    }
}
void GameSystem::onKeyUp(unsigned char key){
    switch(key){
        case'w':case'W':player.moveUp=false;    break;
        case's':case'S':player.moveDown=false;  break;
        case'a':case'A':player.moveLeft=false;  break;
        case'd':case'D':player.moveRight=false; break;
    }
}
void GameSystem::onMouseMove(int x,int y){
    mouseRawX=x;mouseRawY=y;
}
void GameSystem::onMouseClick(int button,int st,int x,int y){
    mouseRawX=x;mouseRawY=y;
    if(button==GLUT_LEFT_BUTTON&&st==GLUT_DOWN){
        if(state==STATE_PLAYING&&!paused&&player.alive)
            if(player.useMana(abilities.shootManaCost)){
                abilities.shoot(player.x,player.y,mouseWorldX,mouseWorldY);
                player.startShootAnim();
            }
    }
    if(button==GLUT_RIGHT_BUTTON&&st==GLUT_DOWN){
        if(state==STATE_PLAYING&&!paused&&player.alive)
            if(player.useMana(abilities.aoeManaCost))
                abilities.castAOE(player.x,player.y);
    }
}
