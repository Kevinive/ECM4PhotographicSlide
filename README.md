# ECM4PhotographicSlide
A Self Designed Electronic Controller Module for Photographic Slide

一个摄像滑轨的电控模块设计项目。

### 0. 项目内容

在这个项目中包含：

1. 电路设计方案
2. 嵌入式软件源码

### 1. 项目简述

该项目基于Atmega 328P-au芯片、使用Arduino Bootloader平台开发，实现了电动滑轨的参数显示、设置和延时任务执行的相关功能。

#### 1.1 滑轨简介

滑轨上的滑块由步进电机控制，滑块行程边缘装有微动开关限位器，控制器能自行校准滑轨行程。

控制器通过红外模块接收遥控器发来的信号，执行相应指令；控制器状态会在OLED屏幕上显示出来。

#### 1.2 功能简介

滑轨主要功能在于延时摄影和平滑移动。

从控制器中可以设定滑轨的移动方向、移动行程与移动速度/时间。

可以设定行程与拍摄时间，也可以设定行程与移动速度。

### 2. 文件功能

#### src/

ElectricSlider.ino：主程序，包含主要控制逻辑

MYStepper.cpp/.h：步进电机驱动程序

OLED.cpp/.h：OLED驱动程序

CodeTable.h：显示用字库

#### Circuit/ : 主要放置PCB设计文件





