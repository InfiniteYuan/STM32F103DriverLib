#ifndef _VECTOR4_H_
#define _VECTOR4_H_



class CraftVector
{
public:
	float roll;
	float pitch;
	float yaw;
	float throttle;
public:
	CraftVector() {roll=0; pitch=0; yaw=0; throttle=0; }
	void operator()(float r,float p,float y,float t){ roll=r; pitch=p; yaw=y; throttle=t;}
};


#endif


