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
#define WHITESPACE " \n\t\r\v\f"

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
inline int brushSize = 5;
inline bool maskInitialized = false;
inline float resolution = 1.f; // height/width or rows/cols of image (ie for landscape images this will be < 1.f)
inline int rows = 0, cols = 0;
inline cv::Mat imgClone, originalImg;

// global constants
inline const std::string ORIGINAL_IMAGE = "Original Image";
inline const std::wstring ORIGINAL_IMAGE_W = L"Original Image";
inline const std::string ORIGINAL_ENERGY_MAP = "Original Energy Map";
inline const std::wstring ORIGINAL_ENERGY_MAP_W = L"Original Energy Map";
inline const std::wstring ALL_SEAMS= L"All Seams";
inline const std::wstring OUTPUT = L"Output";
inline const std::wstring INSPECTOR = L"Inspector";

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

	inline void mouseCallback(int event, int x, int y, int flags, void* data)
	{
		cv::Mat* img = (cv::Mat*)data;

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


	/*! ------------ String Manipulation ------------ */

		// assume str is in pascal or camel case
		// second param determines whether to set non-first capital letters to lower case
	inline std::string ToCapitalCase(std::string str, bool shldUncapitalize = false)
	{
		if (str.empty())
			return str;
		str[0] = static_cast<char>(std::toupper(str[0]));

		for (size_t i = 1; i < str.size(); ++i)
			if (std::isupper(str[i]))
			{
				if (shldUncapitalize)
					str[i] = static_cast<char>(std::tolower(str[i]));
				str.insert(str.begin() + i++, ' ');
			}
		return str;
	}

	inline std::string ToLowerCase(std::string str)
	{
		std::for_each(str.begin(), str.end(), [](auto &elem) { elem = static_cast<char>(std::tolower(static_cast<unsigned char>(elem))); });
		return str;
	}

	inline std::string Quote(std::string const &str, char delim = '"')
	{
		std::ostringstream oss;
		oss << std::quoted(str, delim);
		return oss.str();
	}

	// Helper function to split variable names
	inline std::vector<std::string> SplitString(const std::string &toSplit, char delim = ' ')
	{
		std::string word;
		std::istringstream line(toSplit);
		std::vector<std::string> words;

		while (std::getline(line, word, delim))
			words.push_back(word);
		return words;
	}

	inline std::string TrimString(std::string toTrim)
	{
		size_t pos = toTrim.find_first_not_of(WHITESPACE);
		if (pos != std::string::npos)
			toTrim = toTrim.substr(pos);
		pos = toTrim.find_last_not_of(WHITESPACE);
		if (pos != std::string::npos)
			toTrim = toTrim.substr(0, pos + 1);
		return toTrim;
	}

	// warning: no error checking
	inline std::string TrimString(std::string toTrim, const std::string &toFind, bool findFromTheFront = true, bool cropFromTheFront = true)
	{
		size_t pos = findFromTheFront ? toTrim.find(toFind) : toTrim.rfind(toFind);
		if (pos == std::string::npos)
			return toTrim;

		if (cropFromTheFront)
			toTrim = toTrim.substr(pos + toFind.length());
		else
			toTrim = toTrim.substr(0, pos);
		return toTrim;
	}

}

#endif