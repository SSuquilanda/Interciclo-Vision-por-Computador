#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include <opencv2/opencv.hpp>

namespace Morphology {

    // Basicos
    cv::Mat erode(const cv::Mat& image, cv::Size kernelSize, int iterations = 1);
    cv::Mat dilate(const cv::Mat& image, cv::Size kernelSize, int iterations = 1);
    cv::Mat opening(const cv::Mat& image, cv::Size kernelSize);
    cv::Mat closing(const cv::Mat& image, cv::Size kernelSize);

    // Avanzados
    cv::Mat fillHoles(const cv::Mat& mask);
    cv::Mat gradient(const cv::Mat& image, cv::Size kernelSize);
    cv::Mat topHat(const cv::Mat& image, cv::Size kernelSize);
    cv::Mat blackHat(const cv::Mat& image, cv::Size kernelSize);
    cv::Mat skeleton(const cv::Mat& mask);
    cv::Mat hitOrMiss(const cv::Mat& image, const cv::Mat& kernel1, const cv::Mat& kernel2);

    // Utilidades
    cv::Mat createStructuringElement(int shape, cv::Size ksize); 

}

#endif // MORPHOLOGY_H