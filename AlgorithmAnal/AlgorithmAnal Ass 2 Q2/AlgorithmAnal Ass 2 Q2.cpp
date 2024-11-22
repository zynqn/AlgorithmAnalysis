
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
#include "Editor.h"
#include "WinManager.h"


#include <Windows.h>

edit::Editor editor;

WinManager winManager;




int main()
{

	//ShowCursor(FALSE);

	// ==============
	// LOAD THE IMAGE
	// ==============

#if 0
	// load the image
	originalImg = cv::imread("assets/images/clifford's stick.jpg");

	// ensure image loaded properly
	if (originalImg.empty())
	{
		std::cerr << "Error: Could not load image.\n";
		return -1;
	}
#endif

	// ===============
	// SET THE WINDOWS
	// ===============

	cv::namedWindow(ORIGINAL_IMAGE, cv::WINDOW_NORMAL);
	cv::setWindowProperty(ORIGINAL_IMAGE, cv::WND_PROP_AUTOSIZE, cv::WINDOW_NORMAL);
	//cv::destroyWindow(ORIGINAL_IMAGE);

	cv::namedWindow(ENERGY_MAP, cv::WINDOW_NORMAL);
	cv::setWindowProperty(ENERGY_MAP, cv::WND_PROP_AUTOSIZE, cv::WINDOW_NORMAL);

	cv::namedWindow(CARVED_IMAGE, cv::WINDOW_NORMAL);
	cv::setWindowProperty(CARVED_IMAGE, cv::WND_PROP_AUTOSIZE, cv::WINDOW_NORMAL);

	cv::namedWindow(ALL_SEAMS, cv::WINDOW_NORMAL);
	cv::setWindowProperty(ALL_SEAMS, cv::WND_PROP_AUTOSIZE, cv::WINDOW_NORMAL);

	util::ShowWindow(ORIGINAL_IMAGE_W, false);
	util::ShowWindow(ENERGY_MAP_W, false);
	util::ShowWindow(CARVED_IMAGE_W, false);
	util::ShowWindow(ALL_SEAMS_W, false);

	//cv::namedWindow(CARVED_IMAGE, cv::WINDOW_NORMAL);
	//cv::setWindowProperty(CARVED_IMAGE, cv::WND_PROP_AUTOSIZE, cv::WINDOW_NORMAL);

	//cv::namedWindow(ALL_SEAMS, cv::WINDOW_NORMAL);
	//cv::setWindowProperty(ALL_SEAMS, cv::WND_PROP_AUTOSIZE, cv::WINDOW_NORMAL);

	brushMask = cv::Mat::zeros(originalImg.size(), CV_8UC1);
	maskInitialized = true;

	// set mouse callback (to display the mouse coordinates as will as the respective RGB values of selected pixel)
	//cv::setMouseCallback(ORIGINAL_IMAGE, util::mouseCallback, &originalImg);

	

	editor.Init();

	// game loop
		// =======================
	// GET ORIGINAL ENERGY MAP
	// =======================

	while (true)
	{
		editor.Update();
		int key = cv::waitKey(1);
		util::LockWindow(ORIGINAL_IMAGE_W, 0, 0, static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale), static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale * resolution));
		util::LockWindow(INSPECTOR_W, static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale), 0, static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale), static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale * resolution));
		util::LockWindow(ENERGY_MAP_W, 0, static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale * resolution), static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale), static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale * resolution));
		util::LockWindow(CARVED_IMAGE_W, static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale) + static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale), static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale * resolution), static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale), static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale * resolution));
		util::LockWindow(ALL_SEAMS_W, static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale), static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale * resolution), static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale), static_cast<int>(editor.GetWindow<edit::WindowsManager>()->scale * resolution));



		winManager.UpdateOIWin(editor.GetWindow<edit::WindowsManager>()->shldOpenOriginalImage, originalImg);
		winManager.UpdateEMWin(editor.GetWindow<edit::WindowsManager>()->shldOpenEnergyMap, displayEnergyMap);
		winManager.UpdateCIWin(editor.GetWindow<edit::WindowsManager>()->shldOpenCarvedImage, imgClone);
		winManager.UpdateASWin(editor.GetWindow<edit::WindowsManager>()->shldOpenAllSeams, allSeams);

		if (key == 'h')
			HorizontalSeamCarvingGreedy(imgClone, 500);

		if (key == 'g')
			VerticalSeamCarvingGreedy(imgClone, 400);

		if (key == 'c')
			VerticalSeamCarvingGraphCut(imgClone, 500);

		if (key == 'b')
			HorizontalSeamCarvingGraphCut(imgClone, 400);

		if (key == 'd')
		{
			ContentAwareRemoval(imgClone);
			cv::imshow(ORIGINAL_IMAGE, imgClone);
		}
		else if (key == 'r')
		{
			imgClone = originalImg.clone();
			brushMask = cv::Mat::zeros(originalImg.size(), CV_8UC1);
			cv::imshow(ORIGINAL_IMAGE, imgClone);
		}
		else if (key == cv::ESC_KEY)
			break;
	}

	//ShowCursor(TRUE);

	editor.Shutdown();
	cv::destroyAllWindows();
	return 0;
}
