// ObjectTracking.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv\cv.h"
#include "opencv\highgui.h"
#include <Windows.h>
#include <WinUser.h>
IplImage* imgTracking;
int lastX = -1;
int lastY = -1;


IplImage* GetThresholdedImage(IplImage* imgHSV){
	IplImage* imgThresh = cvCreateImage(cvGetSize(imgHSV), IPL_DEPTH_8U, 1);
	cvInRangeS(imgHSV, cvScalar(170, 160, 60), cvScalar(180, 2556, 256), imgThresh);
	return imgThresh;
}

void trackObject(IplImage* imgThresh){
	
	CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
	cvMoments(imgThresh, moments, 1);
	double moment10 = cvGetSpatialMoment(moments, 1, 0);
	double moment01 = cvGetSpatialMoment(moments, 0, 1);
	double area = cvGetCentralMoment(moments, 0, 0);

	
	if (area>1000){
		
		int posX = moment10 / area;
		int posY = moment01 / area;

		if (lastX >= 0 && lastY >= 0 && posX >= 0 && posY >= 0)
		{
			
			cvLine(imgTracking, cvPoint(posX, posY), cvPoint(lastX, lastY), cvScalar(0, 0, 255), 4);
		}

		lastX = posX;
		lastY = posY;
	}

	free(moments);
}


int main(){

	CvCapture* capture = 0;
	capture = cvCaptureFromCAM(0);
	if (!capture){
		printf("Run out of Memory! :( \n");
		return -1;
	}

	IplImage* frame = 0;
	frame = cvQueryFrame(capture);
	if (!frame) return -1;

	
	imgTracking = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
	cvZero(imgTracking); //covert the image, 'imgTracking' to black

	cvNamedWindow("Source");
	cvNamedWindow("Tracked");

	
	while (true){

		frame = cvQueryFrame(capture);
		if (!frame) break;
		frame = cvCloneImage(frame);

		cvSmooth(frame, frame, CV_GAUSSIAN, 3, 3); 
		SetCursorPos(lastX,lastY);
		IplImage* imgHSV = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
		cvCvtColor(frame, imgHSV, CV_BGR2HSV); 
		IplImage* imgThresh = GetThresholdedImage(imgHSV);

		cvSmooth(imgThresh, imgThresh, CV_GAUSSIAN, 3, 3); 

	
		trackObject(imgThresh);
		cvAdd(frame, imgTracking, frame);

		cvShowImage("Tracked", imgThresh);
		cvShowImage("Source", frame);

		
		cvReleaseImage(&imgHSV);
		cvReleaseImage(&imgThresh);
		cvReleaseImage(&frame);

		
		int c = cvWaitKey(10);
		
		if ((char)c == 27) break;
	}

	cvDestroyAllWindows();
	cvReleaseImage(&imgTracking);
	cvReleaseCapture(&capture);

	return 0;
}
