/**
 * @file SeamCarving.h
 * @brief Header file for seam carving operations and utilities.
 *
 * This file defines a collection of functions to perform content-aware image resizing
 * (seam carving). The operations include energy map computation, seam identification,
 * seam removal, visualization, and boundary drawing for both vertical and horizontal
 * seams. It also supports different algorithms for seam identification, such as
 * Greedy, Dynamic Programming (DP), and Graph Cut methods.
 *
 * Key Features:
 * - Compute energy maps for images.
 * - Identify vertical and horizontal seams using various algorithms.
 * - Remove identified seams to achieve image resizing.
 * - Visualize seams and boundaries for debugging and analysis.
 *
 * Dependencies:
 * - OpenCV: Required for image processing.
 *
 * Usage:
 * - Include this file in your project and link against OpenCV to enable the provided functionality.
 * - Use the provided seam carving functions to resize images while preserving important content.
 *
 * Author: Team 12
 * Date: 21/11/2024
 */


#ifndef SEAMCARVING_H
#define SEAMCARVING_H

// open cv lib
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

// utility functions
#include "Utility2.h"

// plot graph
#include "matplotlibcpp.h"


// =============
// OBJECT REMOVAL
// =============

/**
 * @brief Generates a collection of masks representing the bounded area between two points in a 2D grid.
 *
 * @param start The starting point of the bounded area (cv::Point2i).
 * @param end The ending point of the bounded area (cv::Point2i).
 * @return std::vector<util::Mask> A vector of masks representing the bounded area.
 */
std::vector<util::Mask> GetBoundedArea(cv::Point2i start, cv::Point2i end);


/**
 * @brief Modifies a collection of masks by applying a specified seam to it.
 *
 * @param area A reference to the vector of masks representing the area to be modified.
 * @param seam A vector of integers representing the seam to apply.
 * @return true If the modification was successful.
 * @return false If the modification failed.
 */
bool ModifyMask(std::vector<util::Mask> &area, const std::vector<int> &seam);


/**
 * @brief Modifies the vertical energy map by applying a set value to a specific area.
 *
 * @param energyMap A reference to the energy map matrix to modify (cv::Mat).
 * @param area The area (vector of masks) to modify within the energy map.
 * @param setTo The value to set within the specified area (default is 0.0).
 */
void ModifyVerticalEnergyMap(cv::Mat &energyMap, const std::vector<util::Mask> &area, double setTo = 0.0);


/**
 * @brief Modifies the horizontal energy map by applying a set value to a specific area.
 *
 * @param energyMap A reference to the energy map matrix to modify (cv::Mat).
 * @param area The area (vector of masks) to modify within the energy map.
 * @param setTo The value to set within the specified area (default is 0.0).
 */
void ModifyHorizontalEnergyMap(cv::Mat &energyMap, const std::vector<util::Mask> &area, double setTo = 0.0);


/**
 * @brief Performs content-aware removal on an image.
 *
 * @param img A reference to the image to be processed (cv::Mat).
 */
void ContentAwareRemoval(cv::Mat &img);

// =============
// ENERGY MAP
// =============

/**
 * @brief Calculates the energy map of an image based on its individual color channels.
 *
 * @param channels A vector of image channels (cv::Mat) used to compute the energy map.
 *                 Typically, these channels are derived from splitting the image into its color components.
 * @return cv::Mat The computed energy map as a single-channel matrix.
 */
cv::Mat CalculateEnergyMap(std::vector<cv::Mat> const &channels);

/**
 * @brief Computes the vertical cumulative energy map from a given energy map.
 *
 * @param energyMap A reference to the input energy map (cv::Mat).
 * @return cv::Mat The vertical cumulative energy map, where each pixel contains the cumulative energy
 *                 value from the top row to that pixel.
 */
cv::Mat CalculateVerticalCumMap(const cv::Mat &energyMap);


/**
 * @brief Computes the horizontal cumulative energy map from a given energy map.
 *
 * @param energyMap A reference to the input energy map (cv::Mat).
 * @return cv::Mat The horizontal cumulative energy map, where each pixel contains the cumulative energy
 *                 value from the leftmost column to that pixel.
 */
cv::Mat CalculateHorizontalCumMap(const cv::Mat &energyMap);


// =============
// SEAM CARVING - VERTICAL
// =============

/**
 * @brief Finds a vertical seam in an energy map using a greedy approach.
 *
 * @param energyMap A constant reference to the energy map (cv::Mat) where the seam will be identified.
 * @return std::vector<int> A vector representing the vertical seam, where each element indicates the column index of the seam at a specific row.
 */
std::vector<int> FindVerticalSeamGreedy(cv::Mat const &energyMap);


/**
 * @brief Finds a vertical seam in a cumulative energy map using dynamic programming.
 *
 * @param cumMap A reference to the cumulative energy map (cv::Mat) where the seam will be identified.
 *               This map is modified internally as part of the seam-finding process.
 * @return std::vector<int> A vector representing the vertical seam, where each element indicates the column index of the seam at a specific row.
 */
std::vector<int> FindVerticalSeamDP(cv::Mat &cumMap);


/**
 * @brief Finds a vertical seam in an energy map using a graph cut algorithm.
 *
 * @param energyMap A constant reference to the energy map (cv::Mat) where the seam will be identified.
 * @return std::vector<int> A vector representing the vertical seam, where each element indicates the column index of the seam at a specific row.
 */
std::vector<int> FindVerticalSeamGraphCut(cv::Mat const& energyMap);


/**
 * @brief Removes a vertical seam from an image.
 *
 * @param img A reference to the image (cv::Mat) from which the seam will be removed.
 * @param seam A constant reference to a vector representing the vertical seam to remove. Each element indicates the column index of the seam at a specific row.
 */
void RemoveVerticalSeam(cv::Mat &img, std::vector<int> const &seam);


/**
 * @brief Performs vertical seam carving on the image to resize it to the specified target width using a greedy algorithm.
 *
 * @param img A reference to the input image (cv::Mat) to be resized.
 * @param targetWidth The desired width of the image after seam carving.
 */
void VerticalSeamCarvingGreedy(cv::Mat &img, int targetWidth);


/**
 * @brief Performs vertical seam carving on the image to resize it to the specified target width using dynamic programming.
 *
 * @param img A reference to the input image (cv::Mat) to be resized.
 * @param targetWidth The desired width of the image after seam carving.
 */
void VerticalSeamCarvingDP(cv::Mat &img, int targetWidth);


/**
 * @brief Performs vertical seam carving on the image to resize it to the specified target width using a graph cut algorithm.
 *
 * @param img A reference to the input image (cv::Mat) to be resized.
 * @param targetWidth The desired width of the image after seam carving.
 */
void VerticalSeamCarvingGraphCut(cv::Mat& img, int targetWidth);

// ===============
// SEAM CARVING - HORIZONTAL
// ===============

/**
 * @brief Finds a horizontal seam in an energy map using a greedy approach.
 *
 * @param energyMap A constant reference to the energy map (cv::Mat) where the seam will be identified.
 * @return std::vector<int> A vector representing the horizontal seam, where each element indicates the row index of the seam at a specific column.
 */
std::vector<int> FindHorizontalSeamGreedy(cv::Mat const &energyMap);


/**
 * @brief Finds a horizontal seam in a cumulative energy map using dynamic programming.
 *
 * @param cumMap A reference to the cumulative energy map (cv::Mat) where the seam will be identified.
 *               This map is modified internally as part of the seam-finding process.
 * @return std::vector<int> A vector representing the horizontal seam, where each element indicates the row index of the seam at a specific column.
 */
std::vector<int> FindHorizontalSeamDP(cv::Mat &cumMap);


/**
 * @brief Finds a horizontal seam in an energy map using a graph cut algorithm.
 *
 * @param energyMap A constant reference to the energy map (cv::Mat) where the seam will be identified.
 * @return std::vector<int> A vector representing the horizontal seam, where each element indicates the row index of the seam at a specific column.
 */
std::vector<int> FindHorizontalSeamGraphCut(cv::Mat const& energyMap);


/**
 * @brief Removes a horizontal seam from an image.
 *
 * @param img A reference to the image (cv::Mat) from which the seam will be removed.
 * @param seam A constant reference to a vector representing the horizontal seam to remove. Each element indicates the row index of the seam at a specific column.
 */
void RemoveHorizontalSeam(cv::Mat &img, std::vector<int> const &seam);


/**
 * @brief Performs horizontal seam carving on the image to resize it to the specified target height using a greedy algorithm.
 *
 * @param img A reference to the input image (cv::Mat) to be resized.
 * @param targetHeight The desired height of the image after seam carving.
 */
void HorizontalSeamCarvingGreedy(cv::Mat &img, int targetHeight);


/**
 * @brief Performs horizontal seam carving on the image to resize it to the specified target height using dynamic programming.
 *
 * @param img A reference to the input image (cv::Mat) to be resized.
 * @param targetHeight The desired height of the image after seam carving.
 */
void HorizontalSeamCarvingDP(cv::Mat &img, int targetHeight);


/**
 * @brief Performs horizontal seam carving on the image to resize it to the specified target height using a graph cut algorithm.
 *
 * @param img A reference to the input image (cv::Mat) to be resized.
 * @param targetHeight The desired height of the image after seam carving.
 */
void HorizontalSeamCarvingGraphCut(cv::Mat& img, int targetHeight);

// ===============
// VISUALIZATION
// ===============

/**
 * @brief Visualizes a vertical seam on the image by overlaying a colored line.
 *
 * @param img A reference to the image (cv::Mat) on which the vertical seam will be visualized.
 * @param seam A constant reference to a vector representing the vertical seam. Each element specifies the column index of the seam at a specific row.
 * @param colour A constant reference to the color (cv::Vec3b) of the seam line, defaulting to red (255, 0, 0).
 * @param waitForMs The delay in milliseconds to wait after displaying the visualization, defaulting to 1 ms.
 */
void VisualizeVerticalSeam(cv::Mat &img, std::vector<int> const &seam, cv::Vec3b const &colour = (255, 0, 0)); 


/**
 * @brief Visualizes a horizontal seam on the image by overlaying a colored line.
 *
 * @param img A reference to the image (cv::Mat) on which the horizontal seam will be visualized.
 * @param seam A constant reference to a vector representing the horizontal seam. Each element specifies the row index of the seam at a specific column.
 * @param colour A constant reference to the color (cv::Vec3b) of the seam line, defaulting to red (255, 0, 0).
 * @param waitForMs The delay in milliseconds to wait after displaying the visualization, defaulting to 1 ms.
 */
void VisualizeHorizontalSeam(cv::Mat &img, std::vector<int> const &seam, cv::Vec3b const &colour = (255, 0, 0));


/**
 * @brief Draws a vertical boundary line on the image at the specified column position.
 *
 * @param img A reference to the image (cv::Mat) on which the vertical boundary will be drawn.
 * @param pos The column index at which the boundary line will be drawn.
 * @param colour A constant reference to the color (cv::Vec3b) of the boundary line, defaulting to blue (0, 0, 255).
 */
void DrawVerticalBoundary(cv::Mat &img, int pos, cv::Vec3b const &colour = (0, 0, 255));


/**
 * @brief Draws a horizontal boundary line on the image at the specified row position.
 *
 * @param img A reference to the image (cv::Mat) on which the horizontal boundary will be drawn.
 * @param pos The row index at which the boundary line will be drawn.
 * @param colour A constant reference to the color (cv::Vec3b) of the boundary line, defaulting to blue (0, 0, 255).
 */
void DrawHorizontalBoundary(cv::Mat &img, int pos, cv::Vec3b const &colour = (0, 0, 255));

#endif
