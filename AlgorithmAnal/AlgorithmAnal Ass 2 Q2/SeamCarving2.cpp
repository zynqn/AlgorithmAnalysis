#include "SeamCarving2.h"
#include "Utility2.h"
#include "Editor.h"
#include "WinManager.h"

#include <vector>
#include <iomanip>
#include <iostream>
#include "graph.cpp"

// maxflow graph (for cut graph)
#include "graph.h"

extern edit::Editor editor;
extern WinManager winManager;

// =============
// OBJECT REMOVAL
// =============

std::vector<util::Mask> GetBoundedArea(cv::Point2i start, cv::Point2i end)
{
	std::vector<util::Mask> ret;
	for (int i = start.y; i < end.y; ++i)
		ret.push_back({ start.x, end.x - start.x, i }); // idk why emplace_back got error
	return ret;
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
		if (slice.size > threshold)
			isMaskGone = false;
	}

	// Remove empty masks
	area.erase(std::remove_if(area.begin(), area.end(), [](const util::Mask &m) { return m.size <= 0; }), area.end());
	return isMaskGone;
}

void ModifyVerticalEnergyMap(cv::Mat &energyMap, const std::vector<util::Mask> &area, double setTo)
{
	for (const util::Mask &slice : area)
		for (int curr = slice.start; curr < slice.start + slice.size; ++curr)
			energyMap.at<double>(slice.pos, curr) = setTo;
}

void ModifyHorizontalEnergyMap(cv::Mat& energyMap, const std::vector<util::Mask>& area, double setTo)
{
	for (const util::Mask& slice : area)
		for (int curr = slice.start; curr < slice.start + slice.size; ++curr)
			energyMap.at<double>(curr, slice.pos) = setTo;
}


void ContentAwareRemoval(cv::Mat &img)
{
	if (brushMask.empty() || cv::countNonZero(brushMask) == 0)
		return;

	// Create two temporary images and masks for testing both directions
	cv::Mat imgVertical = img.clone();
	cv::Mat imgHorizontal = img.clone();
	cv::Mat maskVertical = brushMask.clone();
	cv::Mat maskHorizontal = brushMask.clone();

	// Count seams needed for vertical removal
	int verticalSeams = 0;
	{
		std::vector<util::Mask> toRemove;
		for (int y = 0; y < maskVertical.rows; ++y)
		{
			int start = -1;
			int end = -1;

			uchar *row = maskVertical.ptr<uchar>(y);
			for (int x = 0; x < maskVertical.cols; ++x)
			{
				if (row[x] > 0)
				{
					if (start == -1) start = x;
					end = x;
				}
			}

			if (start != -1 && end != -1)
				toRemove.push_back({ start, end - start + 1, y });
		}

		while (!toRemove.empty())
		{
			std::vector<cv::Mat> channels;
			cv::split(imgVertical, channels);
			cv::Mat energyMap = CalculateEnergyMap(channels);
			ModifyVerticalEnergyMap(energyMap, toRemove, -min);

			cv::Mat cumMap = CalculateVerticalCumMap(energyMap);
			std::vector<int> seam = FindVerticalSeamDP(cumMap);

			VisualizeVerticalSeam(img, seam, cv::Vec3b(0, 0, 255));
			RemoveVerticalSeam(imgVertical, seam);
			verticalSeams++;

			if (ModifyMask(toRemove, seam))
				break;
		}
	}

	// Count seams needed for horizontal removal
	int horizontalSeams = 0;
	{
		std::vector<util::Mask> toRemove;
		for (int x = 0; x < maskHorizontal.cols; ++x)
		{
			int start = -1;
			int end = -1;

			for (int y = 0; y < maskHorizontal.rows; ++y)
			{
				if (maskHorizontal.at<uchar>(y, x) > 0)
				{
					if (start == -1) start = y;
					end = y;
				}
			}

			if (start != -1 && end != -1)
				toRemove.push_back({ start, end - start + 1, x });
		}

		while (!toRemove.empty())
		{
			std::vector<cv::Mat> channels;
			cv::split(imgHorizontal, channels);
			cv::Mat energyMap = CalculateEnergyMap(channels);
			ModifyHorizontalEnergyMap(energyMap, toRemove, -min);

			cv::Mat cumMap = CalculateHorizontalCumMap(energyMap);
			std::vector<int> seam = FindHorizontalSeamDP(cumMap);

			VisualizeHorizontalSeam(img, seam, cv::Vec3b(0, 0, 255));
			RemoveHorizontalSeam(imgHorizontal, seam);
			horizontalSeams++;

			if (ModifyMask(toRemove, seam))
				break;
		}
	}

	// Choose the direction that requires fewer seams
	bool useVerticalSeams = verticalSeams <= horizontalSeams;

	// Now perform the actual removal using the better direction
	if (useVerticalSeams)
	{
		std::vector<util::Mask> toRemove;
		for (int y = 0; y < brushMask.rows; ++y)
		{
			int start = -1;
			int end = -1;

			uchar *row = brushMask.ptr<uchar>(y);
			for (int x = 0; x < brushMask.cols; ++x)
			{
				if (row[x] > 0)
				{
					if (start == -1) start = x;
					end = x;
				}
			}

			if (start != -1 && end != -1)
				toRemove.push_back({ start, end - start + 1, y });
		}

		while (!toRemove.empty())
		{
			std::vector<cv::Mat> channels;
			cv::split(img, channels);
			cv::Mat energyMap = CalculateEnergyMap(channels);
			ModifyVerticalEnergyMap(energyMap, toRemove, -min);

			cv::Mat cumMap = CalculateVerticalCumMap(energyMap);
			std::vector<int> seam = FindVerticalSeamDP(cumMap);

			VisualizeVerticalSeam(img, seam, cv::Vec3b(0, 0, 255));
			RemoveVerticalSeam(img, seam);

			if (ModifyMask(toRemove, seam))
				break;
		}
	}
	else
	{
		std::vector<util::Mask> toRemove;
		for (int x = 0; x < brushMask.cols; ++x)
		{
			int start = -1;
			int end = -1;

			for (int y = 0; y < brushMask.rows; ++y)
			{
				if (brushMask.at<uchar>(y, x) > 0)
				{
					if (start == -1) start = y;
					end = y;
				}
			}

			if (start != -1 && end != -1)
				toRemove.push_back({ start, end - start + 1, x });
		}

		while (!toRemove.empty())
		{
			std::vector<cv::Mat> channels;
			cv::split(img, channels);
			cv::Mat energyMap = CalculateEnergyMap(channels);
			ModifyHorizontalEnergyMap(energyMap, toRemove, -min);

			cv::Mat cumMap = CalculateHorizontalCumMap(energyMap);
			std::vector<int> seam = FindHorizontalSeamDP(cumMap);

			VisualizeHorizontalSeam(img, seam, cv::Vec3b(0, 0, 255));
			RemoveHorizontalSeam(img, seam);

			if (ModifyMask(toRemove, seam))
				break;
		}
	}

	brushMask = cv::Mat::zeros(img.size(), CV_8UC1);
}


// =============
// ENERGY MAP
// =============

cv::Mat CalculateEnergyMap(std::vector<cv::Mat> const &channels)
{
	cv::Mat gradX, gradY;
	cv::Mat energyMap = cv::Mat::zeros(channels[0].size(), CV_64F);

	for (const auto &channel : channels)
	{
		cv::Sobel(channel, gradX, CV_64F, 1, 0, 3);
		cv::Sobel(channel, gradY, CV_64F, 0, 1, 3);

		energyMap += cv::abs(gradX) + cv::abs(gradY);
	}

	return energyMap;
}

cv::Mat CalculateVerticalCumMap(const cv::Mat &energyMap)
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

cv::Mat CalculateHorizontalCumMap(const cv::Mat &energyMap)
{
	cv::Mat cumMap(energyMap.size(), CV_64F);
	int rows = energyMap.rows, cols = energyMap.cols;
	double max = 0.0;

	if (!rows || !cols)
		return cumMap;

	// copy last row over
	for (int j = 0; j < rows; ++j)
		cumMap.at<double>(j, cols - 1) = energyMap.at<double>(j, cols - 1);

	for (int i = cols - 2; i > -1; --i)
		for (int j = 0; j < rows; ++j)
		{
			double leftVal = j ? cumMap.at<double>(j - 1, i + 1) : MAX;
			double midVal = cumMap.at<double>(j, i + 1);
			double rightVal = j < rows - 1 ? cumMap.at<double>(j + 1, i + 1) : MAX;
			double minVal = std::min({ leftVal, midVal, rightVal });

			// set the current pixel to its mirror in the original energy map + the lowest value of the 3 adjacent pixels below it
			double &currVal = cumMap.at<double>(j, i);
			currVal = energyMap.at<double>(j, i) + minVal;
			max = currVal > max ? currVal : max;
		}

	// normalise values to 0 to 255
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j)
			cumMap.at<double>(i, j) = cumMap.at<double>(i, j) / max * 255.0;

	return cumMap;
}

// =============
// SEAM CARVING - VERTICAL
// =============

std::vector<int> FindVerticalSeamGreedy(cv::Mat const& energyMap)
{
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

std::vector<int> FindVerticalSeamGraphCut(cv::Mat const& energyMap)
{
	int rows = energyMap.rows;
	int cols = energyMap.cols;

	// graph with nodes
	maxflow::Graph<float, float, float> graph(rows * cols, rows * cols * 2);

	// add nodes to the graph
	graph.add_node(rows * cols);

	// lambda function to get the node index
	auto index = [=](int r, int c) { return r * cols + c; };

	// add edges
	for (int r{}; r < rows; ++r)
	{
		for (int c{}; c < cols; ++c)
		{
			// ensure we are not at the last row (vertical edge)
			if (r < rows - 1)
			{
				float weight = energyMap.at<double>(r, c) + energyMap.at<double>(r + 1, c);
				graph.add_edge(index(r, c), index(r + 1, c), weight, weight);
			}

			// ensure we are not at the left most col (diagonal left edge)
			if (r < rows - 1 && c > 0)
			{
				float weight = energyMap.at<double>(r, c) + energyMap.at<double>(r + 1, c - 1);
				graph.add_edge(index(r, c), index(r + 1, c - 1), weight, weight);
			}

			// ensure we are not at the right most col (diagonal right edge)
			if (r < rows - 1 && c < cols - 1)
			{
				float weight = energyMap.at<double>(r, c) + energyMap.at<double>(r + 1, c + 1);
				graph.add_edge(index(r, c), index(r + 1, c + 1), weight, weight);
			}
		}
	}

	// connect source and sink
	for (int r{}; r < rows; ++r)
	{
		graph.add_tweights(index(r, 0), 1e9, 0); // source connect to first column
		graph.add_tweights(index(r, cols - 1), 0, 1e9); // last column to sink
	}

	// compute max flow
	float flow = graph.maxflow();


	std::vector<int> seam(rows);

	for (int r{}; r < rows; ++r)
	{
		for (int c{}; c < cols; ++c)
		{
			if (graph.what_segment(index(r, c)) == maxflow::Graph<float, float, float>::SINK)
			{
				seam[r] = c;
				break;
			}
		}
	}

	return seam;
}


void RemoveVerticalSeam(cv::Mat &img, std::vector<int> const &seam)
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
}

void VerticalSeamCarvingGreedy(cv::Mat &img, int targetWidth)
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

		VisualizeVerticalSeam(img, seam, (0, 0, 255));
		RemoveVerticalSeam(img, seam);
	}
}

void VerticalSeamCarvingDP(cv::Mat &img, int targetWidth)
{
	if (targetWidth >= img.cols)
	{
		std::cerr << "Target width is " << targetWidth << " but image width is " << img.cols << nl;
		return;
	}

	while (img.cols > targetWidth)
	{
		std::vector<cv::Mat> channels;
		cv::split(img, channels); // channels[0] = blue, channels[1] = green, channels[2] = red

		// recalculate energy map
		cv::Mat energyMap = CalculateEnergyMap(channels);
		cv::normalize(energyMap, energyMap, 0, 255, cv::NORM_MINMAX);

		cv::Mat cumMap = CalculateVerticalCumMap(energyMap);
		std::vector<int> seam = FindVerticalSeamDP(cumMap);

		VisualizeVerticalSeam(img, seam, (0, 0, 255));
		RemoveVerticalSeam(img, seam);
	}
}

void VerticalSeamCarvingGraphCut(cv::Mat& img, int targetWidth)
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

		std::vector<int> seam = FindVerticalSeamGraphCut(energyMap);

		VisualizeVerticalSeam(img, seam, (255, 0, 255));
		RemoveVerticalSeam(img, seam);
	}
}

// ===============
// SEAM CARVING - HORIZONTAL
// ===============

std::vector<int> FindHorizontalSeamGreedy(cv::Mat const &energyMap)
{
	int rows = energyMap.rows;
	int cols = energyMap.cols;

	// Vector to store row indices of the seam pixels
	std::vector<int> seam(cols);

	// Find the smallest energy in the last row first
	double minVal;
	cv::Point minLoc;
	minMaxLoc(energyMap.col(cols - 1).rowRange(1, energyMap.rows - 2), &minVal, nullptr, &minLoc, nullptr);
	seam[cols - 1] = minLoc.y + 1;

	// iterate through all columns to find horizontal seam
	for (int col = cols - 2; col >= 0; --col)
	{
		// get previous row
		int prevRow = seam[col + 1];

		// get start and end row (directly below, left below or right below), (ignore edges)
		int startRow = std::max(1, prevRow - 1);
		int endRow = std::min(rows - 2, prevRow + 1);

		// find minimum energy value in the neighboring columns
		cv::minMaxLoc(energyMap.col(col).rowRange(startRow, endRow), &minVal, nullptr, &minLoc);

		seam[col] = startRow + minLoc.y; // adjust seam index based on the range
	}

	return seam;
}

std::vector<int> FindHorizontalSeamDP(cv::Mat &cumMap)
{
	int rows = cumMap.rows, cols = cumMap.cols;
	std::vector<int> seam(cols);

	if (!rows || !cols)
		return seam;

	// find row with smallest cumulative sum in the first row
	int smallestCum = 0;
	for (int j = 1; j < rows; ++j)
		smallestCum = cumMap.at<double>(j, 0) < cumMap.at<double>(smallestCum, 0) ? j : smallestCum;

	int row = smallestCum;
	seam[0] = row;

	// find path of least resistance (aka the seam to cut)
	for (int i = 0; i < cols - 1; ++i)
	{
		// select column that has the lowest energy
		double leftVal = row ? cumMap.at<double>(row - 1, i + 1) : MAX;
		double midVal = cumMap.at<double>(row, i + 1);
		double rightVal = row < rows - 1 ? cumMap.at<double>(row + 1, i + 1) : MAX;
		seam[i + 1] = row = leftVal < midVal ? leftVal < rightVal ? row - 1 : row + 1 : midVal < rightVal ? row : row + 1;
	}

	return seam;
}

std::vector<int> FindHorizontalSeamGraphCut(cv::Mat const& energyMap)
{
	int rows = energyMap.rows;
	int cols = energyMap.cols;

	// graph with nodes
	maxflow::Graph<float, float, float> graph(rows * cols, rows * cols * 3);

	// add nodes to the graph
	graph.add_node(rows * cols);

	// lambda function to get the node index
	auto index = [=](int r, int c) { return r * cols + c; };

	// add edges
	for (int r{}; r < rows; ++r)
	{
		for (int c{}; c < cols; ++c)
		{
			// ensure we are not at the last col (vertical edge)
			if (c < cols - 1)
			{
				float weight = energyMap.at<double>(r, c) + energyMap.at<double>(r, c + 1);
				graph.add_edge(index(r, c), index(r, c + 1), weight, weight);
			}

			// ensure we are not at the first row (diagonal up edge)
			if (c < cols - 1 && r > 0)
			{
				float weight = energyMap.at<double>(r, c) + energyMap.at<double>(r - 1, c + 1);
				graph.add_edge(index(r, c), index(r - 1, c + 1), weight, weight);
			}

			// ensure we are not at the last row (diagonal down edge)
			if (c < cols - 1 && r < rows - 1)
			{
				float weight = energyMap.at<double>(r, c) + energyMap.at<double>(r + 1, c + 1);
				graph.add_edge(index(r, c), index(r + 1, c + 1), weight, weight);
			}
		}
	}

	// connect source and sink
	for (int c{}; c < cols; ++c)
	{
		graph.add_tweights(index(0, c), 1e9, 0); // source connect to first row
		graph.add_tweights(index(rows - 1, c), 0, 1e9); // last row to sink
	}

	// compute max flow
	float flow = graph.maxflow();


	std::vector<int> seam(cols);

	for (int c{}; c < cols; ++c)
	{
		for (int r{}; r < rows; ++r)
		{
			if (graph.what_segment(index(r, c)) == maxflow::Graph<float, float, float>::SINK)
			{
				seam[c] = r;
				break;
			}
		}
	}

	return seam;
}

void RemoveHorizontalSeam(cv::Mat &img, std::vector<int> const &seam)
{
	int rows = img.rows;
	int cols = img.cols;

	//remove the seam from the image
	for (int col{ }; col < cols; ++col)
	{
		int seamRow = seam[col];
		for (int row = seamRow; row < rows - 1; ++row)
			img.at<cv::Vec3b>(row, col) = img.at<cv::Vec3b>(row + 1, col);

	}

	// resize the whole image 
	img = img.rowRange(0, rows - 1);
}

void HorizontalSeamCarvingGreedy(cv::Mat& img, int targetHeight)
{
	if (targetHeight >= img.rows)
	{
		std::cerr << "Target height must be smaller than the current width!\n";
		return;
	}

	while (img.rows > targetHeight)
	{
		std::vector<cv::Mat> channels;

		cv::split(img, channels); // channels[0] = blue, channels[1] = green, channels[2] = red

		// recalculate energy map
		cv::Mat energyMap = CalculateEnergyMap(channels);

		std::vector<int> seam = FindHorizontalSeamGreedy(energyMap);

		VisualizeHorizontalSeam(img, seam, (0, 0, 255), 50);
		RemoveHorizontalSeam(img, seam);
	}
}

void HorizontalSeamCarvingDP(cv::Mat& img, int targetHeight)
{
	if (targetHeight >= img.rows)
	{
		std::cerr << "Target height is " << targetHeight << " but image height is " << img.rows << nl;
		return;
	}

	while (img.rows > targetHeight)
	{
		std::vector<cv::Mat> channels;
		cv::split(img, channels); // channels[0] = blue, channels[1] = green, channels[2] = red

		// recalculate energy map
		cv::Mat energyMap = CalculateEnergyMap(channels);
		cv::normalize(energyMap, energyMap, 0, 255, cv::NORM_MINMAX);
		energyMap = CalculateHorizontalCumMap(energyMap);

		std::vector<int> seam = FindHorizontalSeamDP(energyMap);
		VisualizeHorizontalSeam(img, seam, (0, 0, 255));
		RemoveHorizontalSeam(img, seam);
	}
}

void HorizontalSeamCarvingGraphCut(cv::Mat& img, int targetHeight)
{
	if (targetHeight >= img.rows)
	{
		std::cerr << "Target height must be smaller than the current width!\n";
		return;
	}

	while (img.rows > targetHeight)
	{
		std::vector<cv::Mat> channels;

		cv::split(img, channels); // channels[0] = blue, channels[1] = green, channels[2] = red

		// recalculate energy map
		cv::Mat energyMap = CalculateEnergyMap(channels);

		std::vector<int> seam = FindHorizontalSeamGraphCut(energyMap);

		VisualizeHorizontalSeam(img, seam, (0, 0, 255), 50);
		RemoveHorizontalSeam(img, seam);
	}
}

// ===============
// VISUALIZATION
// ===============

void VisualizeVerticalSeam(cv::Mat& img, std::vector<int> const& seam, cv::Vec3b const& colour, int waitForMs)
{
	// assign colour to the seam for visualization
	for (int i{}; i < img.rows; ++i)
	{
		img.at<cv::Vec3b>(i, seam[i]) = colour;
		allSeams.at<cv::Vec3b>(i, seam[i]) = colour;
	}

	if (editor.GetWindow<edit::WindowsManager>()->shldOpenCarvedImage)
	{
		util::ShowWindow(CARVED_IMAGE_W, true);
		cv::imshow(CARVED_IMAGE, img);
		winManager.CIWin = true;
	}
	else
	{
		if (winManager.CIWin)
		{
			util::ShowWindow(CARVED_IMAGE_W, false);
			winManager.CIWin = false;
		}
	}

	if (editor.GetWindow<edit::WindowsManager>()->shldOpenAllSeams)
	{
		util::ShowWindow(ALL_SEAMS_W, true);
		cv::imshow(ALL_SEAMS, allSeams);
		winManager.ASWin = true;
	}
	else
	{
		if (winManager.ASWin)
		{
			util::ShowWindow(ALL_SEAMS_W, false);
			winManager.ASWin = false;
		}
	}

	cv::waitKey(waitForMs);
}

void VisualizeHorizontalSeam(cv::Mat& img, std::vector<int> const& seam, cv::Vec3b const& colour, int waitForMs)
{
	// assign colour to the seam for visualization
	for (int i{}; i < img.cols; ++i)
	{
		img.at<cv::Vec3b>(seam[i], i) = colour;
		allSeams.at<cv::Vec3b>(seam[i], i) = colour;
	}

	if (editor.GetWindow<edit::WindowsManager>()->shldOpenCarvedImage)
	{
		util::ShowWindow(CARVED_IMAGE_W, true);
		cv::imshow(CARVED_IMAGE, img);
		winManager.CIWin = true;
	}
	else
	{
		if (winManager.CIWin)
		{
			util::ShowWindow(CARVED_IMAGE_W, false);
			winManager.CIWin = false;
		}
	}

	if (editor.GetWindow<edit::WindowsManager>()->shldOpenAllSeams)
	{
		util::ShowWindow(ALL_SEAMS_W, true);
		cv::imshow(ALL_SEAMS, allSeams);
		winManager.ASWin = true;
	}
	else
	{
		if (winManager.ASWin)
		{
			util::ShowWindow(ALL_SEAMS_W, false);
			winManager.ASWin = false;
		}
	}

	cv::waitKey(waitForMs);
}

void DrawVerticalBoundary(cv::Mat &img, int pos, cv::Vec3b const &colour)
{
	for (int i = 0; i < img.rows; ++i)
		img.at<cv::Vec3b>(i, pos) = colour;

	cv::imshow(ORIGINAL_IMAGE, img);
}

void DrawHorizontalBoundary(cv::Mat &img, int pos, cv::Vec3b const &colour)
{
	for (int i = 0; i < img.cols; ++i)
		img.at<cv::Vec3b>(pos, i) = colour;

	cv::imshow(ORIGINAL_IMAGE, img);
}