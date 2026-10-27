#include <math.h>
#include <Adafruit_PWMServoDriver.h>
#include <SoftwareSerial.h> 

// Bluetooth setup: RX on Pin 3, TX on Pin 2
SoftwareSerial BTSerial(3, 2); 
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

//-------------------------------------------------------PCA9685 SERVO VALUES----------------------------------------------------------
  #define SERVO_0_0 280   // LEFT FRONT hip servo 0       (Calibrated)
  #define SERVO_M90_0 160 // LEFT FRONT hip servo -90      

  #define SERVO_M90_1 500   // LEFT FRONT femur servo -90 
  #define SERVO_M180_1 300  // LEFT FRONT femur servo  -180 

  #define SERVO_90_2 120   // LEFT FRONT tibia servo  90   
  #define SERVO_180_2 330   // LEFT FRONT tibia servo  180  

  #define SERVO_0_3 280   // RIGHT FRONT hip servo 0       
  #define SERVO_M90_3 430   // RIGHT FRONT hip servo -90     

  #define SERVO_M90_4 130 // RIGHT FRONT femur servo -90     
  #define SERVO_M180_4 300  // RIGHT FRONT femur servo   -180     

  #define SERVO_90_5 500   // RIGHT FRONT tibia servo  90    
  #define SERVO_180_5 300   // RIGHT FRONT tibia servo  180     

  #define SERVO_0_6 300 //LEFT BACK hip servo 0               
  #define SERVO_M90_6 440   // LEFT BACK hip servo -90       

  #define SERVO_M90_7 500   // LEFT BACK femur servo -90 
  #define SERVO_M180_7 310  // LEFT BACK femur servo  -180 

  #define SERVO_90_8 100   // LEFT BACK tibia servo  90   
  #define SERVO_180_8 300   // LEFT BACK tibia servo  180  

  #define SERVO_0_9 330   // RIGHT BACK hip servo 0       
  #define SERVO_M90_9 150   // RIGHT BACK hip servo -90     

  #define SERVO_M90_10 100 // RIGHT BACK femur servo -90     
  #define SERVO_M180_10 300  // RIGHT BACK femur servo   -180     

  #define SERVO_90_11 500  // RIGHT BACK tibia servo  90    
  #define SERVO_180_11 300   // RIGHT BACK tibia servo  180     
//-------------------------------------------------------PCA9685 SERVO VALUES----------------------------------------------------------

bool StartPos = false; 

const float L1 = 1.5; // hip vertical
const float L2 = 5.7; // femur
const float L3 = 5.5; // tibia
const float L4 = 5;   // hip horizontal

int servo1, servo2, servo3;

float StartX = 4;
float StartY = -11; // Default Tall Standing Height
float StartZ = -1;

void setup() {
  Serial.begin(115200);
  BTSerial.begin(9600); 
  
  pwm.begin();
  pwm.setPWMFreq(50);

  // Snap to default standing position
  computeIK(StartX,StartY,StartZ,servo1,servo2,servo3,1);
  pwm.setPWM(0,0, servo1); pwm.setPWM(1, 0, servo2); pwm.setPWM(2,0, servo3);
  computeIK(StartX,StartY,StartZ,servo1,servo2,servo3,2);
  pwm.setPWM(3,0, servo1); pwm.setPWM(4, 0, servo2); pwm.setPWM(5,0, servo3);
  computeIK(StartX,StartY,StartZ,servo1,servo2,servo3,3);
  pwm.setPWM(6,0, servo1); pwm.setPWM(7, 0, servo2); pwm.setPWM(8,0, servo3);
  computeIK(StartX,StartY,StartZ,servo1,servo2,servo3,4);
  pwm.setPWM(9,0, servo1); pwm.setPWM(10, 0, servo2); pwm.setPWM(11,0, servo3);

  StartPos = true;
  delay(2000);
  Serial.println("Robot Ready. Waiting for commands...");
}

void loop() {
  if (BTSerial.available()) {
    char command = BTSerial.read();
    
    // Ignore invisible characters
    if (command == '\n' || command == '\r' || command == ' ') return;
    
    Serial.print("Executing: ");
    Serial.println(command);
    
    // Walk Commands
    if (command == 'F') TrotForward(10, 1);
    else if (command == 'B') TrotBackward(10, 1);
    else if (command == 'R') TurnRight(10, 1);
    else if (command == 'L') TurnLeft(10, 1);
    
    // Gesture Commands
    else if (command == 'W') Handshake();
    else if (command == 'S') Sit();
    else if (command == 'P') Pushups();
  }
}

//====================================================================
// GESTURE ENGINE
//====================================================================

//----------------------------------------------------
// Handshake ('W') - DIRECT HARDWARE OVERRIDE
//----------------------------------------------------
void Handshake() {
  float Pos0[4][3] = { {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ} };
  float ShiftPos[4][3] = { 
    {StartX, StartY, StartZ + 2}, // LF Planted
    {StartX, StartY, StartZ},     // RF Getting ready
    {StartX, StartY, StartZ + 2}, // LB Planted
    {StartX, StartY, StartZ + 2}  // RB Planted
  };

  if(StartPos) StartPos = false; 
  interpolation(Pos0, ShiftPos, 10); 
  delay(200);

  int currentHip, currentFemur, currentTibia;
  computeIK(ShiftPos[1][0], ShiftPos[1][1], ShiftPos[1][2], currentHip, currentFemur, currentTibia, 2);

  // Target values to reach forward and outward
  int targetFemur = 180; 
  int targetTibia = 380; 
  int targetHip = 300;   

  // Smoothly lift the leg to the target using Direct Control
  for(int i = 0; i <= 20; i++) {
    int h = currentHip + ((targetHip - currentHip) * i / 20);
    int f = currentFemur + ((targetFemur - currentFemur) * i / 20);
    int t = currentTibia + ((targetTibia - currentTibia) * i / 20);
    pwm.setPWM(3, 0, h); 
    pwm.setPWM(4, 0, f); 
    pwm.setPWM(5, 0, t); 
    delay(15);
  }
  
  // The Wave 
  for(int w = 0; w < 3; w++) {
    for(int i = 0; i <= 10; i++) {
      pwm.setPWM(4, 0, targetFemur + (30 * i / 10)); 
      pwm.setPWM(5, 0, targetTibia + (20 * i / 10)); 
      delay(10);
    }
    for(int i = 0; i <= 10; i++) {
      pwm.setPWM(4, 0, (targetFemur + 30) - (30 * i / 10)); 
      pwm.setPWM(5, 0, (targetTibia + 20) - (20 * i / 10)); 
      delay(10);
    }
  }
  
  delay(300);

  // Smoothly lower the leg
  for(int i = 0; i <= 20; i++) {
    int h = targetHip + ((currentHip - targetHip) * i / 20);
    int f = targetFemur + ((currentFemur - targetFemur) * i / 20);
    int t = targetTibia + ((currentTibia - targetTibia) * i / 20);
    pwm.setPWM(3, 0, h);
    pwm.setPWM(4, 0, f);
    pwm.setPWM(5, 0, t);
    delay(15);
  }

  ReturnToStartPosition(ShiftPos);
  while(BTSerial.available()) BTSerial.read(); 
}

//----------------------------------------------------
// Sit Gesture ('S')
//----------------------------------------------------
void Sit() {
  int ST = 15; // Very slow to look natural
  float Pos0[4][3] = { {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ} };
  
  // Back legs crouch deep
  float SitPos[4][3] = { 
    {StartX, StartY, StartZ}, 
    {StartX, StartY, StartZ}, 
    {StartX, -4, 1},          
    {StartX, -4, 1}           
  };

  if(StartPos) StartPos = false; 

  interpolation(Pos0, SitPos, ST);
  delay(2000); 
  ReturnToStartPosition(SitPos);
  while(BTSerial.available()) BTSerial.read(); 
}

//----------------------------------------------------
// Pushups Gesture ('P')
//----------------------------------------------------
void Pushups() {
  int ST = 6; 
  float Pos0[4][3] = { {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ} };
  
  // All four legs bend deeply
  float DownPos[4][3] = { 
    {StartX, -5, StartZ}, 
    {StartX, -5, StartZ}, 
    {StartX, -5, StartZ}, 
    {StartX, -5, StartZ} 
  };

  if(StartPos) StartPos = false; 

  for(int i = 0; i < 3; i++) {
    interpolation(Pos0, DownPos, ST); delay(200); 
    interpolation(DownPos, Pos0, ST); delay(200); 
  }

  ReturnToStartPosition(Pos0);
  while(BTSerial.available()) BTSerial.read(); 
}

//====================================================================
// CONTINUOUS WALKING ENGINE
//====================================================================

//----------------------------------------------------
// TrotForward 
//----------------------------------------------------
void TrotForward(int t, int cycles) {  
  int X = 3; int BX = 4; int floorY = -12; float BY = -8; int ST = 4; 
  float AZ = -3, BZ = 2, CZ = -1, DZ = -1;

  float OldPos[4][3] = { {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ} };
  float NewPos[4][3] = { {X,floorY,CZ}, {X,floorY,AZ}, {X,floorY,AZ}, {X,floorY,CZ} };

  if(StartPos) { interpolation(OldPos,NewPos,ST); delay(t*100); StartPos = false; }
  
  for(int i = 0; i < cycles; i++) {
    float NewPos1[4][3]={{X,floorY,DZ}, {BX,BY,BZ}, {BX,BY,BZ}, {X,floorY,DZ}};
    interpolation(NewPos,NewPos1,ST); delay(t);

    float NewPos2[4][3]={{X,floorY,AZ}, {X,floorY,CZ}, {X,floorY,CZ}, {X,floorY,AZ}};
    interpolation(NewPos1,NewPos2,ST); delay(t*10);

    float NewPos3[4][3] = {{BX, BY, BZ}, {X, floorY, DZ}, {X, floorY, DZ}, {BX, BY, BZ}};
    interpolation(NewPos2,NewPos3,ST); delay(t);

    float NewPos4[4][3] = {{X, floorY, CZ}, {X, floorY, AZ}, {X, floorY, AZ}, {X, floorY, CZ}};
    interpolation(NewPos3, NewPos4, ST); delay(t*10);
  }
  
  while(BTSerial.available() && (BTSerial.peek() == '\n' || BTSerial.peek() == '\r' || BTSerial.peek() == ' ')) BTSerial.read();
  if(BTSerial.available() && BTSerial.peek() == 'F') return; 
  ReturnToStartPosition(NewPos);
}

//----------------------------------------------------
// TrotBackward
//----------------------------------------------------
void TrotBackward(int t, int cycles){
  float X = 4; int BX = 4; int floorY = -12; float BY = -8; int ST = 6; 
  float AZ = 1, BZ = -3, CZ = -2, DZ = -1;

  float OldPos[4][3] = { {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ} };
  float NewPos[4][3] = { {X,floorY,CZ}, {X,floorY,AZ}, {X,floorY,AZ}, {X,floorY,CZ} };

  if(StartPos) { interpolation(OldPos,NewPos,ST); delay(t*100); StartPos = false; } 
  
  for(int i = 0; i < cycles; i++) {
    float NewPos1[4][3]={{X,floorY,DZ}, {BX,BY,BZ}, {BX,BY,BZ}, {X,floorY,DZ}};
    interpolation(NewPos,NewPos1,ST); delay(t);

    float NewPos2[4][3]={{X,floorY,AZ}, {X,floorY,CZ}, {X,floorY,CZ}, {X,floorY,AZ}};
    interpolation(NewPos1,NewPos2,ST); delay(t*10);

    float NewPos3[4][3] = {{BX, BY, BZ}, {X, floorY, DZ}, {X, floorY, DZ}, {BX, BY, BZ}};
    interpolation(NewPos2,NewPos3,ST); delay(t);

    float NewPos4[4][3] = {{X, floorY, CZ}, {X, floorY, AZ}, {X, floorY, AZ}, {X, floorY, CZ}};
    interpolation(NewPos3, NewPos4, ST); delay(t*10);
  }      

  while(BTSerial.available() && (BTSerial.peek() == '\n' || BTSerial.peek() == '\r' || BTSerial.peek() == ' ')) BTSerial.read();
  if(BTSerial.available() && BTSerial.peek() == 'B') return;
  ReturnToStartPosition(NewPos);
}

//----------------------------------------------------
// TurnRight (Curve Trot)
//----------------------------------------------------
void TurnRight(int t, int cycles){  
  int X = 3; int BX = 4; int floorY = -12; float BY = -8; int ST = 4; 
  float F_AZ = -3, F_BZ = 2, F_CZ = -1, F_DZ = -1;
  float S_AZ = -1, S_BZ = 0, S_CZ = -0.5, S_DZ = -0.5;

  float OldPos[4][3] = { {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ} };
  float NewPos[4][3] = { {X,floorY,F_CZ}, {X,floorY,S_AZ}, {X,floorY,F_AZ}, {X,floorY,S_CZ} };

  if(StartPos) { interpolation(OldPos,NewPos,ST); delay(t*100); StartPos = false; }  
  
  for(int i = 0; i < cycles; i++) {
    float NewPos1[4][3]={{X,floorY,F_DZ}, {BX,BY,S_BZ}, {BX,BY,F_BZ}, {X,floorY,S_DZ}};
    interpolation(NewPos,NewPos1,ST); delay(t);

    float NewPos2[4][3]={{X,floorY,F_AZ}, {X,floorY,S_CZ}, {X,floorY,F_CZ}, {X,floorY,S_AZ}};
    interpolation(NewPos1,NewPos2,ST); delay(t*10);

    float NewPos3[4][3] = {{BX,BY,F_BZ}, {X,floorY,S_DZ}, {X,floorY,F_DZ}, {BX,BY,S_BZ}};
    interpolation(NewPos2,NewPos3,ST); delay(t);

    float NewPos4[4][3] = {{X,floorY,F_CZ}, {X,floorY,S_AZ}, {X,floorY,F_AZ}, {X,floorY,S_CZ}};
    interpolation(NewPos3, NewPos4, ST); delay(t*10);
  }
  
  while(BTSerial.available() && (BTSerial.peek() == '\n' || BTSerial.peek() == '\r' || BTSerial.peek() == ' ')) BTSerial.read();
  if(BTSerial.available() && BTSerial.peek() == 'R') return; 
  ReturnToStartPosition(NewPos);
}

//----------------------------------------------------
// TurnLeft (Curve Trot)
//----------------------------------------------------
void TurnLeft(int t, int cycles){  
  int X = 3; int BX = 4; int floorY = -12; float BY = -8; int ST = 4; 
  float S_AZ = -1, S_BZ = 0, S_CZ = -0.5, S_DZ = -0.5;
  float F_AZ = -3, F_BZ = 2, F_CZ = -1, F_DZ = -1;

  float OldPos[4][3] = { {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ}, {StartX,StartY,StartZ} };
  float NewPos[4][3] = { {X,floorY,S_CZ}, {X,floorY,F_AZ}, {X,floorY,S_AZ}, {X,floorY,F_CZ} };
  
  if(StartPos) { interpolation(OldPos,NewPos,ST); delay(t*100); StartPos = false; }  
  
  for(int i = 0; i < cycles; i++) {
    float NewPos1[4][3]={{X,floorY,S_DZ}, {BX,BY,F_BZ}, {BX,BY,S_BZ}, {X,floorY,F_DZ}};
    interpolation(NewPos,NewPos1,ST); delay(t);

    float NewPos2[4][3]={{X,floorY,S_AZ}, {X,floorY,F_CZ}, {X,floorY,S_CZ}, {X,floorY,F_AZ}};
    interpolation(NewPos1,NewPos2,ST); delay(t*10);
    
    float NewPos3[4][3] = {{BX,BY,S_BZ}, {X,floorY,F_DZ}, {X,floorY,S_DZ}, {BX,BY,F_BZ}};
    interpolation(NewPos2,NewPos3,ST); delay(t);

    float NewPos4[4][3] = {{X,floorY,S_CZ}, {X,floorY,F_AZ}, {X,floorY,S_AZ}, {X,floorY,F_CZ}};
    interpolation(NewPos3, NewPos4, ST); delay(t*10);
  }
  
  while(BTSerial.available() && (BTSerial.peek() == '\n' || BTSerial.peek() == '\r' || BTSerial.peek() == ' ')) BTSerial.read();
  if(BTSerial.available() && BTSerial.peek() == 'L') return;
  ReturnToStartPosition(NewPos);
}

//----------------------------------------------------
// IK and Helper Functions
//----------------------------------------------------
void computeIK(float footX, float footY, float footZ, int &servo1, int &servo2, int &servo3, int leg) {
  float theta1,theta2,theta3;
  theta1 = atan2(footY,footX)+acos(L4/sqrt(footX*footX+footY*footY));
  float A=sqrt(footX*footX+footY*footY-L4*L4);

  float k=(L2*L2+L3*L3-pow((L1-A),2)-footZ*footZ)/(2*L2*L3);
  k=constrain(k,-1,1);
  theta3=acos(k);

  float t=(L2*L2+pow((L1-A),2)+footZ*footZ-L3*L3)/(2*L2*sqrt(pow((L1-A),2)+footZ*footZ));
  t=constrain(t,-1,1);
  theta2 = atan2((L1-A),footZ)-acos(t);

  theta1=theta1*180/M_PI;
  theta2=theta2*180/M_PI;
  theta3=theta3*180/M_PI;

  if(leg==1){
    servo1=map(theta1, 0, -90,SERVO_0_0,SERVO_M90_0);
    servo2=map(theta2, -180, -90, SERVO_M180_1,SERVO_M90_1);
    servo3=map(theta3, 90, 180, SERVO_90_2,SERVO_180_2);
  }else if(leg==2){  
    servo1=map(theta1, 0, -90,SERVO_0_3,SERVO_M90_3);
    servo2=map(theta2,-180 , -90, SERVO_M180_4,SERVO_M90_4);
    servo3=map(theta3, 90, 180, SERVO_90_5,SERVO_180_5);
  }else if(leg==3){
    servo1=map(theta1, 0, -90,SERVO_0_6,SERVO_M90_6);
    servo2=map(theta2,-180 , -90, SERVO_M180_7,SERVO_M90_7);
    servo3=map(theta3, 90, 180, SERVO_90_8,SERVO_180_8);
  }else if(leg==4){
    servo1=map(theta1, 0, -90,SERVO_0_9,SERVO_M90_9);
    servo2=map(theta2,-180 , -90, SERVO_M180_10,SERVO_M90_10);
    servo3=map(theta3, 90, 180, SERVO_90_11,SERVO_180_11);
  }
}

void interpolation(float OldPos[4][3], float NewPos[4][3], int steps) {
  int servo1, servo2, servo3;
  float Increment[4][3];
  float CurrentPos[4][3]; 

  for(int i = 0; i < 4; i++) {
    for(int m = 0; m < 3; m++) {
      Increment[i][m] = (NewPos[i][m] - OldPos[i][m]) / steps;
      CurrentPos[i][m] = OldPos[i][m]; 
    }
  }

  for(int i = 0; i < steps; i++) {
    for(int m = 0; m < 4; m++) {
      computeIK(CurrentPos[m][0], CurrentPos[m][1], CurrentPos[m][2], servo1, servo2, servo3, m+1);
      
      pwm.setPWM(3*m,0,servo1);
      pwm.setPWM(3*m+1,0,servo2);  
      pwm.setPWM(3*m+2,0,servo3);

      CurrentPos[m][0] += Increment[m][0];
      CurrentPos[m][1] += Increment[m][1];
      CurrentPos[m][2] += Increment[m][2];
    }
    delay(10); 
  }
}

void ReturnToStartPosition(float OldPos[4][3]){
  float NewPos[4][3] = {
    {StartX, StartY, StartZ},
    {StartX, StartY, StartZ},
    {StartX, StartY, StartZ},
    {StartX, StartY, StartZ},
  };    
  interpolation(OldPos, NewPos, 10);
  StartPos = true;
}