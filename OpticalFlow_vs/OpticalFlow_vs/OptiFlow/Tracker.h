#pragma once
#include <cv.h>
#include <highgui.h>

class Tracker
{
public:
	Tracker(int max_corners=250, int win_size=10);
	~Tracker(void);
	void start();

private:
	CvCapture *capture;
	IplImage *out, *tmp, *eig_image, *tmp_image, *imgA, *imgB, *pyrA, *pyrB;
	CvSize img_sz, pyr_sz;
	CvPoint2D32f *cornersA, *cornersB;
	int max_corners;
	int win_size; //Size of search window at each pyramid
	char* features_found;
	float* feature_errors;
	bool tracking;

	void initialize();
};

