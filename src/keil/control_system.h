#ifndef __CONTROL_SYSTEM_H
#define __CONTROL_SYSTEM_H

#include "stm32f10x.h"
#include "encoder.h"
#include "motor.h"
#include <stdio.h>

void System_Control(void);
int Rs_To_CPR(float rads);
int Incremental_PI_A(int Encoders_A, int Target_A);
int Incremental_PI_B(int Encoders_B, int Target_B);

extern int L_coder;
extern int R_coder;
extern int Motor_A;
extern int Motor_B;

#endif