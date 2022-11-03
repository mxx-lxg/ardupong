#include <Arduino.h>

#include <font4x6.h>
#include <font6x8.h>
#include <font8x8.h>
#include <font8x8ext.h>
#include <fontALL.h>

#include <TVout.h>

#define BALL_SIZE 2
#define Y_A A0
#define Y_B A1
#define PADDLE_SIZE 20
#define PUSH_A 13
#define PUSH_B 12

#define X_RES 120
#define Y_RES 96

#define FIELD_SIZE_X 100  
#define FIELD_SIZE_Y 80
#define FIELD_OFFSET_X (X_RES - FIELD_SIZE_X)/2
#define FIELD_OFFSET_Y (Y_RES - FIELD_SIZE_Y)/2

#define Y_CENTER FIELD_SIZE_Y/2 + FIELD_OFFSET_Y

TVout TV;

float ySpeed = 1;
float xSpeed = 1;
float yPosition = Y_CENTER;

int paddleAPos = Y_CENTER - PADDLE_SIZE/2;
int paddleBPos = Y_CENTER - PADDLE_SIZE/2;

int scoreA = 0;
int scoreB = 0;

void drawDot(int x, int y){
  TV.set_pixel(x, y, WHITE);
  //TV.print(5, 90, xSpeed);
}

void drawField(){ //xSize 100, ySize 80
  TV.draw_line(FIELD_OFFSET_X,FIELD_OFFSET_Y,FIELD_SIZE_X+FIELD_OFFSET_X,FIELD_OFFSET_Y, WHITE); //105 x Ende
  TV.draw_line(FIELD_OFFSET_X,FIELD_SIZE_Y+FIELD_OFFSET_Y,FIELD_SIZE_X+FIELD_OFFSET_X,FIELD_SIZE_Y+FIELD_OFFSET_Y, WHITE); //90 y second line pos

  TV.draw_line(FIELD_SIZE_X/2+FIELD_OFFSET_X,FIELD_OFFSET_Y-5,FIELD_SIZE_X/2+FIELD_OFFSET_X,FIELD_SIZE_Y+FIELD_OFFSET_Y+5, WHITE);
}

int mapPaddlePos(int paddlePos){
  int mappedPos = map(paddlePos,0,100,FIELD_OFFSET_Y,FIELD_SIZE_Y+FIELD_OFFSET_Y);
  if(mappedPos > (FIELD_SIZE_Y+FIELD_OFFSET_Y-PADDLE_SIZE)) mappedPos = (FIELD_SIZE_Y+FIELD_OFFSET_Y-PADDLE_SIZE);
  if(mappedPos < 0) mappedPos = 0;
  return mappedPos;
}

void drawPaddleA(int dir){
  paddleAPos = paddleAPos + dir;
  if(paddleAPos < 0) paddleAPos = 0;
  if(paddleAPos > 100) paddleAPos = 100;
  int mappedPos = mapPaddlePos(paddleAPos);
  TV.draw_line(FIELD_OFFSET_X-1,mappedPos,FIELD_OFFSET_X-1,mappedPos+PADDLE_SIZE, WHITE);
}

void drawPaddleB(int dir){
  paddleBPos = paddleBPos + dir;
  if(paddleBPos < 0) paddleBPos = 0;
  if(paddleBPos > 100) paddleBPos = 100;
  int mappedPos = mapPaddlePos(paddleBPos);
  TV.draw_line(FIELD_SIZE_X+FIELD_OFFSET_X,mappedPos,FIELD_SIZE_X+FIELD_OFFSET_X,mappedPos+PADDLE_SIZE, WHITE);
}

int randomDirection(){
  int val = random(0, 10);
  if(val<=5) val = -1;
  else val = 1;
  return val;
}

void printScore(){
  TV.select_font(font6x8);
  TV.print(X_RES/3, 0, scoreA);
  TV.print((X_RES/3)*2, 0, scoreB);
}

int processStick(uint8_t input){
    int raw = analogRead(input);
    int y = map(raw,0,960,-3,3);
    return y;
}

void drawFrame(int i){
  TV.clear_screen();
  drawPaddleA(processStick(Y_A));
  drawPaddleB(processStick(Y_B));
  drawField();
  printScore();
  drawDot(i,yPosition);
}

void resetStartA(){
  ySpeed = randomDirection();
  xSpeed = 1;
  yPosition = Y_CENTER;
  int buttonState = 0;
  while(1){
    buttonState = digitalRead(PUSH_A);
    if (buttonState == LOW) {
      //Serial.println("press a");
      return;
    }
  }
}

void resetStartB(){
  ySpeed = randomDirection();
  xSpeed = 1;
  yPosition = Y_CENTER;
  int buttonState = 0;
  while(1){
    buttonState = digitalRead(PUSH_B);
    if (buttonState == LOW) {
      //Serial.println("press b");
      return;
    }
  }
}

void checkWinner(){
  TV.select_font(font6x8);
  if(scoreA >= 10){
    drawFrame(FIELD_OFFSET_X);
    TV.print(20, 45, "left wins!");
    scoreA=0;
    scoreB=0;
  }
  else if(scoreB >= 10){
    drawFrame(FIELD_OFFSET_X);
    TV.print(20, 45, "right wins!");
    scoreA=0;
    scoreB=0;
  }
}

void drawIntro(){
  for(int i = 0; i <= 25; i++){
    TV.clear_screen();
    TV.draw_rect(
      map(i,0,25,X_RES/2,20),
      map(i,0,25,Y_RES/2,30),
      map(i,0,25,0,X_RES/2+20), 
      map(i,0,25,0,Y_RES/2-20), 
      WHITE
    );
    TV.delay(1);
  }

  TV.select_font(font8x8);
  TV.print(28, 40, "ArduPong");
  TV.select_font(font4x6);
  TV.print(38, 50, "by mxx-lxg");  
  TV.print(26, 80, "press right stick");

  for(int i=0; i <=3; i++){
    TV.set_pixel((X_RES/3)*i,0,WHITE);
  }

  int buttonState = 0;
  while(1){
    buttonState = digitalRead(PUSH_B);
    if (buttonState == LOW) {
      return;
    }
  }
}

void detectYBounce(int yPosition){
    if(yPosition<=FIELD_OFFSET_Y ||yPosition>=FIELD_SIZE_Y+FIELD_OFFSET_Y){
      ySpeed = -ySpeed;
    }
}

void setup() {
  TV.begin(PAL, X_RES, Y_RES);
  Serial.begin(9600);
  randomSeed(analogRead(5));
  pinMode(PUSH_A, INPUT_PULLUP);
  pinMode(PUSH_B, INPUT_PULLUP);

  drawIntro();

  drawFrame(FIELD_OFFSET_X);
  resetStartA();
}

void loop() {  

  int mappedPosA = mapPaddlePos(paddleAPos);
  if(!(yPosition > mappedPosA && yPosition < mappedPosA+PADDLE_SIZE)){
    scoreB++;
    checkWinner();
    resetStartA();
  } else {
    ySpeed += ((yPosition-mappedPosA)-(PADDLE_SIZE/2))/10;
  }
  for(float i = FIELD_OFFSET_X; i <= FIELD_SIZE_X+FIELD_OFFSET_X; i+=xSpeed){
    detectYBounce(yPosition);
    yPosition = yPosition+ySpeed;
    drawFrame(i);
    TV.delay(5);
  }

  int mappedPosB = mapPaddlePos(paddleBPos);
  if(!(yPosition > mappedPosB && yPosition < mappedPosB+PADDLE_SIZE)){
    scoreA++;
    checkWinner();
    resetStartB();
  } else {
    ySpeed += ((yPosition-mappedPosB)-(PADDLE_SIZE/2))/10;
  }
  for(float i = FIELD_SIZE_X+FIELD_OFFSET_X; i >= FIELD_OFFSET_X; i-=xSpeed){
    detectYBounce(yPosition);
    yPosition = yPosition+ySpeed;
    drawFrame(i);
    TV.delay(5);
  }

  xSpeed+=0.1;
}
