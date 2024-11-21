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
			//cv::namedWindow(ORIGINAL_IMAGE, cv::WINDOW_NORMAL);
			//cv::setWindowProperty(ORIGINAL_IMAGE, cv::WND_PROP_AUTOSIZE, cv::WINDOW_NORMAL);

			brushMask = cv::Mat::zeros(img.size(), CV_8UC1);
			maskInitialized = true;

			// set mouse callback (to display the mouse coordinates as will as the respective RGB values of selected pixel)
			//cv::setMouseCallback(ORIGINAL_IMAGE, util::mouseCallback, &img);
			//cv::imshow(ORIGINAL_IMAGE, img);
			util::ShowWindow(ORIGINAL_IMAGE_W, true);
			OIWin = true;
		}
	}
	else
	{
		if (OIWin)
		{
			util::ShowWindow(ORIGINAL_IMAGE_W, false);
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
			//cv::imshow(ENERGY_MAP, em);
			util::ShowWindow(ENERGY_MAP_W, true);
			EMWin = true;
		}
	}
	else
	{
		if (EMWin)
		{
			EMWin = false;
			util::ShowWindow(ENERGY_MAP_W, false);
		}
	}
}

void WinManager::UpdateCIWin(bool toggle, cv::Mat &em)
{
	if (!editor.GetWindow<edit::ImageLoader>()->isFileLoaded)
		return;

	if (toggle)
	{
		if (!CIWin)
		{
			//cv::imshow(CARVED_IMAGE, em);
			util::ShowWindow(CARVED_IMAGE_W, true);
			CIWin = true;
		}
	}
	else
	{
		if (CIWin)
		{
			CIWin = false;
			util::ShowWindow(CARVED_IMAGE_W, false);
		}
	}
}

void WinManager::UpdateASWin(bool toggle, cv::Mat &em)
{
	if (!editor.GetWindow<edit::ImageLoader>()->isFileLoaded)
		return;

	if (toggle)
	{
		if (!ASWin)
		{
			//cv::imshow(ALL_SEAMS, em);
			util::ShowWindow(ALL_SEAMS_W, true);
			ASWin = true;
		}
	}
	else
	{
		if (ASWin)
		{
			ASWin = false;
			util::ShowWindow(ALL_SEAMS_W, false);
		}
	}
}