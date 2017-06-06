#include "StepMotor.h"
#include "USART.h"

extern USART com;

bool StepMotor::bInitSPWM = false;
float StepMotor::SPWM[256] = {0};


void StepMotor::InitSPWM()
{
	for(int i=0;i<256;i++)
	{
		SPWM[i] = sin(i*2.0*3.1415926/256.0);
	}
}

StepMotor::StepMotor(PWM *pwma, u8 cha, PWM *pwmb, u8 chb, PWM *pwmc, u8 chc, float power)
{
	_pwm_a = pwma;
	_pwm_b = pwmb;
	_pwm_c = pwmc;
	_ch_a = cha;
	_ch_b = chb;
	_ch_c = chc;
	_maxPower = power;
	_armed = false;
	if(bInitSPWM) return;
	bInitSPWM = true;
	InitSPWM();
}

void StepMotor::Initialize(PWM *pwma, u8 cha, PWM *pwmb, u8 chb, PWM *pwmc, u8 chc, float power)
{
	_pwm_a = pwma;
	_pwm_b = pwmb;
	_pwm_c = pwmc;
	_ch_a = cha;
	_ch_b = chb;
	_ch_c = chc;
	_maxPower = power;

}


void StepMotor::SetPosition(int position)
{
	if(!_armed) return;
	float a,b,c;
	
	u16 pos = position & 0xFF;
	
	a = SPWM[pos%256];
	b = SPWM[(pos+85)%256];
	c = SPWM[(pos+170)%256];

	a = (_maxPower*a + 1.0)/2.0; 
	b = (_maxPower*b + 1.0)/2.0; 
	c = (_maxPower*c + 1.0)/2.0; 
	
	_pwm_a->SetDuty(_ch_a,a*100);
	_pwm_b->SetDuty(_ch_b,b*100);
	_pwm_c->SetDuty(_ch_c,c*100);
}
void StepMotor::Enable()
{
	_armed = true;
}
void StepMotor::Disable()
{
	_armed = false;
	_pwm_a->SetDuty(_ch_a,0);
	_pwm_b->SetDuty(_ch_b,0);
	_pwm_c->SetDuty(_ch_c,0);
}


