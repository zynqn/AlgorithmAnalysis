// file header please

#ifndef UTILITY_H
#define UTILITY_H

// lib
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>


#define nl '\n'



// global variables
namespace 
{
	//double zoomFactor       = 1.0;
	double zoomLevel           = 1.0;
	const double zoomIncrement = 0.1;
	cv::Point2d zoomCenter (0, 0);
}

namespace cv
{
	const int ESC_KEY = 27;
}

namespace util
{
	//// Function to handle zoom in/out
	//void handleZoom(int direction, cv::Mat& img, cv::Mat& displayImg)
	//{
	//	if (direction > 0) // scroll up
	//		zoomFactor += zoomIncrement;

	//	else // scroll down
	//		zoomFactor = std::max(0.1, zoomFactor - zoomIncrement);  // no negative zoom


	//	// Resize image based on zoom factor
	//	cv::resize(img, displayImg, cv::Size(), zoomFactor, zoomFactor, cv::INTER_LINEAR);
	//}

	// Function to zoom into the image
	cv::Mat zoomImage(const cv::Mat& img, double zoom, const cv::Point2d& center) 
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

	void mouseCallback(int event, int x, int y, int flags, void* data)
	{
		//  get the image data from the void*
		cv::Mat* img = (cv::Mat*)data;

		switch (event)
		{
#if 0
			// mouse scroll (zoom in and out)
			case cv::EVENT_MOUSEWHEEL:
			{
				int delta = cv::getMouseWheelDelta(flags);

				zoomLevel += (delta > 0) ? zoomIncrement : -zoomIncrement;

				zoomLevel = std::max(0.1, zoomLevel);
				
				zoomCenter = cv::Point2d(x, y);

				cv::Mat zoomedImg = zoomImage(*img, zoomLevel, zoomCenter);

				cv::Mat resizedZoomedImg;
				cv::resize(zoomedImg, resizedZoomedImg, cv::Size(img->cols, img->rows), cv::INTER_CUBIC);//= zoomImage(*img, zoomLevel, zoomCenter);


				cv::imshow("Original Image", resizedZoomedImg);
				//handleZoom(scrollDirection, *img, *img);
				break;
			}
#endif

			// displaying mouse coordinate and RGB 
			case cv::EVENT_MOUSEMOVE:
			{
				// clone image for displaying overlay text
				cv::Mat displayImg = img->clone();

				// Calculate the corresponding coordinates in the original image
				int origX = static_cast<int>(x / zoomLevel);
				int origY = static_cast<int>(y / zoomLevel);

				// ensure mouse is clicked in the corresponding window
				if (origX >= 0 && origX < img->cols && origY >= 0 && origY < img->rows)
				{
					cv::Vec3b pixelValue = img->at<cv::Vec3b>(origY, origX);

					// Create a string with the RGB values
					std::string text = "X = " + std::to_string(origX) +
						" Y = " + std::to_string(origY) +
						" R = " + std::to_string((int)pixelValue[2]) +
						" G = " + std::to_string((int)pixelValue[1]) +
						" B = " + std::to_string((int)pixelValue[0]);

					// Display the text on the image (top-left corner)
					cv::putText(displayImg, text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));

					// Show the image with the text overlay
					cv::imshow("Original Image", displayImg);

				}
				break;
			}

		} // end switch
	}
}



#endif
