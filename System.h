#ifndef SYSTEM_H
#define SYSTEM_H

#include "Player.h"
#include "Enemy.h"
#include "Abilities.h"
#include "UI.h"
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glu.h>

enum GameState{STATE_MENU,STATE_PLAYING,STATE_GAME_OVER};

// Glowing sword pickup item on the ground
struct SwordPickup {
    float x,y;
    float rotAngle;  // visual spin
    float bobTimer;
    bool  active;
};

class GameSystem {
public:
    static const int WORLD_W=800;
    static const int WORLD_H=600;

    Player    player;
    std::vector<Enemy> enemies;
    Abilities abilities;
    UI        ui;

    GameState state;
    int  score,wave,highScore;
    bool paused;

    float waveBannerTimer;
    float betweenWaveTimer;
    bool  waveClearPending;

    // Raw screen mouse
    int mouseRawX, mouseRawY;
    // Computed world-space aim point (on ground y=0)
    float mouseWorldX, mouseWorldY;

    // Matrices captured each frame for unprojection
    double mvMatrix[16], projMatrix[16];
    int    viewport[4];

    SwordPickup swordPickup;

    struct HitSpark{
        float x,y,vy3d,life,maxLife,r,g,b;
    };
    std::vector<HitSpark> sparks;

    GameSystem();
    void startGame();
    void update(float dt);
    void draw();

    void onKeyDown(unsigned char key);
    void onKeyUp(unsigned char key);
    void onMouseMove(int x,int y);
    void onMouseClick(int button,int st,int x,int y);

private:
    void spawnWave(int waveNum);
    void checkCollisions();
    void doCleave();
    void spawnSpark(float x,float y,float r,float g,float b);
    void updateSparks(float dt);
    void drawSparks();
    void drawArena();
    void setup3DCamera();
    void begin2D();
    void end2D();
    void unprojectMouse();    // updates mouseWorldX/Y from raw screen coords
    void updateSwordPickup(float dt);
    void drawSwordPickup();
};

extern GameSystem* g_game;
#endif
