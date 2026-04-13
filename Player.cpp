#include "Player.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cmath>

static const float PP = 3.14159265f;

// ── primitive helpers ────────────────────────────────────────────────────────

static void box(float w,float h,float d,float r,float g,float b){
    float hw=w*.5f,hh=h*.5f,hd=d*.5f;
    // Top
    glColor3f(r*1.22f<1?r*1.22f:1,g*1.22f<1?g*1.22f:1,b*1.22f<1?b*1.22f:1);
    glBegin(GL_QUADS);
    glVertex3f(-hw,hh,-hd);glVertex3f(hw,hh,-hd);
    glVertex3f(hw,hh,hd); glVertex3f(-hw,hh,hd);
    glEnd();
    // Bottom
    glColor3f(r*.4f,g*.4f,b*.4f);
    glBegin(GL_QUADS);
    glVertex3f(-hw,-hh,hd);glVertex3f(hw,-hh,hd);
    glVertex3f(hw,-hh,-hd);glVertex3f(-hw,-hh,-hd);
    glEnd();
    // Front
    glColor3f(r,g,b);
    glBegin(GL_QUADS);
    glVertex3f(-hw,-hh,hd);glVertex3f(hw,-hh,hd);
    glVertex3f(hw,hh,hd); glVertex3f(-hw,hh,hd);
    glEnd();
    // Back
    glColor3f(r*.58f,g*.58f,b*.58f);
    glBegin(GL_QUADS);
    glVertex3f(hw,-hh,-hd);glVertex3f(-hw,-hh,-hd);
    glVertex3f(-hw,hh,-hd);glVertex3f(hw,hh,-hd);
    glEnd();
    // Left
    glColor3f(r*.72f,g*.72f,b*.72f);
    glBegin(GL_QUADS);
    glVertex3f(-hw,-hh,-hd);glVertex3f(-hw,-hh,hd);
    glVertex3f(-hw,hh,hd); glVertex3f(-hw,hh,-hd);
    glEnd();
    // Right
    glColor3f(r*.84f,g*.84f,b*.84f);
    glBegin(GL_QUADS);
    glVertex3f(hw,-hh,hd);glVertex3f(hw,-hh,-hd);
    glVertex3f(hw,hh,-hd);glVertex3f(hw,hh,hd);
    glEnd();
}

static void cyl(float r,float h,int s,float cr,float cg,float cb){
    glColor3f(cr*.82f,cg*.82f,cb*.82f);
    glBegin(GL_QUAD_STRIP);
    for(int i=0;i<=s;i++){float a=i*2*PP/s;
        glVertex3f(cosf(a)*r,0,sinf(a)*r);
        glVertex3f(cosf(a)*r,h,sinf(a)*r);}
    glEnd();
    glColor3f(cr,cg,cb);
    glBegin(GL_TRIANGLE_FAN);glVertex3f(0,h,0);
    for(int i=0;i<=s;i++){float a=i*2*PP/s;glVertex3f(cosf(a)*r,h,sinf(a)*r);}
    glEnd();
}

// ── Gun model (blocky Lego pistol) ───────────────────────────────────────────
static void drawGun(float recoil, bool hasSword){
    // recoil: 0..1 tilts the gun backward
    float kickback = recoil * 8.0f;

    glPushMatrix();
    glTranslatef(0,-4,-kickback);  // recoil pushes backward

    // Grip (dark handle)
    box(6,14,5, 0.22f,0.22f,0.24f);
    // Trigger guard
    glTranslatef(0,-8,-1);
    box(6,3,3,  0.18f,0.18f,0.20f);
    glTranslatef(0,8,1);

    // Slide / barrel (longer box on top)
    glTranslatef(0,5,4);
    box(5,7,20, 0.30f,0.30f,0.34f);

    // Barrel tip highlight
    glColor3f(0.55f,0.55f,0.60f);
    glBegin(GL_QUADS);
    glVertex3f(-2.5f,-3.5f,10);glVertex3f(2.5f,-3.5f,10);
    glVertex3f(2.5f, 3.5f,10);glVertex3f(-2.5f, 3.5f,10);
    glEnd();

    // Muzzle flash when recoil is high
    if(recoil>0.5f){
        float fi=recoil-0.5f;
        glColor4f(1.0f,0.9f,0.3f, fi*2.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0,0,12);
        for(int i=0;i<=12;i++){float a=i*2*PP/12;
            glVertex3f(cosf(a)*8*fi,sinf(a)*8*fi,10+i%2*4);}
        glEnd();
    }

    // Sight on top
    glTranslatef(0,4,-6);
    box(2,3,3, 0.40f,0.40f,0.44f);

    glPopMatrix();
}

// ── Special Sword model (for cleave swing) ───────────────────────────────────
static void drawSpecialSword(){
    glPushMatrix();
    // Handle
    box(5,14,5, 0.55f,0.32f,0.08f);
    // Cross guard (gold)
    glTranslatef(0,-8,0);
    box(20,4,5, 0.88f,0.72f,0.18f);
    // Blade (longer, glowing cyan)
    glTranslatef(0,-28,0);
    // Blade core
    glColor3f(0.72f,0.94f,1.0f);
    glBegin(GL_QUADS);
    // Front face
    glVertex3f(-3,-30,2.5f);glVertex3f(3,-30,2.5f);
    glVertex3f(1.5f,0,2.5f);glVertex3f(-1.5f,0,2.5f);
    glEnd();
    glBegin(GL_QUADS);
    glVertex3f(-3,-30,-2.5f);glVertex3f(3,-30,-2.5f);
    glVertex3f(1.5f,0,-2.5f);glVertex3f(-1.5f,0,-2.5f);
    glEnd();
    // Side edges
    glColor3f(0.4f,0.85f,1.0f);
    glBegin(GL_QUADS);
    glVertex3f(3,-30,2.5f);glVertex3f(3,-30,-2.5f);
    glVertex3f(1.5f,0,-2.5f);glVertex3f(1.5f,0,2.5f);
    glEnd();
    glBegin(GL_QUADS);
    glVertex3f(-3,-30,2.5f);glVertex3f(-3,-30,-2.5f);
    glVertex3f(-1.5f,0,-2.5f);glVertex3f(-1.5f,0,2.5f);
    glEnd();
    // Tip
    glColor3f(1,1,1);
    glBegin(GL_TRIANGLES);
    glVertex3f(0,-34,0);
    glVertex3f(-3,-30,2);glVertex3f(3,-30,2);
    glEnd();
    // Glow aura
    glColor4f(0.3f,0.9f,1.0f,0.35f);
    glBegin(GL_QUADS);
    glVertex3f(-8,-34,0);glVertex3f(8,-34,0);
    glVertex3f(5,0,0);glVertex3f(-5,0,0);
    glEnd();

    glPopMatrix();
}

// ── Player ───────────────────────────────────────────────────────────────────

Player::Player(){reset();}

void Player::reset(){
    x=400;y=300;speed=220;
    hp=maxHp=100;mana=maxMana=80;
    radius=18;alive=true;
    manaRegen=8;manaAccum=0;
    moveUp=moveDown=moveLeft=moveRight=false;
    walkTimer=0;faceAngle=180;hurtFlash=0;
    shootRecoil=0;
    hasSword=false;swordCharges=0;
    swordRechargeTimer=0;
    swingTimer=0;isSwinging=false;swingAngle=0;
}

void Player::update(float dt,float worldW,float worldH){
    if(!alive)return;
    bool moving=false;
    if(moveUp){y+=speed*dt;moving=true;}
    if(moveDown){y-=speed*dt;moving=true;}
    if(moveLeft){x-=speed*dt;moving=true;}
    if(moveRight){x+=speed*dt;moving=true;}
    if(x<radius)x=radius;if(x>worldW-radius)x=worldW-radius;
    if(y<radius)y=radius;if(y>worldH-radius)y=worldH-radius;

    if(moving)walkTimer+=dt*7.0f;
    if(hurtFlash>0)hurtFlash-=dt;
    if(shootRecoil>0)shootRecoil-=dt*8.0f;
    if(shootRecoil<0)shootRecoil=0;

    // Sword recharge
    if(hasSword&&swordCharges<MAX_CHARGES){
        swordRechargeTimer+=dt;
        if(swordRechargeTimer>=SWORD_RECHARGE_TIME){
            swordCharges++;
            swordRechargeTimer=0;
        }
    }

    // Swing animation
    if(isSwinging){
        swingTimer-=dt;
        // Sweep angle from -80 to +80 over swing duration
        float t=1.0f-(swingTimer/0.50f);
        swingAngle=-80.0f+160.0f*t;
        if(swingTimer<=0){isSwinging=false;swingAngle=0;}
    }

    manaAccum+=manaRegen*dt;
    if(manaAccum>=1.f){int g=(int)manaAccum;mana+=g;if(mana>maxMana)mana=maxMana;manaAccum-=g;}
}

void Player::draw(){
    if(!alive)return;

    glPushMatrix();
    glTranslatef(x,0,-y);
    glRotatef(faceAngle,0,1,0);

    bool flash=(hurtFlash>0);
    float tR=0.18f,tG=0.38f,tB=0.82f;
    float sR=0.97f,sG=0.82f,sB=0.42f;
    float lR=0.14f,lG=0.16f,lB=0.22f;
    if(flash){tR=tG=tB=sR=sG=sB=lR=lG=lB=1;}

    float legSwing=sinf(walkTimer)*22.0f;

    // Shadow
    glDepthMask(GL_FALSE);
    glColor4f(0,0,0,0.28f);
    glBegin(GL_TRIANGLE_FAN);glVertex3f(0,0.2f,0);
    for(int i=0;i<=20;i++){float a=i*2*PP/20;glVertex3f(cosf(a)*16,0.2f,sinf(a)*10);}
    glEnd();
    glDepthMask(GL_TRUE);

    // Left leg
    glPushMatrix();glTranslatef(-8,18,0);glRotatef(-legSwing,1,0,0);
    glTranslatef(0,-9,0);box(12,18,11,lR,lG,lB);glPopMatrix();
    // Right leg
    glPushMatrix();glTranslatef(8,18,0);glRotatef(legSwing,1,0,0);
    glTranslatef(0,-9,0);box(12,18,11,lR,lG,lB);glPopMatrix();

    // Torso
    glPushMatrix();glTranslatef(0,34,0);
    box(28,22,16,tR,tG,tB);
    // Belt
    glColor3f(0.08f,0.08f,0.12f);
    glBegin(GL_QUADS);
    glVertex3f(-14,-1,8.2f);glVertex3f(14,-1,8.2f);
    glVertex3f(14,1,8.2f);glVertex3f(-14,1,8.2f);
    glEnd();
    glPopMatrix();

    // Left arm (idle swing opposite leg)
    glPushMatrix();glTranslatef(-18,38,0);
    glRotatef(legSwing*0.4f,1,0,0);
    box(9,16,9,tR,tG,tB);
    glTranslatef(0,-13,0);box(9,8,9,sR,sG,sB);
    glPopMatrix();

    // Right arm: either gun or special sword
    glPushMatrix();
    glTranslatef(18,38,0);

    if(isSwinging){
        // Swing arm rotates around Y axis: sweep wide arc
        glRotatef(swingAngle,0,1,0);
        // Arm itself swings forward
        glRotatef(-50,1,0,0);
        box(9,16,9,tR,tG,tB);
        glTranslatef(0,-13,0);box(9,8,9,sR,sG,sB);
        glTranslatef(0,-8,0);
        // Special sword during swing
        drawSpecialSword();

        // Swing trail arc (transparent glowing strip)
        glDepthMask(GL_FALSE);
        glPushMatrix();
        glTranslatef(0,-40,0);
        float trailAlpha=(swingTimer/0.50f)*0.6f;
        // Draw arc segments behind current angle
        for(int seg=0;seg<8;seg++){
            float segAngle=swingAngle-(seg+1)*8.0f;
            float sa=trailAlpha*(1.0f-seg/8.0f);
            glColor4f(0.3f,0.9f,1.0f,sa);
            // Transform trail segment
            glPushMatrix();
            glRotatef(segAngle-swingAngle,0,1,0);
            glBegin(GL_QUADS);
            glVertex3f(-6,0,4); glVertex3f(6,0,4);
            glVertex3f(6,28,4); glVertex3f(-6,28,4);
            glEnd();
            glPopMatrix();
        }
        glPopMatrix();
        glDepthMask(GL_TRUE);

    } else {
        // Normal: gun arm with recoil
        float shootAng=legSwing*(-0.35f);
        float recoilAng=shootRecoil*(-30.0f);
        glRotatef(shootAng+recoilAng,1,0,0);
        box(9,16,9,tR,tG,tB);
        glTranslatef(0,-13,0);box(9,8,9,sR,sG,sB);
        glTranslatef(0,-4,0);
        // Hold gun
        glRotatef(-90,1,0,0);  // point gun forward
        drawGun(shootRecoil/0.12f,hasSword);

        // If has sword but not swinging: sword on back/left side (visual only)
        if(hasSword){
            glPopMatrix(); // exit right arm, re-enter to draw back sword
            glPushMatrix();
            glTranslatef(-5,46,-10);
            glRotatef(25,1,0,0);
            glRotatef(8,0,1,0);
            drawSpecialSword();
            glPushMatrix();// done
            glPopMatrix();
            glPushMatrix();// dummy to balance
            glPopMatrix();
            // NOTE: extra pushmatrix to keep stack balanced handled below
        }
    }
    glPopMatrix(); // right arm end

    // Head
    glPushMatrix();glTranslatef(0,57,0);
    box(24,22,22,sR,sG,sB);
    // Eyes
    glPushMatrix();glTranslatef(0,2,11.5f);
    box(4,4,1,0.06f,0.04f,0.06f);
    glTranslatef(-8,0,0);box(4,4,1,0.06f,0.04f,0.06f);
    glTranslatef(16,0,0);box(4,4,1,0.06f,0.04f,0.06f);
    glPopMatrix();
    // Smile
    glPushMatrix();glTranslatef(0,-4,11.5f);box(10,2,1,0.2f,0.1f,0.05f);glPopMatrix();
    // Stud
    glPushMatrix();glTranslatef(0,11,0);cyl(5,7,12,sR*.9f,sG*.9f,sB*.8f);glPopMatrix();
    glPopMatrix(); // head

    glPopMatrix(); // character
}

void Player::takeDamage(int dmg){hp-=dmg;hurtFlash=0.18f;if(hp<=0){hp=0;alive=false;}}
bool Player::useMana(int a){if(mana<a)return false;mana-=a;return true;}
