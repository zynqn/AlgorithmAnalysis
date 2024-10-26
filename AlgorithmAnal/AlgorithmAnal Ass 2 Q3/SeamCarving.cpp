#include "SeamCarving.h"

#include <vector>
#include <iomanip>
#include <iostream>

cv::Mat CalculateEnergyMap(std::vector<cv::Mat> const& channels)
{
	cv::Mat gradX, gradY;
	cv::Mat energyMap = cv::Mat::zeros(channels[0].size(), CV_64F);

	for (const auto& channel : channels) 
	{
		cv::Sobel(channel, gradX, CV_64F, 1, 0);
		cv::Sobel(channel, gradY, CV_64F, 0, 1);

		energyMap += cv::abs(gradX) + cv::abs(gradY);
	}

	return energyMap;
}

std::vector<int> FindVerticalSeamGreedy(cv::Mat const& energyMap)
{
	int rows = energyMap.rows;
	int cols = energyMap.cols;
	std::vector<int> seam(rows);

	// Start at the minimum energy pixel in the first row
	double minVal;
	cv::Point minLoc;
	cv::minMaxLoc(energyMap.row(0).colRange(1, cols - 1), &minVal, nullptr, &minLoc, nullptr);
	seam[0] = minLoc.x;

	//return seam;

#if 1

	// iterate through the subsequent rows to find the min energy pixel
	for (int i = 1; i < rows; ++i)
	{
		// get previous column index from the seam vector
		int prevX = seam[i - 1];

		// assign the previous column first
		seam[i] = prevX;

		// check left neighbour
		if (prevX > 0 && energyMap.at<uchar>(i, prevX - 1) < energyMap.at<uchar>(i, seam[i]))
			seam[i] = prevX - 1;

		// check right neighbour
		if (prevX < cols && energyMap.at<uchar>(i, prevX + 1) < energyMap.at<uchar>(i, seam[i]))
			seam[i] = prevX + 1;
	}
#endif
	return seam;

	//// iterate through all rows to find vertical seam
	//for (int row = rows - 2; row >= 0; --row)
	//{
	//	// get previous column
	//	int prevCol = seam[row + 1];

	//	// get start and end col (directly below, left below or right below)
	//	int startCol = std::max(0, prevCol - 1);
	//	int endCol = std::min(cols - 1, prevCol + 1);

	//	// find minimum energy value in the neighboring columns
	//	cv::minMaxLoc(energyMap.row(row).colRange(startCol, endCol + 1), &minVal, nullptr, &minLoc);

	//	seam[row] = startCol + minLoc.x; // adjust seam index based on the range
	//}

	//VisualizeSeam(img, seam);
	//cv::imshow("Seam Visualization", img);
	//cv::waitKey(50);

	// //remove the seam from the image
	//for (int row{}; row < rows; ++row)
	//{
	//	int seamCol = seam[row];

	//	for (int col = seamCol; col < cols - 1; ++col)
	//		img.at<cv::Vec3b>(row, col) = img.at<cv::Vec3b>(row, col + 1);

	//}

	//// crop the last column
	//img = img.colRange(0, cols - 1);
}



void RemoveVerticalSeam(cv::Mat& img, std::vector<int> const& seam)
{
	for (int i{}; i < img.rows; ++i)
	{
		int col = seam[i];
		img.row(i).colRange(0, col).copyTo(img.row(i).colRange(0, col));
		img.row(i).colRange(col + 1, img.cols).copyTo(img.row(i).colRange(col, img.cols));
	}
}


void SeamCarvingToWidth(cv::Mat& img, int targetWidth)
{
	if (targetWidth >= img.cols)
	{
		std::cerr << "Target width must be smaller than the current width!\n";
		return;
	}

	std::vector<cv::Mat> channels;
	cv::split(img, channels);

	cv::Mat energyMap = CalculateEnergyMap(channels);

	std::vector<int> seam = FindVerticalSeamGreedy(energyMap);

	VisualizeSeam(img, seam);

	//while (img.cols > targetWidth)
	//{
	//	std::vector<cv::Mat> channels;

	//	cv::split(img, channels); // channels[0] = blue, channels[1] = green, channels[2] = red

	//	// recalculate energy map
	//	cv::Mat energyMap = CalculateEnergyMap(channels);

	//	//std::vector<int> seam = FindVerticalSeamGreedy(energyMap);

	//	//VisualizeSeam(img, seam);
	//	//RemoveVerticalSeam(img, seam);
	//}
}

void VisualizeSeam(cv::Mat& img, std::vector<int> const& seam, cv::Vec3b const& colour, int waitForMs)
{
	// assign colour to the seam for visualization
	for (int i{}; i < img.rows; ++i)
		img.at<cv::Vec3b>(i, seam[i]) = colour;

	cv::imshow("Seam Visualization", img);
	cv::waitKey(waitForMs);
}
