#include "Abilities.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glut.h>
#include <cmath>

static const float AP=3.14159265f;

Abilities::Abilities(){reset();}
void Abilities::reset(){
    projectiles.clear();cleaveFXs.clear();
    shootCooldown=0.22f;shootTimer=0;shootManaCost=8;shootDamage=18;
    aoeCooldown=5.0f;aoeTimer2=0;aoeManaCost=40;aoeDamage=55;
    cleaveDamage=60;
}

void Abilities::update(float dt){
    if(shootTimer>0)shootTimer-=dt;
    if(aoeTimer2>0) aoeTimer2-=dt;
    for(auto&p:projectiles){
        if(!p.active)continue;p.age+=dt;
        if(p.isAOE){p.aoeTimer-=dt;p.aoeCurRadius+=p.aoeMaxRadius/p.aoeDuration*dt;if(p.aoeTimer<=0)p.active=false;}
        else{p.x+=p.vx*dt;p.y+=p.vy*dt;p.lifetime-=dt;if(p.lifetime<=0)p.active=false;}
    }
    for(int i=(int)projectiles.size()-1;i>=0;--i)
        if(!projectiles[i].active)projectiles.erase(projectiles.begin()+i);
    for(auto&c:cleaveFXs){if(c.active)c.timer-=dt;if(c.timer<=0)c.active=false;}
    for(int i=(int)cleaveFXs.size()-1;i>=0;--i)
        if(!cleaveFXs[i].active)cleaveFXs.erase(cleaveFXs.begin()+i);
}

bool Abilities::shoot(float ox,float oy,float tx,float ty){
    if(shootTimer>0)return false;
    float dx=tx-ox,dy=ty-oy,d=sqrtf(dx*dx+dy*dy);
    if(d<0.5f)return false;
    Projectile p{};
    p.x=ox;p.y=oy;
    float spd=620;
    p.vx=(dx/d)*spd;p.vy=(dy/d)*spd;
    p.radius=5;p.damage=shootDamage;p.active=true;
    p.lifetime=1.4f;p.age=0;p.isAOE=false;
    projectiles.push_back(p);
    shootTimer=shootCooldown;
    return true;
}

bool Abilities::castAOE(float ox,float oy){
    if(aoeTimer2>0)return false;
    Projectile p{};
    p.x=ox;p.y=oy;p.damage=aoeDamage;p.active=true;p.age=0;p.isAOE=true;
    p.aoeMaxRadius=140;p.aoeCurRadius=0;p.aoeDuration=.45f;p.aoeTimer=p.aoeDuration;
    projectiles.push_back(p);aoeTimer2=aoeCooldown;return true;
}

void Abilities::spawnCleaveFX(float ox,float oy,float angle){
    CleaveFX c;c.x=ox;c.y=oy;c.angle=angle;c.timer=c.maxTimer=0.55f;c.active=true;
    cleaveFXs.push_back(c);
}

float Abilities::shootCDFrac()const{float f=1-shootTimer/shootCooldown;return f<0?0:(f>1?1:f);}
float Abilities::aoeCDFrac()  const{float f=1-aoeTimer2/aoeCooldown;  return f<0?0:(f>1?1:f);}

// ── draw ─────────────────────────────────────────────────────────────────────
void Abilities::draw(){
    glDepthMask(GL_FALSE);

    // ── Bullets ──────────────────────────────────────────────────────
    for(auto&p:projectiles){
        if(!p.active)continue;
        if(p.isAOE){
            float pct=p.aoeTimer/p.aoeDuration,r=p.aoeCurRadius;
            // Ground disc
            glColor4f(1,.65f,.1f,pct*.18f);
            glBegin(GL_TRIANGLE_FAN);glVertex3f(p.x,1,-p.y);
            for(int i=0;i<=30;i++){float a=i*2*AP/30;
                glVertex3f(p.x+cosf(a)*r,1,-p.y+sinf(a)*r);}
            glEnd();
            // Ring
            for(float s=1.f;s>=.5f;s-=.5f){
                glColor4f(1,.85f,.25f,pct*(1.6f-s));glLineWidth(s*6);
                glBegin(GL_LINE_LOOP);
                for(int i=0;i<30;i++){float a=i*2*AP/30;
                    glVertex3f(p.x+cosf(a)*r,2,-p.y+sinf(a)*r);}
                glEnd();
            }
            // Vertical spikes
            glLineWidth(2);
            for(int i=0;i<12;i++){float a=i*2*AP/12+p.age*3;
                float sx=p.x+cosf(a)*r,sy=-p.y+sinf(a)*r,ht=30*pct;
                glColor4f(1,.8f,.1f,pct*.7f);
                glBegin(GL_LINES);glVertex3f(sx,0,sy);glVertex3f(sx,ht,sy);glEnd();}
        } else {
            // Bullet – small fast glowing tracer
            float fh=30.0f;
            float speed=sqrtf(p.vx*p.vx+p.vy*p.vy);
            float ndx=-p.vx/speed*0.9f,ndy=-p.vy/speed*0.9f;

            // Long bright tracer trail
            float tlen=55.0f;
            glLineWidth(3.5f);
            glBegin(GL_LINES);
            glColor4f(1.0f,0.95f,0.6f, 0.9f);
            glVertex3f(p.x,fh,-p.y);
            glColor4f(1.0f,0.55f,0.05f, 0.0f);
            glVertex3f(p.x+ndx*tlen,fh,-p.y+ndy*tlen);
            glEnd();
            glLineWidth(1.5f);

            // Bright bullet core
            glColor4f(1.0f,1.0f,0.8f, 0.95f);
            glBegin(GL_TRIANGLE_FAN);glVertex3f(p.x,fh,-p.y);
            for(int i=0;i<=10;i++){float a=i*2*AP/10;
                glVertex3f(p.x+cosf(a)*p.radius,fh+sinf(a)*p.radius,-p.y);}
            glEnd();
            // Glow
            glColor4f(1.0f,0.6f,0.1f,0.30f);
            glBegin(GL_TRIANGLE_FAN);glVertex3f(p.x,fh,-p.y);
            for(int i=0;i<=10;i++){float a=i*2*AP/10;
                glVertex3f(p.x+cosf(a)*p.radius*2.8f,fh+sinf(a)*p.radius*2.8f,-p.y);}
            glEnd();
        }
    }

    // ── Cleave arc FX ─────────────────────────────────────────────────
    for(auto&c:cleaveFXs){
        if(!c.active)continue;
        float t=c.timer/c.maxTimer;   // 1→0
        float alpha=t;
        float arcR=110.0f*(1.0f-t*0.3f);
        float arcAngle=100.0f;         // degrees wide
        float baseA=(c.angle-arcAngle*0.5f)*AP/180.0f;
        int segs=24;

        glPushMatrix();
        glTranslatef(c.x,5,-c.y);
        glRotatef(0,0,1,0);

        // Filled swept arc
        glColor4f(0.35f,0.95f,1.0f, alpha*0.32f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0,0,0);
        for(int i=0;i<=segs;i++){
            float a=baseA+i*(arcAngle*AP/180.0f)/segs;
            glVertex3f(cosf(a)*arcR,0,sinf(a)*arcR);
        }
        glEnd();

        // Bright outer edge
        glColor4f(0.6f,1.0f,1.0f, alpha*0.85f);
        glLineWidth(4.0f*(t));
        glBegin(GL_LINE_STRIP);
        for(int i=0;i<=segs;i++){
            float a=baseA+i*(arcAngle*AP/180.0f)/segs;
            glVertex3f(cosf(a)*arcR,0,sinf(a)*arcR);
        }
        glEnd();

        // Inner edge
        glColor4f(1.0f,1.0f,1.0f, alpha*0.5f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        for(int i=0;i<=segs;i++){
            float a=baseA+i*(arcAngle*AP/180.0f)/segs;
            glVertex3f(cosf(a)*arcR*0.6f,0,sinf(a)*arcR*0.6f);
        }
        glEnd();

        // Radial streak lines
        glLineWidth(2.0f);
        for(int i=0;i<=6;i++){
            float a=baseA+i*(arcAngle*AP/180.0f)/6;
            glColor4f(0.5f,1.0f,1.0f, alpha*0.55f);
            glBegin(GL_LINES);
            glVertex3f(cosf(a)*arcR*0.5f,0,sinf(a)*arcR*0.5f);
            glVertex3f(cosf(a)*arcR,0,sinf(a)*arcR);
            glEnd();
        }

        // Sparkle dots along arc edge
        glColor4f(1,1,1,alpha*0.9f);
        glPointSize(5);
        glBegin(GL_POINTS);
        for(int i=0;i<=8;i++){
            float a=baseA+i*(arcAngle*AP/180.0f)/8;
            glVertex3f(cosf(a)*arcR,2,sinf(a)*arcR);
        }
        glEnd();
        glPointSize(1);
        glLineWidth(1);
        glPopMatrix();
    }

    glDepthMask(GL_TRUE);
}
