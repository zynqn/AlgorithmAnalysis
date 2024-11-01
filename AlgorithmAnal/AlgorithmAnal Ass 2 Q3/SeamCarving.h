#ifndef SEAMCARVING_H
#define SEAMCARVING_H

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include "Utility.h"

std::vector<util::Mask> GetBoundedArea(cv::Point2i start, cv::Point2i end);

cv::Mat CalculateEnergyMap(std::vector<cv::Mat> const& channels);

void ModifyEnergyMap(cv::Mat& energyMap, const std::vector<util::Mask>& area, double setTo = 0.0);
void ModifyEnergyMapH(cv::Mat &energyMap, const std::vector<util::Mask> &area, double setTo = 0.0);

cv::Mat CalculateCumMap(const cv::Mat& energyMap);
cv::Mat CalculateCumMapH(const cv::Mat &energyMap);

bool ModifyMask(std::vector<util::Mask> &area, const std::vector<int> &seam);

// =============
// VERTICAL SEAM
// =============
// - start from top row. At each pixel, choose the pixel with the least energy from the row below (directly under or one pixel to the left/right)
std::vector<int> FindVerticalSeamGreedy(cv::Mat const& energyMap);
std::vector<int> FindVerticalSeamDP(cv::Mat &cumMap);
std::vector<int> FindHorizontalSeamDP(cv::Mat& cumMap);

void RemoveVerticalSeam(cv::Mat const& img, std::vector<int> const& seam);
void RemoveHorizontalSeam(cv::Mat const& img, std::vector<int> const& seam);

// perform seam carving on the img to the specified target width
void SeamCarvingToWidth(cv::Mat &img, int targetWidth);
void SeamCarvingToWidthDP(cv::Mat& img, int targetWidth, bool isRemovingObject = false);
void SeamCarvingToHeightDP(cv::Mat& img, int targetHeight, bool isRemovingObject = false);

void VisualizeSeam(cv::Mat& img, std::vector<int> const& seam, cv::Vec3b const& colour = (255, 0, 0), int waitForMs = 1); // ms was 50
void VisualizeSeamH(cv::Mat& img, std::vector<int> const& seam, cv::Vec3b const& colour = (255, 0, 0), int waitForMs = 1); // ms was 50

// ===============
// HORIZONTAL SEAM
// ===============
std::vector<int> FindHorizontalSeamGreedy(cv::Mat const& energyMap);

void RemoveHorizontalSeam(cv::Mat const& img, std::vector<int> const& seam);

void SeamCarvingToHeight(cv::Mat& img, int targetHeight);

void VisualizeHSeam(cv::Mat& img, std::vector<int> const& seam, cv::Vec3b const& colour = (255, 0, 0), int waitForMs = 1); // ms was 50

void DrawBoundary(cv::Mat &img, int pos, cv::Vec3b const &colour = (0, 0, 255));
void DrawBoundary(cv::Mat& img, int pos, cv::Vec3b const& colour = (0, 0, 255));
void DrawBoundaryH(cv::Mat &img, int pos, cv::Vec3b const &colour = (0, 0, 255));

#endif
