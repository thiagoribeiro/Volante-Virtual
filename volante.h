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

extern "C" void stereoInit( int w, int h );
extern "C" void stereoUpload( const unsigned char *left, const unsigned char *right );
extern "C" void stereoProcess();
extern "C" void stereoDownload( float *disparityLeft, float *disparityRight );

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
	IplImage *frame1, *frame2, *rframe1, *rframe2;
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
		printf("oi\n\n");
		cvSetCaptureProperty(this->capture1,CV_CAP_PROP_FRAME_WIDTH,this->width);
		cvSetCaptureProperty(this->capture1,CV_CAP_PROP_FRAME_HEIGHT,this->height);
		cvSetCaptureProperty(this->capture2,CV_CAP_PROP_FRAME_WIDTH,this->width);
		cvSetCaptureProperty(this->capture2,CV_CAP_PROP_FRAME_HEIGHT,this->height);

		this->rframe1 = cvCreateImage(cvSize(width,height),8,3);
		this->rframe2 = cvCreateImage(cvSize(width,height),8,3);

		return 1;

	}
	void free() {
		cvReleaseImage(&this->rframe1);
		cvReleaseImage(&this->rframe2);

	}
	IplImage* getFrame(int dev) {

		if(dev==0) {
			return this->rframe1;
		}
		else if(dev==1) {
			return this->rframe2;
		}
		else {
			return NULL;
		}

	}
	int nextFrame() {
		try {
			if(cvGrabFrame(capture1) && cvGrabFrame(capture2)) {
				this->frame1 = cvQueryFrame(capture1);
				this->frame2 = cvQueryFrame(capture2);

				cvConvertScale(this->frame1,this->rframe1);
				cvConvertScale(this->frame2,this->rframe2);

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
	IplImage *frameLeftRGB, *frameRightRGB, *frameLeftHSV, *frameRightHSV, *HSVleft_H, *HSVleft_S, *HSVleft_V, *HSVright_H, *HSVright_S, *HSVright_V, *RGBleft_R, *RGBleft_G, *RGBleft_B, *RGBright_R, *RGBright_G, *RGBright_B;
	IplImage *tmp, *passo_left, *passo_right, *bg_disp_left, *bg_disp_right, *disparidade_left, *disparidade_right;
	BGsubstract  *bgsLeft, *bgsRight;
	Cameras* pseye;

	//-------------------

	int get_next_frame() {

		this->pseye->nextFrame();

		this->frameLeftRGB = this->pseye->getFrame(0);
		this->frameRightRGB = this->pseye->getFrame(1);



		cvCvtPixToPlane(this->frameLeftRGB,this->RGBleft_R,this->RGBleft_G,this->RGBleft_B,0);
		cvCvtPixToPlane(this->frameRightRGB,this->RGBright_R,this->RGBright_G,this->RGBright_B,0);

		cvCvtColor(this->frameLeftRGB,this->frameLeftHSV,CV_RGB2HSV);
		cvCvtColor(this->frameRightRGB,this->frameRightHSV,CV_RGB2HSV);

		cvCvtPixToPlane(this->frameLeftHSV,this->HSVleft_H,this->HSVleft_S,this->HSVleft_V,0);
		cvCvtPixToPlane(this->frameRightHSV,this->HSVright_H,this->HSVright_S,this->HSVright_V,0);

		return 1;

	}
	int estereo_preprocessamento1() {
		//CANAL VERMELHO
		cvSmooth(this->RGBleft_R, this->passo_left, CV_GAUSSIAN, 7, 7 );
		cvSmooth(this->RGBright_R, this->passo_right, CV_GAUSSIAN, 7, 7 );

		return 1;
	}
	int estereo_posprocessamento1() {
		return 1;
	}
	int estereo_cuda() {
		stereoUpload((unsigned char*) this->passo_left->imageData,(unsigned char*) this->passo_right->imageData);
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
	bool _disparity2image(IplImage *imagem, const float *disp )
	{
	    float dmin,dmax,d;
	    int x,y;
	    int w = this->width;
	    int h = this->height;

	    dmin = FLT_MAX;
	    dmax = -FLT_MAX;
	    for(y=0; y<h; y++) {
	        for(x=0; x<w; x++) {
	            d = disp[y*w+x];
	            if(d>250)
	                continue;
	            if(d<dmin)
	                dmin = d;
	            if(d>dmax)
	                dmax = d;
	        }
	    }

	    for(y=0; y<h; y++) {
	        for(x=0; x<w; x++) {
	            unsigned char r,g,b;
	            d = disp[y*w+x];
	            if(d>250) {
	                r = 0;
	                g = 0;
	                b = 0;
	            } else {
	                r = (unsigned char)(255*d/(dmax-dmin));
	                g = b = r;
	            }
	            //out.write((char*)&r,1);
	            //out.write((char*)&g,1);
	            //out.write((char*)&b,1);
	            //corrigir
	            imagem->imageData[3*(y*w+x)]   = (char) r;
	            imagem->imageData[3*(y*w+x)+1] = (char) g;
	            imagem->imageData[3*(y*w+x)+2] = (char) b;

	        }
	    }
	    return true;
	}

	int disparidade2imagem() {
		this->_disparity2image(this->disparidade_left,this->disp_left);
		this->_disparity2image(this->disparidade_right,this->disp_right);
		return 1;
	}

public:
	void init() {
		this->tmp = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,3);

		this->passo_left = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->passo_right = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		this->HSVleft_H = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVleft_S = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVleft_V = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVright_H = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVright_S = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVright_V = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		this->RGBleft_R = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->RGBleft_G = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->RGBleft_B = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->RGBright_R = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->RGBright_G = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->RGBright_B = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		this->frameLeftHSV = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,3);
		this->frameRightHSV = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,3);

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

		this->disparidade_left = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,3);
		this->disparidade_right = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,3);

		this->bg_disp_left = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->bg_disp_right = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		stereoInit(this->width,this->height);


		cvNamedWindow("disp_left",0);
		cvNamedWindow("disp_right",0);
		cvNamedWindow("original_left",0);
		cvNamedWindow("original_right",0);
		cvNamedWindow("AVG",0);
		cvNamedWindow("disparidade_mascara",0);
	}
	int free() {
		cvReleaseImage(&this->tmp);
		cvReleaseImage(&this->HSVleft_H);
		cvReleaseImage(&this->HSVleft_S);
		cvReleaseImage(&this->HSVleft_V);
		cvReleaseImage(&this->HSVright_H);
		cvReleaseImage(&this->HSVright_S);
		cvReleaseImage(&this->HSVright_V);
		cvReleaseImage(&this->RGBleft_R);
		cvReleaseImage(&this->RGBleft_G);
		cvReleaseImage(&this->RGBleft_B);
		cvReleaseImage(&this->RGBright_R);
		cvReleaseImage(&this->RGBright_G);
		cvReleaseImage(&this->RGBright_B);
		cvReleaseImage(&this->passo_left);
		cvReleaseImage(&this->passo_right);
		cvReleaseImage(&this->frameLeftHSV);
		cvReleaseImage(&this->frameRightHSV);
		cvReleaseImage(&this->bg_disp_left);
		cvReleaseImage(&this->bg_disp_right);

		this->pseye->free();
		this->bgsLeft->free();
		this->bgsRight->free();

		//free(this->disp_left);
		//free(this->disp_right);

		cvReleaseImage(&this->disparidade_left);
		cvReleaseImage(&this->disparidade_right);

		return  1;
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

				}
				//BG REMOVE TREINADO
				else {
					cvShowImage("original_left",this->RGBleft_R);
					cvShowImage("original_right",this->RGBright_R);
					this->estereo_cuda();

					this->disparidade2imagem();

					cvCvtColor(this->disparidade_left,this->bg_disp_left,CV_RGB2GRAY);
					cvCvtColor(this->disparidade_right,this->bg_disp_right,CV_RGB2GRAY);

					cvShowImage("disp_left",this->bg_disp_left);
					cvShowImage("disp_right",this->bg_disp_right);

					this->bg_disp_left = this->bgsLeft->filter(this->bg_disp_left);
					this->bg_disp_left = this->bgsRight->filter(this->bg_disp_right);
					//cvShowImage("disp_left",this->bg_disp_left);
					//cvShowImage("disp_right",this->bg_disp_right);

					cvSetImageROI(this->bg_disp_left,cvRect(this->eye_distance,0,this->width,this->height));
					cvCopy(this->bg_disp_left,this->bg_disp_left);
					cvResetImageROI(this->bg_disp_left);
					cvSetImageROI(this->bg_disp_right,cvRect(0,0,width-this->eye_distance,this->height));
					cvCopy(this->bg_disp_right,this->bg_disp_right);
					cvResetImageROI(this->bg_disp_right);
					cvShowImage("AVG",this->bg_disp_left);
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
