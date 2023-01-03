#include "stdafx.h"
#include <cv.h>
#include <highgui.h>

#define MAX_CORNERS 100

int flow()
{
    //Load two images for OF sequence
    IplImage *imgA = cvLoadImage("image0.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    IplImage *imgB = cvLoadImage("image1.jpg", CV_LOAD_IMAGE_GRAYSCALE);

    //Image for pretty pictures
    IplImage *out  = cvLoadImage("image0.jpg", CV_LOAD_IMAGE_COLOR);

    CvSize img_sz = cvGetSize(imgA);
    int win_size = 10; //Size of search window at each pyramid

    //Scratch space for Shi-Tomasi
    IplImage *eig_image = cvCreateImage(img_sz, IPL_DEPTH_32F, 1);
    IplImage *tmp_image = cvCreateImage(img_sz, IPL_DEPTH_32F, 1);

    int corner_count = MAX_CORNERS;
    CvPoint2D32f *cornersA = new CvPoint2D32f[MAX_CORNERS];

    /*
        img
        eig_image, tmp_image - scratch space
        corners - will hold corner_count features after call
        corner_count - IN find at most # corners OUT corners found
        quality, min_distance
    */
    cvGoodFeaturesToTrack( imgA, eig_image, tmp_image, cornersA,
                            &corner_count, 0.01, 5.0);

    cvFindCornerSubPix( imgA, cornersA, corner_count,
                        cvSize(win_size, win_size), cvSize(-1,-1),
                        cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03));

    char features_found[MAX_CORNERS];
    float feature_errors[MAX_CORNERS];

    //Min size required by algorithm for scratch space
    CvSize pyr_sz = cvSize(imgA->width+8, imgA->height/3);

    IplImage *pyrA = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);
    IplImage *pyrB = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);

    CvPoint2D32f *cornersB = new CvPoint2D32f[MAX_CORNERS];

    cvCalcOpticalFlowPyrLK( imgA, imgB, pyrA, pyrB, cornersA, cornersB,
                            corner_count, cvSize(win_size,win_size),
                            5, features_found, feature_errors,
                            cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.3), 0);

    for(int i = 0; i < corner_count; i++)
    {
        //Feature lost or error too high (probably didn't find it)
        if(features_found[i] == 0 || feature_errors[i] > 550)
            continue;

        CvPoint p0 = cvPoint(cvRound(cornersA[i].x), cvRound(cornersA[i].y));
        CvPoint p1 = cvPoint(cvRound(cornersB[i].x), cvRound(cornersB[i].y));

        cvCircle(out, p0, 3, CV_RGB(0,0,255), -1, 8);
        cvLine(out, p0, p1, CV_RGB(0, 255, 0), 2, 8);
    }

    cvNamedWindow("Img0", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Img1", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Optical Flow", CV_WINDOW_AUTOSIZE);

    cvShowImage("Img0", imgA);
    cvShowImage("Img1", imgB);
    cvShowImage("Optical Flow", out);

    cvWaitKey(0);

    //Cleanup
    cvDestroyWindow("Img0");
    cvDestroyWindow("Img1");
    cvDestroyWindow("Optical Flow");

    cvReleaseImage(&imgA);
    cvReleaseImage(&imgB);
    cvReleaseImage(&eig_image);
    cvReleaseImage(&tmp_image);
    cvReleaseImage(&out);
    cvReleaseImage(&pyrA);
    cvReleaseImage(&pyrB);

    return 0;
}
