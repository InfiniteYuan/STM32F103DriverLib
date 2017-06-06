#ifndef _PID_PARAMETER_H_
#define _PID_PARAMETER_H_

class PIDParameter
{
	public:
		float Kp;
		float Ki;
		float Kd;
	
		float Dt;      //control interval, unit s
		float thetaI;  //the error angle that integrate actived,when greater than this angle, disable integrate 
		float maxI;    //the max control value for integrate
	
		float _P;
		float _I;
		float _D;
	
		float _lstErr;
	
		float _Derr;
	public:
		PIDParameter(float kp=0, float ki=0, float kd=0,float dt=0.002, float angleI=5,float satI=5)
		{ 
			Kp = kp;  
			Ki = ki;  
			Kd = kd; 
			
			Dt = dt;
			thetaI = angleI;
			maxI = satI;
			
			_P = 0;
			_I = 0;
			_D = 0;
			_lstErr = 0;
			_Derr = 0;
		}
		void Set_PID(float kp=0, float ki=0, float kd=0)
		{
			Kp = kp;  
			Ki = ki;  
			Kd = kd; 
		}
		void operator()(float kp, float ki, float kd,float dt, float angleI, float satI)
		{
			Kp = kp;  
			Ki = ki;  
			Kd = kd; 
			
			Dt = dt;
			thetaI = angleI;
			maxI = satI;
		}
		
		float ComputePID(float target, float now)
		{
			float err = target - now;
			
			_P = Kp*err;
			
			float Derr = (err - _lstErr)/Dt;
			
			_Derr += 0.02*(Derr - _Derr);
			
			_D = Kd*_Derr;
//			_D = Kd * Derr;
						
			_lstErr = err;
			
			if(__fabs(err)<thetaI)
			{
				_I += Ki*err*Dt;
				if(_I>maxI)  _I = maxI;
				if(_I<-maxI) _I = -maxI;		
			}
			//else
			//	_I = 0;
			return _P + _D + _I;
		}
		
};

#endif


