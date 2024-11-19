#ifndef SEAMCARVING_H
#define SEAMCARVING_H

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include "Utility2.h"

// =============
// OBJECT REMOVAL
// =============

std::vector<util::Mask> GetBoundedArea(cv::Point2i start, cv::Point2i end);
bool ModifyMask(std::vector<util::Mask> &area, const std::vector<int> &seam);

void ModifyVerticalEnergyMap(cv::Mat &energyMap, const std::vector<util::Mask> &area, double setTo = 0.0);
void ModifyHorizontalEnergyMap(cv::Mat &energyMap, const std::vector<util::Mask> &area, double setTo = 0.0);

void ContentAwareRemoval(cv::Mat &img);

// =============
// ENERGY MAP
// =============

cv::Mat CalculateEnergyMap(std::vector<cv::Mat> const &channels);

cv::Mat CalculateVerticalCumMap(const cv::Mat &energyMap);
cv::Mat CalculateHorizontalCumMap(const cv::Mat &energyMap);

// =============
// SEAM CARVING - VERTICAL
// =============

// - start from top row. At each pixel, choose the pixel with the least energy from the row below (directly under or one pixel to the left/right)
std::vector<int> FindVerticalSeamGreedy(cv::Mat const &energyMap);
std::vector<int> FindVerticalSeamDP(cv::Mat &cumMap);
std::vector<int> FindVerticalSeamCutGraph(cv::Mat const& energyMap);

void RemoveVerticalSeam(cv::Mat &img, std::vector<int> const &seam);

// perform seam carving on the img to the specified target width
void VerticalSeamCarvingGreedy(cv::Mat &img, int targetWidth);
void VerticalSeamCarvingDP(cv::Mat &img, int targetWidth);

// ===============
// SEAM CARVING - HORIZONTAL
// ===============

// - start from left col. At each pixel, choose the pixel with the least energy from the row below (directly right or one pixel to the top/bottom)
std::vector<int> FindHorizontalSeamGreedy(cv::Mat const &energyMap);
std::vector<int> FindHorizontalSeamDP(cv::Mat &cumMap);

void RemoveHorizontalSeam(cv::Mat const &img, std::vector<int> const &seam);

// perform seam carving on the img to the specified target height
void HorizontalSeamCarvingGreedy(cv::Mat &img, int targetHeight);
void HorizontalSeamCarvingDP(cv::Mat &img, int targetHeight);

// ===============
// VISUALIZATION
// ===============

void VisualizeVerticalSeam(cv::Mat &img, std::vector<int> const &seam, cv::Vec3b const &colour = (255, 0, 0), int waitForMs = 1); // ms was 50
void VisualizeHorizontalSeam(cv::Mat &img, std::vector<int> const &seam, cv::Vec3b const &colour = (255, 0, 0), int waitForMs = 1); // ms was 50

void DrawVerticalBoundary(cv::Mat &img, int pos, cv::Vec3b const &colour = (0, 0, 255));
void DrawHorizontalBoundary(cv::Mat &img, int pos, cv::Vec3b const &colour = (0, 0, 255));

#endif
