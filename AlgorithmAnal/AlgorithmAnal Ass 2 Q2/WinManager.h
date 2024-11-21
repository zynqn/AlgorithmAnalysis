#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <array>

#include "Utility2.h"

// openCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>


class WinManager
{
public:

	WinManager();

	void UpdateOIWin(bool toggle, cv::Mat& img);
	void UpdateEMWin(bool toggle, cv::Mat &img);
	void UpdateCIWin(bool toggle, cv::Mat &img);
	void UpdateASWin(bool toggle, cv::Mat& img);

	bool OIWin = false;
	bool EMWin = false;
	bool CIWin = false;
	bool ASWin = false;
};
