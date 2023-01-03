#include "stdafx.h"
#include "between.h"

void load (Image**, char**);
void get_corners (Image*, CvPoint2D32f**);
Image* lucas_kanade (Image**, CvPoint2D32f**);
Image* draw (CvPoint2D32f**, char*, float*);
void display (Image**, Image*);

int between_frames(int argc, char* argv[])
{
  /* You can specify the two image names as the first two command line
   * arguments to the executable. EG: $program.out a.jpg b.jpg. Otherwise,
   * we fall back on hardcoded defaults defined above.
   */

 int i;
 Image* frame[2];		// << storage for 2 successive image frames
 Image* result;			// << output image with drawn flow field
 CvPoint2D32f* corners[2];	// << tracking information from first frame


    // Init: allocate array for "corner" information
    corners[0] = (CvPoint2D32f*)calloc(N_CORNERS, sizeof(CvPoint2D32f));
    corners[1] = (CvPoint2D32f*)calloc(N_CORNERS, sizeof(CvPoint2D32f));


    // 1) Open image files, store them in array.
    load( frame, argv );

    // 2) Get corners from first frame to track into second.
    get_corners( frame[0], &corners[0] );

    // 3) Configure and call Lucas-Kanade logic, render result.
    result = lucas_kanade( frame, corners );
    
    // 4) Draw result to screen
    display( frame, result );

    // Coda: delete objects --------------------------------------- //

    cvReleaseImage(&result);
    for (i = 0; i <= 1; ++i) {
        cvReleaseImage(&frame[i]);
        free(corners[i]);
    }

    return (0);
}

/* Get Corners:  selects corner features to track between images */
/******************************************************************************/
static void get_corners(Image* image, CvPoint2D32f** corners)
{
 int n_corners;
 CvSize size;
 Image* eig;
 Image* tmp;

    size = cvGetSize(image);
    eig = cvCreateImage(size, IPL_DEPTH_32F, 1);
    tmp = cvCreateImage(size, IPL_DEPTH_32F, 1);
    n_corners = N_CORNERS;
    
    // finds corners with big eigenvalues in the image.
    cvGoodFeaturesToTrack(image, eig, tmp, *corners, &n_corners, 
        .05f, 5.f, 0, 3, 0, .04f);
    
    cvFindCornerSubPix(image, corners[0], n_corners, 
        (CvSize)cvSize(WINDOW_SIZE, WINDOW_SIZE), (CvSize)cvSize(-1, -1),
        cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03f));
    
    cvReleaseImage(&eig);
    cvReleaseImage(&tmp);
}


/* Lukas Kanade:  applys Lukas-Kanade pyamidal analysis; then draws */
/******************************************************************************/
static Image* lucas_kanade(Image** frame, CvPoint2D32f** corners)
{
 char features[N_CORNERS];
 float errors[N_CORNERS];
 CvSize scratch;
 Image* pyr[2];
 
    // set scratch space size
    scratch = cvSize(frame[0]->width + 8, frame[0]->height / 3);

 	// allocate pyramid data storage
    pyr[0] = cvCreateImage(scratch, IPL_DEPTH_32F, 1);
    pyr[1] = cvCreateImage(scratch, IPL_DEPTH_32F, 1);

    cvCalcOpticalFlowPyrLK(
        frame[0], frame[1], pyr[0], pyr[1], corners[0], corners[1],
        N_CORNERS, (CvSize)cvSize(WINDOW_SIZE, WINDOW_SIZE), 5, features, 
        errors, cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, .3f), 0);

    cvReleaseImage(&pyr[0]);
    cvReleaseImage(&pyr[1]);

    return ( draw(corners, features, errors) ); 	// << draw here
}

/* Draw:  generates image with vectors illustrating the optical flow  */
/******************************************************************************/
static Image* draw(CvPoint2D32f** corners, char* features, float* errors)
{
 int i, j;
 CvPoint p[2];
 Image* out;

    // create image to draw on
    out = cvLoadImage("image0.jpg", CV_LOAD_IMAGE_UNCHANGED);

    for (i = 0; i < N_CORNERS; ++i)
    {
        if ( !features[i] || errors[i] > 550 ) { continue; }
        
        for (j = 0; j <= 1; ++j)
            p[j] = cvPoint(cvRound(corners[j][i].x), cvRound(corners[j][i].y));

        cvCircle(out, p[0], 3, CV_RGB(0, 0, 255), -1, 8);
        cvLine(out, p[0], p[1], CV_RGB(0, 255, 0), 2, 8);
    }
    
    return (out);
}

/* Load:  reads in input files */
/******************************************************************************/
static void load(Image** frame, char** argv)
{
 char fn[64];
 
    memset(fn, 0, 64 * sizeof(char));
    strcpy(fn, (argv && argv[2]) ? argv[2] : DEFAULT_IMAGE_0);
    
    // open first image
    if ( (frame[0] = cvLoadImage(fn, CV_LOAD_IMAGE_GRAYSCALE)) == NULL ) 
    {
        fprintf(stderr, "Could not open %s. Exiting...\n", fn);
        exit(EXIT_FAILURE);
    }

    memset(fn, 0, 64 * sizeof(char));
    strcpy(fn, (argv && argv[3] && argv[2]) ? argv[3] : DEFAULT_IMAGE_1);
    
    // open second image
    if ( (frame[1] = cvLoadImage(fn, CV_LOAD_IMAGE_GRAYSCALE)) == NULL ) 
    {
        fprintf(stderr, "Could not open %s. Exiting...\n", fn);
        exit(EXIT_FAILURE);
    }
}

/* Display:  Shows output */
/******************************************************************************/
static void display(Image** frame, Image* result)
{
    cvNamedWindow("A", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("B", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("flow(A, B)", CV_WINDOW_AUTOSIZE);

    cvShowImage("A", frame[0]);
    cvShowImage("B", frame[1]);
    cvShowImage("flow(A, B)", result);

    cvWaitKey(0);
    
    cvDestroyWindow("A");
    cvDestroyWindow("A");
    cvDestroyWindow("flow(A, B)");
}

