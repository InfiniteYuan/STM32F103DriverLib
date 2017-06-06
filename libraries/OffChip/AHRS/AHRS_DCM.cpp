
#include "AHRS_DCM.h"

#include "USART.h"
extern USART com;

AHRS_DCM::AHRS_DCM(InertialSensor &ins,Compass *compass, Barometer *baro):AHRS(ins,compass,baro)
{
	_omega.Zero();
	_omega_I.Zero();
	_omega_P.Zero();
	_omega_yaw.Zero();
	_dcm_matrix.Identity();
}

bool AHRS_DCM::Update()
{
	static u8 cnt = 0;
	
	if(_ins.Update(_acc,_gyro))
	{
		UpdateMatrix();
		if(++cnt>10)
		{
			cnt = 0;
			DriftCorrection();
		}
	}
	if(_compass) _compass->Update(_mag);
	if(_baro) _baro->Update(_pressure);
	return true;
}
void AHRS_DCM::UpdateMatrix()
{
		Vector3f delta = (_gyro + _omega_I + _omega_P + _omega_yaw)*0.002;
		_dcm_matrix.Rotate(delta);
		_dcm_matrix.Normalize();
		_dcm_matrix.ToEuler(&_roll,&_pitch,&_yaw);
}
void AHRS_DCM::DriftCorrection()
{
	Matrix3<float> temp_dcm = _dcm_matrix;
	
	Vector3f GA_b = temp_dcm * _acc;
	//com<<GA_b.x<<","<<GA_b.y<<","<<GA_b.z<<"\n";
	GA_b /= GA_b.Length();
  Vector3f error = GA_b % Vector3f(0,0,-1);
	error = _dcm_matrix.mul_transpose(error);
	//com<<error.x<<","<<error.y<<","<<error.z<<"\n";
	_omega_P = -error*0.9;
	_omega_I += -error*0.02;
}

bool AHRS_DCM::Ready()
{
	return _ins.Ready();
}


