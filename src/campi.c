#include "campi.h"
#include "definitions.h"

// #include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"


#include <stdio.h>

static void video_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
	static int frame_count = 0;
	static int frame_post_count = 0;
	static struct timespec t1;
	struct timespec t2;
	MMAL_BUFFER_HEADER_T *new_buffer;
	PORTUSERDATA_T * userdata = (PORTUSERDATA_T *) port->userdata;
	MMAL_POOL_T *pool = userdata->camVideoPortPool;

	if (frame_count == 0)
		clock_gettime(CLOCK_MONOTONIC, &t1);

	frame_count++;

	mmal_buffer_header_mem_lock(buffer);
	memcpy(userdata->image, buffer->data, userdata->video_width * userdata->video_height);
	mmal_buffer_header_mem_unlock(buffer);
	
	if (sem_trywait(&userdata->outFrameReady) < 0) {
		sem_post(&userdata->outFrameReady);
		frame_post_count++;
	}

	if (frame_count % 10 == 0) {
		// print framerate every n frame
		clock_gettime(CLOCK_MONOTONIC, &t2);
		float d = (t2.tv_sec + t2.tv_nsec / 1000000000.0) - (t1.tv_sec + t1.tv_nsec / 1000000000.0);
		float fps = 0.0;

		if (d > 0) {
			fps = frame_count / d;
		} else {
			fps = frame_count;
		}
		userdata->video_fps = fps;
		// printf("  Frame = %d, Frame Post %d, Framerate = %.0f fps \n", frame_count, frame_post_count, fps);
	}

	mmal_buffer_header_release(buffer);
	// and send one back to the port (if still open)
	if (port->is_enabled) {
		MMAL_STATUS_T status;
	
		new_buffer = mmal_queue_get(pool->queue);

		if (new_buffer)
			status = mmal_port_send_buffer(port, new_buffer);

		if (!new_buffer || status != MMAL_SUCCESS)
			fprintf(stderr, "[warning] Unable to return a buffer to the video port\n");

	}

}


CamPi::CamPi(PORTUSERDATA_T *usrdata) {
	m_usr = usrdata;
	m_cam = 0;
	m_preview = 0;
	m_camPreviewConn = 0;
}

int CamPi::initCam() {
	MMAL_PORT_T *camPreviewPort = NULL, *camVideoPort = NULL; // ports that can be connected to the components
	MMAL_POOL_T *videoPool;

	int rt;

	/** CREATION **/
	m_status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &m_cam);
	if (m_status != MMAL_SUCCESS) {
		fprintf(stderr, "[error] creating camera %x\n", m_status);
		return -1;
	}
	// Connecting port to components
	camPreviewPort = m_cam->output[MMAL_CAMERA_PREVIEW_PORT];
	camVideoPort = m_cam->output[MMAL_CAMERA_VIDEO_PORT];

	/** SETTINGS **/
	// -> camera component
	{
		MMAL_PARAMETER_CAMERA_CONFIG_T cam_config = {
			{ MMAL_PARAMETER_CAMERA_CONFIG, sizeof (cam_config)},
			.max_stills_w = m_usr->video_width,
			.max_stills_h = m_usr->video_height,
			.stills_yuv422 = 0,		// still capture disabled (I assume)
			.one_shot_stills = 0, 	// continues shooting disabled (I assume)
			.max_preview_video_w = m_usr->video_width,
			.max_preview_video_h = m_usr->video_height,
			.num_preview_video_frames = 2,
			.stills_capture_circular_buffer_height = 0, // since the still capture is not enabled, this one either
			.fast_preview_resume = 1, 	// Allows fast preview resume, when a still frame is captured and then processed
			.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC // Timestamp of First captured frame is zero, and then STC continues from that
		};

		mmal_port_parameter_set(m_cam->control, &cam_config.hdr);	// setting the parameters
	}

	// -> video port 
	rt = portSettings(camVideoPort, MMAL_ENCODING_I420, m_usr->video_width, m_usr->video_height, 1);
	if(rt < 0) return -1;
	
	// -> Preview port 
	rt = portSettings(camPreviewPort, MMAL_ENCODING_OPAQUE, m_usr->preview_width, m_usr->preview_height, 0);
	if (rt < 0) return -1;

	// -> pool of buffers for video port
	videoPool = (MMAL_POOL_T *) mmal_port_pool_create(camVideoPort, camVideoPort->buffer_num, camVideoPort->buffer_size);
	m_usr->camVideoPortPool = videoPool;
	camVideoPort->userdata = (struct MMAL_PORT_USERDATA_T *) m_usr;

	/** ENABLING **/
	// -> port 
	m_status = mmal_port_enable(camVideoPort, video_buffer_callback);
	if (m_status != MMAL_SUCCESS) {
		fprintf(stderr, "[error] enabling camera video port (%u)\n", m_status);
		return -1;
	}
	// -> camera
	m_status = mmal_component_enable(m_cam);	
	return 0;
}

int CamPi::initRender() {
	MMAL_PORT_T *previewInPort = NULL; 		// component

	/** CREATION **/
	m_status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &m_preview);
	if (m_status != MMAL_SUCCESS){
		fprintf(stderr, "[error] creating render (%u)\n", m_status);
		return -1;
	}
	previewInPort = m_preview->input[0];

	/** SETTINGS **/
	{
		MMAL_DISPLAYREGION_T param;
		param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
		param.hdr.size = sizeof (MMAL_DISPLAYREGION_T);
		param.set = MMAL_DISPLAY_SET_LAYER;
		param.layer = 0;
		param.set |= MMAL_DISPLAY_SET_FULLSCREEN;
		param.fullscreen = 1;
		m_status = mmal_port_parameter_set(previewInPort, &param.hdr);
		if (m_status != MMAL_SUCCESS && m_status != MMAL_ENOSYS) {
			fprintf(stderr, "[error]: set preview port parameters (%u)\n", m_status);
			return -1;
		}
	}

	return 0;
}

int CamPi::plugRender2Cam(){
	MMAL_PORT_T *camPreviewPort = m_cam->output[MMAL_CAMERA_PREVIEW_PORT];
	MMAL_PORT_T *previewInPort = m_preview->input[0];

	// -> create connection
	m_status = mmal_connection_create(&m_camPreviewConn, camPreviewPort, previewInPort, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
	if (m_status != MMAL_SUCCESS) {
		fprintf(stderr, "[error] creating connection (%u)\n", m_status);
		return -1;
	}
	
	// -> enable connection
	m_status = mmal_connection_enable(m_camPreviewConn);
	if (m_status != MMAL_SUCCESS) {
		printf("[error] enabling connection (%u)\n", m_status);
		return -1;
	}	
	return 0;
}

void CamPi::loadBuffers() {

	int num = mmal_queue_length(m_usr->camVideoPortPool->queue);
	int q;
	MMAL_PORT_T *camVideoPort = NULL; // ports that can be connected to the components
	camVideoPort = m_cam->output[MMAL_CAMERA_VIDEO_PORT];


	for (q = 0; q < num; q++) {
		MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(m_usr->camVideoPortPool->queue);

		if (!buffer)
			fprintf(stderr, "[warning] Unable to get a required buffer %d from pool queue\n", q);

		if (mmal_port_send_buffer(camVideoPort, buffer) != MMAL_SUCCESS)
			printf("[warning] Unable to send a buffer to encoder output port (%d)\n", q);
	}

	if (mmal_port_parameter_set_boolean(camVideoPort, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
		printf("[warning] %s: Failed to start capture\n", __func__);
}

int CamPi::portSettings(MMAL_PORT_T* port, unsigned encoder, int w, int h, int addSet) {
	MMAL_ES_FORMAT_T *format;
	format = port->format;

	format->encoding = encoder;		// raw data, this is for opencv processing purposes
	format->encoding_variant = MMAL_ENCODING_I420; 	// variation of that encoder? nop, use raw data.

	format->es->video.width = w;
	format->es->video.height = h;
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = w;
	format->es->video.crop.height = h;

	if (addSet) {
		format->es->video.frame_rate.num = 30;
		format->es->video.frame_rate.den = 1;
		port->buffer_size = w * h * 12 / 8; // 1.5 times of the first channel
		port->buffer_num = 1; 				// how many buffers of that size will be used
	}

	m_status = mmal_port_format_commit(port); // load settings
	if (m_status != MMAL_SUCCESS) {
		fprintf(stderr, "[error] commiting camera video port format (%u)\n", m_status);
		return -1;
	}	
	return 0;
}

void CamPi::coronavirus() {
	mmal_connection_destroy(m_camPreviewConn);
	mmal_component_destroy(m_cam);
	mmal_component_destroy(m_preview);
	mmal_pool_destroy(m_usr->camVideoPortPool);
}
