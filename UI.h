#ifndef UI_H
#define UI_H

class UI {
public:
    int windowW,windowH;
    UI(int w=800,int h=600);

    void drawHUD(int hp,int maxHp,int mana,int maxMana,
                 int score,int wave,
                 float shootCDFrac,float aoeCDFrac,bool paused,
                 bool hasSword,int swordCharges,int maxCharges,
                 float rechargeTimer,float rechargeTime);
    void drawMenu(int highScore);
    void drawGameOver(int score,int wave,int highScore);
    void drawWaveBanner(int wave,float alpha);
    void drawSwordHint();

private:
    void drawBar(float x,float y,float w,float h,float frac,
                 float fr,float fg,float fb,float br,float bg,float bb);
    void drawText(float x,float y,const char*t,float r,float g,float b,void*font=0);
    void drawTextA(float x,float y,const char*t,float r,float g,float b,float a,void*font=0);
    void drawAbilityIcon(float x,float y,float sz,int type,float cd,const char*hk);
    void drawSwordIcon(float x,float y,float sz,int charges,int maxC,float rechargeFrac);
};
#endif
