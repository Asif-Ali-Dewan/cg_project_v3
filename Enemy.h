#ifndef ENEMY_H
#define ENEMY_H

class Enemy {
public:
    float x, y;
    float speed;
    int   hp, maxHp;
    float radius;
    bool  alive;
    int   damage;
    float attackCooldown;
    float attackTimer;
    int   type;
    float flashTimer;
    float floatTimer;
    float wispPhase;
    float faceAngle;
    float hoverY;

    Enemy();
    Enemy(float x,float y,int type);
    void update(float dt,float px,float py);
    void draw();
    void takeDamage(int dmg);
    bool overlapsCircle(float cx,float cy,float cr)const;
    bool canAttack()const{return attackTimer<=0;}
    void resetAttack(){attackTimer=attackCooldown;}
};
#endif
