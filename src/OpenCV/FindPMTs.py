#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

int main(int /*argc*/, char** /*argv*/)
{
    Mat src; Mat src_gray; Mat canny_output;
    int thresh = 30;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    src = imread("20180920_00_0_25_img_0051.jpg", 1);

    cvtColor(src, src_gray, CV_BGR2GRAY);
    blur(src_gray, src_gray, Size(3, 3));
    Canny(src_gray, canny_output, thresh, thresh * 2, 3);
    /// Find contours
    findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    /// Draw contours
    Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
    for (int i = 0; i< contours.size(); i++)
    {
        if (contourArea(contours[i]) > 100) {       //filter noise
            drawContours(drawing, contours, i, Scalar(255, 255, 255), -1, 8, hierarchy, 0, Point());
        }
    }
    namedWindow("Comb", CV_WINDOW_AUTOSIZE);
    imshow("Comb", drawing);
    waitKey(0);
    return(0);
}
