#ifndef _AHRS_DCM_H_
#define _AHRS_DCM_H_

#include "stm32f10x.h"
#include "AHRS.h"
#include "Mathtool.h"

class AHRS_DCM:public AHRS
{
private:
	Matrix3<float> _dcm_matrix;
	Vector3f _omega;
	Vector3f _omega_I;
	Vector3f _omega_P;
	Vector3f _omega_yaw;
	Vector3f _acc_ef;
  Vector3f _ra_sum;
	float _ra_deltat;
	
public:
	AHRS_DCM(InertialSensor &ins,Compass *compass=0, Barometer *baro=0);
	virtual bool Update(); 
  virtual bool Ready();
	void UpdateMatrix();
	void DriftCorrection();
};

#endif

