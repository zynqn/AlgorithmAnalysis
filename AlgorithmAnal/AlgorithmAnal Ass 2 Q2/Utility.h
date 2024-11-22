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
//constexpr double MIN = -2000.0;

#ifdef _DEBUG
#define WRAP(x) x
#else
#define WRAP(x)
#endif

// global variables
inline float min = 2000.f;
inline int threshold = 8;
inline cv::Mat brushMask;
inline bool isDrawing = false;
inline int brushSize = 5;
inline bool maskInitialized = false;
inline float resolution = 1.f; // height/width or rows/cols of image (ie for landscape images this will be < 1.f)
inline int rows = 0, cols = 0;
inline cv::Mat imgClone, originalImg, energyMap, displayEnergyMap, allSeams;
inline int waitFor = 1;

// global constants
inline const std::string ORIGINAL_IMAGE = "Original Image";
inline const std::wstring ORIGINAL_IMAGE_W = L"Original Image";
inline const std::string ENERGY_MAP = "Energy Map";
inline const std::wstring ENERGY_MAP_W = L"Energy Map";
inline const std::string ALL_SEAMS = "All Seams";
inline const std::wstring ALL_SEAMS_W = L"All Seams";
inline const std::string CARVED_IMAGE = "Carved Image";
inline const std::wstring CARVED_IMAGE_W = L"Carved Image";
inline const std::wstring INSPECTOR_W = L"Inspector";

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
		if (HWND hwnd = FindWindow(nullptr, windowName.c_str()))
			SetWindowPos(hwnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	inline void ShowWindow(const std::wstring &windowName, bool shldShow)
	{
		if (HWND hwnd = FindWindow(nullptr, windowName.c_str()))
			ShowWindow(hwnd, shldShow ? SW_SHOW : SW_HIDE);
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