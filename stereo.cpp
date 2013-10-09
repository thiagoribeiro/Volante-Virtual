//#define IMAGE_W     450
//#define IMAGE_H     375
#define IMAGE_W     640
#define IMAGE_H     480


#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>

#include "volante.h"

using namespace std;
#include <float.h>
#include <stdio.h>
#include <iostream>
#include <time.h>




//VERDADEIRO
int main() {

	EyeEngine* ee = new EyeEngine();


	ee->set_dimensao(640,480);
	ee->set_eye_distance(27);
	ee->set_run_mode(1);
	ee->init();
	ee->rodar();
	ee->free();

	return 1;
}

