/*
 * volante.cpp
 *
 *  Created on: May 11, 2011
 *      Author: thiago
 */

#include "volante.h"
//int socketId
//

/////////////////////////////////////////////////////////
//////////////////                 NETWORK
/////////////////////////////////////////////////////////


int Network::init() {

	printf("net1\n\n");

	this->socketId=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

	this->serverAddr.sin_family = AF_INET;
	this->serverAddr.sin_port = htons(NW_PORT);
	this->serverAddr.sin_addr.s_addr = INADDR_ANY;


	bind(this->socketId, (struct sockaddr *)&this->serverAddr, sizeof(this->serverAddr));

	this->size= sizeof(this->clientAddr);
	this->qtde_clients=0;

	return 1;
}
int Network::send(comandos msg) {

	int rcvReturn, sndReturn;
	comandos cmd_rcv, cmd_snd;

	int i=0;


	rcvReturn = recvfrom( this->socketId,&cmd_rcv,sizeof(struct comandos),MSG_DONTWAIT,&this->clientAddrCast[this->qtde_clients],&this->size);
	if((rcvReturn>0) && (cmd_rcv.status==NW_READY)) {
		this->qtde_clients++;
			printf("Novo leitor %p\n\n",this->clientAddrCast[this->qtde_clients].sa_data);

	}

	sndReturn=0;
	for(int z=0;z<this->qtde_clients;z++) {
		sndReturn += sendto(this->socketId,&msg,sizeof(struct comandos),0,&(this->clientAddrCast[z]),this->size);
		printf("%d > ENVIANDO TILT: %2.4f DEEP: %2.4f\n",z, msg.left_right,msg.accel_break);
	}

	return sndReturn;


	return 1;

}

int Network::sendMove(float x, float y) {
	comandos msg;
	msg.left_right=x;
	msg.accel_break=y;
	msg.status=NW_SEND_DATA;
	return this->send(msg);

}
int Network::free() {
	close(socketId);
	return 1;
}
/////////////////////////////////////////////////////////
//////////////////                 HAND DETECTION
/////////////////////////////////////////////////////////
int HandDetection::play() {
	this->paused=0;

}
int HandDetection::pause() {
	this->paused=1;
}
int HandDetection::is_paused() {
	return this->paused==1;
}
bool HandDetection::run() {

		if( this->m_num == 0 ) return false;
		if(this->is_paused()) return true;
		this->faces = cvHaarDetectObjects(this->imagem, this->cascade, this->storage, this->haar_scale, this->min_neighbors, this->haar_flags, this->min_size);

		this->pause();

		return true;
}

int HandDetection::find_hands(IplImage *imagem) {

	cvResize(imagem,this->imagem, CV_INTER_LINEAR);
	cvEqualizeHist(this->imagem,this->imagem);
	this->play();

	return 1;
}
int HandDetection::translate(IplImage* disp) {



	this->disparidade = disp;
	this->detect_hands();
	this->remove_falses();
	this->filter_result();

	return 1;
}
int HandDetection::detect_hands() {
	CvRect *rcheck;

	this->idMaos=0;
	int soma1, soma2;
	CvScalar cor,media,desvio;

	for(int i=0;i<this->faces->total;i++) {
		rcheck = (CvRect*) cvGetSeqElem(this->faces,i);
		if((rcheck->width < 70) && (rcheck->height < 70)) {
			rcheck->x*=this->image_scale;
			rcheck->y*=this->image_scale;
			rcheck->width*=this->image_scale;
			rcheck->height*=this->image_scale;

			cvSetImageROI(this->disparidade,(*rcheck));
			cvAvgSdv(this->disparidade, &media, &desvio);
			cvResetImageROI(this->disparidade);
			//if(media.val[0]>15){
				printf("IMAGEM [%i] MEDIA: %2.4f DESVIO: %2.4f\n",i,media.val[0],desvio.val[0]);
				this->maos[idMaos]= i;
				this->idMaos++;
			//}



		}

	}

	printf("MAOS ENCONTRADAS: %i de %i \n",idMaos, this->faces->total);
	if(this->idMaos>=2) {

		CvRect* r1 = (CvRect*) cvGetSeqElem(this->faces,this->maos[0]);
		CvRect* r2 = (CvRect*) cvGetSeqElem(this->faces,this->maos[1]);


		if(r1->x > r2->x) {
			r2 = (CvRect*) cvGetSeqElem(this->faces,this->maos[0]);
			r1 = (CvRect*) cvGetSeqElem(this->faces,this->maos[1]);


		}
		this->mao1.x = r1->x;
		this->mao1.y = r1->y;
		this->mao1.width = r1->width;
		this->mao1.height = r1->height;
		this->mao2.x = r2->x;
		this->mao2.y = r2->y;
		this->mao2.width = r2->width;
		this->mao2.height = r2->height;


	}


	return 1;
}
int HandDetection::remove_falses() {

	return 1;
}
int HandDetection::filter_result() {

	return 1;
}
CvRect HandDetection::getHandRect(int hand) {

	if(hand==0) {
		return this->mao1;
	}
	else {
		return this->mao2;
	}
}
int HandDetection::init(char *cascade_file, int width, int height) {
	this->cascade = (CvHaarClassifierCascade*)cvLoad( cascade_file, 0, 0, 0 );

	if( !this->cascade )  {
		fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
	    return -1;
	}
	this->pause();
	//PENSAR EM TROCAR POR this->min_size = cvSize(32,36);

	//PORT PARA O CASCADE.XML
	/*
	this->min_size = cvSize(32,32);
	this->image_scale = 1.5;
	this->haar_scale = 1.8;
	this->min_neighbors = 8;
	this->haar_flags = 0;
	*/

	//PORT pARA O CASCADE3.XML
	/*
	this->min_size = cvSize(32,32);
	this->image_scale = 1.5;
	this->haar_scale = 1.1;
	this->min_neighbors = 1;
	this->haar_flags = 0;
	*/
	//OUTRO PoRT PARA O CASCADE3.XML
	/*
	this->min_size = cvSize(32,32);
	this->image_scale = 1.5;
	this->haar_scale = 1.2;
	this->min_neighbors = 5;
	this->haar_flags = 0;
	*/

	//PORT PARA O CASCADE6
	this->min_size = cvSize(32,32);
	this->image_scale = 1.4;
	this->haar_scale = 1.2;
	this->min_neighbors = 2;
	this->haar_flags = 0;
	this->storage=cvCreateMemStorage(0);

	printf("\n-------------------------\nIMAGEM HAAR; %d %d\n\n\n",width,height);
	this->imagem = cvCreateImage(cvSize(cvRound(width/this->image_scale),cvRound(height/this->image_scale)),IPL_DEPTH_8U,1);

	this->start();

	return 1;

}
int HandDetection::free() {
	cvReleaseImage(&this->imagem);
	cvReleaseHaarClassifierCascade( &this->cascade );
	cvReleaseMemStorage( &this->storage );

	return 1;
}
int HandDetection::printHands(IplImage *img) {
	CvRect *rcheck;
	for(int i=0;i<this->idMaos;i++) {
		rcheck = (CvRect*) cvGetSeqElem(this->faces,this->maos[i]);

		cvRectangle(img,cvPoint(rcheck->x,rcheck->y),cvPoint(rcheck->x+rcheck->width,rcheck->y+rcheck->height),CV_RGB(255,255,255),3,8,0);
	}


	return 1;
}
/////////////////////////////////////////////////////////
//////////////////                 HAND 2 MOVIMENT
/////////////////////////////////////////////////////////
int Hand2Move::step(IplImage *imagem, IplImage *disparidade, IplImage *background) {
	this->imagem = imagem;
	this->disparidade = disparidade;
	this->background = background;


	this->hand_process();
	this->deep_detect();

	this->network_send();


	return 1;
}
int Hand2Move::deep_detect() {

	CvScalar media1, media2, desvio_padrao1, desvio_padrao2;
	float taxa_aceleracao;
	int i,j, v1, v2,qtv1,qtv2, mask1, mask2, soma1, soma2;
/*
	cvSetImageROI(this->disparidade,this->hand->getHandRect(0));
	cvAvgSdv(this->disparidade, &media1, &desvio_padrao1);
	cvResetImageROI(this->disparidade);

	cvSetImageROI(this->disparidade,this->hand->getHandRect(1));
	cvAvgSdv(this->disparidade, &media2, &desvio_padrao2);
	cvResetImageROI(this->disparidade);

	taxa_aceleracao = ((media1.val[0]+0.3*desvio_padrao1.val[0]) + (media2.val[0]+0.3*desvio_padrao2.val[0]))/2;

	if(taxa_aceleracao<=10) {
		taxa_aceleracao=0;
	}
	else if(taxa_aceleracao>80) {
		taxa_aceleracao=80;
	}
	taxa_aceleracao/=40.0f;
	taxa_aceleracao-=1;
*/
	cvSetImageROI(this->disparidade,this->hand->getHandRect(0));
	cvResize(this->disparidade,this->img_mao1);
	cvResetImageROI(this->disparidade);

	cvSetImageROI(this->disparidade,this->hand->getHandRect(1));
	cvResize(this->disparidade,this->img_mao2);
	cvResetImageROI(this->disparidade);

	cvSetImageROI(this->background,this->hand->getHandRect(0));
	cvResize(this->background,this->rec_mao1);
	cvResetImageROI(this->background);

	cvSetImageROI(this->background,this->hand->getHandRect(1));
	cvResize(this->background,this->rec_mao2);
	cvResetImageROI(this->background);


	soma1=0;
	soma2=0;
	qtv1=0;
	qtv2=0;
	for(i=0;i<this->img_mao1->width;i++) {
		for(j=0;j<this->img_mao1->height;j++) {
			mask1 = cvGet2D(this->rec_mao1,i,j).val[0];
			mask2 = cvGet2D(this->rec_mao2,i,j).val[0];
			v1 = cvGet2D(this->img_mao1,i,j).val[0];
			v2 = cvGet2D(this->img_mao2,i,j).val[0];
			if(v1>50 && mask1>150) {
				soma1+=v1;
				qtv1++;
			}
			if(v2>50 && mask1>150) {
				soma2+=v2;
				qtv2++;
			}
		}
	}
	if(qtv1>0 && qtv2>0) {
		taxa_aceleracao = (soma1/qtv1 + soma2/qtv2)/2;
		if(taxa_aceleracao>180) {
			taxa_aceleracao=180;
		}
		else if(taxa_aceleracao<80) {
			taxa_aceleracao=80;
		}
		taxa_aceleracao = (taxa_aceleracao-80)/100;
		if(taxa_aceleracao<0.1) {
			taxa_aceleracao=-4*(0.2 - taxa_aceleracao);
		}
	}
	else {
		taxa_aceleracao=0;
	}



	/*
	cvSetImageROI(this->disparidade,this->mao1);
	cvCopy(this->disparidade,this->img_mao1);
	cvResetImageROI(this->disparidade);
	cvSetImageROI(this->disparidade,this->mao2);
	cvCopy(this->disparidade,this->img_mao2);
	cvResetImageROI(this->disparidade);
	*/
	/*
	MatND hist;

	calcHist( this->disparidade, 1, 1, Mat(), // do not use mask
	        hist, 1, {20}, {{0,256}},
	        true, // the histogram is uniform
	        false );

	*/
	printf("TAXA DE ACELERACAO: %2.4f\n",taxa_aceleracao);
	//this->deep = 0.7;
	/*
	if(taxa_aceleracao<0) {
		taxa_aceleracao=0.1;
	}
	*/
	this->deep = taxa_aceleracao;
	//this->deep = 0.2;

	return 1;

}
int Hand2Move::hand_detect(IplImage *imagem) {
	this->hand->find_hands(imagem);

	return 1;
}
int Hand2Move::running_hand_detect() {
	return !this->hand->is_paused();
}
int Hand2Move::hand_process() {
	float ponto_central_x1, ponto_central_x2, ponto_central_y1, ponto_central_y2;


	this->hand->translate(this->disparidade);
	this->mao1 = this->hand->getHandRect(0);
	this->mao2 = this->hand->getHandRect(1);
	//printf("RECT 1: X %2.4d  Y %2.4d  W  %2.4d  H %2.4d\n",this->mao1.x,this->mao1.y,this->mao1.width,this->mao1.height);
	//printf("RECT 2: X %2.4d  Y %2.4d  W  %2.4d  H %2.4d\n",this->mao2.x,this->mao2.y,this->mao2.width,this->mao2.height);



	ponto_central_x1= this->mao1.width/2.0f + this->mao1.x;
	ponto_central_x2= this->mao2.width/2.0f + this->mao2.x;
	ponto_central_y1= this->mao1.height/2.0f + this->mao1.y;
	ponto_central_y2= this->mao2.height/2.0f + this->mao2.y;
/*

	ponto_central_x1 = this->mao1.x;
	ponto_central_y1 = this->mao1.y;
	ponto_central_x2 = this->mao2.x;
	ponto_central_y2 = this->mao2.y;
*/

	//this->tilt = atan(ponto_central_y1/ponto_central_x1) - atan(ponto_central_y2/ponto_central_x2);
	this->tilt_angle = atan2(ponto_central_y1-ponto_central_y2, ponto_central_x1-ponto_central_x2);
	this->tilt = tilt_angle - 3.141592653589793/1.6;

	if(this->tilt_angle>=0) {
		this->tilt = (3.141592653589793 - this->tilt_angle)/3.141592653589793;
	}
	else {
		this->tilt = -1*(3.141592653589793 + this->tilt_angle)/ 3.141592653589793;
	}


	//printf("pcx: %2.4f | pcy: %2.4f | ANGLE %2.4f | TILT: %2.4f\n\n",ponto_central_x1,ponto_central_y1,tilt_angle, this->tilt);

	return 1;
}
int Hand2Move::network_send() {

	this->net->sendMove(this->tilt,this->deep);
	return 1;
}
int Hand2Move::init(int width, int height) {
	this->width=width;
	this->height=height;

	this->hand = new HandDetection(1);
	this->hand->init("cascade.xml",width,height);

	this->img_mao1 = cvCreateImage(cvSize(32,32),IPL_DEPTH_8U,1);
	this->img_mao2 = cvCreateImage(cvSize(32,32),IPL_DEPTH_8U,1);

	this->rec_mao1 = cvCreateImage(cvSize(32,32),IPL_DEPTH_8U,1);
	this->rec_mao2 = cvCreateImage(cvSize(32,32),IPL_DEPTH_8U,1);

	this->drawVolante = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);

	this->net = new Network();
	this->net->init();

	return 1;
}
int Hand2Move::free() {
	this->hand->stop();
	this->hand->free();
	this->net->free();
	cvReleaseImage(&this->img_mao1);
	cvReleaseImage(&this->img_mao2);
	cvReleaseImage(&this->rec_mao1);
	cvReleaseImage(&this->rec_mao2);
	cvReleaseImage(&this->drawVolante);

	return 1;
}
int Hand2Move::printHands(IplImage *img) {
	this->hand->printHands(img);

	return 1;
}
int Hand2Move::drawResult() {
	this->drawVolante = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);

	cvFillImage(this->drawVolante,0);
	cvEllipse(this->drawVolante,cvPoint(width/2,height/2),cvSize(width/4,height/8),180-(90+(this->tilt+1)*90),0,360,cvScalar(255,255,255),6);
	cvRectangle(this->drawVolante,cvPoint(width-width/5,height/8),cvPoint(width-20,height-height/8),cvScalar(600,80,0),5);
	cvRectangle(this->drawVolante,cvPoint(width-width/5,height/2 - 5),cvPoint(width-20,height/2 + 5),cvScalar(60,80,0),5);

	cvRectangle(this->drawVolante,cvPoint(width-width/5,height/2),cvPoint(width-20,height/2 - ((height/2.4)*this->deep)),cvScalar(0,0,220),-1);


	cvNamedWindow("volante",0);
	cvShowImage("volante",this->drawVolante);
	return 1;
}
/////////////////////////////////////////////////////////
//////////////////                 CAMERA
/////////////////////////////////////////////////////////
void Cameras::set_dimensao(int width, int height) {
		this->width=width;
		this->height=height;
	}
void Cameras::set_devices(int d1, int d2) {

	    this->capture1 = cvCaptureFromCAM(d1);
	    this->capture2 = cvCaptureFromCAM(d2);

	}
int Cameras::prepare() {

		cvSetCaptureProperty(this->capture1,CV_CAP_PROP_FRAME_WIDTH,this->width);
		cvSetCaptureProperty(this->capture1,CV_CAP_PROP_FRAME_HEIGHT,this->height);
		cvSetCaptureProperty(this->capture1,CV_CAP_PROP_FPS,60);
		cvSetCaptureProperty(this->capture2,CV_CAP_PROP_FRAME_WIDTH,this->width);
		cvSetCaptureProperty(this->capture2,CV_CAP_PROP_FRAME_HEIGHT,this->height);
		cvSetCaptureProperty(this->capture2,CV_CAP_PROP_FPS,60);

		this->rframe1 = cvCreateImage(cvSize(width,height),8,3);
		this->rframe2 = cvCreateImage(cvSize(width,height),8,3);

		return 1;

	}
void Cameras::free() {
		printf("4.1 \n");
		cvReleaseImage(&this->rframe1);
		cvReleaseImage(&this->rframe2);
		printf("4.2 \n");

	}
IplImage* Cameras::getFrame(int dev) {

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
int Cameras::nextFrame() {
		try {
			if(cvGrabFrame(capture1) && cvGrabFrame(capture2)) {
				this->frame1 = cvQueryFrame(capture1);
				this->frame2 = cvQueryFrame(capture2);


				cvSet(this->rframe1,cvScalar(0));
				cvSet(this->rframe2,cvScalar(0));
				cvResize(this->frame1,this->rframe1);
				cvResize(this->frame2,this->rframe2);


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


/////////////////////////////////////////////////////////
//////////////////                 BG SUBSTRACT
/////////////////////////////////////////////////////////
int BGsubstract::init(IplImage *img) {
		this->threshold = cvCloneImage(img);
		this->filtered = cvCreateImage(cvSize(img->width,img->height),img->depth,img->nChannels);
		this->step_filter = cvCreateImage(cvSize(img->width,img->height),img->depth,img->nChannels);
		this->steps=1;


		return 1;
	}
int BGsubstract::step(IplImage *img) {
		this->steps++;
		for(int j=0;j<this->threshold->nSize;j++) {
			this->threshold->imageData[j] = ((this->threshold->imageData[j]*(this->steps-1)+img->imageData[j])/2);
		}

		return 1;
	}
IplImage* BGsubstract::filter(IplImage* img) {
		cvSet(this->step_filter,cvScalar(0));
		cvSet(this->filtered,cvScalar(0));
		cvAbsDiff(img, this->threshold, this->step_filter);
		cvThreshold(this->step_filter,this->filtered, 30, 255,CV_THRESH_BINARY);
		//cvSmooth(this->filtered, this->filtered, CV_GAUSSIAN, 7, 7 );
		cvErode(this->filtered,this->filtered,NULL,1);

		cvNamedWindow( "BG MASK",1);
		cvShowImage( "BG MASK", this->filtered);


		return this->filtered;
	}
int BGsubstract::free() {
		printf("4.3 \n");
		cvReleaseImage(&this->threshold);
		printf("4.3.1 \n");
		//cvReleaseImage(&this->filtered);
		printf("4.3.2 \n");
		cvReleaseImage(&this->step_filter);
		printf("4.4 \n");

		return 1;
	}

/////////////////////////////////////////////////////////
//////////////////                 EYE ENGINE
/////////////////////////////////////////////////////////
int EyeEngine::get_next_frame() {

		this->pseye->nextFrame();

		this->frameLeftRGB = this->pseye->getFrame(0);
		this->frameRightRGB = this->pseye->getFrame(1);



		cvCvtPixToPlane(this->frameLeftRGB,this->RGBleft_R,this->RGBleft_G,this->RGBleft_B,0);
		cvCvtPixToPlane(this->frameRightRGB,this->RGBright_R,this->RGBright_G,this->RGBright_B,0);

		cvCvtColor(this->frameLeftRGB,this->gray, CV_RGB2GRAY);

		cvCvtColor(this->frameLeftRGB,this->frameLeftHSV,CV_RGB2HSV);
		cvCvtColor(this->frameRightRGB,this->frameRightHSV,CV_RGB2HSV);

		cvCvtPixToPlane(this->frameLeftHSV,this->HSVleft_H,this->HSVleft_S,this->HSVleft_V,0);
		cvCvtPixToPlane(this->frameRightHSV,this->HSVright_H,this->HSVright_S,this->HSVright_V,0);

		return 1;

	}
int EyeEngine::estereo_preprocessamento1() {

		cvSet(this->passo_left,cvScalar(0));
		cvSet(this->passo_right,cvScalar(0));
		//TRABALHANDO COM O CANAL VERMELHO DO RGB | ORIGINAL
		cvSmooth(this->RGBleft_R, this->passo_left, CV_GAUSSIAN, 7, 7 );
		cvSmooth(this->RGBright_R, this->passo_right, CV_GAUSSIAN, 7, 7 );
		cvSmooth(this->gray, this->gray, CV_GAUSSIAN, 7, 7 );

		//TENTATIVA DE EQUALIZAR A IMAGEM PARA TODO O PROCESSO
		//PROBLEMA COM O BG REMOVE. EQ ALTERA TON DOS PIXELS QUANDO ME APROXIMO DA CAMERA
		//DEVIDO A MUDANCA De LUMINOSIDADE
		//cvEqualizeHist(this->passo_left,this->passo_left);
		//cvEqualizeHist(this->passo_right,this->passo_right);

		//TRABALHANDO COM O CANAL V DO HSV
		//cvSmooth(this->HSVleft_V, this->passo_left, CV_GAUSSIAN, 7, 7 );
		//cvSmooth(this->HSVright_V, this->passo_right, CV_GAUSSIAN, 7, 7 );

		cvSet(this->tmp1c,cvScalar(0));
		cvAdd(this->passo_left,this->passo_right,this->tmp1c);
		cvShowImage("ADD LEFT RIGHT",this->tmp1c);


		return 1;
	}
int EyeEngine::estereo_posprocessamento1() {
		return 1;
	}
int EyeEngine::estereo_cuda() {
		stereoUpload((unsigned char*) this->passo_left->imageData,(unsigned char*) this->passo_right->imageData);
	    stereoProcess();
	    stereoDownload(this->disp_left,this->disp_right);

		return 1;
	}
int EyeEngine::estereo_opencv() {
		return 1;
	}
int EyeEngine::unir_imagens() {
		return 1;
	}
bool EyeEngine::_disparity2image(IplImage *imagem, const float *disp )
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

int EyeEngine::disparidade2imagem() {
		this->_disparity2image(this->disparidade_left,this->disp_left);
		this->_disparity2image(this->disparidade_right,this->disp_right);
		return 1;
	}

void EyeEngine::init() {
		this->tmp3c = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,3);
		this->tmp1c = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		this->passo_left = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->passo_right = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		this->HSVleft_H = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVleft_S = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVleft_V = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVright_H = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVright_S = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->HSVright_V = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		this->gray = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

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

		this->disparidade_avg = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);

		this->bg_disp_left = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->bg_disp_right = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);
		this->bg_maskAVG = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,1);



		this->hsv_mask = cvCreateImage( cvSize(this->width,this->height), 8, 1);

		this->hsv_min = cvScalar(0, 30, 80, 0);

		this->hsv_max = cvScalar(20, 150, 255, 0);

		stereoInit(this->width,this->height);

		this->h2m = new Hand2Move();
		this->h2m->init(this->width,this->height);


		cvNamedWindow("disp_left",0);
		cvNamedWindow("disp_right",0);
		cvNamedWindow("original_left",0);
		cvNamedWindow("original_right",0);
		cvNamedWindow("AVG",0);
		cvNamedWindow("ADD LEFT RIGHT",0);
	}
int EyeEngine::free() {
		printf("1 \n");
		cvReleaseImage(&this->tmp3c);
		cvReleaseImage(&this->tmp1c);
		cvReleaseImage(&this->HSVleft_H);
		cvReleaseImage(&this->HSVleft_S);
		cvReleaseImage(&this->HSVleft_V);
		cvReleaseImage(&this->HSVright_H);
		cvReleaseImage(&this->HSVright_S);
		cvReleaseImage(&this->HSVright_V);
		cvReleaseImage(&this->gray);
		cvReleaseImage(&this->RGBleft_R);
		cvReleaseImage(&this->RGBleft_G);
		cvReleaseImage(&this->RGBleft_B);
		cvReleaseImage(&this->RGBright_R);
		cvReleaseImage(&this->RGBright_G);
		cvReleaseImage(&this->RGBright_B);
		printf("2 \n");
		cvReleaseImage(&this->passo_left);
		cvReleaseImage(&this->passo_right);
		cvReleaseImage(&this->frameLeftHSV);
		cvReleaseImage(&this->frameRightHSV);
		cvReleaseImage(&this->bg_disp_left);
		cvReleaseImage(&this->bg_disp_right);
		printf("3 \n");
		cvReleaseImage(&this->bg_maskAVG);
		cvReleaseImage(&this->bg_maskL);
		cvReleaseImage(&this->bg_maskR);
		printf("4 \n");

		this->pseye->free();
		this->bgsLeft->free();
		this->bgsRight->free();
		this->h2m->free();
		printf("5 \n");
		//free(this->disp_left);
		//free(this->disp_right);

		cvReleaseImage(&this->disparidade_left);
		cvReleaseImage(&this->disparidade_right);
		cvReleaseImage(&this->disparidade_avg);
		printf("6 \n");


		cvReleaseImage(&this->hsv_mask);

		return  1;
	}
void EyeEngine::rodar() {
		int start_treino=100;
		int treino_bg=start_treino;
		int64 tInit, tGetFrame, tProc, tHaar, tCuda,tCuda2Img, tProc2, tHand;
		if(this->run_mode==1) {



			int key=0;

			//REPETICAO A CADA FRAME
			do {
				tInit = cvGetTickCount();

				this->get_next_frame();
				tGetFrame = cvGetTickCount();




				this->estereo_preprocessamento1();
				tProc= cvGetTickCount();

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
				//BG REMOVE JA TREINADO
				else {
					//cvShowImage("original_left",this->passo_left);
					//cvShowImage("original_right",this->passo_right);

					this->h2m->hand_detect(this->gray);
					tHaar= cvGetTickCount();

					this->estereo_cuda();
					tCuda= cvGetTickCount();

					this->disparidade2imagem();
					tCuda2Img = cvGetTickCount();



					cvCvtColor(this->disparidade_left,this->bg_disp_left,CV_RGB2GRAY);
					cvCvtColor(this->disparidade_right,this->bg_disp_right,CV_RGB2GRAY);

					/*
					cvInRangeS (this->passo_right, this->hsv_min, this->hsv_max, this->hsv_mask);

					cvNamedWindow( "hsv-msk",1);
					cvShowImage( "hsv-msk", this->hsv_mask);

					this->hsv_mask->origin = 1;
					*/


					//AQUI FAZ APENAS O DA ESQUERDA
					this->bg_maskL = this->bgsLeft->filter(this->passo_left);
					cvSet(this->disparidade_avg,cvScalar(0));
					cvCopy(this->bg_disp_left,this->disparidade_avg,this->bg_maskL);
					cvErode(this->disparidade_avg,this->disparidade_avg,NULL,1);
					cvDilate(this->disparidade_avg,this->disparidade_avg,NULL,1);
					cvSmooth(this->disparidade_avg, this->disparidade_avg, CV_GAUSSIAN, 7, 7 );

					tProc2 = cvGetTickCount();


					while(this->h2m->running_hand_detect()) { usleep(1); }
					this-h2m->step(this->passo_left,this->disparidade_avg, this->bg_maskL);
					tHand = cvGetTickCount();

					this->h2m->printHands(this->disparidade_avg);

					cvShowImage("AVG",this->disparidade_avg);
					cvShowImage("disp_left",this->disparidade_left);

					this->h2m->drawResult();

					//AQUI FAZ O OR DAS MASCARAS E TRADA IMG DA ESQUERDA E DA DIREITA
					/*

					this->bg_maskL = this->bgsLeft->filter(this->passo_left);
					this->bg_maskR = this->bgsRight->filter(this->passo_right);



					cvShowImage("disp_right",this->bg_maskR);


					cvSetImageROI(this->bg_maskL,cvRect(this->eye_distance,0,this->width,this->height));
					cvSetImageROI(this->bg_maskR,cvRect(0,0,this->width-this->eye_distance,this->height));
					cvSetImageROI(this->bg_maskAVG,cvRect(this->eye_distance,0,width,this->height));

					cvOr(this->bg_maskL,this->bg_maskR,this->bg_maskAVG);

					cvResetImageROI(this->bg_maskAVG);
					cvResetImageROI(this->bg_maskR);
					cvResetImageROI(this->bg_maskL);


					cvShowImage("disp_left",this->bg_maskAVG);


					cvSet(this->disparidade_avg,cvScalar(0));
					cvSetImageROI(this->bg_disp_left,cvRect(this->eye_distance,0,this->width,this->height));
					cvSetImageROI(this->bg_disp_right,cvRect(0,0,this->width-this->eye_distance,this->height));
					cvSetImageROI(this->disparidade_avg,cvRect(this->eye_distance,0,width,this->height));

					for(int i=0;i<this->bg_disp_left->imageSize;i++) {
						this->disparidade_avg->imageData[i] = (this->bg_disp_left->imageData[i] + this->bg_disp_right->imageData[i])/2;
					}

					cvResetImageROI(this->disparidade_avg);
					cvResetImageROI(this->bg_disp_right);
					cvResetImageROI(this->bg_disp_left);


					cvErode(this->disparidade_avg,this->disparidade_avg,NULL,1);
					cvSmooth(this->disparidade_avg, this->disparidade_avg, CV_GAUSSIAN, 7, 7 );
					cvSet(this->bg_disp_left,cvScalar(0));
					cvCopy(this->disparidade_avg,this->bg_disp_left,this->bg_maskAVG);

					cvShowImage("AVG",this->bg_disp_left);
					*/

					printf("------Time-----------------------\n");
					printf("|   GET FRAME      : %2.4f  |\n",((tGetFrame-tInit)/(cvGetTickFrequency()*1000.0f)));
					printf("|   PREPROC1       : %2.4f   |\n",((tProc-tGetFrame)/(cvGetTickFrequency()*1000.0f)));
					printf("|   HAAR           : %2.4f  |\n",((tHaar - tProc)/(cvGetTickFrequency()*1000.0f)));
					printf("|   CUDA           : %2.4f  |\n",((tCuda - tHaar)/(cvGetTickFrequency()*1000.0f)));
					printf("|   CUDA 2 IMG     : %2.4f   |\n",((tCuda2Img-tCuda)/(cvGetTickFrequency()*1000.0f)));
					printf("|   PROC 2         : %2.4f   |\n",((tProc2-tCuda2Img)/(cvGetTickFrequency()*1000.0f)));
					printf("|   HAND PROC      : %2.4f  |\n",((tHand-tProc2)/(cvGetTickFrequency()*1000.0f)));
					printf("-------------------------------\n");
					printf("|   TOTAL          : %2.4f |\n",((tHand-tInit)/(cvGetTickFrequency()*1000.0f)));
					printf("-------------------------------\n\n");

					//int64 tInit, tGetFrame, tProc, tCuda,tCuda2Img, tProc2, tHand;
				}

				key = cvWaitKey( 3 );
				//printf("char: %c = %d | %d | %d\n",key,key=='q',key,'q');

			} while(key<10); //'q'
		}
	}

int EyeEngine::set_eye_distance(int ed) {
		this->eye_distance = ed;

		return 1;

	}

int EyeEngine::set_dimensao(int width, int height) {
		this->width=width;
		this->height=height;

		return 1;
	}
int EyeEngine::set_run_mode(int modo) {
		this->run_mode=modo;

		return 1;
	}
int EyeEngine::salvar_disparidade() {

		return 1;
	}
