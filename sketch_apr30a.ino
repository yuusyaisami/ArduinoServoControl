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

void setup() {
  Bluetooth.begin(9600); //opens serial port, sets data rate to 9600 bps
  Serial.begin(9600); //opens serial port, sets data rate to 9600 bps
  Serial.println("Connect your device with 1234 as Paring Key\n");
  RightDownservo.attach(ServoRD);
  RightUpservo.attach(ServoRU);
  LeftDownservo.attach(ServoLD);
  LeftUpservo.attach(ServoLU);
}
 int speed = -35,olddata;
void loop() {
  if(Bluetooth.available()){
    int input = Bluetooth.read();
    //if(input != olddata){
      //olddata = input;
    
      if(input == 0 || input == 0x30){ //テキストデータとして送信するなら0を0x30に変える
        Stop();
        Serial.println("Stop");
      }
      else if(input == 1 || input == 0x31){ //テキストデータとして送信するなら1を0x31に変える
        Forward(speed);
        Serial.println("前進");
      }
      else if(input == 2 || input == 0x32){
        Backward(speed);
        Serial.println("更新");
      }
      else if(input == "3" || input == 0x33){
        LeftRotation(speed);
        Serial.println("左回転");
      }
      else if(input == "4" || input == 0x34){
        RightRotation(speed);
        Serial.println("右回転");
      }
      else if(input == "5" || input == 0x35){
        ForwardLeftFold(speed);
        Serial.println("前進左");
    }
      else if(input == "6" || input == 0x36){
        ForwardRightFold(speed);
        Serial.println("前進右");
      }
      else if(input == "7" || input == 0x37){
        BackwardLeftFold(speed);
        Serial.println("後進左");
      }
      else if(input == "8" || input == 0x38){
        BackwardRightFold(speed);
        Serial.println("後進右");
      }
    //}
  }
  delay(10);
}
void Forward(int Speed){
  RightDownservo.write(132 + Speed);
  RightUpservo.write(130 + Speed);
  LeftDownservo.write(50 - Speed);
  LeftUpservo.write(52 - Speed);
}
void Backward(int Speed){
  RightDownservo.write(52 - Speed);
  RightUpservo.write(50 - Speed);
  LeftDownservo.write(130 + Speed);
  LeftUpservo.write(132 + Speed);
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
  LeftDownservo.write(50 + Speed);
  LeftUpservo.write(52 + Speed);
}
void BackwardLeftFold(int Speed){
  RightDownservo.write(132 - Speed);
  RightUpservo.write(130 - Speed);
  LeftDownservo.write(90);
  LeftUpservo.write(92);
}
void Stop(){
  RightDownservo.write(92);
  RightUpservo.write(90);
  LeftDownservo.write(90);
  LeftUpservo.write(92);
}
