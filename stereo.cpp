/* Copyright (c) 2007, University of North Carolina at Chapel Hill
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY University of North Carolina at Chapel Hill ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL University of North Carolina at Chapel Hill BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//#define IMAGE_W     450
//#define IMAGE_H     375
#define IMAGE_W     640
#define IMAGE_H     480

#define PROCESS_CUDA	        1
#define PROCESS_CV	  	        2
#define RUN_ONE_FRAME	        4
#define RUN_ONLINE		   	    8
#define SHOW_ON_FILE 	       16
#define SHOW_WINDOW_DISPARITY  32
#define SHOW_WINDOW_GAUSSIAN   64
#define SHOW_WINDOW_NORM	  128
#define SHOW_WINDOW_ORIGINAL  256
#define EXEC_NOISE_FILTER	  512
#define SHOW_WINDOW_BGREMOVE	1024
#define SHOW_ALL_WINDOWS SHOW_WINDOW_DISPARITY +  SHOW_WINDOW_GAUSSIAN + SHOW_WINDOW_NORM + SHOW_WINDOW_ORIGINAL + SHOW_WINDOW_BGREMOVE

#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>
using namespace std;
#include <float.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include "volante.h"

FILE* PGM_lido;
// Stereo functions in Stereo.cu.
extern "C" void stereoInit( int w, int h );
extern "C" void stereoUpload( const unsigned char *left, const unsigned char *right );
extern "C" void stereoProcess();
extern "C" void stereoDownload( float *disparityLeft, float *disparityRight );
int stereo_init=0;
IplImage *thresLeft, *thresRight, *disparidade;

bool readPGM( const char *filename, unsigned char *im, int w, int h )
{
    std::ifstream in;
    int state;
    char token[1024];
    int ww,hh,r;
    
    in.open(filename,std::ios_base::in|std::ios_base::binary);
    if(!in.is_open())
        return false;

    state = 0;
    while(!in.eof() && state<4) {
        in >> token;
        if(token[0]=='#')
            in.getline(token,sizeof(token));
        else {
            switch(state) {
                case 0:
                    if(strcmp(token,"P5")!=0)
                        return false;
                    state++;
                    break;
                case 1:
                    ww = atoi(token);
                    state++;
                    break;
                case 2:
                    hh = atoi(token);
                    state++;
                    break;
                case 3:
                    r = atoi(token);
                    state++;
                    break;
            }
        }
    }
    if(w!=ww || h!=hh)
        return false;

    in.read((char*)im,1);
    in.read((char*)im,w*h);



    return true;
}

bool writeDisparityPPM( const char *filename, const float *disp, int w, int h )
{
    std::ofstream out;
    float dmin,dmax,d;
    int x,y;

    dmin = FLT_MAX;
    dmax = -FLT_MAX;
    for(y=0; y<h; y++) {
        for(x=0; x<w; x++) {
            d = disp[y*w+x];
            if(d==255)
                continue;
            if(d<dmin)
                dmin = d;
            if(d>dmax)
                dmax = d;
        }
    }

    out.open(filename,std::ios_base::out|std::ios_base::binary);
    if(!out.is_open())
        return false;
    out << "P6\n" << w << " " << h << "\n255\n";
    for(y=0; y<h; y++) {
        for(x=0; x<w; x++) {
            unsigned char r,g,b;
            d = disp[y*w+x];
            if(d==255) {
            	//vermelho
                r = 255;
                g = 255;
                b = 255;
            } else {
                r = (unsigned char)(255*d/(dmax-dmin));
                g = b = r;
            }
            out.write((char*)&r,1);
            out.write((char*)&g,1);
            out.write((char*)&b,1);
        }
    }
    return true;
}


bool disparity2image(IplImage *imagem, const float *disp, int w, int h )
{
    float dmin,dmax,d;
    int x,y;

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
/**************************************/
/**
 * Retorna se há a opcao selecionada na lista de opcoes requisitadas na funcao
 * @var options soma das opcoes (constantes)
 * @var opt opcao requisitada
 * @return bool
 */
int has_option(int options, int opt) {
	int expo;
	int retorno = false;
	for(int i=10;i>=0;i--) {
		expo = pow(2,i);
		if(opt==options) {
			retorno=true;
			break;
		}
		else if(expo<options) {
			options-=expo;

			if(opt==expo) {
				retorno = true;
				break;
			}
		}
	}

	return retorno;
}
/**
 * Basicamente cria as janelas necessarias para serem usadas posteriormente
 */
void init_options(int options) {
	if(has_option(options,SHOW_WINDOW_DISPARITY)) {
		cvNamedWindow("disp_left",0);
		cvNamedWindow("disp_right",0);

	}
	if(has_option(options,SHOW_WINDOW_GAUSSIAN)) {
		cvNamedWindow("gaus_left",0);
		cvNamedWindow("gaus_right",0);

	}
	if(has_option(options,SHOW_WINDOW_NORM)) {
		cvNamedWindow("norm_left",0);
		cvNamedWindow("norm_right",0);
	}
	if(has_option(options,SHOW_WINDOW_ORIGINAL)) {
		cvNamedWindow("original_left",0);
		cvNamedWindow("original_right",0);

	}
	if(has_option(options,SHOW_WINDOW_BGREMOVE)) {
		cvNamedWindow("bgremove_left",0);
		cvNamedWindow("bgremove_right",0);
	}

	cvNamedWindow("AVG",0);
	cvNamedWindow("disparidade_mascara",0);

	cvWaitKey(5);

}

int deteriminarAlinhamento(int width, int height,IplImage* img1, IplImage* img2,int windowSize) {
	int pos = 0;
	int betterPos=0;
	int betterSSD=0;
	int SSD=0;
	int verticalpos=33;
	int left_i, i, j;
	int calcSSD;
	int z=0;

	CvSURFPoint* ponto;

	IplImage *img3, *img4;
	img3 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	img4 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);

	//cvSmooth(img1, img3, CV_GAUSSIAN, 7, 7 );
	//cvSmooth(img2, img4, CV_GAUSSIAN, 7, 7 );
	img3 = img1;
	img4 = img2;

	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq *objectKeypoints;

	CvSURFParams params = cvSURFParams(340, 0);
	cvExtractSURF( img4, 0, &objectKeypoints, NULL, storage, params, 0);
	printf("\n Total: %d",objectKeypoints->total);
	for(pos=0;pos<objectKeypoints->total;pos++) {
		ponto  = (CvSURFPoint*) cvGetSeqElem(objectKeypoints,pos);

		betterSSD+= (int)ponto->pt.x;
		img2->imageData[((int)ponto->pt.x) + ((int)ponto->pt.y)*width] = 255;

	}
	cvReleaseMemStorage(&storage);
	cvRelease((void **)&objectKeypoints);

	SSD = 0;

	for(z=0;z<100;z++) {
		storage = cvCreateMemStorage(0);
		objectKeypoints = 0;

		calcSSD=0;

		params = cvSURFParams(340, 0);
		cvExtractSURF( img3, 0, &objectKeypoints, NULL, storage, params, 0);
		//printf("\n Total: %d",objectKeypoints->total);
		for(pos=0;pos<objectKeypoints->total;pos++) {
			ponto  = (CvSURFPoint*) cvGetSeqElem(objectKeypoints,pos);
			calcSSD+= sqrt(pow((int)ponto->pt.x - z,2));
			//img3->imageData[((int)ponto->pt.x) + ((int)ponto->pt.y)*width] = 255;

		}
		printf("\n%d < %d",(int)sqrt(pow(calcSSD-betterSSD,2)),(int)sqrt(pow(SSD-betterSSD,2)));
		if(sqrt(pow(calcSSD-betterSSD,2))<sqrt(pow(SSD-betterSSD,2))) {
			SSD = calcSSD;
			betterPos=z;
		}
		cvReleaseMemStorage(&storage);
		cvRelease((void **)&objectKeypoints);
	}
	printf("\n----> %d <----",betterPos);
	/*
	do {
		SSD=0;
		for(i=0;i<windowSize;i++) {
			left_i = i+pos;
			for(j=verticalpos;j<verticalpos+windowSize;j++) {
				img1->imageData[left_i+j*width]= 255;
				img2->imageData[i+j*width]= 255;
				SSD+=pow(img1->imageData[left_i+j*width] - img2->imageData[i+j*width],2);
			}
		}
		printf("\n [ %d",SSD);
		if(SSD>betterSSD) {
			betterSSD = SSD;
			betterPos=pos;
		}
		pos++;

	} while(pos<width/2);
*/
	betterPos = 10;
	return betterPos;
}

/**
 * Efetua o processamento da estereoscopia usando as funcoes do CUDA
 */
int processaCUDA(int width, int height,unsigned char* imLeft,unsigned char* imRight, int options,int eye_distance) {
		float *dispLeft, *dispRight;
		dispLeft = new float[width*height];
		dispRight = new float[width*height];


		if(!stereo_init) {
			stereoInit(width,height);
			stereo_init=1;
		}

	    stereoUpload(imLeft,imRight);
	    stereoProcess();
	    stereoDownload(dispLeft,dispRight);



	    if(has_option(options,SHOW_ON_FILE)) {
	    	writeDisparityPPM("./left-disparity.ppm",dispLeft,width,height);
	    	writeDisparityPPM("./right-disparity.ppm",dispRight,width,height);
	    }
	    if(has_option(options,SHOW_WINDOW_DISPARITY)) {
	    	IplImage *frame11, *frame22;
	    	frame11 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	    	frame22 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	    	disparity2image(frame11,dispLeft,width,height);
	    	disparity2image(frame22,dispRight,width,height);
	    	//cvNot(frame11,frame11);
	    	//cvNot(frame22,frame22);


	    	cvShowImage("disp_left",frame11);
	    	cvShowImage("disp_right",frame22);

	    	IplImage *frameA, *frameB;
	    	frameA = cvCreateImage(cvSize(width-eye_distance,height),IPL_DEPTH_8U,3);
	    	frameB = cvCreateImage(cvSize(width-eye_distance,height),IPL_DEPTH_8U,3);


	    	cvSetImageROI(frame11,cvRect(eye_distance,0,width,height));
	    	cvCopy(frame11,frameA);
	    	cvResetImageROI(frame11);
	    	cvSetImageROI(frame22,cvRect(0,0,width-eye_distance,height));
	    	cvCopy(frame22,frameB);
	    	cvResetImageROI(frame22);


	    	for(int i=0;i<frameA->imageSize;i++) {
	    		frameA->imageData[i] = (frameA->imageData[i] + frameB->imageData[i])/2;
	    	}

	    	if(has_option(options,SHOW_ON_FILE)) {
	    		cvSaveImage("and.jpg",frameA);
	    	}
	    	cvCopy(frameA,disparidade);

	    	cvShowImage("AVG",frameA);

	    	cvReleaseImage(&frameA);
	    	cvReleaseImage(&frameB);


	    	cvReleaseImage(&frame11);
	    	cvReleaseImage(&frame22);
   	    }

	    free(dispLeft);
	    free(dispRight);
	return 1;
}
/**
 * Efetua o processamento da estereoscopia usando as funcoes do OPENCV
 */
int processaCV() {

	return 1;
}
/**
 * Metodo que centraliza a chamada do processamento da estereoscopia
 */
int processar(int width, int height,IplImage* img1, IplImage* img2, int options, int eye_distance) {
	if(has_option(options,SHOW_WINDOW_ORIGINAL)) {

		IplImage* frame22 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
		cvCopy(img1,frame22);

		cvAdd(frame22,img2,frame22);
		cvShowImage("original_left",frame22);
		cvShowImage("original_right",img2);
	}
	//cvErode(img1,img1,NULL,1);
	//cvErode(img2,img2,NULL,1);



	cvSmooth(img1, img1, CV_GAUSSIAN, 7, 7 );
	cvSmooth(img2, img2, CV_GAUSSIAN, 7, 7 );
	//cvEqualizeHist( img2, img2 );
	IplImage *imageA, *imageB;
	imageA = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	imageB = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);

	if(has_option(options,SHOW_WINDOW_GAUSSIAN) || 1) {
		/*THRESHOLD*/



		//cvCvtColor(img1,imageA,CV_RGB2GRAY);
		//cvCvtColor(img2,imageB,CV_RGB2GRAY);
		cvCopy(img1,imageA);
		cvCopy(img2,imageB);

		cvAbsDiff(imageA, thresLeft, imageA);
		cvThreshold(imageA,imageA, 60, 255,CV_THRESH_BINARY);


		cvAbsDiff(imageB, thresRight, imageB);
		//cvSaveImage("diferenca.png",imageB);
		cvThreshold(imageB,imageB, 60, 255,CV_THRESH_BINARY);


		cvShowImage("bgremove_left",imageA);
		cvShowImage("bgremove_right",imageB);
		//cvAnd(imageA,imageB,imageB);
		//cvSaveImage("diferenca.png",imageB);

    	/////////////

    	//cvShowImage("gaus_left",imageA);
    	//cvShowImage("gaus_right",imageB);

		cvShowImage("gaus_left",img1);
		cvShowImage("gaus_right",img2);
	}

	//cvNormalize( img1, img1, 0, 128, CV_MINMAX );
	//cvNormalize( img2, img2, 0, 128, CV_MINMAX );

	if(has_option(options,SHOW_WINDOW_NORM)) {
		cvShowImage("norm_left",img1);
		cvShowImage("norm_right",img2);
	}


	if(has_option(options,PROCESS_CUDA)) {
		unsigned char *imLeft, *imRight;
		imLeft = (unsigned char*)(img1->imageData);
		imRight = (unsigned char*)(img2->imageData);
		processaCUDA(width, height, imLeft, imRight, options, eye_distance);


		IplImage* frameA = cvCreateImage(cvSize(width-eye_distance,height),IPL_DEPTH_8U,1);
		IplImage* frameB = cvCreateImage(cvSize(width-eye_distance,height),IPL_DEPTH_8U,1);
		IplImage* frameC = cvCreateImage(cvSize(width-eye_distance,height),IPL_DEPTH_8U,3);


		cvSetImageROI(imageA,cvRect(eye_distance,0,width,height));
		cvCopy(imageA,frameA);
		cvResetImageROI(imageA);
		cvSetImageROI(imageB,cvRect(0,0,width-eye_distance,height));
		cvCopy(imageB,frameB);
		cvResetImageROI(imageB);
		cvOr(frameA,frameB,frameA);
		cvCopy(disparidade,frameC,frameA);
		cvErode(frameC,frameC,NULL,1);
		cvSmooth(frameC, frameC, CV_GAUSSIAN, 7, 7 );

		//cvCvtColor(frameC,frameA,CV_RGB2GRAY);

		cvShowImage("disparidade_mascara",frameC);
	}

	if(has_option(options,PROCESS_CV)) {
		processaCV();
	}

	return 1;
}
/**
 * Rodar estereoscopia sobre 2 imagens PGM passadas por parametro
 */
int rodaImagens(const char *filename1,  const char *filename2,int width, int height, int options, int eye_distance) {
	init_options(options);

	unsigned char *imLeft, *imRight;
    imLeft = new unsigned char[width*height];
    imRight = new unsigned char[width*height];
    IplImage *frame11, *frame22;
    frame11 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
    frame22 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);


	readPGM(filename1,imLeft,width,height);
	readPGM(filename2,imRight,width,height);

	frame11->imageData = (char*) imLeft;
	frame22->imageData = (char*) imRight;

	printf("\n\n%d\n\n",deteriminarAlinhamento(width, height,frame11, frame22,8));

	processar(width, height, frame11,frame22, options, eye_distance);

	cvReleaseImage(&frame11);
	cvReleaseImage(&frame22);

	return 1;
}


int alimenta_threshold(CvCapture* capture1, CvCapture* capture2, int width, int height, int frames) {
	thresLeft = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	thresRight = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);



	IplImage *frame1, *frame2, *frame11, *frame22,*frametmp;

    frame11 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
    frame22 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
    frametmp = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);

    IplImage *hue,*sat,*val;
    hue = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
    sat = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
    val = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);



	//try {

		if(!cvGrabFrame(capture1)) {
			printf("\n\nERRO AQUI - alimenta_threshold");
		}
		cvGrabFrame(capture2);
		frame1 = cvQueryFrame(capture1);
		frame2 = cvQueryFrame(capture2);


		if(true) {
			//cvCvtColor(frame1,frame1,CV_RGB2HSV);
			//cvCvtColor(frame2,frame2,CV_RGB2HSV);

			cvConvertScale(frame1,frametmp);

			cvCvtPixToPlane(frametmp,thresLeft,sat,val,0);

			cvConvertScale(frame2,frametmp);
			cvCvtPixToPlane(frametmp,thresRight,hue,val,0);
		}
		else {
			cvConvertScale(frame1,frametmp);
			cvCvtPixToPlane(frametmp,thresLeft,sat,val,0);

			cvConvertScale(frame2,frametmp);
			cvCvtPixToPlane(frametmp,thresRight,sat,val,0);

		}



	//}
	//catch(...) {
	//	printf("Você conectou as cameras??");
	//}




	for(int i=2;i<frames;i++) {


		if(!cvGrabFrame(capture1)) {
			break;
		}
		cvGrabFrame(capture2);

		frame1 = cvQueryFrame(capture1);
		frame2 = cvQueryFrame(capture2);

		if(true) {
			//cvCvtColor(frame1,frame1,CV_RGB2HSV);
			//cvCvtColor(frame2,frame2,CV_RGB2HSV);


			cvConvertScale(frame1,frametmp);
			cvCvtPixToPlane(frametmp,frame11,sat,val,0);

			cvConvertScale(frame2,frametmp);
			cvCvtPixToPlane(frametmp,frame22,sat,val,0);
		}
		else {
			cvConvertScale(frame1,frametmp);
			cvCvtPixToPlane(frametmp,frame11,hue,sat,0);

			cvConvertScale(frame2,frametmp);
			cvCvtPixToPlane(frametmp,frame22,hue,sat,0);

		}

		for(int j=2;j<thresLeft->nSize;j++) {
			thresLeft->imageData[j] = ((thresLeft->imageData[j]*(j-1)+frame11->imageData[j])/2);
			thresRight->imageData[j] = ((thresRight->imageData[j]*(j-1)+frame22->imageData[j])/2);
		}


		cvWaitKey( 3 );

	 }
	cvSaveImage("thresLeft.png",thresLeft);
	cvSaveImage("thresRight.png",thresRight);

	//exit(0);
	return 1;
}
/**
 * Rodar estereoscopia sobre imagens de 2 cameras
 */
int rodaCameras(int device1,  int device2,int width, int height, int options, int eye_distance) {

		init_options(options);

    	CvCapture* capture1;
	    CvCapture* capture2;

	    capture1 = cvCaptureFromCAM(device1);
	    capture2 = cvCaptureFromCAM(device2);

	    disparidade = cvCreateImage(cvSize(width-eye_distance,height),IPL_DEPTH_8U,3);

	    IplImage *frame1, *frame2;
	    IplImage *frame11, *frame22;

	    frame11 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	    frame22 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);

	    cvSetCaptureProperty(capture1,CV_CAP_PROP_FRAME_WIDTH,width);
	    cvSetCaptureProperty(capture1,CV_CAP_PROP_FRAME_HEIGHT,height);
	    cvSetCaptureProperty(capture2,CV_CAP_PROP_FRAME_WIDTH,width);
	    cvSetCaptureProperty(capture2,CV_CAP_PROP_FRAME_HEIGHT,height);


	    //threshold aqui
	    alimenta_threshold(capture1, capture2, width, height, 100);

	    int key=0;
	    int umavez=0;

	    IplImage *hue,*sat,*val;
	    hue = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	    sat = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	    val = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);

	    do {

			try {
				if(!cvGrabFrame(capture1)) {
					break;
				}

				cvGrabFrame(capture2);

				frame1 = cvQueryFrame(capture1);
				frame2 = cvQueryFrame(capture2);

			}
			catch(...) {
				printf("Você conectou as cameras??");
			}
			//cvCvtColor(frame1,frame1,CV_RGB2HSV);
			//cvCvtColor(frame2,frame2,CV_RGB2HSV);


			cvCvtPixToPlane(frame1,hue,sat,val,0);

			//cvAdd(sat,val,frame11);
			cvCopy(hue,frame11);

			cvCvtPixToPlane(frame2,hue,sat,val,0);

			//cvAdd(sat,val,frame22);
			cvCopy(hue,frame22);
			//cvSub(sat,val,frame22);

			//cvCvtColor(frame1,frame11,CV_BGR2GRAY);
			//cvCvtColor(frame2,frame22,CV_BGR2GRAY);


		    cvSaveImage("./left-capture.ppm",frame11);
		    cvSaveImage("./right-capture.ppm",frame22);

		    if(!umavez) {
		    	//printf("\n\n%d\n\n",deteriminarAlinhamento(width, height,frame11, frame22,8));

		    	umavez++;
		    }


		    if(key=='p') {
		    	processar(width, height, frame11,frame22, options + SHOW_ON_FILE, eye_distance);
		    }
		    else {
		    	processar(width, height, frame11,frame22, options, eye_distance);
		    }

			key = cvWaitKey( 3 );

	    } while(has_option(options,RUN_ONLINE) && (key!='q'));

	    cvReleaseImage(&frame11);
	    cvReleaseImage(&frame22);
	return 1;
}
/**
 * Rodar estereoscopia sobre 1 video contendo em cada frame as 2 imagens que serao separadas
 */
int rodaVideo(const char *filename1,int width, int height, int options, int eye_distance) {
	init_options(options);

    CvCapture* capture1;


	capture1 = cvCaptureFromAVI(filename1);
	IplImage *frame1, *frame2;
	IplImage *frame11, *frame22;
    frame11 = cvCreateImage(cvSize(width/2,height),IPL_DEPTH_8U,1);
    frame22 = cvCreateImage(cvSize(width/2,height),IPL_DEPTH_8U,1);
    frame2 = cvCreateImage(cvSize(width/2,height),8,3);

    int key=0;

    do {

    	if(!cvGrabFrame(capture1)) {
    		break;
    	}

		frame1 = cvQueryFrame(capture1);

		cvSetImageROI(frame1,cvRect(0,0,width/2,height));
		cvCopy(frame1,frame2);
		cvResetImageROI(frame1);
		cvCvtColor(frame2,frame11,CV_BGR2GRAY);

		cvSetImageROI(frame1,cvRect(width/2,0,width/2,height));
		cvCopy(frame1,frame2);
		cvResetImageROI(frame1);
		cvCvtColor(frame2,frame22,CV_BGR2GRAY);


		processar(width/2, height, frame11,frame22, options, eye_distance);

		//Apenas para dar tempo de renderizar na tela
		key = cvWaitKey( 3 );

    } while(has_option(options,RUN_ONLINE) && (key!='q'));

	cvReleaseImage(&frame11);
	cvReleaseImage(&frame22);
	cvReleaseImage(&frame2);

	return 1;
}
/**
 * Salva as imagens esquerda e direita em arquivos
 */
int salvaImagens() {

	return 1;
}
/**
 * Salva o mapa de disparidade em arquivo
 */
int salvaDisparity(float* dispLeft, float* dispRight, int width, int height) {
    writeDisparityPPM("./left-disparity.ppm",dispLeft,width,height);
    writeDisparityPPM("./right-disparity.ppm",dispRight,width,height);


	return 1;
}



int main() {

	/*std::istringstream mouse("126 211 0 250 345 2 111 123 1");
	int Mx(0), My(0), Button(0); // yeah, bad 'style'. Put on separate lines.
	while( mouse >Mx >My >Button ){
		if( Button == 2 ){
			std::cout << "Mx=" << Mx << " My=" << My <<std::endl;
		} // if(2)
	//} // while(mouse)
	 *
	 */
	//rodaImagens("praca_left.pgm","praca_right.pgm",800,600,PROCESS_CUDA + RUN_ONE_FRAME + SHOW_ON_FILE + SHOW_ALL_WINDOWS,10	);
	//rodaImagens("./tigre_left.pgm","./tigre_right.pgm",531,800,PROCESS_CUDA + RUN_ONE_FRAME + SHOW_ON_FILE + SHOW_ALL_WINDOWS,10);
	//rodaImagens("/home/aluno/bin/linux/release/img/tsukuba/scene_l.pgm","/home/aluno/bin/linux/release/img/tsukuba/scene_r.pgm",384,288,PROCESS_CUDA + RUN_ONE_FRAME + SHOW_ON_FILE + SHOW_ALL_WINDOWS,13);
	//rodaImagens("./img/Dolls/view1.pgm","./img/Dolls/view5.pgm",460,368,PROCESS_CUDA + RUN_ONE_FRAME + SHOW_ON_FILE + SHOW_ALL_WINDOWS,24);
	//rodaImagens("./img/Moebius/view1.pgm","./img/Moebius/view5.pgm",460,370,PROCESS_CUDA + RUN_ONE_FRAME + SHOW_ON_FILE + SHOW_ALL_WINDOWS,25);
	//rodaImagens("acabeca_left.pgm","acabeca_right.pgm",540,480,PROCESS_CUDA + RUN_ONE_FRAME + SHOW_ON_FILE + SHOW_ALL_WINDOWS,10);
	//rodaVideo("03p.divx",800,600,PROCESS_CUDA + RUN_ONLINE + SHOW_WINDOW_ORIGINAL + SHOW_WINDOW_NORM + SHOW_WINDOW_DISPARITY,10);
	//rodaVideo("01p.divx",640,480,PROCESS_CUDA + RUN_ONLINE + SHOW_WINDOW_ORIGINAL + SHOW_WINDOW_NORM + SHOW_WINDOW_DISPARITY,10);
	//rodaVideo("04p.divx",800,600,PROCESS_CUDA + RUN_ONLINE + SHOW_WINDOW_ORIGINAL + SHOW_WINDOW_NORM + SHOW_WINDOW_DISPARITY,10);

	//rodaImagens("./img/Aloe/view1-1.pgm","./img/Aloe/view5-1.pgm",427,370,PROCESS_CUDA + RUN_ONE_FRAME + SHOW_ON_FILE + SHOW_ALL_WINDOWS,32);
	rodaCameras(0,1,640,480,PROCESS_CUDA + RUN_ONLINE + SHOW_WINDOW_ORIGINAL + SHOW_WINDOW_NORM + SHOW_WINDOW_DISPARITY + EXEC_NOISE_FILTER,27);

	Origem* o = new Cameras();
	o->getFrame(1);
	cvWaitKey(0);
	return 1;
}

/*
int main()
{
    unsigned char *imLeft, *imRight;
    float *dispLeft, *dispRight;

    imLeft = new unsigned char[IMAGE_W*IMAGE_H];
    imRight = new unsigned char[IMAGE_W*IMAGE_H];
    dispLeft = new float[IMAGE_W*IMAGE_H];
    dispRight = new float[IMAGE_W*IMAGE_H];



    //readPGM("./left-image.pgm",imLeft,IMAGE_W,IMAGE_H);
    //readPGM("./right-image.pgm",imRight,IMAGE_W,IMAGE_H);
    //readPGM("./left_foto.pgm",imLeft,IMAGE_W,IMAGE_H);
    //readPGM("./right_foto.pgm",imRight,IMAGE_W,IMAGE_H);


    CvCapture* capture1;
    CvCapture* capture2;
    try {
    	capture1 = cvCaptureFromAVI("03p.divx");
//  	capture1 = cvCaptureFromCAM(0);
//    	capture2 = cvCaptureFromCAM(1);

    }
    catch(...) {
    	printf("CONECTA AS CAMERAS, CABECAO!!!");
    }
    IplImage *frame1, *frame2;
    IplImage *frame11, *frame22;

    frame11 = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
    frame22 = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);

    cvSetCaptureProperty(capture1,CV_CAP_PROP_FRAME_WIDTH,640);
    cvSetCaptureProperty(capture1,CV_CAP_PROP_FRAME_HEIGHT,480);
    cvSetCaptureProperty(capture2,CV_CAP_PROP_FRAME_WIDTH,640);
    cvSetCaptureProperty(capture2,CV_CAP_PROP_FRAME_HEIGHT,480);

    try {
		cvGrabFrame(capture1);
		//cvGrabFrame(capture2);

		frame1 = cvQueryFrame(capture1);
		frame2 = cvQueryFrame(capture2);

    }
    catch(...) {
    	printf("Você conectou as cameras??");
    }
    cvCvtColor(frame1,frame11,CV_BGR2GRAY);
    cvCvtColor(frame2,frame22,CV_BGR2GRAY);

    imLeft = (unsigned char*) frame11->imageData;
    imRight = (unsigned char*) frame22->imageData;



    stereoInit(IMAGE_W,IMAGE_H);
    stereoUpload(imLeft,imRight);
    stereoProcess();
    stereoDownload(dispLeft,dispRight);

    writeDisparityPPM("./left-disparity.ppm",dispLeft,IMAGE_W,IMAGE_H);
    writeDisparityPPM("./right-disparity.ppm",dispRight,IMAGE_W,IMAGE_H);

    cvReleaseImage(&frame11);
    cvReleaseImage(&frame22);

    return 0;
}

cvRectangle( multiplicador, cvPoint(0,0), cvPoint(width,height), cvScalarAll(j), CV_FILLED );
	cvRectangle( divisor, cvPoint(0,0), cvPoint(width,height), cvScalarAll(i), CV_FILLED );

	printf("\npreenche matriz com %2.2f > %2.2f",j,multiplicador->imageData[0]);
	printf("\nDivide %2.2f por %2.2f > ",frame11->imageData[0],(float)i);

	cvDiv(frame11,divisor,frame11);
	printf("%2.2f",frame11->imageData[0]);

	cvMul(multiplicador,athresLeft,tmpthresLeft);

	printf("\nMultiplica %2.2f por %2.2f > %2.2f",multiplicador->imageData[0],athresLeft->imageData[0],tmpthresLeft->imageData[0]);
	printf("\nDivide %2.2f por %2.2f > ",tmpthresLeft->imageData[0],(float)i);

	cvDiv(tmpthresLeft,divisor,tmpthresLeft);

	printf("%2.2f",tmpthresLeft->imageData[0]);

	cvAdd(frame11,tmpthresLeft,athresLeft);

	printf("\nSoma %2.2f com %2.2f > %2.2f\n---",frame11->imageData[0],tmpthresLeft->imageData[0],athresLeft->imageData[0]);


	cvDiv(NULL,frame22,frame22,2);
	cvMul(multiplicador,athresRight,tmpthresRight);
	cvDiv(NULL,tmpthresRight,tmpthresRight,i);
	cvAdd(frame22,tmpthresRight,athresRight);


	cvConvertScaleAbs(athresLeft,thresLeft);
	cvConvertScaleAbs(athresRight,thresRight);
	cvSaveImage("multiplicador.png",thresRight);

*/
