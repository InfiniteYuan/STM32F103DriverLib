#include "Control.h"

Control::Control(PWM &Moto) :mMoto(Moto)
{
	OldTime = 0;

	//积分限幅
	pitch_angle_PID.iLimit = 5;
	roll_angle_PID.iLimit = 5;

	pitch_rate_PID.iLimit = 30;
	roll_rate_PID.iLimit = 30;
	//总输出限幅
	pitch_angle_PID.OLimit = 50;
	roll_angle_PID.OLimit = 50;

	FlyThr = 20;

}


//读取PID
bool Control::ReadPID(flash info, u16 Page, u16 position)
{
	u16 data[9];
	if (!info.Read(Page, position, data, 9))
		return false;

	SetPID_PIT(data[0] / 1000.0, data[1] / 1000.0, data[2] / 1000.0);
	SetPID_ROL(data[3] / 1000.0, data[2] / 1000.0, data[5] / 1000.0);
	SetPID_YAW(data[6] / 1000.0, data[7] / 1000.0, data[8] / 1000.0);

	return true;

}

//保存PID
bool Control::SavePID(flash info, u16 Page, u16 position)
{
	u16 data[9];
	data[0] = pitch_angle_PID.P * 1000;
	data[1] = pitch_angle_PID.I * 1000;
	data[2] = pitch_angle_PID.D * 1000;

	data[3] = roll_angle_PID.P * 1000;
	data[4] = roll_angle_PID.I * 1000;
	data[5] = roll_angle_PID.D * 1000;

	data[6] = yaw_angle_PID.P * 1000;
	data[7] = yaw_angle_PID.I * 1000;
	data[8] = yaw_angle_PID.D * 1000;

	info.Clear(Page);
	if (!info.Write(Page, position, data, 9))
		return false;
	return true;
}

bool Control::PIDControl(Vector3f angle, Vector3<float> gyr, u16 RcThr, u16 RcPit, u16 RcRol, u16 PcYaw)
{

	//规范化接收的遥控器值  1000 - 2000  平衡位置度量在50内
	if (RcThr < 1000) RcThr = 1000;
	if (RcThr > 2000) RcThr = 2000;
	if (RcPit < 1000)	 RcPit = 1000;
	if (RcPit > 2000)	 RcPit = 2000;
	if (RcPit > 1450 && RcPit < 1550) RcPit = 1500;
	if (RcRol < 1000)	 RcRol = 1000;
	if (RcRol > 2000)	 RcRol = 2000;
	if (RcRol>1450 && RcRol < 1550) RcPit = 1500;
	if (PcYaw < 1000)	 PcYaw = 1000;
	if (PcYaw > 2000)	 PcYaw = 2000;
	if (PcYaw>1450 && PcYaw < 1550) PcYaw = 1500;

	float MOTO1 = 0, MOTO2 = 0, MOTO3 = 0, MOTO4 = 0;
	float Thr = (RcThr - 1000) / 10; //将接收到的油门量转化为百分比
	float TargetRoll = (RcRol - 1500)*0.08; //遥控器控制在+-40°
	float TargetPitch = (RcRol - 1500)*0.08;


	//计算时间间隔
	if (OldTime == 0)
		TimeInterval = 0.008;
	else
		TimeInterval = tskmgr.Time() - OldTime;
	OldTime = tskmgr.Time();


	//PITCH轴		
	//比例
	pitch_angle_PID.Error = TargetPitch - angle.y; //期望角度减去当前角度,这里将遥控器范围规定在+-20°
	pitch_angle_PID.Proportion = pitch_angle_PID.P * pitch_angle_PID.Error; // 区间在 P*20
	//积分
	if (Thr > FlyThr) //大于起飞油门时才开始积分
		pitch_angle_PID.CumulativeError += pitch_angle_PID.Error *TimeInterval;
	pitch_angle_PID.Integral = pitch_angle_PID.I * pitch_angle_PID.CumulativeError;
	//积分限幅  的油门量
	if (pitch_angle_PID.Integral > pitch_angle_PID.iLimit)
		pitch_angle_PID.Integral = pitch_angle_PID.iLimit;
	if (pitch_angle_PID.Integral < -pitch_angle_PID.iLimit)
		pitch_angle_PID.Integral = -pitch_angle_PID.iLimit;
	//微分
	pitch_angle_PID.Differential = -pitch_angle_PID.D * gyr.y;//  下偏陀螺仪为正，上偏为负
	pitch_angle_PID.Output = pitch_angle_PID.Proportion + pitch_angle_PID.Integral + pitch_angle_PID.Differential;

	//PID总和限幅
	//		if(pitch_angle_PID.Output >pitch_angle_PID.OLimit)
	//			pitch_angle_PID.Output=pitch_angle_PID.OLimit;
	//		if(pitch_angle_PID.Output<-pitch_angle_PID.OLimit)
	//			pitch_angle_PID.Output=-pitch_angle_PID.OLimit;

	MOTO1 = Thr - pitch_angle_PID.Output;
	MOTO3 = Thr + pitch_angle_PID.Output;

	//ROLL轴
	//思考例子：当前-20度，也就是飞机向左偏了，目标是0度，误差就是20，由于向0度运动时陀螺仪是正数，于是微分项添加一个负号
	//要想回到0，MOTO2要减速,MOTO4要加速
	//比例
	roll_angle_PID.Error = TargetRoll - angle.x;
	roll_angle_PID.Proportion = roll_angle_PID.P *roll_angle_PID.Error;
	//积分
	if (Thr > FlyThr)
		roll_angle_PID.CumulativeError += roll_angle_PID.Error *TimeInterval;
	roll_angle_PID.Integral = roll_angle_PID.I * roll_angle_PID.CumulativeError;
	//积分限幅
	if (roll_angle_PID.Integral > roll_angle_PID.iLimit)
		roll_angle_PID.Integral = roll_angle_PID.iLimit;
	if (roll_angle_PID.Integral < -roll_angle_PID.iLimit)
		roll_angle_PID.Integral = -roll_angle_PID.iLimit;
	//微分
	roll_angle_PID.Differential = -roll_angle_PID.D * gyr.x;//微分  左偏为负 右偏为正

	roll_angle_PID.Output = roll_angle_PID.Proportion + roll_angle_PID.Integral + roll_angle_PID.Differential;

	//PID总和限幅
	//		if(roll_angle_PID.Output >roll_angle_PID.OLimit)
	//			roll_angle_PID.Output=roll_angle_PID.OLimit;
	//		if(roll_angle_PID.Output<-roll_angle_PID.OLimit)
	//			roll_angle_PID.Output=-roll_angle_PID.OLimit;

	MOTO2 = Thr - roll_angle_PID.Output;
	MOTO4 = Thr + roll_angle_PID.Output;

	//输出
	if (MOTO1 < 0)
		MOTO1 = 0;
	if (MOTO2 < 0)
		MOTO2 = 0;

#ifdef DUBUG_PITCH
	if (Thr < FlyThr)
		mMoto.SetDuty(Thr, 0, Thr, 0);
	else
		mMoto.SetDuty(MOTO1, 0, MOTO3, 0);
#endif

#ifdef DUBUG_ROLL
	if (Thr < FlyThr)
		mMoto.SetDuty(0, Thr, 0, Thr);
	else
		mMoto.SetDuty(0, MOTO2, 0, MOTO4);
#endif

#ifdef NORMAL
	if (Thr < FlyThr)
		mMoto.SetDuty(Thr, Thr, Thr, Thr);
	else
		mMoto.SetDuty(MOTO1, MOTO2, MOTO3, MOTO4);
#endif

	return true;
}

bool Control::SeriesPIDComtrol(Vector3f angle, Vector3<float> gyr, u16 RcThr, u16 RcPit, u16 RcRol, u16 PcYaw)
{
	//角度环（外环）
	float TargetRcThr = (RcThr - 1000) / 10;
	float TargetPitch = (RcPit - 1500)*0.08; //期望角度控制在+-40°
	float TargetRoll = (RcRol - 1500)*0.08;

	//计算时间间隔
	if (OldTime == 0)
		TimeInterval = 0.008;
	else
		TimeInterval = tskmgr.Time() - OldTime;
	OldTime = tskmgr.Time();

	TOOL_PID_Postion_Cal(&pitch_angle_PID, TargetPitch, angle.y, TargetRcThr, TimeInterval);
	TOOL_PID_Postion_Cal(&roll_angle_PID, TargetRoll, angle.x, TargetRcThr, TimeInterval);

	//角速度环（内环）
	TOOL_PID_Postion_Cal(&pitch_rate_PID, pitch_angle_PID.Output, gyr.y, TargetRcThr, TimeInterval);
	TOOL_PID_Postion_Cal(&roll_rate_PID, roll_angle_PID.Output, gyr.x, TargetRcThr, TimeInterval);

	//	TOOL_PID_Postion_Cal(&roll_rate_PID,TargetRoll,gyr.x,TargetRcThr,TimeInterval);

	//电机控制
	if (TargetRcThr > 10)
		mMoto.SetDuty(0, TargetRcThr - roll_rate_PID.Output, 0, TargetRcThr + roll_rate_PID.Output);
	else
		mMoto.SetDuty(0, TargetRcThr, 0, TargetRcThr);

	return true;
}

bool Control::TOOL_PID_Postion_Cal(PID_Typedef * PID, float target, float measure, float Thr, double dertT)
{
	float tempI = 0; //积分项暂存

	PID->Error = target - measure; //计算误差
	PID->Differential = (PID->Error - PID->LastError) / dertT; //计算微分值
	PID->Output = (PID->P * PID->Error) + (PID->I * PID->Integral) + (PID->D * PID->Differential);  //PID:比例环节+积分环节+微分环节
	PID->LastError = PID->Error;//保存误差


	if (fabs(PID->Output) < Thr) //比油门还大时不积分
	{
		tempI = (PID->Integral) + (PID->Error) * dertT;     //积分环节
		if (tempI > -PID->iLimit && tempI<PID->iLimit &&PID->Output > -PID->iLimit && PID->Output < PID->iLimit)//在输出小于30才累计
			PID->Integral = tempI;
	}
	return true;

}


