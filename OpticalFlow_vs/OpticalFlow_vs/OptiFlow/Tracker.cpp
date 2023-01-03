#include "StdAfx.h"
#include "Tracker.h"

Tracker::Tracker(int max_corners, int win_size) 
	: max_corners(max_corners), win_size(win_size), tracking(false)
{
	initialize();
}

Tracker::~Tracker(void)
{
	//Cleanup
    cvDestroyWindow("Img0");
    cvDestroyWindow("Img1");
    cvDestroyWindow("Optical Flow");

    cvReleaseCapture(&capture);

    cvReleaseImage(&imgA);
    cvReleaseImage(&imgB);
    cvReleaseImage(&eig_image);
    cvReleaseImage(&tmp_image);
    cvReleaseImage(&out);
    cvReleaseImage(&tmp);
    cvReleaseImage(&pyrA);
    cvReleaseImage(&pyrB);

	delete features_found;
	delete feature_errors;
}

void Tracker::start()
{
	cvNamedWindow("Img0", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Img1", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Optical Flow", 0);
	cvResizeWindow("Optical Flow", (int)(480/1.5), (int)(340/1.5));

    //Grab initial frame from capture and convert it to B/W
    tmp = cvQueryFrame(capture);
    cvConvertImage(tmp, imgA, 0); 

	tracking = true;

    do
    {
		/* KEYBOARD CONTROLS: 
		 *  w : increment (by 10) number of corners to track (max=400)
		 *	s : decrement (by 10) number of corners to track (min=10)
		 *	f : double pyramid window size (max=64)
		 *	a : half pyramid window size (min=2)
		 */
		
		switch (cvWaitKey(1))
		{
		case 27:
			tracking = false;
			break;
		case 'w':
			max_corners = max_corners < 400 ? max_corners + 10 : 400;
			break;
		case 's':
			max_corners = max_corners > 10 ? max_corners - 10 : 10;
			break;
		case 'a':
			win_size = win_size > 2 ? win_size / 2 : 2;
			break;
		case 'd':
			win_size = win_size < 64 ? win_size * 2 : 64; 
			break;
		}

        // Make something to draw on
        cvCopy(tmp, out, 0); /**/

        // Grab frame from capture and convert it to B/W
        tmp = cvQueryFrame(capture);
        cvConvertImage(tmp, imgB, 0);

        int corner_count = this->max_corners;

        /*
            img
            eig_image, tmp_image - scratch space
            corners - will hold max_corners features after call
            corner_count - IN find at most # corners OUT corners found
            quality, min_distance
        */

        cvGoodFeaturesToTrack( imgA, eig_image, tmp_image, cornersA,
             &corner_count, 0.01, 5.0);

        cvFindCornerSubPix( imgA, cornersA, corner_count,
			cvSize(win_size, win_size), cvSize(-1,-1),
			cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03));

        cvCalcOpticalFlowPyrLK( imgA, imgB, pyrA, pyrB, cornersA, cornersB,
            max_corners, cvSize(win_size,win_size),
            5, features_found, feature_errors,
            cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.3), 0);

		// Draw the vectors
        for(int i = 0; i < max_corners; i++)
        {
            //Feature lost or error too high (probably didn't find it)
            if(features_found[i] == 0 || feature_errors[i] > 550)
                continue;

            CvPoint p0 = cvPoint(cvRound(cornersA[i].x), cvRound(cornersA[i].y));
            CvPoint p1 = cvPoint(cvRound(cornersB[i].x), cvRound(cornersB[i].y));

            cvCircle(out, p0, 3, CV_RGB(0,0,255), -1, 8);
            cvLine(out, p0, p1, CV_RGB(0, 255, 0), 2, 8);
        }
		
        cvShowImage("Img0", imgA);
        cvShowImage("Img1", imgB);
        cvShowImage("Optical Flow", out);

        //Push frame back in sequence
        cvCopy(imgB, imgA, 0); 
    
	} while (tracking);

	system("pause");
}

void Tracker::initialize()
{
	 //Create a capture from the first camera
	capture = cvCaptureFromCAM(0); 
	
	if(!capture){
		printf("could not connect to camera");
	}

    //Grab image size of a frame from the capture
	IplImage* frame = cvQueryFrame(capture);
	while(!frame){
		frame = cvQueryFrame(capture);
	}
	
    img_sz = cvGetSize(frame); 

    imgA = cvCreateImage(img_sz, IPL_DEPTH_8U, 1); 
    imgB = cvCreateImage(img_sz, IPL_DEPTH_8U, 1); 

    out = cvCreateImage(img_sz, IPL_DEPTH_8U, 3); 
    tmp = cvCreateImage(img_sz, IPL_DEPTH_8U, 3); 

    //Scratch space for Shi-Tomasi
    eig_image = cvCreateImage(img_sz, IPL_DEPTH_32F, 1);
    tmp_image = cvCreateImage(img_sz, IPL_DEPTH_32F, 1);

    cornersA = new CvPoint2D32f[400];
    cornersB = new CvPoint2D32f[400];

    //Features details for PyrLK
    features_found = new char[400];
    feature_errors = new float[400];

	//Min size required by algorithm for scratch space
    pyr_sz = cvSize(imgA->width+8, imgA->height/3);

    pyrA = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);
    pyrB = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);
}