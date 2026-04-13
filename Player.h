#ifndef PLAYER_H
#define PLAYER_H

class Player {
public:
    float x, y;
    float speed;
    int   hp, maxHp;
    int   mana, maxMana;
    float radius;
    bool  alive;
    float manaRegen;
    float manaAccum;

    bool moveUp, moveDown, moveLeft, moveRight;

    // 3D animation
    float walkTimer;
    float faceAngle;     // Y-rotation degrees (toward mouse)
    float hurtFlash;

    // Gun fire animation
    float shootRecoil;   // countdown, tilts gun arm back

    // Special sword (picked up after wave 2)
    bool  hasSword;
    int   swordCharges;
    static const int MAX_CHARGES = 3;
    float swordRechargeTimer;
    static constexpr float SWORD_RECHARGE_TIME = 8.0f;

    // Cleave swing animation
    float swingTimer;      // countdown while swinging
    float swingAngle;      // current arc angle (visual)
    bool  isSwinging;

    Player();
    void reset();
    void update(float dt, float worldW, float worldH);
    void draw();
    void takeDamage(int dmg);
    bool useMana(int amount);
    void startShootAnim()  { shootRecoil = 0.12f; }
    void startCleaveSwing(){ swingTimer = 0.50f; isSwinging = true; swingAngle = -80.0f; }
};

#endif
