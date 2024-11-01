
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

int carveWidth = 0;
int carveHeight = 0;

// idk why need to forward declare again
void DrawBoundary(cv::Mat& img, int pos, cv::Vec3b const& colour = (0, 255, 0));
void DrawBoundaryH(cv::Mat &img, int pos, cv::Vec3b const &colour = (0, 255, 0));


void SeamCarvingToWidthDP(cv::Mat &img, int targetWidth, bool isRemovingObject = false);
void SeamCarvingToHeightDP(cv::Mat& img, int targetHeight, bool isRemovingObject = false);

cv::Mat CalculateCumMap(const cv::Mat &energyMap);

void callback(int pos, void *userData)
{
	carveWidth = pos;
	cv::Mat imgClone = static_cast<cv::Mat *>(userData)->clone();
	DrawBoundary(imgClone, pos);
}

void callbackH(int pos, void* userData)
{
	carveHeight = pos;
	cv::Mat imgClone = static_cast<cv::Mat*>(userData)->clone();
	DrawBoundaryH(imgClone, pos);
}

int main()
{
	// ==============
	// LOAD THE IMAGE
	// ==============

	// load the image
	cv::Mat img = cv::imread("assets/clock.png");

	// ensure image loaded properly
	if (img.empty())
	{
		std::cerr << "Error: Could not load image.\n";
		return -1;
	}

	// ===============
	// SET THE WINDOWS
	// ===============

	// what's this for
	//cv::namedWindow("Output", cv::WINDOW_AUTOSIZE);
	//cv::namedWindow("Input", cv::WINDOW_AUTOSIZE);

	// =======================
	// GET ORIGINAL ENERGY MAP
	// =======================

#if 1
	// Split the image into its 3 channels (B, G, R)
	std::vector<cv::Mat> channels;
	cv::split(img, channels);  // channels[0] = Blue, channels[1] = Green, channels[2] = Red

	// get original energy map
	cv::Mat energyMap = CalculateEnergyMap(channels);
	cv::Mat displayEnergyMap;

	// Convert the energy map back to 8-bit format for display
	cv::normalize(energyMap, energyMap, 0, 255, cv::NORM_MINMAX);
	//ModifyEnergyMap(energyMap, { 0, 400 }, { 150, 540 }, 0.0);
	energyMap.convertTo(displayEnergyMap, CV_8U);
	cv::Mat cumMap = CalculateCumMap(energyMap);
	cv::Mat displayCumMap;
	cumMap.convertTo(displayCumMap, CV_8U);
#endif

	// ===================
	// DISPLAY THE WINDOWS
	// ===================


	//cv::moveWindow("Original Image", 0, 45);
	//cv::moveWindow("Original Energy Map", img.cols, 45);

	// clone the original image for the seam carving
	carveWidth = img.cols;
	cv::Mat imgClone = img.clone();

	// draw windows and ui
	cv::imshow("Input", img);
	cv::putText(img, "Press 'space' to start carving!", { 10, 30 }, cv::FONT_HERSHEY_SIMPLEX, 0.5, { 255, 255, 255 });
	cv::createTrackbar("Width", "Input", nullptr, img.cols, callback, &img);
	cv::createTrackbar("Height", "Input", nullptr, img.rows, callbackH, &img);
	cv::imshow("Output", imgClone);
	cv::imshow("Object Removal", displayCumMap);

	// set mouse callback (to display the mouse coordinates as will as the respective RGB values of selected pixel)
	cv::setMouseCallback("Output", util::mouseCallback, &imgClone);

	// game loop
	while (true)
	{
		int key = cv::waitKey(0);

		if (key == ' ')
		{
			if (carveWidth > imgClone.cols)
				imgClone = img.clone();
			SeamCarvingToWidthDP(imgClone, carveWidth, true);
		}

		if (key == '1')
		{
			if (carveHeight > imgClone.rows)
				imgClone = img.clone();
			SeamCarvingToHeightDP(imgClone, carveHeight, false);
		}

		if (key == cv::ESC_KEY)
			break;
	}

	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;
}