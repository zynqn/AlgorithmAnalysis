// file header please

#ifndef UTILITY_H
#define UTILITY_H

// lib
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>

#include <chrono>

#undef MAX
#undef max
#undef min

#define nl '\n'
#define NOW std::chrono::system_clock::now()

constexpr double MAX = std::numeric_limits<double>::max(); // MAX was 255
constexpr double MIN = -2000.0;
constexpr int THRESHOLD = 8;

#ifdef _DEBUG
#define WRAP(x) x
#else
#define WRAP(x)
#endif

// global variables
inline cv::Mat brushMask;
inline bool isDrawing = false;
inline int brushSize = 10;
inline bool maskInitialized = false;
inline float scale = 1000.f;
inline float resolution = 1.f; // height/width or rows/cols of image (ie for landscape images this will be < 1.f)

// global constants
inline const std::string ORIGINAL_IMAGE = "Original Image";
inline const std::wstring ORIGINAL_IMAGE_W = L"Original Image";

// global variables
namespace
{
	double zoomLevel = 1.0;
	const double zoomIncrement = 0.1;
	cv::Point2d zoomCenter(0, 0);
	std::chrono::system_clock::time_point start = NOW;
}

namespace cv
{
	const int ESC_KEY = 27;
}

namespace util
{
	inline void LockWindow(const std::wstring &windowName, int x, int y, int width, int height) 
	{
		HWND hwnd = FindWindow(nullptr, windowName.c_str()); // Find OpenCV window handle
		if (hwnd)
			// Fix window size and position
			SetWindowPos(hwnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	inline void initializeBrushMask(const cv::Mat &img)
	{
		if (!maskInitialized)
		{
			brushMask = cv::Mat::zeros(img.size(), CV_8UC1);
			maskInitialized = true;
		}
	}

	inline void drawBrush(cv::Mat &img, cv::Point point)
	{
		if (brushMask.size() != img.size())
		{
			cv::Mat newMask;
			cv::resize(brushMask, newMask, img.size(), 0, 0, cv::INTER_NEAREST);
			brushMask = newMask;
		}

		cv::circle(brushMask, point, brushSize, cv::Scalar(255), -1);
		cv::Mat displayImg = img.clone();
		displayImg.setTo(cv::Scalar(0, 0, 255), brushMask);
		cv::addWeighted(displayImg, 1.0, img, 0.0, 0, displayImg);
		cv::imshow(ORIGINAL_IMAGE, displayImg);
	}

	struct Mask
	{
		int start = 0;
		int size = 0;
		int pos = 0;
	};

	inline void BeginProfile()
	{
		start = NOW;
	}

	inline void EndProfile(const std::string &label)
	{
		double elapsed = std::chrono::duration<double, std::milli>(NOW - start).count();
		std::cout << label << ": " << elapsed << " ms \n";
	}

	// Function to zoom into the image
	inline cv::Mat zoomImage(const cv::Mat &img, double zoom, const cv::Point2d &center)
	{
		int width = static_cast<int>(img.cols / zoom);
		int height = static_cast<int>(img.rows / zoom);

		cv::Rect roi(center.x - width / 2, center.y - height / 2, width, height);
		roi = roi & cv::Rect(0, 0, img.cols, img.rows);  // Ensure ROI is within image bounds

		// Update the new zoom center based on the scaling
		zoomCenter.x = (zoomCenter.x - roi.x) / zoom;  // Adjusting the zoom center x
		zoomCenter.y = (zoomCenter.y - roi.y) / zoom;  // Adjusting the zoom center y

		return img(roi).clone();  // Return the zoomed image
	}

	inline void mouseCallback(int event, int x, int y, int flags, void *data)
	{
		cv::Mat *img = (cv::Mat *)data;

		if (!maskInitialized || brushMask.empty())
			initializeBrushMask(*img);

		if (brushMask.size() != img->size())
			brushMask = cv::Mat::zeros(img->size(), CV_8UC1);

		switch (event)
		{
		case cv::EVENT_LBUTTONDOWN:
			isDrawing = true;
			drawBrush(*img, cv::Point(x, y));
			break;

		case cv::EVENT_MOUSEMOVE:
			if (isDrawing)
				drawBrush(*img, cv::Point(x, y));
			break;

		case cv::EVENT_LBUTTONUP:
			isDrawing = false;
			break;

		}
	}
}

#endif