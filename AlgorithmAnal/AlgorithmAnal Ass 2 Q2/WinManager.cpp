#pragma once
#include "WinManager.h"
#include "Editor.h"

extern edit::Editor editor;

WinManager::WinManager()
{

}

void WinManager::UpdateOIWin(bool toggle, cv::Mat& img)
{
	if (!editor.GetWindow<edit::ImageLoader>()->isFileLoaded)
		return;

	if (toggle)
	{
		if (!OIWin)
		{
			cv::namedWindow(ORIGINAL_IMAGE, cv::WINDOW_NORMAL);
			cv::setWindowProperty(ORIGINAL_IMAGE, cv::WND_PROP_AUTOSIZE, cv::WINDOW_NORMAL);

			brushMask = cv::Mat::zeros(img.size(), CV_8UC1);
			maskInitialized = true;

			// set mouse callback (to display the mouse coordinates as will as the respective RGB values of selected pixel)
			cv::setMouseCallback(ORIGINAL_IMAGE, util::mouseCallback, &img);
			cv::imshow(ORIGINAL_IMAGE, img);
			OIWin = true;
		}
	}
	else
	{
		if (OIWin)
		{
			cv::destroyWindow(ORIGINAL_IMAGE);
			OIWin = false;
		}
	}
}

void WinManager::UpdateEMWin(bool toggle, cv::Mat& em)
{
	if (!editor.GetWindow<edit::ImageLoader>()->isFileLoaded)
		return;

	if (toggle)
	{
		if (!EMWin)
		{
			cv::imshow(ENERGY_MAP, em);
			EMWin = true;
		}
	}
	else
	{
		if (EMWin)
		{
			EMWin = false;
			cv::destroyWindow(ENERGY_MAP);
		}
	}
}