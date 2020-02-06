#include "kalman.h"

Kalman1Linear::Kalman1Linear() {
	m_R = 1.0; 		// deafult error
	m_Xhat = 0.0;	// initial state
	m_p = 1.0;		// default prediction error
	m_ep = 1.001; // keep this as small as possible
}

void Kalman1Linear::predict() {
}
void Kalman1Linear::setP0(double p) { 
	m_p = p;
}
void Kalman1Linear::setR(double R) {
	m_R = R;
}
void Kalman1Linear::setX0(double x) {
	m_Xhat = x;
}
double Kalman1Linear::update(double Z) {
	m_K = m_p / (m_p + m_R);
	m_Xhat += m_K * (Z - m_Xhat);
	m_p = (1 - m_K) * m_p * m_ep;
	return m_Xhat;
}

