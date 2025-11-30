#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include <opencv2/opencv.hpp>

namespace Morphology {

    // --- OPERACIONES BÁSICAS ---
    cv::Mat erode(const cv::Mat& image, cv::Size kernelSize, int iterations = 1);
    cv::Mat dilate(const cv::Mat& image, cv::Size kernelSize, int iterations = 1);
    cv::Mat opening(const cv::Mat& image, cv::Size kernelSize);
    cv::Mat closing(const cv::Mat& image, cv::Size kernelSize);

    // --- OPERACIONES AVANZADAS ---
    cv::Mat fillHoles(const cv::Mat& mask);
    cv::Mat gradient(const cv::Mat& image, cv::Size kernelSize); // Útil para bordes

    // --- UTILIDADES ---
    // Wrapper para facilitar la creación de kernels elípticos (los más usados en medicina)
    cv::Mat createStructuringElement(int shape, cv::Size ksize); 

}

#endif // MORPHOLOGY_H