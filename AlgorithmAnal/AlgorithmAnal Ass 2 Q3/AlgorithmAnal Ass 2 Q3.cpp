#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <vector>
#include <iostream>


//using namespace cv;

int main()
{
	// load the image
	cv::Mat img = cv::imread("assets/test3.png");
	

	// ensure image loaded properly
	if (img.empty())
	{
		std::cerr << "Error: Could not load image.\n";
		return -1;
	}

	// convert the image to greyscale first to calculate the energy map, as greyscale is better to calculate intensity value
	//cv::Mat gray;
	//cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

	// Split the image into its 3 channels (B, G, R)
	std::vector<cv::Mat> channels;
	cv::split(img, channels);  // channels[0] = Blue, channels[1] = Green, channels[2] = Red

	cv::Mat energyMap = cv::Mat::zeros(img.size(), CV_64F);  // Initialize energy map

	for (int i{}; i < 3; ++i)
	{
		// Compute the gradients in X and Y directions for each color channel
		cv::Mat grad_x, grad_y;
		cv::Sobel(channels[i], grad_x, CV_64F, 1, 0, 3);  // X direction gradient
		cv::Sobel(channels[i], grad_y, CV_64F, 0, 1, 3);  // Y direction gradient

		// Compute the energy as the sum of absolute gradients for the current channel
		cv::Mat abs_grad_x, abs_grad_y;
		cv::convertScaleAbs(grad_x, abs_grad_x);  // Absolute value of X gradient
		cv::convertScaleAbs(grad_y, abs_grad_y);  // Absolute value of Y gradient

		// Add the energy of this channel to the overall energy map
		energyMap += abs_grad_x + abs_grad_y;
	}

	// Convert the energy map back to 8-bit format for display
	cv::Mat displayEnergyMap;
	cv::normalize(energyMap, energyMap, 0, 255, cv::NORM_MINMAX);
	energyMap.convertTo(displayEnergyMap, CV_8U);

	// calculate gradient using sobel
	//cv::Mat gradX, gradY;
	//cv::Sobel(gray, gradX, CV_64F, 1, 0); // gradient in X direction
	//cv::Sobel(gray, gradY, CV_64F, 0, 1); // gradient in Y direction

	// compute energy map as sum of absolute gradients
	//cv::Mat absGradX, absGradY, energyMap;

	// compute energy map, which will be the sum of the absolute of the x and y gradient
	//cv::convertScaleAbs(gradX, absGradX);
	//cv::convertScaleAbs(gradY, absGradY);

	//cv::addWeighted(absGradX, 0.5, absGradY, 0.5, 0, energyMap);

	cv::imshow("Original Image", img);
	cv::imshow("Energy Map", displayEnergyMap);

	cv::moveWindow("Original Image", 0, 45);
	cv::moveWindow("Energy Map", img.cols, 45);

	// the name of the window has to be the same, test open CV for all 3 etc
	//cv::namedWindow("Test open CV 3", cv::WINDOW_AUTOSIZE);
	//cv::imshow("Test open CV 3", img);
	//cv::moveWindow("Test open CV 3", 0, 45);
	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;

}
