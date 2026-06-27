#include "pid.h"

void PID_Init(PID_TypeDef *pid,
              float Kp,
              float Ki,
              float Kd,
              float dt,
              float output_min,
              float output_max)
{
    if (pid == 0)
    {
        return;
    }

    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

    pid->dt = dt;

    /* Target can be set later by PID_SetTarget() */
    pid->target = 0.0f;

    /* Internal states */
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output = 0.0f;

    /* Output limits, for your LED brightness 0~100% */
    pid->output_min = output_min;
    pid->output_max = output_max;

    /* Anti-windup integral limits */
    pid->integral_min = -1000.0f;
    pid->integral_max = 1000.0f;
}


float PID_Update(PID_TypeDef *pid, float measured)
{
    float error;
    float p_term;
    float i_term;
    float d_term;
    float delta;
    float output;

    if (pid == 0)
    {
        return 0.0f;
    }

    error = pid->target - measured;

    p_term = pid->Kp * error;

    pid->integral += error * pid->dt;
    pid->integral = PID_Clamp(pid->integral,
                              pid->integral_min,
                              pid->integral_max);

    i_term = pid->Ki * pid->integral;

    d_term = pid->Kd * (error - pid->prev_error) / pid->dt;

    delta = p_term + i_term + d_term;

    output = pid->output + delta;

    output = PID_Clamp(output,
                       pid->output_min,
                       pid->output_max);

    pid->output = output;
    pid->prev_error = error;

    return pid->output;
}

void PID_Reset(PID_TypeDef *pid, float initial_output)
{
    if (pid == 0)
    {
        return;
    }

    pid->integral = 0.0f;
    pid->prev_error = 0.0f;

    pid->output = PID_Clamp(initial_output,
                            pid->output_min,
                            pid->output_max);
}



void PID_SetDt(PID_TypeDef *pid, float dt)
{
	if (pid == 0)
    {
        return;
    }
	
	pid->dt = dt;
}

void PID_SetTunings(PID_TypeDef *pid,
                    float Kp,
                    float Ki,
                    float Kd)
{
    if (pid == 0)
    {
        return;
    }

    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

}


void PID_SetTarget(PID_TypeDef *pid, float target)
{
	
	if (pid == 0)
    {
        return;
    }
	
	pid->target = target;
}

void PID_SetOutputLimits(PID_TypeDef *pid,
                         float output_min,
                         float output_max)
{
	 if (pid == 0)
    {
        return;
    }
	
	pid->output_min = output_min;
    pid->output_max = output_max;
}

void PID_SetIntegralLimits(PID_TypeDef *pid,
                           float integral_min,
                           float integral_max)
{
	if (pid == 0)
    {
        return;
    }
	
	pid->integral_min = integral_min;
    pid->integral_max = integral_max;
}


float PID_Clamp(float value, float min, float max)
{
	if (value >= max)
	{
		return max;
		
	} else if (value <= min)
	{
		
		return min;
		
	} else 
	{
		return value;
	}

}

float PID_GetError(PID_TypeDef *pid, float measured)
{
    if (pid == 0)
    {
        return 0.0f;
    }

    return pid->target - measured;
}

float PID_GetOutput(PID_TypeDef *pid)
{

	if (pid == 0)
    {
        return 0.0f;
    }
	
	return pid -> output;
}

float PID_GetTarget(PID_TypeDef *pid)
{

	if (pid == 0)
    {
        return 0.0f;
    }
	
	return pid -> target;
}


