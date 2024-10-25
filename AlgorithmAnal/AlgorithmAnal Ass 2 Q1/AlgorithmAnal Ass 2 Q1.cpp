#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>

#undef MAX
#define nl "\n"
#define MAX std::numeric_limits<double>::max()
// MAX was 255

int main()
{
	// each project has their own asset folder, make sure you place the image you want inside the respective asset folders and it should copy over. 
	cv::Mat img = cv::imread("assets/test3_cropped.png");

	// Split the image into its 3 channels (B, G, R)
	std::vector<cv::Mat> channels;
	cv::split(img, channels);  // channels[0] = Blue, channels[1] = Green, channels[2] = Red
	cv::Mat energyMap = cv::Mat::zeros(img.size(), CV_64F);  // Initialize energy map

	// 3 because there's 3 channels
	for (int i = 0; i < 3; ++i)
	{
		// Compute the gradients in X and Y directions for each color channel
		cv::Mat grad_x, grad_y;
		cv::Sobel(channels[i], grad_x, CV_64F, 1, 0, 1);  // X direction gradient
		cv::Sobel(channels[i], grad_y, CV_64F, 0, 1, 1);  // Y direction gradient

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

	int rows = energyMap.rows, cols = energyMap.cols;
	cv::Mat cumMap(energyMap.size(), CV_64F); // cumulative energy map

	double max = 0.0;
#if 0
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j)
			max = energyMap.at<double>(i, j) > max ? energyMap.at<double>(i, j) : max;

	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j)
			energyMap.at<double>(i, j) = energyMap.at<double>(i, j) / max;
#endif

	// copy last row over
	for (int j = 0; j < cols; ++j)
		cumMap.at<double>(rows - 1, j) = energyMap.at<double>(rows - 1, j);

	// cumulatively sum best energy value from bottom to top, taking only 3 pixels into account
	max = 0.0;
	for (int i = rows - 2; i > -1; --i)
		for (int j = 0; j < cols; ++j)
		{
			double leftVal = j ? cumMap.at<double>(i + 1, j - 1) : MAX;
			double midVal = cumMap.at<double>(i + 1, j);
			double rightVal = j < cols - 1 ? cumMap.at<double>(i + 1, j + 1) : MAX;
			double minVal = std::min({ leftVal, midVal, rightVal });
			double &currVal = cumMap.at<double>(i, j);
			currVal = energyMap.at<double>(i, j) + minVal;
			max = currVal > max ? currVal : max;
		}

	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j)
			cumMap.at<double>(i, j) = cumMap.at<double>(i, j) / max * 255.0;

	// find col with smallest cumulative sum in the first row
	int smallestCum = 0;
	for (int j = 1; j < cols; ++j)
		smallestCum = cumMap.at<double>(0, j) < cumMap.at<double>(0, smallestCum) ? j : smallestCum;

	// find path of least resistance (aka the seam to cut)
	int col = smallestCum;
	for (int i = 0; i < rows; ++i)
	{
		if (col)
			cumMap.at<double>(i, col - 1) = 255.0;
		cumMap.at<double>(i, col) = 255.0;
		if (col - 1 < cols)
			cumMap.at<double>(i, col + 1) = 255.0;
		if (i + 1 == rows)
			break;

		double leftVal = col ? cumMap.at<double>(i + 1, col - 1) : MAX;
		double midVal = cumMap.at<double>(i + 1, col);
		double rightVal = col < cols - 1 ? cumMap.at<double>(i + 1, col + 1) : MAX;
		col = leftVal < midVal ? leftVal < rightVal ? col - 1 : col + 1 : midVal < rightVal ? col : col + 1;
	}

	cumMap.convertTo(img, CV_8U);

	// the name of the window has to be the same, test open CV for all 3 etc
	cv::namedWindow("Original Image", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("Energy Map", cv::WINDOW_AUTOSIZE);
	cv::imshow("Original Image", img);
	cv::imshow("Energy Map", displayEnergyMap);

	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;

}
