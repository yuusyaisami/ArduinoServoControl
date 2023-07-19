/*
  ArduinoServoControll
  作者 : Yuisami
  概要 : ロボットの制御文

  license : unlicense
*/

#include<SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial Bluetooth(7,8); //RX | TX
Servo ServoL;          // Servoオブジェクトの宣言 右下サーボ
Servo ServoR;          // Servoオブジェクトの宣言 右上サーボ
const int SERVO_L_PIN = 6;   // サーボモーターL 5
const int SERVO_R_PIN = 5;   // サーボモーターR 6

const int trigPinR = 4;
const int echoPinR = 2;

const int trigPinL = 11;
const int echoPinL = 12;

const int side_reflector_R = A0;
const int side_reflector_L = A1;

const int center_reflector_R = A2;
const int center_reflector_L = A3;
const int color_black = 900;
const int color_write = 950;

int ServoSpeed = 5;

int Map[7][7] = {
  {999, 999, 999, 999, 999, 999, 999},
  {999,   0,   0,   0,   0,   0, 999},
  {999,   0, 999,   0, 999, 999, 999},
  {999,   0,   0,   0,   0, 999, 999},
  {999, 999,   0, 999,   0,   0,   0},
  {999,   0,   0,   0,   0, 999, 999},
  {999, 999, 999, 999, 999, 999, 999}
};
int BestMap[7][7] = {
  {999, 999, 999, 999, 999, 999, 999},
  {999,   0,   0,   0,   0,   0, 999},//   1
  {999,   0, 999,   0, 999, 999, 999},// 2   4
  {999,   0,   0,   0,   0, 999, 999},//   3
  {999, 999,   0, 999,   0,   0,   0},
  {999,   0,   0,   0,   0, 999, 999},
  {999, 999, 999, 999, 999, 999, 999}
};
struct Car{
  int x = 1;
  int y = 1;
  int direction = 4;
};
void setup() {
  // put your setup code here, to run once:
  Bluetooth.begin(9600); //opens serial port, sets data rate to 9600 bpsrd
  Serial.begin(9600);

  ServoL.attach(SERVO_L_PIN);
  ServoR.attach(SERVO_R_PIN);

  pinMode(trigPinL, OUTPUT);
  pinMode(echoPinL, OUTPUT);
  pinMode(trigPinR, OUTPUT);
  pinMode(echoPinR, OUTPUT);
  ServoL.write(90);
  ServoR.write(90);
}
int ControllMode = -1;
Car car; //構造体carをインスタンス化
void loop() {
  int data = Recv();//シリアル通信で受け取ったデータを読み込む
  //--------------------切り替え--------------------
  if(data == 100){
    ControllMode = 0;
    Bluetooth.write("ControllMode Manual");
  }
  else if(data == 101){
    ControllMode = 1;
    Bluetooth.write("ControllMode Automatic");
    Serial.write("初期化中");
    car.x = 1;
    car.y = 1;
    car.direction = 4;
    BestMapClear();
    BestMap[1][1] = 1;
    Serial.write("初期化完了");
  }
  if(data == 102){
    ControllMode = 2;
    Bluetooth.write("Linetracer");
  }
  if(data == 103){
    ControllMode = 3;
    Bluetooth.write("speed change");
    Serial.print("ServoSpeed : ");
  
    Serial.print(ServoSpeed);
    Serial.println();
  }
  //--------------------関数実行-----------------------
  switch(ControllMode){
    case 0:
      ManualDrive(data); //手動走行
      break;
    case 1:
      AutomaticDrive(); //自動走行
      break;
    case 2:
      LinetracerDrive(); //ライントレース走行
      ControllMode = 0;
    case 3:
      delay(1000);
      ServoSpeed = Recv();
      Serial.write("データ");
      ControllMode = 0;
  }
}

void AutomaticDrive(){
  Serial.write("Automatic Controller起動");
      while(free){
        Serial.println("どうしますか?");
        Serial.println("起動 : 1");
        Serial.println("設定 : 2");
        BestMapClear();
        
        while(free){ //Automatic loop
            int ControllType = Recv();
            switch(ControllType){
              case 1: //マップ移動操作
                Serial.println("目的地を入力してください(x)");
                int goal_x = Recv();
                Serial.print("x = ");
                Serial.print(goal_x);
                Serial.println("目的地を入力してください(y)");
                int goal_y = Recv();
                Serial.print("y = ");
                Serial.print(goal_y);
                if(goal_x == car.x && goal_y == car.y){
                  Serial.write("目的地はここです");
                  break;
                }
                else if(BestMap[goal_y][goal_x] == 999){
                  Serial.write("目的地は外壁です");
                  break;
                }
                BestMap[goal_y][goal_x] = 995; //目的地を設定
                BestMap[car.y][car.x] = 1;
                BestMapWaterFlushing();
                BestMapOptimalRoute();
                BestMap[car.y][car.x] = 0; //車の位置(スタート地点)
                BestMap[goal_y][goal_x] = 990; //ゴール
                int go_direction; //進む方向 (1 : そのまま   2 : 左に曲がる   3 : 後ろを向く   4 : 右に曲がる )
                bool goal_flag = false;
                for(int Count = 0; ;Count++){ //ゴミコード
                  switch(car.direction){
                  case 1:
                    if(BestMap[car.y - 1][car.x] == 990){
                      go_direction = 1;
                      car.y--;
                    }
                    else if(BestMap[car.y][car.x - 1] == 990){
                      go_direction = 2;
                      car.x--;
                    }
                    else if(BestMap[car.y + 1][car.x] == 990){
                      go_direction = 3;
                      car.y++;
                    }
                    else if(BestMap[car.y][car.x + 1] == 990){
                      go_direction = 4;
                      car.x++;
                    }
                    else{
                      Serial.println("おそらく目的地はいまここだろう");
                      goal_flag = true;
                    }
                    break;
                  case 2:
                    if(BestMap[car.y - 1][car.x] == 990){
                      go_direction = 4;
                      car.y--;
                    }
                    else if(BestMap[car.y][car.x - 1] == 990){
                      go_direction = 1;
                      car.x--;
                    }
                    else if(BestMap[car.y + 1][car.x] == 990){
                      go_direction = 2;
                      car.y++;
                    }
                    else if(BestMap[car.y][car.x + 1] == 990){
                      go_direction = 3;
                      car.x++;
                    }
                    else{
                      Serial.println("おそらく目的地はいまここだろう");
                      goal_flag = true;
                    }
                    break;
                  case 3:
                    if(BestMap[car.y - 1][car.x] == 990){
                      go_direction = 3;
                      car.y--;
                    }
                    else if(BestMap[car.y][car.x - 1] == 990){
                      go_direction = 4;
                      car.x--;
                    }
                    else if(BestMap[car.y + 1][car.x] == 990){
                      go_direction = 1;
                      car.y++;
                    }
                    else if(BestMap[car.y][car.x + 1] == 990){
                      go_direction = 2;
                      car.x++;
                    }
                    else{
                      Serial.println("おそらく目的地はいまここだろう");
                      goal_flag = true;
                    }
                    break;
                  case 4:
                    if(BestMap[car.y - 1][car.x] == 990){
                      go_direction = 2;
                      car.y--;
                    }
                    else if(BestMap[car.y][car.x - 1] == 990){
                      go_direction = 3;
                      car.x--;
                    }
                    else if(BestMap[car.y + 1][car.x] == 990){
                      go_direction = 4;
                      car.y++;
                    }
                    else if(BestMap[car.y][car.x + 1] == 990){
                      go_direction = 1;
                      car.x++;
                    }
                    else{
                      Serial.println("おそらく目的地はいまここだろう");
                      goal_flag = true;
                    }
                    break;
                  }
                  if(goal_flag){
                    break;
                  }
                  switch(go_direction){
                    case 1: //そのまま
                      
                      
                      break;
                    case 2: //左に曲がる
                      LeftTurn();
                      delay(200);
                      for(int i = 0;i < 80 ; i++){
                        if(ReadCarLine(center_reflector_R) && ReadCarLine(center_reflector_L)){
                          Serial.print("曲がり角です! time : ");
                          Serial.print((i * 10 + 200) * 0.001);
                          Serial.print("s");
                          Serial.println();
                          break;
                        }
                        delay(10);
                      }
                      Stop();
                      break;
                    case 3: //後ろ
                      LeftTurn();
                      delay(200);
                      for(int i = 0;i < 80 ; i++){
                        if(ReadCarLine(center_reflector_R) && ReadCarLine(center_reflector_L)){
                          Serial.print("曲がり角です! time : ");
                          Serial.print((i * 10 + 200) * 0.001);
                          Serial.print("s");
                          Serial.println();
                          break;
                        }
                        delay(10);
                      }
                      LeftTurn();
                      delay(200);
                      for(int i = 0;i < 80 ; i++){
                        if(ReadCarLine(center_reflector_R) && ReadCarLine(center_reflector_L)){
                          Serial.print("曲がり角です! time : ");
                          Serial.print((i * 10 + 200) * 0.001);
                          Serial.print("s");
                          Serial.println();
                          break;
                        }
                        delay(10);
                      }
                      Stop();
                      break;
                    case 4: //右に曲がる
                      RightTurn();
                      delay(200);
                      for(int i = 0; i < 80 ; i++){
                        if(ReadCarLine(center_reflector_R) && ReadCarLine(center_reflector_L)){
                          Serial.print("曲がり角です! time : ");
                          Serial.print((i * 10 + 200) * 0.001);
                          Serial.print("s");
                          Serial.println();
                          break;
                        }
                        delay(10);
                      }
                      Stop();
                      break;
                  }
                  car.direction = go_direction;
                  //前進
                  while(free){
                    LinetracerDrive();
                    if(ReadCarLine(side_reflector_R) && ReadCarLine(side_reflector_L)){
                      delay(200);
                      break;
                    }
                  }
                }
                Serial.println("------------------------------結果--------------------------------");
                Serial.print("car.x : "); Serial.print(car.x); Serial.println();
                Serial.print("car.y : "); Serial.print(car.y); Serial.println();
                Serial.print("car.d : "); Serial.print(car.direction); Serial.println();
                Serial.println("------------------------------終了-------------------------------");
                break; //ControllType から 抜ける
              case 2:
              while(free){
                Serial.println("------------------------------設定---------------------------------");
                Serial.print("car.x : "); Serial.print(car.x); Serial.println();
                Serial.print("car.y : "); Serial.print(car.y); Serial.println();
                Serial.print("car.d : "); Serial.print(car.direction); Serial.println();
                Serial.println("-------------------------------------------------------------------");
                Serial.println("何を変更しますか?");
                Serial.println("car.x を変更するには 1"); 
                Serial.println("car.y を変更するには 2"); 
                Serial.println("car.d を変更するには 3"); 
                Serial.println("終了するには 4"); 
                
                int recv_data = Recv();
                switch(recv_data){
                  case 1: //car.x
                    Serial.println("値を入力してください");
                    recv_data = Recv();
                    car.x = recv_data;
                   break;
                  case 2: //car.y
                    Serial.println("値を入力してください");
                    recv_data = Recv();
                    car.y = recv_data;
                   break;
                  case 3:
                    Serial.println("値を入力してください");
                    recv_data = Recv();
                    car.direction = recv_data;
                   break;
                }
                if(recv_data == 4){
                  break;
                }
              }
                break;
              }
              Serial.println("どうしますか?");
              Serial.println("稼働 : 1");
              Serial.println("設定 : 2");
              BestMapClear();
            
          }
        }
}
int Recv(){
  int Recv_data = 2525;
  int Delete_data;
  while(free){
    if(Bluetooth.available() ){
      Recv_data = Bluetooth.read();
      if(Recv_data < 10){
        break;
      }
      else if(Recv_data < 100){
        Recv_sub();
        break;
      }
      else if(Recv_data < 1000){
        Recv_sub();
        Recv_sub();
        break;
      }
    }
  }
  return Recv_data;
}
int Recv_sub(){
  int Recv_data = 2525;
  int Delete_data;
  while(free){
    if(Bluetooth.available() ){
      Recv_data = Bluetooth.read();
      break;
    }
  }
  return Recv_data;
}

//黒だったらtrue 白だったらfalse
bool ReadCarLine(int pin){
  if(analogRead(pin) < color_write){
    return true;
  }
  return false;
}

float DistanceL(){
  float duration;
  //測定するための関数たち-----------
  digitalWrite(trigPinL,LOW);              // 計測前に一旦トリガーピンをLowに
  delayMicroseconds(2);

  digitalWrite(trigPinL,HIGH);             // トリガーピンを10μsの時間だけHighに
  delayMicroseconds(10);
  digitalWrite(trigPinL,LOW);
//----------------------------------
  duration = pulseIn(echoPinL,HIGH);      // エコーピンからの入力
  
  duration = duration / 2;               // 距離を1/2に
  return duration*340*100/1000000;   // 音速を掛けて単位をcmに
}

float DistanceR(){
  float duration;
  //測定するための関数たち-----------
  digitalWrite(trigPinR,LOW);              // 計測前に一旦トリガーピンをLowに
  delayMicroseconds(2);

  digitalWrite(trigPinR,HIGH);             // トリガーピンを10μsの時間だけHighに
  delayMicroseconds(10);
  digitalWrite(trigPinR,LOW);
//----------------------------------
  duration = pulseIn(echoPinR,HIGH);      // エコーピンからの入力
  
  duration = duration / 2;               // 距離を1/2に
  return duration * 340 * 100 / 1000000;   // 音速を掛けて単位をcmに
}

int ServoR_condition = 92;
int ServoL_condition = 92;
void Forward(){
  ServoR.write(ServoR_condition + ServoSpeed);
  ServoL.write(ServoL_condition - ServoSpeed);
}
void Backward(){
  ServoR.write(ServoR_condition - ServoSpeed);
  ServoL.write(ServoL_condition + ServoSpeed);
}
void LeftTurn(){
  ServoR.write(ServoR_condition + ServoSpeed);
  ServoL.write(ServoL_condition + ServoSpeed);
}
void LeftForward(){
  ServoR.write(ServoR_condition + ServoSpeed);
  ServoL.write(90);
}
void RightForward(){
  ServoR.write(90);
  ServoL.write(ServoL_condition- ServoSpeed);
}
void RightTurn(){
  ServoR.write(ServoR_condition - ServoSpeed);
  ServoL.write(ServoL_condition - ServoSpeed);
}
void Stop(){
  ServoR.write(90);
  ServoL.write(90);
}
void BestMapClear(){
  int row = sizeof(BestMap) / sizeof(BestMap[0]);
  int col = sizeof(BestMap[0]) / sizeof(int);
  for(int i = 0 ; i < row ; i++){
    for(int j = 0 ; j < col ; j++){
      BestMap[i][j] = Map[i][j];
    }
  }
}
void BestMapOptimalRoute(){
  int row = sizeof(BestMap) / sizeof(BestMap[0]);
  int col = sizeof(BestMap[0]) / sizeof(int);
  int x = 0, y = 0, NowNumber = 990, Step;
  for(int i = 0; i < row; i++)
            {
                for(int j = 0;  j < col; j++)
                {
                    if (BestMap[i][j] == 995)
                    {
                        x = j;
                        y = i;
                    }
                }
            }
            for(int i = 0; ; i++)
            {
                int Directions = 0;
                if (BestMap[y + 1][x] < NowNumber && BestMap[y + 1][x] != 0)  //十字の方向で一番小さい数値を探す
                {
                    NowNumber = BestMap[y + 1][x];
                    Directions = 1;
                }
                if (BestMap[y - 1][x] < NowNumber && BestMap[y - 1][x] != 0)
                {
                    NowNumber = BestMap[y - 1][x];
                    Directions = 2;

                }
                if (BestMap[y][x + 1] < NowNumber && BestMap[y][x + 1] != 0)
                {
                    NowNumber = BestMap[y][x + 1];
                    Directions = 3;
                }
                if (BestMap[y][x - 1] < NowNumber && BestMap[y][x - 1] != 0)
                {
                    NowNumber = BestMap[y][x - 1];
                    Directions = 4;
                }
                if (Directions != 0)
                {
                    switch (Directions)
                    {
                        case 1:
                            y++;
                            break;
                        case 2:
                            y--;
                            break;
                        case 3:
                            x++;
                            break;
                        case 4:
                            x--;
                            break;
                    }
                    BestMap[y][x] = 990;
                }
                else
                {
                    Step = i;
                    break;
                }
            }
}
void BestMapWaterFlushing(){
  for (int Count = 1; ; Count++) {
                //1の数値から値を増やしていく
                bool ColoringFlag = false;
                int row = sizeof(BestMap) / sizeof(BestMap[0]), col = sizeof(BestMap[0]) / sizeof(int);
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j < col; j++)
                    {
                        if (BestMap[i][j] == Count)
                        {
                            if (BestMap[i + 1][j] == 0)  //下が0だったら次の文字を入力
                            {
                                BestMap[i + 1][j] = Count + 1;
                            }
                            if (BestMap[i - 1][j] == 0)  //上が0だったら次の文字を入力
                            {
                                BestMap[i - 1][j] = Count + 1;
                            }
                            if (BestMap[i][j + 1] == 0)  //右が0だったら次の文字を入力
                            {
                                BestMap[i][j + 1] = Count + 1;
                            }
                            if (BestMap[i][j - 1] == 0)  //左が0だったら次の文字を入力
                            {
                                BestMap[i][j - 1] = Count + 1;
                            }
                            ColoringFlag = true;
                        }
                    }
                }
                if(!ColoringFlag)
                {
                    break;
                }
  }
}
void ManualDrive(int data){
  int command = data; 
  Serial.println(command);
  switch(command){
    case 0:
      Stop();
      Serial.print("stop");
      break;
    case 1:
      Forward();
      Serial.print("forward");
      break;
    case 2:
      LeftTurn();
      Serial.print("LeftTurn");
      break;
    case 3:
      Backward();
      Serial.print("Backward");
      break;
    case 4:
      RightTurn();
      Serial.print("RightTurn");
      break;
  }
  Bluetooth.write("come");
}
void LinetracerDrive(){
  while(free){
    Serial.println();
    Serial.print("side L : ");
    Serial.print(ReadCarLine(side_reflector_L));
    Serial.print("side R : ");
    Serial.print(ReadCarLine(side_reflector_R));
    Serial.print("center L : ");
    Serial.print(ReadCarLine(center_reflector_L));
    Serial.print("center R : ");
    Serial.print(ReadCarLine(center_reflector_R));
    if(ReadCarLine(center_reflector_R) && ReadCarLine(center_reflector_L)){
      Forward();
    }
    else if(ReadCarLine(center_reflector_R)){
      RightForward();
    }
    else if(ReadCarLine(center_reflector_L)){
      LeftForward();
    }
    else if(ReadCarLine(side_reflector_R) && ReadCarLine(side_reflector_L) == false){
      RightForward();
    }
    else if(ReadCarLine(side_reflector_R) == false && ReadCarLine(side_reflector_L)){
      LeftForward();
    }
    else{
      Forward();
    }
    if(Bluetooth.available() ){
      if(Bluetooth.read() == 0){
        break;
      }
    }
  }
}
class Ardcar{
  public:

};