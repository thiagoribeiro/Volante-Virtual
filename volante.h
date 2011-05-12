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
#include <fstream>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>

class Origem {
public:

	virtual IplImage* getFrame(int dev) {
	}
	virtual int nextFrame() {
	}
	virtual int prepare() {
	}
	virtual void set_dimensao(int width, int height) {
	}
	virtual void free() {
	}
};

class Cameras : public Origem {
private:
	int width, height;
	CvCapture *capture1, *capture2;
	IplImage *frame[2], *rframe[2];
public:

	void set_dimensao(int width, int height) {
		this->width=width;
		this->height=height;
	}
	void set_devices(int d1, int d2) {

	    this->capture1 = cvCaptureFromCAM(d1);
	    this->capture2 = cvCaptureFromCAM(d2);

	}
	int prepare() {

		cvSetCaptureProperty(this->capture1,CV_CAP_PROP_FRAME_WIDTH,this->width);
		cvSetCaptureProperty(this->capture1,CV_CAP_PROP_FRAME_HEIGHT,this->height);
		cvSetCaptureProperty(this->capture2,CV_CAP_PROP_FRAME_WIDTH,this->width);
		cvSetCaptureProperty(this->capture2,CV_CAP_PROP_FRAME_HEIGHT,this->height);

		this->rframe[0] = cvCreateImage(cvSize(width,height),8,3);
		this->rframe[1] = cvCreateImage(cvSize(width,height),8,3);

		return 1;

	}
	void free() {
		cvReleaseImage(&this->rframe[0]);
		cvReleaseImage(&this->rframe[1]);

	}
	IplImage* getFrame(int dev) {
		if(dev==0 || dev==1) {
			return this->rframe[dev];
		}
		else {
			return NULL;
		}

	}
	int nextFrame() {
		try {
			if(cvGrabFrame(capture1) && cvGrabFrame(capture2)) {
				this->frame[0] = cvQueryFrame(capture1);
				this->frame[1] = cvQueryFrame(capture2);

				cvConvertScale(this->frame[0],this->rframe[0]);
				cvConvertScale(this->frame[1],this->rframe[1]);

				return true;
			}
			else {
				return false;
			}
		}
		catch(...) {
			printf("Você conectou as cameras??");
			return false;
		}

	}

};

class EyeEngine {
private:
	int eye_distance, width, height, run_mode;
	IplImage *thresLeft, *thresRight, *HSV[3], *RGB[3];

	//-------------------

	int get_next_frame() {

	}
	int treinar_bg_remove() {

	}
	int estereo_preprocessamento1() {

	}
	int estereo_posprocessamento1() {

	}
	int estereo_cuda() {

	}
	int estereo_opencv() {

	}
	int remover_bg() {

	}
	int unir_imagens() {

	}

	int free() {

	}

public:
	void rodar() {
		this->thresLeft = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->thresRight = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);


		int treino_bg=100;
		if(this->run_mode==1) {



			int key=0;
			do {
				this->get_next_frame();

				this->estereo_preprocessamento1();

				if(treino_bg>0) {
					treino_bg--;
					this->treinar_bg_remove();
				}
				else {
					this->estereo_cuda();
					this->remover_bg();
					this->estereo_posprocessamento1();
				}

				key = cvWaitKey( 3 );

			} while(key!='q');
		}
	}

	int set_eye_distance(int ed) {
		this->eye_distance = ed;

	}
	int set_dimensao(int width, int height) {
		this->width=width;
		this->height=height;

	}
	int set_run_mode(int modo) {
		this->run_mode=modo;

	}
	int salvar_disparidade() {

	}
};


#endif /* VOLANTE_H_ */
