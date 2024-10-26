#ifndef SEAMCARVING_H
#define SEAMCARVING_H

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>



cv::Mat CalculateEnergyMap(std::vector<cv::Mat> const& channels);



// =============
// VERTICAL SEAM
// =============
// - start from top row. At each pixel, choose the pixel with the least energy from the row below (directly under or one pixel to the left/right)
std::vector<int> FindVerticalSeamGreedy(cv::Mat const& energyMap);

void RemoveVerticalSeam(cv::Mat const& img, std::vector<int> const& seam);


// perform seam carving on the img to the specified target width
void SeamCarvingToWidth(cv::Mat& img, int targetWidth);

void VisualizeSeam(cv::Mat& img, std::vector<int> const& seam, cv::Vec3b const& colour = (255, 0, 0), int waitForMs = 50);


#endif
