
// openCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

// std lib
#include <vector>
#include <iostream>

// seam carving
#include "SeamCarving.h"
#include "utility.h"

//using namespace cv;
cv::Mat brushMask;
bool isDrawing = false;
int brushSize = 10;
bool maskInitialized = false;
int main()
{
	// ==============
	// LOAD THE IMAGE
	// ==============
	
	// load the image
	cv::Mat img = cv::imread("assets/test3.png");

	// ensure image loaded properly
	if (img.empty())
	{
		std::cerr << "Error: Could not load image.\n";
		return -1;
	}

	// ===============
	// SET THE WINDOWS
	// ===============

	cv::namedWindow("Original Image", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("Original Energy Map", cv::WINDOW_AUTOSIZE);

	brushMask = cv::Mat::zeros(img.size(), CV_8UC1);
	maskInitialized = true;

	// set mouse callback (to display the mouse coordinates as will as the respective RGB values of selected pixel)
	cv::setMouseCallback("Original Image", util::mouseCallback, &img);

	
	// =======================
	// GET ORIGINAL ENERGY MAP
	// =======================
	
	// Split the image into its 3 channels (B, G, R)
	std::vector<cv::Mat> channels;
	cv::split(img, channels);  // channels[0] = Blue, channels[1] = Green, channels[2] = Red
	
	// get original energy map
	cv::Mat energyMap = CalculateEnergyMap(channels);

	cv::Mat displayEnergyMap;

	// Convert the energy map back to 8-bit format for display
	cv::normalize(energyMap, energyMap, 0, 255, cv::NORM_MINMAX);
	energyMap.convertTo(displayEnergyMap, CV_8U);

	int rows = energyMap.rows, cols = energyMap.cols;
	

	// ===================
	// DISPLAY THE WINDOWS
	// ===================

	cv::imshow("Original Image", img);
	cv::imshow("Original Energy Map", displayEnergyMap);

	cv::moveWindow("Original Image", 0, 45);
	cv::moveWindow("Original Energy Map", img.cols, 45);


	// clone the original image for the seam carving
	cv::Mat imgClone = img.clone();
	cv::Mat originalImg = img.clone(); 

	// game loop
	while (true) 
	{
		int key = cv::waitKey(1);

		if (key == 'c')
			SeamCarvingToWidth(imgClone, 500);

		if (key == 'b')
			SeamCarvingToHeight(imgClone, 400);

		if (key == 'd') 
		{
			ContentAwareRemoval(imgClone);
			cv::imshow("Original Image", imgClone);
		}
		else if (key == 'r') 
		{
			imgClone = originalImg.clone();
			brushMask = cv::Mat::zeros(img.size(), CV_8UC1);
			cv::imshow("Original Image", imgClone);
		}
		else if (key == cv::ESC_KEY)
			break;
	}
	


	cv::destroyAllWindows();

	return 0;

}
