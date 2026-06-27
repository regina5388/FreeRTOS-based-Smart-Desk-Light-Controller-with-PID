#ifndef __PID_H
#define __PID_H



typedef struct
{
    float Kp;
    float Ki;
    float Kd;

    float dt;              // PID update period, unit: seconds
    float target;          // target lux

    float integral;
    float prev_error;

    float output;          // current output, e.g. brightness 0~100

    float output_min;
    float output_max;

    float integral_min;
    float integral_max;

} PID_TypeDef;


/* Basic functions */
void PID_Init(PID_TypeDef *pid,
              float Kp,
              float Ki,
              float Kd,
              float dt,
              float output_min,
              float output_max);

void PID_Reset(PID_TypeDef *pid, float initial_output);

float PID_Update(PID_TypeDef *pid, float measured);


/* Runtime tuning functions using UART etc.*/ 
void PID_SetTarget(PID_TypeDef *pid, float target);

void PID_SetTunings(PID_TypeDef *pid,
                    float Kp,
                    float Ki,
                    float Kd);

void PID_SetOutputLimits(PID_TypeDef *pid,
                         float output_min,
                         float output_max);

void PID_SetIntegralLimits(PID_TypeDef *pid,
                           float integral_min,
                           float integral_max);

void PID_SetDt(PID_TypeDef *pid, float dt);


/* Optional getter functions for debug / print */
float PID_GetError(PID_TypeDef *pid, float measured);
float PID_GetOutput(PID_TypeDef *pid);
float PID_GetTarget(PID_TypeDef *pid);


/* Utility */
float PID_Clamp(float value, float min, float max);


#endif