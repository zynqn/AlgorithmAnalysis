// openCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

// std lib
#include <vector>
#include <iostream>

// seam carving
#include "SeamCarving.h"
#include "utility.h"

// graph
#include "matplotlibcpp.h"

#include <functional> // For std::bind
#include <thread>


// maxflow for graph cut
#include "graph.h"

namespace plt = matplotlibcpp;

cv::Mat brushMask;
bool isDrawing = false;
int brushSize = 10;
bool maskInitialized = false;




void plotter(cv::Mat imgTar)
{
    std::cout << "plotting\n";

    // Split the image into its 3 channels (B, G, R)
    std::vector<cv::Mat> channels2;
    cv::split(imgTar, channels2);  // channels[0] = Blue, channels[1] = Green, channels[2] = Red

    // get original energy map
    cv::Mat energyMapper = CalculateEnergyMap(channels2);
    cv::Mat displayEnergyMapper;

    // Convert the energy map back to 8-bit format for display
    cv::normalize(energyMapper, energyMapper, 0, 255, cv::NORM_MINMAX);
    energyMapper.convertTo(displayEnergyMapper, CV_8U);

    int rows2 = energyMapper.rows, cols2 = energyMapper.cols;

    // ===================== 
    // PREPARE 3D PLOT 
    // =====================
    std::vector<std::vector<double>> X2(rows2, std::vector<double>(cols2));
    std::vector<std::vector<double>> Y2(rows2, std::vector<double>(cols2));
    std::vector<std::vector<double>> Z2(rows2, std::vector<double>(cols2));

    for (int i = 0; i < rows2; ++i)
    {
        for (int j = 0; j < cols2; ++j)
        {
            X2[i][j] = j;  // X-coordinate (columns)
            Y2[i][j] = i;  // Y-coordinate (rows)
            Z2[i][j] = displayEnergyMapper.at<uchar>(i, j);  // Z-value (intensity)
        }
    }

    //plt::figure();
    plt::plot_surface(X2, Y2, Z2);  // Plot the 3D surface
    //plt::save("assets/output.png"); // Saves the plot as a PNG file
    /*cv::Mat energyMapImg = cv::imread("assets/output.png");
    cv::imshow("Energy Map Graph", energyMapImg);*/

    plt::show();
    //plt::pause(1.0);

    

    //plt::clf();
    //plt::close();
    //plt::show();  // Display the plot once

}

void plotter2(std::vector<std::vector<double>>& X2, std::vector<std::vector<double>>& Y2, std::vector<std::vector<double>>& Z2)
{
    //plt::figure();
    plt::plot_surface(X2, Y2, Z2);  // Plot the 3D surface
   
    plt::show();
    plt::pause(1.0);
    

}


void multiThreadPlot(cv::Mat imgTarget)
{
    std::thread plot_thread(std::bind(plotter, std::ref(imgTarget)));
    plot_thread.detach(); // Let the thread run independently
}

void multiThreadPlot2(std::vector<std::vector<double>>& X2, std::vector<std::vector<double>>& Y2, std::vector<std::vector<double>>& Z2)
{
    //std::thread plot_thread(std::bind(plotter2, std::ref(X2), std::ref(Y2), std::ref(Z2) ) );
    std::thread plot_thread(std::bind(plotter2, X2, Y2, Z2));
    plot_thread.detach(); // Let the thread run independently
}

int main()
{



    // ==============
    // LOAD THE IMAGE
    // ==============
    cv::Mat img = cv::imread("assets/arch_sunset.jpg");

    if (img.empty())
    {
        std::cerr << "Error: Could not load image.\n";
        return -1;
    }

    // ===============
    // SET THE WINDOWS
    // ===============
    cv::namedWindow("Original Image", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Original Energy Map", cv::WINDOW_AUTOSIZE);

    brushMask = cv::Mat::zeros(img.size(), CV_8UC1);
    maskInitialized = true;

    // set mouse callback (to display the mouse coordinates as well as the respective RGB values of selected pixel)
    cv::setMouseCallback("Original Image", util::mouseCallback, &img);

    // =======================
    // GET ORIGINAL ENERGY MAP
    // =======================
    std::vector<cv::Mat> channels;
    cv::split(img, channels);  // channels[0] = Blue, channels[1] = Green, channels[2] = Red

    cv::Mat energyMap = CalculateEnergyMap(channels);
    cv::Mat smallEnergyMap;
    cv::resize(energyMap, smallEnergyMap, cv::Size(energyMap.cols / 6, energyMap.rows / 6)); // Reduce resolution by factor of 2

    // Normalize for visualization
    cv::Mat displayEnergyMap;
    cv::normalize(smallEnergyMap, displayEnergyMap, 0, 255, cv::NORM_MINMAX);
    displayEnergyMap.convertTo(displayEnergyMap, CV_8U);

    int rows = displayEnergyMap.rows, cols = displayEnergyMap.cols;

    // ===================
    // DISPLAY THE WINDOWS
    // ===================
    cv::imshow("Original Image", img);
    cv::imshow("Original Energy Map", displayEnergyMap);

    //cv::moveWindow("Original Image", 0, 45);
    //cv::moveWindow("Original Energy Map", img.cols, 45);

    // clone the original image for the seam carving
    cv::Mat imgClone = img.clone();
    cv::Mat originalImg = img.clone();


#if 01
    // ===================== 
    // PREPARE 3D PLOT 
    // =====================
    std::vector<std::vector<double>> X(rows, std::vector<double>(cols));
    std::vector<std::vector<double>> Y(rows, std::vector<double>(cols));
    std::vector<std::vector<double>> Z(rows, std::vector<double>(cols));

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            X[i][j] = j;  // X-coordinate (columns)
            Y[i][j] = i;  // Y-coordinate (rows)
            Z[i][j] = displayEnergyMap.at<uchar>(i, j);  // Z-value (intensity)
        }
    }

    //plt::plot_surface(X, Y, Z);  // Plot the 3D surface
    //plt::show();  // Display the plot once
    //plt::clf();
    //plt::close();

    //plotter(imgClone);

    //std::thread plot_thread(std::bind(plotter, imgClone));

    //plot_thread.detach(); // Let the thread run independently
    // 
    //std::thread plot_thread(plotter);

    multiThreadPlot2(X,Y,Z);
    //plotter2(X, Y, Z);
    

#endif



    // Game loop
    while (true)
    {
        int key = cv::waitKey(1);

        if (key == 'c')
        {
            SeamCarvingToWidth(imgClone, 500);
            //
        }
            

        if (key == 'b')
            SeamCarvingToHeight(imgClone, 400);

        if (key == 'd')
        {
            ContentAwareRemoval(imgClone);
            cv::imshow("Original Image", imgClone);
            //multiThreadPlot(imgClone);
            //multiThreadPlot2(X, Y, Z);
            
        }
        else if (key == 'r')
        {
            imgClone = originalImg.clone();
            brushMask = cv::Mat::zeros(img.size(), CV_8UC1);
            cv::imshow("Original Image", imgClone);
        }
        else if (key == cv::ESC_KEY)
            break;

        //multiThreadPlot2(X, Y, Z);
    }

    cv::destroyAllWindows();
    //std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}
