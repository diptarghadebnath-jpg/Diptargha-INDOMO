#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
Adafruit_ST7735 tft(10,9,8);
const uint8_t BUTTON=2;
const uint16_t SKY=0x6E7F, GREEN=0x07E0, DARK=0x0320;
uint16_t score=0;
int16_t birdY=65, velocity=0, pipeX=128, gapY=60;
bool playing=false, passed=false, rawLast=true, stable=true;
uint32_t edgeTime=0, tick=0;
uint16_t bg(){return score>=100 ? ST77XX_BLACK : SKY;}
void hud(){
 tft.fillRect(0,0,128,16,bg());
 const uint16_t colors[]={ST77XX_WHITE,ST77XX_YELLOW,ST77XX_MAGENTA,ST77XX_CYAN,0xFC00};
 tft.setTextSize(1); tft.setTextColor(colors[score%5]);
 tft.setCursor(2,4); tft.print(F("SCORE ")); tft.print(score);
}
void scenery(){
 tft.fillRect(0,16,128,98,bg());
 if(score>=100){
  tft.fillCircle(110,28,9,ST77XX_WHITE);
  tft.fillCircle(114,25,8,ST77XX_BLACK);
  tft.drawPixel(80,22,ST77XX_WHITE); tft.drawPixel(94,36,ST77XX_WHITE);
 }else{
  tft.fillCircle(110,28,9,ST77XX_YELLOW);
 }
 tft.fillRect(0,114,128,14,0xBDA6);
 tft.fillRect(0,114,128,3,GREEN);
}
void pipe(){
 int x=max(0,pipeX), right=min(128,pipeX+19);
 if(right<=x)return;
 int w=right-x;
 tft.fillRect(x,16,w,gapY-22-16,GREEN);
 tft.fillRect(x,gapY+22,w,114-gapY-22,GREEN);
 tft.drawFastVLine(x,16,gapY-38,DARK);
 tft.drawFastVLine(x,gapY+22,92-gapY,DARK);
}
void bird(){
 tft.fillCircle(30,birdY,5,ST77XX_YELLOW);
 tft.fillCircle(28,birdY+2,3,0xFC00);
 tft.fillCircle(32,birdY-2,2,ST77XX_WHITE);
 tft.drawPixel(33,birdY-2,ST77XX_BLACK);
 tft.fillRect(34,birdY,4,2,ST77XX_RED);
}
void message(bool ended){
 tft.fillRect(8,48,112,34,ST77XX_BLACK);
 tft.setTextColor(ST77XX_WHITE); tft.setCursor(18,53);
 tft.print(ended ? F("GAME OVER") : F("FLAPPY BIRD"));
 tft.setCursor(16,69); tft.print(F("Press to play"));
}
void start(){
 score=0; birdY=65; velocity=-28; pipeX=128;
 gapY=random(42,88); passed=false; playing=true;
 scenery(); hud(); bird(); tick=millis();
}
void setup(){
 pinMode(BUTTON,INPUT_PULLUP);
 pinMode(4,OUTPUT); digitalWrite(4,HIGH); // unused SD socket deselected
 delay(150); tft.initR(INITR_144GREENTAB); tft.setRotation(0);
 tft.setTextWrap(false); tft.fillScreen(SKY); scenery(); hud(); message(false);
}
void loop(){
 uint32_t now=millis(); bool raw=digitalRead(BUTTON);
 if(raw!=rawLast){rawLast=raw; edgeTime=now;}
 if(now-edgeTime>=20 && raw!=stable){
  stable=raw;
  if(!stable){if(!playing){randomSeed(micros());start();}else velocity=-28;}
 }
 if(!playing || now-tick<40)return;
 tick=now;
 // Restore only moving objects; never repaint a static full screen each frame.
 tft.fillRect(24,birdY-6,15,13,bg());
 int oldX=max(0,pipeX), oldRight=min(128,pipeX+19);
 if(oldRight>oldX){
  tft.fillRect(oldX,16,oldRight-oldX,gapY-38,bg());
  tft.fillRect(oldX,gapY+22,oldRight-oldX,92-gapY,bg());
 }
 velocity=min((int)velocity+3,36); birdY+=velocity/8;
 pipeX-=2;
 if(!passed && pipeX+19<25){passed=true; if(score<65535)score++; if(score==100)scenery();hud();}
 if(pipeX<-19){pipeX=128;gapY=random(42,88);passed=false;}
 // Rebuild the small sun/moon region after pipe erasure.
 tft.fillRect(100,18,20,20,bg());
 tft.fillCircle(110,28,9,score>=100 ? ST77XX_WHITE:ST77XX_YELLOW);
 if(score>=100)tft.fillCircle(114,25,8,ST77XX_BLACK);
 bool crash=birdY<22 || birdY>108 ||
  (pipeX<38 && pipeX+19>25 && (birdY-5<gapY-22 || birdY+5>gapY+22));
 if(crash){birdY=constrain(birdY,22,108);}
 pipe();bird();
 if(crash){playing=false;message(true);}
}
