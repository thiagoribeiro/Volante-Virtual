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

class BGsubstract {
private:
	IplImage *threshold,*filtered;

public:
	int init(IplImage *img) {
		this->threshold = cvCloneImage(img);
		this->filtered = cvCreateImage(cvSize(img->width,img->height),img->depth,img->nChannels);

		return 1;
	}
	int step(IplImage *img) {
		for(int j=2;j<this->threshold->nSize;j++) {
			this->threshold->imageData[j] = ((this->threshold->imageData[j]*(j-1)+img->imageData[j])/2);
		}

		return 1;
	}
	IplImage* filter(IplImage* img) {
		cvAbsDiff(img, this->threshold, this->filtered);
		cvThreshold(this->filtered,this->filtered, 60, 255,CV_THRESH_BINARY);

		return this->filtered;
	}
	int free() {
		cvReleaseImage(&this->threshold);
		cvReleaseImage(&this->filtered);

		return 1;
	}
};

class EyeEngine {
private:
	int eye_distance, width, height, run_mode;
	float *disp_left, *disp_right;
	IplImage *frameLeftRGB, *frameRightRGB, *frameLeftHSV, *frameRightHSV, *HSVleft[3], *HSVright[3], *RGBleft[3], *RGBright[3];
	IplImage *tmp, *passo_left, *passo_right, *bg_disp_left, *bg_disp_right, *disparidade_left, *disparidade_right;
	BGsubstract  *bgsLeft, *bgsRight;
	Cameras* pseye;

	//-------------------

	int get_next_frame() {
		this->pseye->nextFrame();

		this->frameLeftRGB = this->pseye->getFrame(0);
		this->frameRightRGB = this->pseye->getFrame(1);

		cvCvtPixToPlane(this->frameLeftRGB,this->RGBleft[0],this->RGBleft[1],this->RGBleft[2],0);
		cvCvtPixToPlane(this->frameRightRGB,this->RGBright[0],this->RGBright[1],this->RGBright[2],0);

		cvCvtColor(this->frameLeftRGB,this->frameLeftHSV,CV_RGB2HSV);
		cvCvtColor(this->frameRightRGB,this->frameRightHSV,CV_RGB2HSV);

		cvCvtPixToPlane(this->frameLeftHSV,this->HSVleft[0],this->HSVleft[1],this->HSVleft[2],0);
		cvCvtPixToPlane(this->frameRightHSV,this->HSVright[0],this->HSVright[1],this->HSVright[2],0);

		return 1;

	}
	int estereo_preprocessamento1() {
		//CANAL VERMELHO
		cvSmooth(this->frameLeftRGB[0], this->passo_left, CV_GAUSSIAN, 7, 7 );
		cvSmooth(this->frameRightRGB[0], this->passo_right, CV_GAUSSIAN, 7, 7 );

		return 1;
	}
	int estereo_posprocessamento1() {
		return 1;
	}
	int estereo_cuda() {
		stereoUpload(this->passo_left,this->passo_right);
	    stereoProcess();
	    stereoDownload(this->disp_left,this->disp_right);
		return 1;
	}
	int estereo_opencv() {
		return 1;
	}
	int unir_imagens() {
		return 1;
	}


public:
	void init() {
		this->tmp = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,3);

		this->passo_left = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->passo_right = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		this->HSVleft[0] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVleft[1] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVleft[2] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVright[0] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVright[1] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVright[2] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		this->RGBleft[0] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->RGBleft[1] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->RGBleft[2] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->RGBright[0] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->RGBright[1] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->RGBright[2] = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		this->bgsLeft=  new BGsubstract();
		this->bgsRight= new BGsubstract();

		this->pseye = new Cameras();
		this->pseye->set_devices(0,1);
		this->pseye->set_dimensao(this->width,this->height);
		this->pseye->prepare();

		this->frameLeftRGB=NULL;
		this->frameRightRGB=NULL;

		this->disp_left = new float[this->width*this->height];
		this->disp_right = new float[this->width*this->height];

		this->disparidade_left = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->disparidade_right = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		stereoInit(this->width,this->height);
	}
	int free() {
		cvReleaseImage(&this->tmp);
		cvReleaseImage(&this->HSVleft[0]);
		cvReleaseImage(&this->HSVleft[1]);
		cvReleaseImage(&this->HSVleft[2]);
		cvReleaseImage(&this->HSVright[0]);
		cvReleaseImage(&this->HSVright[1]);
		cvReleaseImage(&this->HSVright[2]);
		cvReleaseImage(&this->RGBleft[0]);
		cvReleaseImage(&this->RGBleft[1]);
		cvReleaseImage(&this->RGBleft[2]);
		cvReleaseImage(&this->RGBright[0]);
		cvReleaseImage(&this->RGBright[1]);
		cvReleaseImage(&this->RGBright[2]);
		cvReleaseImage(&this->passo_left);
		cvReleaseImage(&this->passo_right);
		this->pseye->free();
		this->bgsLeft->free();
		this->bgsRight->free();

		free(this->disp_left);
		free(this->disp_right);

		cvReleaseImage(&this->disparidade_left);
		cvReleaseImage(&this->disparidade_right);
	}
	void rodar() {
		int start_treino=100;
		int treino_bg=start_treino;
		if(this->run_mode==1) {

			int key=0;

			//REPETICAO A CADA FRAME
			do {
				this->get_next_frame();

				this->estereo_preprocessamento1();

				//TREINAR BG REMOVE
				if(treino_bg>0) {
					if(treino_bg==start_treino) {
						this->bgsLeft->init(this->passo_left);
						this->bgsRight->init(this->passo_right);
					}
					else {
						this->bgsLeft->step(this->passo_left);
						this->bgsRight->step(this->passo_right);
					}
					treino_bg--;
					this->treinar_bg_remove();
				}
				//BG REMOVE TREINADO
				else {
					this->estereo_cuda();

					this->disparidade_left->imageData = this->disp_left;
					cvCvtColor(this->disparidade_left,this->bg_disp_left,CV_RGB2GRAY);
					this->disparidade_right->imageData = this->disp_right;
					cvCvtColor(this->disparidade_right,this->bg_disp_right,CV_RGB2GRAY);

					this->bg_disp_left = this->bgsLeft->filter(this->bg_disp_left);
					this->bg_disp_left = this->bgsRight->filter(this->bg_disp_right);

					this->estereo_posprocessamento1();
				}

				key = cvWaitKey( 3 );

			} while(key!='q');
		}
	}

	int set_eye_distance(int ed) {
		this->eye_distance = ed;

		return 1;

	}
	int set_dimensao(int width, int height) {
		this->width=width;
		this->height=height;

		return 1;
	}
	int set_run_mode(int modo) {
		this->run_mode=modo;

		return 1;
	}
	int salvar_disparidade() {

		return 1;
	}
};


#endif /* VOLANTE_H_ */
