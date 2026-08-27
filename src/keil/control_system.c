#include "control_system.h"

// PID 参数
// control_system.c
#define KP_A 25.0f      // 比例系数
#define KI_A 1.5f       // 积分系数
#define KD_A 0.3f       // 微分系数

#define KP_B 25.0f
#define KI_B 1.5f
#define KD_B 0.3f
// PID 变量
static int last_error_A = 0;
static int integral_A = 0;
static int last_error_B = 0;
static int integral_B = 0;

// 限制 PWM 输出范围
#define PWM_MAX 5000
#define PWM_MIN -5000

// ★★★ 新增：初始 PWM 值，让电机一开始就有动力 ★★★
#define INITIAL_PWM 2000  // 初始占空比，让电机立刻转起来

int Incremental_PI_A(int Encoders_A, int Target_A)
{
    int error;
    int output;
    
    error = Target_A - Encoders_A;
    integral_A += error;
    
    if (integral_A > 500) integral_A = 500;
    if (integral_A < -500) integral_A = -500;
    
    output = KP_A * error + KI_A * integral_A + KD_A * (error - last_error_A);
    last_error_A = error;
    
    // ★★★ 关键修改：如果输出太小，加一个初始值 ★★★
    if (Target_A != 0) {  // 有目标速度时
        if (output > 0 && output < INITIAL_PWM) {
            output = INITIAL_PWM;  // 正转时给最小启动值
        } else if (output < 0 && output > -INITIAL_PWM) {
            output = -INITIAL_PWM; // 反转时给最小启动值
        }
    }
    
    if (output > PWM_MAX) output = PWM_MAX;
    if (output < PWM_MIN) output = PWM_MIN;
    
    return output;
}

int Incremental_PI_B(int Encoders_B, int Target_B)
{
    int error;
    int output;
    
    error = Target_B - Encoders_B;
    integral_B += error;
    
    if (integral_B > 500) integral_B = 500;
    if (integral_B < -500) integral_B = -500;
    
    output = KP_B * error + KI_B * integral_B + KD_B * (error - last_error_B);
    last_error_B = error;
    
    // ★★★ 同样的修改 ★★★
    if (Target_B != 0) {
        if (output > 0 && output < INITIAL_PWM) {
            output = INITIAL_PWM;
        } else if (output < 0 && output > -INITIAL_PWM) {
            output = -INITIAL_PWM;
        }
    }
    
    if (output > PWM_MAX) output = PWM_MAX;
    if (output < PWM_MIN) output = PWM_MIN;
    
    return output;
}

void System_Control(void)
{
    // 留空
}