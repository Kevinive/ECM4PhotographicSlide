#include "Include.h"

IRrecv irrecv(IR_RECV_PIN);
decode_results results;
MYStepper mystepper(STEPPER_S_P_R,STEPPER_PIN1,STEPPER_PIN2,STEPPER_PIN3,STEPPER_PIN4);

int stepper_maxSpeed = STEPPER_MAXSPEED;
int stepper_maxAcc = STEPPER_MAXACC;
long total_Step = TOTAL_STEP;

typedef struct {
  char Cali;      //1需要校准，0不需要
  char Reset;     //1需要复位 0不需要
  int Seconds;    //中间量，表示运行完的时间 (1-3600)
  long Speed;       //最终量，运行速度，单位r/sec    (1-1000000)
  long Steps;   //最终量，步数(128000)
  int Length;   //中间量，总长度(1-1000)
  
} timeLapseTask;

timeLapseTask task;



void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(KEY,INPUT);
  
  if(EEPROM.read(0) == 0xAD){
    total_Step = ((long)EEPROM.read(1))<<24;
    total_Step |= ((long)EEPROM.read(2))<<16;
    total_Step |= ((long)EEPROM.read(3))<<8;
    total_Step |= ((long)EEPROM.read(4));
  }else{
    calibrate_range();
    EEPROM.write(0, 0xAD);
    EEPROM.write(1, (char)(total_Step>>24));
    EEPROM.write(2, (char)((total_Step>>16)&0xff));
    EEPROM.write(3, (char)((total_Step>>8)&0xff));
    EEPROM.write(4, (char)(total_Step&0xff));
  }
  
  systemInit();
  OLED_Config();
  mystepper.setSpeed(stepper_maxSpeed);
  
}

void loop() {
  config_page();
  prepareTask();
  run_stepper();
}

/*
    The Moving Process is running in this function. 
    The Function Exit When Pressing a buttom or Slider Hit the edge.
    para:   steps total number of steps;
*/
void run_stepper(){
  long totalStep = abs(task.Steps);
  char dir = 0;
  char exitFlag = 0;
  int cmd;
  if(task.Steps>0) dir = 1;
  else dir = -1;
  
  while(totalStep > 0){
    mystepper.step(8 * dir);
    totalStep = totalStep - 8;
    if(!digitalRead(KEY)){    //限位开关触发
      exitFlag = 1;
    }
    
    if (irrecv.decode(&results)) {    //红外接收到信息
      Serial.println(results.value, HEX);
      irrecv.resume(); // Receive the next value
      cmd = 0xffff & results.value;
      if(cmd != 0xffff){    //红外消息匹配
        cmd = IRRemote_decode(cmd);
        switch(cmd){
          case 17:
            task.Speed *= 0.9;
            break;
          case 18:
            task.Speed *= 1.1;
            break;
          case 19:
            exitFlag = 1;
            break;
        }
        task.Speed = task.Speed > stepper_maxSpeed ? task.Speed : stepper_maxSpeed;
        mystepper.setSpeed(task.Speed);
      }
    }
    if(exitFlag) break;
  }
  if(exitFlag){
    OLED_Write_Str16X8(3,0,"Task Not Finish!");
  }else{
    OLED_Write_Str16X8(3,0,"<---Finished--->");
  }
  delay(5000);
}

/*
  可以设置：
  模式：1走完全程 2走完指定路程 （range:full specific)
  速度: set by time / set by speed
  方向: direction 
  系统设置：calibrate
*/
void config_page(){
  char pageNum = 1;
  unsigned char cursorPos[2] = {0,0};
  int command;
  unsigned char screenBuf[3][8] = {0};
  unsigned char displayBuf[3][8] = {0};
  char curPos[2] = {0,0};
  char nextNumFlag = 0, exitFlag = 0;
  char i,j;
  
  OLED_Write_Str16X8(0,0,"<----CONFIG---->");
  OLED_Write_Str16X8(1,0,"LEN :   o+00000 ");
  OLED_Write_Str16X8(2,0,"TIME:   o+00000 ");
  OLED_Write_Str16X8(3,0,"CALI:   o+00000 ");
  
  while(1){
    command = 0xffff;
    while(command == 0xffff){
      if (irrecv.decode(&results)) {    //红外接收到信息
        command = 0xffff & results.value;
        irrecv.resume(); // Receive the next value
      }
    }
    
    command = IRRemote_decode(command);
    mystepper.setSpeed(stepper_maxSpeed);
    
    if(command < 10){
      screenBuf[curPos[1]][curPos[0]] = command;
      nextNumFlag = 1;
    }else{
      switch(command){
        case 11:
          curPos[1]--;
          if(curPos[1] < 0) curPos[1] = 2;
          break;
        case 12:
          curPos[1]++;
          if(curPos[1] >2) curPos[1] = 0;
          break;
        case 13:
          curPos[0]--;
          if(curPos[0] < 0) curPos[0] = 6;
          break;
        case 14:
          curPos[0]++;
          if(curPos[0] > 6) curPos[0] = 0;
          break;
        case 16:
          exitFlag = 1;
          break;
        case 17:
          do{
            mystepper.step(128);
            command = 0;
            if (irrecv.decode(&results)) {    //红外接收到信息
              command = 0xffff & results.value;
              irrecv.resume(); // Receive the next value
            }
          }while(command == 0xffff);
          break;
        case 18:
          do{
            mystepper.step(-128);
            command = 0;
            if (irrecv.decode(&results)) {    //红外接收到信息
              command = 0xffff & results.value;
              irrecv.resume(); // Receive the next value
            }
          }while(command == 0xffff);
          break;
        default:
          break;
      }
    }
    
    for(i=0;i<3;i++){
      if(screenBuf[i][0]%2 == 0){
        displayBuf[i][0] = 'o';
      }else{
        displayBuf[i][0] = 'x';
      }
      if(screenBuf[i][1]%2 == 0){
        displayBuf[i][1] = '+';
      }else{
        displayBuf[i][1] = '-';
      }
      for(j=0;j<5;j++){
        displayBuf[i][j+2] = screenBuf[i][j+2] + '0';
      }
      OLED_Write_Str16X8(i+1,8,displayBuf[i]);
    }
    
    if(nextNumFlag){
      nextNumFlag = 0;
      curPos[0]++;
      if(curPos[0] > 6){
        curPos[0] = 0;
        curPos[1]++;
        if(curPos[1] > 2) curPos[1] = 0;
      }
    }
    
    OLED_Write_Byte16X8_F(curPos[1] + 1, curPos[0] + 8, displayBuf[curPos[1]][curPos[0]]);
    
    
    if(exitFlag) break;
    
    command = 0xffff;
  }
  
  task.Seconds = 0;
  task.Speed = 0;
  task.Steps = 0;
  task.Length = 0;
  task.Cali = 0;
  task.Reset = 0;
  
  if(screenBuf[0][0]%2 == 0){
    task.Length += screenBuf[0][2]*10000 + screenBuf[0][3]*1000 + screenBuf[0][4]*100 + screenBuf[0][5]*10 + screenBuf[0][6];
  }else{
    task.Steps = total_Step;
  }
  if(screenBuf[0][1]%2 == 1){
    task.Length *= -1;
    task.Steps *= -1;
  }
  
  if(screenBuf[1][0]%2 == 0){
    task.Seconds = screenBuf[1][2]*10000 + screenBuf[1][3]*1000 + screenBuf[1][4]*100 + screenBuf[1][5]*10 + screenBuf[1][6];
  }
  
  if(screenBuf[2][0]%2 == 0){
    task.Cali = 0;
  }else{
    task.Cali = 1;
  }
  
  if(screenBuf[2][1] != 0){
    task.Reset = 1;
  }else{
    task.Reset = 0;
  }
  
}

/*
    设置一切参数供run_stepper调用
    计算和设置速度
    计算和设置总路程
    滑块归位
*/
void prepareTask(){
  float tempSpd = 0.0;
  if(task.Steps == 0){
    task.Steps = ((long)task.Length) * 128;
  }
  
  if(task.Speed == 0){
    tempSpd = 1.0 * task.Seconds / task.Steps;
    task.Speed = abs((long)(tempSpd * 1000000));
  }
  
  if(task.Cali == 1){
    task.Cali = 0;
    calibrate_range();
  }
  
  Serial.print("Seconds:");
  Serial.println(task.Seconds);
  Serial.print("Length:");
  Serial.println(task.Length);
  Serial.print("Steps:");
  Serial.println(task.Steps);
  Serial.print("Speed:");
  Serial.println(task.Speed);
  
  OLED_Full(0);
  OLED_Write_Str16X8(0,0,"<-Almost There->");
  OLED_Write_Str16X8(1,0,"<---Reseting--->");
  
  if(task.Reset){
    if(task.Steps >= 0){
      reset_Slider(0);
    }else{
      reset_Slider(1);
    }
  }
  
  mystepper.setSpeed(task.Speed);
  OLED_Full(0);
  OLED_Write_Str16X8(0,0,"<--I'm Moving-->");
  OLED_Write_Str16X8(1,0,"<----Slowly---->");
  
}

/*
  1、显示欢迎页面
  2、滑块归位
  3、读取行程信息
*/
void systemInit(){
  mystepper.setSpeed(stepper_maxSpeed);
  reset_Slider(0);
}

/*
  将滑块位置复位 0为靠近电机 1为远离电机
*/
void reset_Slider(char dir){
  char blockFlag = 0;
  int tempStepCount = 0;
  char metaStep = (dir == 0 ? -8 : 8);
  mystepper.setSpeed(STEPPER_MAXSPEED);
  Serial.println("reseting Range......");
  while(!digitalRead(KEY)){
    mystepper.step(metaStep);
    tempStepCount += 8;
    if(tempStepCount > 512){
      blockFlag = 1;
      break;
    }
  }
  Serial.print(1);
  if(!blockFlag){
    tempStepCount = 0;
    while(digitalRead(KEY)){
      mystepper.step(metaStep);
    }
  }
  Serial.print(2);
  metaStep *= -1;
  while(!digitalRead(KEY)){
    mystepper.step(metaStep);
    Serial.println(digitalRead(KEY));
  }
  Serial.print(3);
}

void calibrate_range(){
  reset_Slider(0);
  long range = 0;
  Serial.println("Calibrating...");
  while(digitalRead(KEY)){
    mystepper.step(8);
    range += 8;
  }
  total_Step = range;
  
  Serial.println("Calibrate Finished!");
  
}


/*
  红外解码部分
*/
int IRRemote_decode(int cod){
  switch(cod){
    case 0x08F7:
      return 1;
    case 0x8877:
      return 2;
    case 0x48B7:
      return 3;
    case 0x28D7:
      return 4;
    case 0xA857:
      return 5;
    case 0x6897:
      return 6;
    case 0x18E7:
      return 7;
    case 0x9867:
      return 8;
    case 0x58A7:
      return 9;
    case 0x30CF:
      return 0;   
    case 0x807F:
      return 11;    //上
    case 0x906F:
      return 12;    //下
    case 0x20DF:
      return 13;    //左
    case 0x609F:
      return 14;    //右
    case 0x40BF:
      return 15;    //退格
    case 0xA05F:
      return 16;    //确认
    case 0x50AF:
      return 17;    //加速
    case 0x10EF:
      return 18;    //减速
    case 0x00FF:
      return 19;    //急停
    case 0xFFFF:
      return 99;
  }
}
