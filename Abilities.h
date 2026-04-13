#ifndef ABILITIES_H
#define ABILITIES_H
#include <vector>

struct Projectile {
    float x,y,vx,vy,radius;
    int   damage;
    bool  active;
    float lifetime,age;
    bool  isAOE;
    float aoeCurRadius,aoeMaxRadius,aoeDuration,aoeTimer;
};

// Cleave arc visual effect (separate from projectiles)
struct CleaveFX {
    float x,y;        // origin (player pos)
    float angle;      // facing angle (degrees)
    float timer,maxTimer;
    bool  active;
};

class Abilities {
public:
    std::vector<Projectile> projectiles;
    std::vector<CleaveFX>   cleaveFXs;

    float shootCooldown,  shootTimer;
    int   shootManaCost,  shootDamage;
    float aoeCooldown,    aoeTimer2;
    int   aoeManaCost,    aoeDamage;
    int   cleaveDamage;

    Abilities();
    void reset();
    void update(float dt);
    void draw();

    bool shoot(float ox,float oy,float tx,float ty);
    bool castAOE(float ox,float oy);
    void spawnCleaveFX(float ox,float oy,float angle);

    bool  canShoot()const{return shootTimer<=0;}
    bool  canAOE()  const{return aoeTimer2<=0;}
    float shootCDFrac()const;
    float aoeCDFrac()  const;
};
#endif
