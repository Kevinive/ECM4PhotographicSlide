#ifndef __INCLUDE_H
#define __INCLUDE_H

#include "Arduino.h"
#include <avr/pgmspace.h>
#include <Wire.h>
#include <EEPROM.h>
#include "CodeTable.h"
#include "OLED.h"
#include <IRremote.h>
#include "MYStepper.h"

//设备信息定义部分
#define IR_RECV_PIN         1
#define KEY                  2
#define STEPPER_S_P_R       8
#define STEPPER_PIN1        6
#define STEPPER_PIN2        7
#define STEPPER_PIN3        8
#define STEPPER_PIN4        9
#define STEPPER_MAXSPEED    1200
#define STEPPER_MAXACC      12
#define TOTAL_STEP          100000L
#define OLED_SCL            16
#define OLED_SDA            17
#define OLED_ADDR           0X78


#endif

