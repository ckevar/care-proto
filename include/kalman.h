#ifndef KALMAN_H
#define KALMAN_H 
class Kalman1Linear
{
public:
	Kalman1Linear();
	// ~kalman();
	void setR(double R); // process error
	void setP0(double p);
	void setX0(double x);
	void predict();
	double update(double Z);
private:
	double m_R;		// process error
	double m_Xhat;	// state
	double m_p;		// predition error
	double m_K; 	// kalman gain
	double m_ep; 	// influence of error estimation
};

#endif