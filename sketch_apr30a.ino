#include <Servo.h>
#include<SoftwareSerial.h>

SoftwareSerial Bluetooth(10,11); //RX | TX

Servo RightDownservo;          // Servoオブジェクトの宣言 右下サーボ
Servo RightUpservo;          // Servoオブジェクトの宣言 右上サーボ
Servo LeftDownservo;          // Servoオブジェクトの宣言 左下サーボ
Servo LeftUpservo;          // Servoオブジェクトの宣言 左上サーボ
const int ServoRD = 6;   // サーボモーターをデジタルピン6に
const int ServoRU = 9;   // サーボモーターをデジタルピン9に
const int ServoLD = 3;   // サーボモーターをデジタルピン3に
const int ServoLU = 5;   // サーボモーターをデジタルピン6に
const int SoundPin = 1000; //ワロタ
#define trigPinR 12  // トリガーピンRをD8に
#define echoPinR 13  // エコーピンRをD9に

#define trigPinL 7  // トリガーピンRをD12に
#define echoPinL 8  // エコーピンRをD13に

int CarPositionX = 1;
int CarPositionY = 3;
//---------------サウンドHz---------------------
int C[7] = {33,65,131,262,523,1046,2093};
int Db[7]= {65,69,138,277,554,1109,2217};
int D[7] = {37,73,147,294,587,1175,2349};
int Eb[7]= {39,78,155,311,622,1244,2489};
int E[7] = {41,82,165,330,659,1318,2637};
int F[7] = {44,87,175,349,698,1397,2794};
int Gb[7]= {46,92,185,367,740,1480,2960};
int G[7] = {49,98,196,392,784,1568,3136};
int Ab[7]= {52,104,208,415,831,1661,3322};
int A[7] = {55,110,220,440,880,1760,3520};
int Bb[7]= {58,116,233,466,932,1865,3729};
int B[7] = {62,123,247,494,988,1976,3951};
//------------------------------------------------------
void setup() {
  Bluetooth.begin(9600); //opens serial port, sets data rate to 9600 bps
  Serial.begin(9600); //opens serial port, sets data rate to 9600 bpsrd
  Serial.println("Connect your device with 1234 as Paring Key\n");
  RightDownservo.attach(ServoRD);//サーボattach宣言
  RightUpservo.attach(ServoRU);
  LeftDownservo.attach(ServoLD);
  LeftUpservo.attach(ServoLU);

  pinMode(echoPinR,INPUT);   // エコーピンを入力に
  pinMode(trigPinR,OUTPUT);  // トリガーピンを出力に

   pinMode(echoPinL,INPUT);
  pinMode(trigPinL,OUTPUT);
}
//speed 速度調整-30で問題なし
//oldddata 重複したデータを選別
//SelectMode バスのモード
//input bluetoothのデータ
// 1 : 操縦, 2 : 自動走行
 int speed = -30,olddata,SelectMode = 1,input;
 double distanceR,distanceL = 10;
void loop() {
  
  //BGM
  //WindSceneBGM();
  if(Bluetooth.available()){
    //bluetoothから受け取ったデータ
    input = Bluetooth.read();
    Serial.println(input);
    if(input == 201){
      SelectMode = 1;
    }
    else if(input == 202){
      SelectMode = 2;
    }
    else if(input == 203){
      SelectMode = 3;
      Serial.println("Mode 3");
    }
    if(SelectMode == 1){
      //手動での操作
      ManualControl();
    }
    if(SelectMode == 3){
    MapAutomaticControl();
    }

    
  }
  if(SelectMode == 2){
      AutomaticControl();
  }
  
  delay(20);
}
//MapAutomaticControl関数用の変数
//現在地 : 2, 壁 : 99
int Map[5][5] = {
  {99,99,99,99,99},
  {99, 0, 0, 0,99}, //1 , 2, 3
  {99, 0,99, 0,99},// 4 ,  , 5
  {99, 0, 0, 0,99},// 6 , 7, 8
  {99,99,99,99,99}
};
int BestMap[5][5] = {
  {99,99,99,99,99},
  {99, 0, 0, 0,99},//  1
  {99, 0,99, 0,99},//3   2
  {99, 0, 0, 0,99},//  4
  {99,99,99,99,99}
};
int CarPosition = 6,CarDirections = 1;
int CarPx, CarPy, EndPx, EndPy,MoveMode = 1,Goal = 0;
double Timer = 0;
//MoveMode 1 : 相対移動 MoveMode 2 : 絶対移動
void MapAutomaticControl(){
  Serial.println("MapAutomaticControl起動");
  delay(2000);
  Serial.println("Start地点の北側から見て左下にセットしてください");
  if(input > 0 && input < 9){
    StartEndSetting();
    Serial.print("Start");
    Serial.print(CarPosition);
     BestWayMapF();
    Serial.print("車のx座標とY座標 : ");
    Serial.print(CarPx);Serial.println(CarPy);
    Serial.println("現在 マップの状況");
    Serial.print(BestMap[1][1]);Serial.print(",");
    Serial.print(BestMap[1][2]);Serial.print(",");
    Serial.print(BestMap[1][3]);Serial.println(",");

    Serial.print(BestMap[2][1]);Serial.print(",");
    Serial.println(BestMap[2][3]);Serial.println(",");

    Serial.print(BestMap[3][1]);Serial.print(",");
    Serial.print(BestMap[3][2]);Serial.print(",");
    Serial.print(BestMap[3][3]);Serial.println(",");
    if(input == CarPosition){
        Serial.print("現在目的地にいます");
        return;
      }
    for(;;){
      
      NextDirection();
      Serial.print("次のいくべき方向 : ");
      Serial.println(CarDirections);
      ForwardControl("Forward");
      delay(400);
      for(;;){
        Serial.print("MoveModeは");
        Serial.println(MoveMode);
      if(MoveMode == 1){
        RelativeControl();
      }
      else if(MoveMode == 2){
        AbsoluteControl();
      }
      if(Timer > 10){
        Serial.print("1マス移動しました");
        Serial.print(Timer);
        Timer = 0;
        Stop();
        break;
      }
    
      delay(50);
      }
      if(Goal == 1){
        Serial.print("現在目的地にいます...いるよね?");
        CarPosition = input;
        Serial.print(CarPosition);
        break;
      }
    }
  }
  else if(input == 100){
    CarPosition = 6;CarDirections = 1;
    MoveMode = 1;Goal = 0;
  }
  input = 0;
}
void RelativeControl(){
String Direction = "Forward";
  DistanceR();
  if(distanceR < 5.5){
    Direction = "Left";
    if(distanceR < 3.5){
      Direction = "HighLeft";
    }
  }
  DistanceL();
  if(distanceL < 6.5){
    Direction = "Right";
    if(distanceL < 3.5){
      Direction = "HighRight";
    }
  }
  Timer = Timer + 0.4;
  ForwardControl(Direction);
}
void AbsoluteControl(){
  String Direction = "Forward";
  DistanceR();
  if(distanceR < 5.5){
    Direction = "Left";
    if(distanceR < 3.5){
      Direction = "HighLeft";
    }
  }
  if(distanceR > 15){
    Timer = Timer + 1.2;
  }
  DistanceL();
  if(distanceL < 6.5){
    Direction = "Right";
    if(distanceL < 3.5){
      Direction = "HighRight";
    }
  }
  if(distanceL > 15){
    Timer = Timer + 1.2;
  }
  ForwardControl(Direction);
  

}
void NextDirection(){
  Serial.println("現在次に移動するべき道を計算しています");
  int NextForwardDirection = 0;
  MoveMode = 2;
  if(BestMap[CarPy + 1][CarPx] == 9 || BestMap[CarPy + 1][CarPx] == 95){
    NextForwardDirection = 4;
    if(BestMap[CarPy + 1][CarPx] == 95){
      Goal = 1;
    }
    if(BestMap[CarPy + 1][CarPx + 1] == 99 && BestMap[CarPy + 1][CarPx - 1] == 99){
      MoveMode = 1;
    }
    BestMap[CarPy ][CarPx] = 0;
    CarPy = CarPy + 1;
  }
  else if(BestMap[CarPy][CarPx + 1] == 9 || BestMap[CarPy][CarPx + 1] == 95){
    NextForwardDirection = 2;
    if(BestMap[CarPy][CarPx + 1] == 95){
      Goal = 1;
    }
    if(BestMap[CarPy + 1][CarPx + 1] == 99 && BestMap[CarPy - 1][CarPx + 1] == 99){
      MoveMode = 1;
    }
    BestMap[CarPy ][CarPx] = 0;
    CarPx = CarPx + 1;
  }
  else if(BestMap[CarPy - 1][CarPx] == 9 || BestMap[CarPy - 1][CarPx] == 95){
    NextForwardDirection = 1;
    if(BestMap[CarPy - 1][CarPx] == 95){
      Goal = 1;
    }
    if(BestMap[CarPy - 1][CarPx + 1] == 99 && BestMap[CarPy - 1][CarPx - 1] == 99){
      MoveMode = 1;
    }
    BestMap[CarPy ][CarPx] = 0;
    CarPy = CarPy - 1;
  }
  else if(BestMap[CarPy][CarPx - 1] == 9 || BestMap[CarPy][CarPx - 1] == 95){
    NextForwardDirection = 3;
    if(BestMap[CarPy][CarPx - 1] == 95){
      Goal = 1;
    }
    if(BestMap[CarPy + 1][CarPx - 1] == 99 && BestMap[CarPy - 1][CarPx - 1] == 99){
      MoveMode = 1;
      MoveMode = 1;
    }
    BestMap[CarPy ][CarPx] = 0;
    CarPx = CarPx - 1;
  }
  Serial.print("車の向き : ");
  Serial.println(CarDirections);
  Serial.print("進むべき向き : ");
  Serial.println(NextForwardDirection);
  if(CarDirections == NextForwardDirection){
    return;
  }
  if(CarDirections == 1 && NextForwardDirection == 2){
    RightTurn();
    CarDirections = 2;
  }
  else if(CarDirections == 1 && NextForwardDirection == 3){
    LeftTurn();
    CarDirections = 3;
  }
  else if(CarDirections == 1 && NextForwardDirection == 4){
    BackTurn();
    CarDirections = 4;
  }
  else if(CarDirections == 2 && NextForwardDirection == 1){
    LeftTurn();
    CarDirections = 1;
  }
  else if(CarDirections == 2 && NextForwardDirection == 4){
    RightTurn();
    CarDirections = 4;
  }
  else if(CarDirections == 2 && NextForwardDirection == 3){
    BackTurn();
    CarDirections = 3;
  }
  else if(CarDirections == 3 && NextForwardDirection == 1){
    RightTurn();
    CarDirections = 1;
  }
  else if(CarDirections == 3 && NextForwardDirection == 4){
    LeftTurn();
    CarDirections = 4;
  }
  else if(CarDirections == 3 && NextForwardDirection == 2){
    BackTurn();
    CarDirections = 2;
  }
  else if(CarDirections == 4 && NextForwardDirection == 2){
    LeftTurn();
    CarDirections = 2;
  }
  else if(CarDirections == 4 && NextForwardDirection == 3){
    RightTurn();
    CarDirections = 3;
  }
  else if(CarDirections == 4 && NextForwardDirection == 1){
    BackTurn();
    CarDirections = 1;
  }

}
void StartEndSetting(){
  BestMap[1][1] = BestMap[1][2] = BestMap[1][3] = BestMap[2][1] = BestMap[2][3] = BestMap[3][1] = BestMap[3][2] = BestMap[3][3] = 0;
  if(input == 1){
    BestMap[1][1] = 95;
    EndPx = 1;
    EndPy = 1;
  }
  else if(input == 2){
    BestMap[1][2] = 95;
    EndPx = 2;
    EndPy = 1;
  }
  else if(input == 3){
    BestMap[1][3] = 95;
    EndPx = 3;
    EndPy = 1;
  }
  else if(input == 4){
    BestMap[2][1] = 95;
    EndPx = 1;
    EndPy = 2;
  }
  else if(input == 5){
    BestMap[2][3] = 95;
    EndPx = 3;
    EndPy = 2;
  }
  else if(input == 6){
    BestMap[3][1] = 95;
    EndPx = 1;
    EndPy = 3;
  }
  else if(input == 7){
    BestMap[3][2] = 95;
    EndPx = 2;
    EndPy = 3;
  }
  else if(input == 8){
    BestMap[3][3] = 95;
    EndPx = 3;
    EndPy = 3;
  }
  if(CarPosition == 1){
    BestMap[1][1] = 1;
    CarPx = 1;
    CarPy = 1;
  }
  else if(CarPosition == 2){
    BestMap[1][2] = 1;
    CarPx = 2;
    CarPy = 1;
  }
  else if(CarPosition == 3){
    BestMap[1][3] = 1;
    CarPx = 3;
    CarPy = 1;
  }
  else if(CarPosition == 4){
    BestMap[2][1] = 1;
    CarPx = 1;
    CarPy = 2;
  }
  else if(CarPosition == 5){
    BestMap[2][3] = 1;
    CarPx = 3;
    CarPy = 2;
  }
  else if(CarPosition == 6){
    BestMap[3][1] = 1;
    CarPx = 1;
    CarPy = 3;
  }
  else if(CarPosition == 7){
    BestMap[3][2] = 1;
    CarPx = 2;
    CarPy = 3;
  }
  else if(CarPosition == 8){
    BestMap[3][3] = 1;
    CarPx = 3;
    CarPy = 3;
  }
}
void BestWayMapF(){
int End = 0;
int row = sizeof(BestMap) / sizeof(BestMap[0]);
int col = sizeof(BestMap[0]) / sizeof(int);
            for (int Count = 1; ; Count++)
            {
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j <  col; j++)
                    {
                        if (BestMap[i][j] == Count)
                        {
                            if (i < row - 1)
                                if (BestMap[i + 1][j] == 0)
                                {
                                    BestMap[i + 1][j] = Count + 1;
                                    End = 1;
                                }
                            if (j < col - 1)
                                if (BestMap[i][j + 1] == 0)
                                {
                                    BestMap[i][j + 1] = Count + 1;
                                    End = 1;
                                }
                            if (i != 0)
                                if (BestMap[i - 1][j] == 0)
                                {
                                    BestMap[i - 1][j] = Count + 1;
                                    End = 1;
                                }
                            if (j != 0)
                                if (BestMap[i][j - 1] == 0)
                                {
                                    BestMap[i][j - 1] = Count + 1;
                                    End = 1;
                                }

                        }


                    }

                }
                if (End == 1)
                {
                    End = 0;
                }
                else if (End == 0)
                {
                    break;
                }
            }
            Serial.println("12345678マップの状況");
    Serial.print(BestMap[1][1]);Serial.print(",");
    Serial.print(BestMap[1][2]);Serial.print(",");
    Serial.print(BestMap[1][3]);Serial.println(",");

    Serial.print(BestMap[2][1]);Serial.print(",");
    Serial.println(BestMap[2][3]);Serial.println(",");

    Serial.print(BestMap[3][1]);Serial.print(",");
    Serial.print(BestMap[3][2]);Serial.print(",");
    Serial.print(BestMap[3][3]);Serial.println(",");
    int CountStep = 0;
             for (int i = 0; i < row; i++)
            {
                for (int j = 0; j <  col; j++)
                {
                    if (BestMap[i][j] == 95)
                    {
                        int x = j;
                        int y = i;
                        int Value = 0;
                        int Temp = 0,DirectionIndex = 5;
                        for (int k = 0; ; k++)
                        {
                          DirectionIndex = 5;
                          if (CountStep == 0)
                            {
                          if (y < row - 1 && Value == 0)
                            { 
                                if(BestMap[y + 1][x] == 1)
                                {
                                    Value = 2;
                                }
                                Temp = BestMap[y + 1][x];
                                    DirectionIndex = 4;
                            }
                            if (x < col - 1 && Value == 0)
                            {
                                if(BestMap[y][x + 1] == 1)
                                {
                                    Value = 2;
                                }
                                if (Temp > BestMap[y][x + 1])
                                    {
                                        Temp = BestMap[y][x + 1];
                                        DirectionIndex = 2;
                                    }
                            }
                            if(y != 0 && Value == 0)
                            {
                                if(BestMap[y - 1][x] == 1)
                                {
                                    Value = 2;
                                }
                                if (Temp > BestMap[y - 1][x])
                                    {
                                        Temp = BestMap[y - 1][x];
                                        DirectionIndex = 1;
                                    }
                            }
                            if(x != 0 && Value == 0)
                            {
                                if(BestMap[y][x - 1] == 1)
                                {
                                    Value = 2;
                                }
                                if (Temp > BestMap[y][x - 1])
                                    {
                                        Temp = BestMap[y][x - 1];
                                        DirectionIndex = 3;
                                    }
                            }
                            
                            }
                            CountStep++;
                            if (y < row - 1 && Value == 0){
                              if (DirectionIndex == 4 || DirectionIndex == 5)
                              {
                                if (BestMap[y][x] > BestMap[y + 1][x])
                                {

                                    if (BestMap[y][x] != 95)
                                    BestMap[y][x] = 9;
                                    y = y + 1;
                                    Value = 1;
                                }
                              }
                            }
                            if (x < col - 1&& Value == 0){
                              if (DirectionIndex == 2 || DirectionIndex == 5)
                              {
                                if (BestMap[y][x] > BestMap[y][x + 1])
                                {
                                    if (BestMap[y][x] != 95)
                                        BestMap[y][x] = 9;
                                    x = x + 1;
                                    Value = 1;
                                }
                              }
                            }
                            if (y != 0 && Value == 0){
                              if (DirectionIndex == 1|| DirectionIndex == 5)
                                {
                                if (BestMap[y][x] > BestMap[y - 1][x])
                                {
                                    if (BestMap[y][x] != 95)
                                        BestMap[y][x] = 9;
                                    y = y - 1;
                                    Value = 1;
                                }
                                }
                            }
                            if (x != 0 && Value == 0){
                              if (DirectionIndex == 3 || DirectionIndex == 5)
                                {
                                if (BestMap[y][x] > BestMap[y][x - 1])
                                {
                                    if (BestMap[y][x] != 95)
                                        BestMap[y][x] = 9;
                                    x = x - 1;
                                    Value = 1;
                                }
                                }
                            }
                            if(Value == 0 || Value == 2)
                            {
                                break;
                            }
                            else
                            {
                                Value = 0;
                            }
                        }
                    }
                }

            }
            Serial.println("90マップの状況");
    Serial.print(BestMap[1][1]);Serial.print(",");
    Serial.print(BestMap[1][2]);Serial.print(",");
    Serial.print(BestMap[1][3]);Serial.println(",");

    Serial.print(BestMap[2][1]);Serial.print(",");
    Serial.println(BestMap[2][3]);Serial.println(",");

    Serial.print(BestMap[3][1]);Serial.print(",");
    Serial.print(BestMap[3][2]);Serial.print(",");
    Serial.print(BestMap[3][3]);Serial.println(",");
}
void AutomaticControl(){
  String Direction = "Forward";
  DistanceR();
  if(distanceR < 5){
    Direction = "Left";
    if(distanceR < 3){
      Direction = "HighLeft";
    }
  }
  DistanceL();
  if(distanceL < 5){
    Direction = "Right";
    if(distanceL < 3){
      Direction = "HighRight";
    }
  }
  ForwardControl(Direction);
}
void RightTurn(){
  RightDownservo.write(95);
  RightUpservo.write(95);
  LeftDownservo.write(95);
  LeftUpservo.write(95);
  delay(2200);
}
void LeftTurn(){
  RightDownservo.write(85);
  RightUpservo.write(85);
  LeftDownservo.write(85);
  LeftUpservo.write(85);
  delay(2000);
}
void BackTurn(){
  RightDownservo.write(95);
  RightUpservo.write(95);
  LeftDownservo.write(95);
  LeftUpservo.write(95);
  delay(4000);
}

void ForwardControl(String direction){
  if(direction == "Left"){
    RightDownservo.write(95);
  RightUpservo.write(95);
  LeftDownservo.write(88);
  LeftUpservo.write(88);
  }
  if(direction == "Right"){
    RightDownservo.write(92);
  RightUpservo.write(92);
  LeftDownservo.write(85);
  LeftUpservo.write(85);
  }
  if(direction == "HighLeft"){
    RightDownservo.write(95);
  RightUpservo.write(95);
  LeftDownservo.write(90);
  LeftUpservo.write(90);
  }
  if(direction == "HighRight"){
    RightDownservo.write(90);
  RightUpservo.write(90);
  LeftDownservo.write(85);
  LeftUpservo.write(85);
  }
  if(direction == "Forward"){
    Forward();
  }
}


void DistanceR(){
  double duration;
  //測定するための関数たち-----------
  digitalWrite(trigPinR,LOW);              // 計測前に一旦トリガーピンをLowに
  delayMicroseconds(2);

  digitalWrite(trigPinR,HIGH);             // トリガーピンを10μsの時間だけHighに
  delayMicroseconds(10);
  digitalWrite(trigPinR,LOW);
//----------------------------------
  duration = pulseIn(echoPinR,HIGH);      // エコーピンからの入力
  
  duration = duration / 2;               // 距離を1/2に
  distanceR = duration*340*100/1000000;   // 音速を掛けて単位をcmに
}


void DistanceL(){
  double duration;
  //測定するための関数たち-----------
  digitalWrite(trigPinL,LOW);              // 計測前に一旦トリガーピンをLowに
  delayMicroseconds(2);

  digitalWrite(trigPinL,HIGH);             // トリガーピンを10μsの時間だけHighに
  delayMicroseconds(10);
  digitalWrite(trigPinL,LOW);
//----------------------------------
  duration = pulseIn(echoPinL,HIGH);      // エコーピンからの入力
  
  duration = duration / 2;               // 距離を1/2に
  distanceL = duration*340*100/1000000;   // 音速を掛けて単位をcmに
}




























































void Forward(){
  RightDownservo.write(95);
  RightUpservo.write(95);
  LeftDownservo.write(85);
  LeftUpservo.write(85);
}
void Backward(int Speed){
  RightDownservo.write(50 - Speed);
  RightUpservo.write(50 - Speed);
  LeftDownservo.write(130 + Speed);
  LeftUpservo.write(130 + Speed);
}
void RightRotation(int Speed){
  RightDownservo.write(132 - Speed);
  RightUpservo.write(130 - Speed);
  LeftDownservo.write(130 + Speed);
  LeftUpservo.write(132 + Speed);
}
void LeftRotation(int Speed){
  RightDownservo.write(52 - Speed);
  RightUpservo.write(50 - Speed);
  LeftDownservo.write(50 + Speed);
  LeftUpservo.write(52 + Speed);
}

void ForwardRightFold(int Speed){
  RightDownservo.write(92);
  RightUpservo.write(90);
  LeftDownservo.write(130 + Speed);
  LeftUpservo.write(132 + Speed);
}
void ForwardLeftFold(int Speed){
  RightDownservo.write(50 - Speed);
  RightUpservo.write(50 - Speed);
  LeftDownservo.write(90);
  LeftUpservo.write(92);
}
void BackwardRightFold(int Speed){
  RightDownservo.write(92);
  RightUpservo.write(90);
  LeftDownservo.write(50 - Speed);
  LeftUpservo.write(52 - Speed);
}
void BackwardLeftFold(int Speed){
  RightDownservo.write(132 + Speed);
  RightUpservo.write(130 + Speed);
  LeftDownservo.write(90);
  LeftUpservo.write(92);
}
void Stop(){
  RightDownservo.write(92);
  RightUpservo.write(90);
  LeftDownservo.write(90);
  LeftUpservo.write(92);
}


void ManualControl(){
  if(input == 0 || input == 0x30){ //テキストデータとして送信するなら0を0x30に変える
        Stop();
      }
      else if(input == 1 || input == 0x31){ //テキストデータとして送信するなら1を0x31に変える
        Forward();
      }
      else if(input == 2 || input == 0x32){
        Backward(speed);
      }
      else if(input == 3 || input == 0x33){
        LeftRotation(speed);
      }
      else if(input == 4 || input == 0x34){
        RightRotation(speed);
      }
      else if(input == 5 || input == 0x35){
        ForwardLeftFold(speed);
    }
      else if(input == 6 || input == 0x36){
        ForwardRightFold(speed);
      }
      else if(input == 7 || input == 0x37){
        BackwardLeftFold(speed);
      }
      else if(input == 8 || input == 0x38){
        BackwardRightFold(speed);
      }
      else if(input == "101" || input == 0x364){
        speed = -5;
      }
      else if(input == "102" || input == 0x365){
        speed = -15;
      }
      else if(input == "103" || input == 0x366){
        speed = -26;
      }
      else if(input == "104" || input == 0x366){
        speed = -36;
      }
}





