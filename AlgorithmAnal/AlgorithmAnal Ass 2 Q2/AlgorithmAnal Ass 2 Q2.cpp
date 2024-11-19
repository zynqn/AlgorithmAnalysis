
// openCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

// std lib
#include <vector>
#include <iostream>

// seam carving
#include "SeamCarving2.h"
#include "Utility2.h"


int main()
{
	ShowCursor(FALSE);

	// ==============
	// LOAD THE IMAGE
	// ==============

	// load the image
	cv::Mat img = cv::imread("assets/surfer.jpg");

	// ensure image loaded properly
	if (img.empty())
	{
		std::cerr << "Error: Could not load image.\n";
		return -1;
	}

	// ===============
	// SET THE WINDOWS
	// ===============

	cv::namedWindow(ORIGINAL_IMAGE, cv::WINDOW_NORMAL);
	cv::setWindowProperty(ORIGINAL_IMAGE, cv::WND_PROP_AUTOSIZE, cv::WINDOW_NORMAL);

	brushMask = cv::Mat::zeros(img.size(), CV_8UC1);
	maskInitialized = true;

	// set mouse callback (to display the mouse coordinates as will as the respective RGB values of selected pixel)
	cv::setMouseCallback(ORIGINAL_IMAGE, util::mouseCallback, &img);

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

	resolution = static_cast<float>(rows) / static_cast<float>(cols);
	cv::imshow(ORIGINAL_IMAGE, img);

	// clone the original image for the seam carving
	cv::Mat imgClone = img.clone();
	cv::Mat originalImg = img.clone();

	editor.Init();

	// game loop
	while (true)
	{
		editor.Update();
		util::LockWindow(ORIGINAL_IMAGE_W, 0, 0, static_cast<int>(scale), static_cast<int>(scale * resolution));
		int key = cv::waitKey(1);

		if (key == 'c')
			VerticalSeamCarvingGraphCut(imgClone, 500);

		if (key == 'b')
			HorizontalSeamCarvingDP(imgClone, 400);

		if (key == 'd')
		{
			ContentAwareRemoval(imgClone);
			cv::imshow(ORIGINAL_IMAGE, imgClone);
		}
		else if (key == 'r')
		{
			imgClone = originalImg.clone();
			brushMask = cv::Mat::zeros(img.size(), CV_8UC1);
			cv::imshow(ORIGINAL_IMAGE, imgClone);
		}
		else if (key == cv::ESC_KEY)
			break;
	}

	ShowCursor(TRUE);

	editor.Shutdown();
	cv::destroyAllWindows();
}
