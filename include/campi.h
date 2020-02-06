#ifndef CAM_PI_H
#define CAM_PI_H 

#include "types.h"

class CamPi {
public:
	CamPi(PORTUSERDATA_T *usr);
	
	int initCam();
	int initRender();
	int plugRender2Cam();
	void loadBuffers();
	void coronavirus();
	// ~CamPi();

private:
	PORTUSERDATA_T 		*m_usr;			// userdata
	MMAL_COMPONENT_T 	*m_cam;			// camera
	MMAL_CONNECTION_T 	*m_camPreviewConn; // To connect 2 ports
	MMAL_COMPONENT_T 	*m_preview;	 	// Preview component
    MMAL_STATUS_T 		m_status;		// status of cofigurations for mmal

	int portSettings(MMAL_PORT_T* port, unsigned encoder, int w, int h, int addSet);

};

#endif

