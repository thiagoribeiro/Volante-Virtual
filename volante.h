/*
MyOpenCvObject ocv;
MyOgreObject ogre;

ocv.Init();
ogre.Init();

while (!myApp.quit)
{
    ocv.ProcessOneFrame();
    ogre.PutTrackData(ocv.GetNewTrackResults());
    ogre.Update();
}

 */





/*
 * volante.h
 *
 *  Created on: May 11, 2011
 *      Author: thiago
 */

#ifndef VOLANTE_H_
#define VOLANTE_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>

#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>

#include <pthread.h>


/*NETWORKING*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>
#include <set>
/////////////////////



extern "C" void stereoInit( int w, int h );
extern "C" void stereoUpload( const unsigned char *left, const unsigned char *right );
extern "C" void stereoProcess();
extern "C" void stereoDownload( float *disparityLeft, float *disparityRight );


#define NW_BUFLEN 512
#define NW_NPACK 10
#define NW_PORT 9930


#define NW_READY 1
#define NW_SEND_DATA 2
struct comandos {
	float left_right;
	float accel_break;
	int status;

};

class WorkerThread {
public:
	WorkerThread(std::string threadName) {
		m_threadName = threadName;
		m_error = 0;
		m_running = false;
	}

	~WorkerThread() {
		stop();
	}

	bool start() {
		m_running = true;
		m_error = pthread_create(&m_thread, NULL, _exec, (void *)this);
		if( m_error != 0 ) return false;
	}

	void stop() {
		m_running = false;
	}

	static void *_exec(void *instance) {

		WorkerThread *pThis = reinterpret_cast<WorkerThread *>(instance);
		bool exitRet=true;
		while( (exitRet = pThis->run()) && pThis->isRunning() ) {
			;
		}

		void *ret;
		ret = (void *)(exitRet?0:-1);
		pthread_exit(ret);

	}

	virtual bool run()=0;

	__inline useconds_t isRunning() { return m_running; }

private:

	std::string m_threadName;
	pthread_t m_thread;
	int m_error;
	bool m_running;

};




//sockaddr_in serverAddr;

class Network {

	public:
	int init();
	int send(comandos msg);
	int free();
	int sendMove(float x, float y);


	private:
	sockaddr_in clientAddr;//[10];
	sockaddr clientAddrCast[10];
	int socketId;
	int qtde_clients;
	unsigned int size;
	struct sockaddr_in serverAddr;

};


class HandDetection : public WorkerThread {
	private:
	CvHaarClassifierCascade* cascade;
	CvSize min_size;
	double image_scale;
	double haar_scale;
	int min_neighbors;
	int haar_flags;
	IplImage *imagem,*disparidade;
	CvMemStorage* storage;
	CvSeq* faces;
	double tild_hand; //inclinacao da mao
	CvRect mao1, mao2;
	int maos[100];
	int idMaos;
	float media1,media2, desvio_padrao1,desvio_padrao2;

	int m_num;
	int paused;

	int detect_hands();
	int remove_falses();
	int filter_result();

	public:
	int find_hands(IplImage *imagem);
	int translate(IplImage* disp);
	int printHands(IplImage *img);
	int init(char *cascade_file, int width, int height);
	CvRect getHandRect(int hand);
	int free();

	int is_paused();
	bool run();
	HandDetection(int num) : WorkerThread("HandDetection")	{
		m_num = num;
	}
	int pause();
	int play();


};

class Hand2Move {
	private:
	IplImage *imagem, *disparidade, *background, *img_mao1, *img_mao2, *drawVolante, *rec_mao1, *rec_mao2;
	int x,y;
	int width,height;
	HandDetection* hand;
	double tilt, deep;
	float tilt_angle;
	CvRect mao1, mao2;
	Network* net;


	int hand_process();
	int deep_detect();
	int network_send();


	public:
	int running_hand_detect();
	int hand_detect(IplImage *imagem);
	int step(IplImage *imagem,IplImage *disparidade,IplImage *background);
	int init(int width, int height);
	int drawResult();
	int free();

	int printHands(IplImage *img);

};
/*
class Origem {
public:

	virtual IplImage* getFrame(int dev);
	virtual int nextFrame();
	virtual int prepare();
	virtual void set_dimensao(int width, int height);
	virtual void free();
};
*/
//class Cameras : public Origem {
class Cameras {
private:
	int width, height;
	CvCapture *capture1, *capture2;
	IplImage *frame1, *frame2, *rframe1, *rframe2;
public:

	void set_dimensao(int width, int height);
	void set_devices(int d1, int d2);

	int prepare();
	void free();
	IplImage* getFrame(int dev);
	int nextFrame();

};

class BGsubstract {
private:
	IplImage *threshold,*filtered, *step_filter;
	int steps;

public:
	int init(IplImage *img);
	int step(IplImage *img);
	IplImage* filter(IplImage* img);
	int free();
};

class EyeEngine {
private:
	int eye_distance, width, height, run_mode;
	float *disp_left, *disp_right;
	IplImage *gray, *frameLeftRGB, *frameRightRGB, *frameLeftHSV, *frameRightHSV, *HSVleft_H, *HSVleft_S, *HSVleft_V, *HSVright_H, *HSVright_S, *HSVright_V, *RGBleft_R, *RGBleft_G, *RGBleft_B, *RGBright_R, *RGBright_G, *RGBright_B;
	IplImage *tmp3c, *tmp1c, *passo_left, *passo_right, *bg_disp_left, *bg_disp_right, *disparidade_left, *disparidade_right, *disparidade_avg, *bg_maskL, *bg_maskR, *bg_maskAVG;
	BGsubstract  *bgsLeft, *bgsRight;
	Cameras* pseye;
	Hand2Move* h2m;

	IplImage* hsv_mask;
	CvScalar  hsv_min;
	CvScalar  hsv_max;


	//-------------------

	int get_next_frame();
	int estereo_preprocessamento1();
	int estereo_posprocessamento1();
	int estereo_cuda();
	int estereo_opencv();
	int unir_imagens();
	bool _disparity2image(IplImage *imagem, const float *disp );

	int disparidade2imagem();

public:
	void init();
	int free();
	void rodar();

	int set_eye_distance(int ed);
	int set_dimensao(int width, int height);
	int set_run_mode(int modo);
	int salvar_disparidade();
};


#endif /* VOLANTE_H_ */
