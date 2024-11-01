#include "SeamCarving.h"
#include "Utility.h"

#include <vector>
#include <iomanip>
#include <iostream>



std::vector<util::Mask> GetBoundedArea(cv::Point2i start, cv::Point2i end)
{
	std::vector<util::Mask> ret;
	for (int i = start.y; i < end.y; ++i)
		ret.push_back({ start.x, end.x - start.x, i }); // idk why emplace_back got error
	return ret;
}

cv::Mat CalculateEnergyMap(std::vector<cv::Mat> const& channels)
{
	cv::Mat gradX, gradY;
	cv::Mat energyMap = cv::Mat::zeros(channels[0].size(), CV_64F);

	for (const auto& channel : channels) 
	{
		cv::Sobel(channel, gradX, CV_64F, 1, 0, 3);
		cv::Sobel(channel, gradY, CV_64F, 0, 1, 3);

		energyMap += cv::abs(gradX) + cv::abs(gradY);
	}

	return energyMap;
}

void ModifyEnergyMap(cv::Mat &energyMap, const std::vector<util::Mask> &area, double setTo)
{
#if 0
	for (int i = 0; i < energyMap.rows; ++i)
		for (int j = 0; j < energyMap.cols; ++j)
			if (j > start.x && j < end.x && i > start.y && i < end.y)
			{
				double &currVal = energyMap.at<double>(i, j);
				currVal = 0.0;
				//currVal = std::max(0.0, currVal - setTo);
				//std::cout << currVal << nl;
			}
#endif
	
	for (const util::Mask &slice : area)
		for (int curr = slice.start; curr < slice.start + slice.size; ++curr)
			energyMap.at<double>(slice.pos, curr) = setTo;
}

std::vector<int> FindVerticalSeamGreedy(cv::Mat const& energyMap)
{
#if 0
	int rows = energyMap.rows;
	int cols = energyMap.cols;
	std::vector<int> seam(rows);

	// Start at the minimum energy pixel in the first row
	double minVal;
	cv::Point minLoc;
	cv::minMaxLoc(energyMap.row(0).colRange(1, cols - 1), &minVal, nullptr, &minLoc, nullptr);
	seam[0] = minLoc.x;

	//return seam;

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
	return seam;
#endif

	int rows = energyMap.rows;
	int cols = energyMap.cols;

	// Vector to store column indices of the seam pixels
	std::vector<int> seam(rows);

	// Find the smallest energy in the last row first
	double minVal;
	cv::Point minLoc;
	//cv::minMaxLoc(energyMap.row(rows - 1), &minVal, nullptr, &minLoc);
	minMaxLoc(energyMap.row(rows - 1).colRange(1, energyMap.cols - 2), &minVal, nullptr, &minLoc, nullptr);
	seam[rows - 1] = minLoc.x + 1;

	// iterate through all rows to find vertical seam
	for (int row = rows - 2; row >= 0; --row)
	{
		// get previous column
		int prevCol = seam[row + 1];

		// get start and end col (directly below, left below or right below), (ignore edges)
		int startCol = std::max(1, prevCol - 1);
		int endCol = std::min(cols - 2, prevCol + 1);

		// find minimum energy value in the neighboring columns
		cv::minMaxLoc(energyMap.row(row).colRange(startCol, endCol), &minVal, nullptr, &minLoc);

		seam[row] = startCol + minLoc.x; // adjust seam index based on the range
	}

	return seam;
#if 0

	VisualizeSeam(img, seam);
	cv::imshow("Seam Visualization", img);
	cv::waitKey(50);

	 //remove the seam from the image
	for (int row{}; row < rows; ++row)
	{
		int seamCol = seam[row];

		for (int col = seamCol; col < cols - 1; ++col)
			img.at<cv::Vec3b>(row, col) = img.at<cv::Vec3b>(row, col + 1);

	}

	// crop the last column
	img = img.colRange(0, cols - 1);
#endif
}

cv::Mat CalculateCumMap(const cv::Mat &energyMap)
{
	cv::Mat cumMap(energyMap.size(), CV_64F);
	int rows = energyMap.rows, cols = energyMap.cols;
	double max = 0.0;

	if (!rows || !cols)
		return cumMap;

	// copy last row over
	for (int j = 0; j < cols; ++j)
		cumMap.at<double>(rows - 1, j) = energyMap.at<double>(rows - 1, j);

	// cumulatively sum best energy value from bottom to top, taking only 3 pixels into account
	for (int i = rows - 2; i > -1; --i)
		for (int j = 0; j < cols; ++j)
		{
			double leftVal = j ? cumMap.at<double>(i + 1, j - 1) : MAX;
			double midVal = cumMap.at<double>(i + 1, j);
			double rightVal = j < cols - 1 ? cumMap.at<double>(i + 1, j + 1) : MAX;
			double minVal = std::min({ leftVal, midVal, rightVal });

			// set the current pixel to its mirror in the original energy map + the lowest value of the 3 adjacent pixels below it
			double &currVal = cumMap.at<double>(i, j);
			currVal = energyMap.at<double>(i, j) + minVal;
			max = currVal > max ? currVal : max;
		}

	// normalise values to 0 to 255
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j)
			cumMap.at<double>(i, j) = cumMap.at<double>(i, j) / max * 255.0;

	return cumMap;
}

bool ModifyMask(std::vector<util::Mask> &area, const std::vector<int> &seam)
{
	bool isMaskGone = true;

	for (util::Mask &slice : area)
	{
		int currSeam = seam[slice.pos];

		if (currSeam < slice.start)
			--slice.start;
		else if (currSeam >= slice.start && currSeam < slice.start + slice.size)
			--slice.size;

		// mask is gone when all slices have a size of THRESHOLD or less
		if (slice.size > THRESHOLD)
			isMaskGone = false;
	}

	return isMaskGone;
}

std::vector<int> FindVerticalSeamDP(cv::Mat &cumMap)
{
	int rows = cumMap.rows, cols = cumMap.cols;
	std::vector<int> seam(rows);

	if (!rows || !cols)
		return seam;

	// find col with smallest cumulative sum in the first row
	int smallestCum = 0;
	for (int j = 1; j < cols; ++j)
		smallestCum = cumMap.at<double>(0, j) < cumMap.at<double>(0, smallestCum) ? j : smallestCum;

	int col = smallestCum;
	seam[0] = col;

	// find path of least resistance (aka the seam to cut)
	for (int i = 0; i < rows - 1; ++i)
	{
		// select column that has the lowest energy
		double leftVal = col ? cumMap.at<double>(i + 1, col - 1) : MAX;
		double midVal = cumMap.at<double>(i + 1, col);
		double rightVal = col < cols - 1 ? cumMap.at<double>(i + 1, col + 1) : MAX;
		seam[i + 1] = col = leftVal < midVal ? leftVal < rightVal ? col - 1 : col + 1 : midVal < rightVal ? col : col + 1;
	}

	return seam;
}

void RemoveVerticalSeam(cv::Mat& img, std::vector<int> const& seam)
{
	int rows = img.rows;
	int cols = img.cols;

	//remove the seam from the image
	for (int row{}; row < rows; ++row)
	{
		int seamCol = seam[row];
		for (int col = seamCol; col < cols - 1; ++col)
			img.at<cv::Vec3b>(row, col) = img.at<cv::Vec3b>(row, col + 1);

	}

	// resize the whole image 
	img = img.colRange(0, cols - 1);

#if 0
	for (int i{}; i < img.rows; ++i)
	{
		int col = seam[i];
		img.row(i).colRange(0, col).copyTo(img.row(i).colRange(0, col));
		img.row(i).colRange(col + 1, img.cols).copyTo(img.row(i).colRange(col, img.cols));
	}
#endif
}

void SeamCarvingToWidth(cv::Mat &img, int targetWidth)
{
	if (targetWidth >= img.cols)
	{
		std::cerr << "Target width must be smaller than the current width!\n";
		return;
	}



	while (img.cols > targetWidth)
	{
		std::vector<cv::Mat> channels;

		cv::split(img, channels); // channels[0] = blue, channels[1] = green, channels[2] = red

		// recalculate energy map
		cv::Mat energyMap = CalculateEnergyMap(channels);

		std::vector<int> seam = FindVerticalSeamGreedy(energyMap);


		VisualizeSeam(img, seam, (0, 0, 255), 50);
		RemoveVerticalSeam(img, seam);
	}
}

void SeamCarvingToWidthDP(cv::Mat& img, int targetWidth, bool isRemovingObject)
{
	if (targetWidth >= img.cols)
	{
		std::cerr << "Target width is " << targetWidth << " but image width is " << img.cols << nl;
		return;
	}

	std::vector<util::Mask> toRemove = GetBoundedArea({ 30, 0 }, { 150, 350 }); // clock pic
	//std::vector<util::Mask> toRemove = GetBoundedArea({ 639, 460 }, { 845, 604 }); // surfer pic

	while (true)
	{
		std::vector<cv::Mat> channels;

		WRAP(util::BeginProfile());
		cv::split(img, channels); // channels[0] = blue, channels[1] = green, channels[2] = red
		WRAP(util::EndProfile("Split Channels"));

		// recalculate energy map
		WRAP(util::BeginProfile());
		cv::Mat energyMap = CalculateEnergyMap(channels);
		WRAP(util::EndProfile("Energy Map"));

		ModifyEnergyMap(energyMap, toRemove, 0.0);
		cv::normalize(energyMap, energyMap, 0, 255, cv::NORM_MINMAX);

		WRAP(util::BeginProfile());
		cv::Mat cumMap = CalculateCumMap(energyMap);
		WRAP(util::EndProfile("Cum Map"));

		WRAP(util::BeginProfile());
		std::vector<int> seam = FindVerticalSeamDP(cumMap);
		WRAP(util::EndProfile("Find Seam"));

		WRAP(util::BeginProfile());
		VisualizeSeam(img, seam, (0, 0, 255));
		WRAP(util::EndProfile("Visualize Seam"));

		WRAP(util::BeginProfile());
		RemoveVerticalSeam(img, seam);
		WRAP(util::EndProfile("Remove Seam"));

		// returns true if mask is gone
		if (ModifyMask(toRemove, seam) && isRemovingObject)
			break;

		if (img.cols <= targetWidth && !isRemovingObject)
			break;
	}
}

void VisualizeSeam(cv::Mat& img, std::vector<int> const& seam, cv::Vec3b const& colour, int waitForMs)
{
	static cv::Mat imgClone = img.clone();
	// assign colour to the seam for visualization
	for (int i{}; i < img.rows; ++i)
	{
		img.at<cv::Vec3b>(i, seam[i]) = colour;
		imgClone.at<cv::Vec3b>(i, seam[i]) = colour;
	}

	cv::imshow("Output", img);
	cv::imshow("All Seams", imgClone);

	cv::waitKey(waitForMs);
}

void DrawBoundary(cv::Mat &img, int pos, cv::Vec3b const &colour)
{
	for (int i = 0; i < img.rows; ++i)
		img.at<cv::Vec3b>(i, pos) = colour;

	cv::imshow("Input", img);
}