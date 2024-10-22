#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
using namespace cv;

int main()
{
	// each project has their own asset folder, make sure you place the image you want inside the respective asset folders and it should copy over. 
	cv::Mat img = cv::imread("assets/test3.png");

	// the name of the window has to be the same, test open CV for all 3 etc
	namedWindow("Test open CV 3", WINDOW_AUTOSIZE);
	cv::imshow("Test open CV 3", img);
	cv::moveWindow("Test open CV 3", 0, 45);
	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;

}
