#include "morphology.h"

namespace Morphology {

    cv::Mat createStructuringElement(int shape, cv::Size ksize) {
        return cv::getStructuringElement(shape, ksize);
    }

    cv::Mat erode(const cv::Mat& image, cv::Size kernelSize, int iterations) {
        cv::Mat result;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSize);
        cv::erode(image, result, kernel, cv::Point(-1,-1), iterations);
        return result;
    }

    cv::Mat dilate(const cv::Mat& image, cv::Size kernelSize, int iterations) {
        cv::Mat result;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSize);
        cv::dilate(image, result, kernel, cv::Point(-1,-1), iterations);
        return result;
    }

    cv::Mat opening(const cv::Mat& image, cv::Size kernelSize) {
        cv::Mat result;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSize);
        cv::morphologyEx(image, result, cv::MORPH_OPEN, kernel);
        return result;
    }

    cv::Mat closing(const cv::Mat& image, cv::Size kernelSize) {
        cv::Mat result;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSize);
        cv::morphologyEx(image, result, cv::MORPH_CLOSE, kernel);
        return result;
    }

    cv::Mat gradient(const cv::Mat& image, cv::Size kernelSize) {
        cv::Mat result;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSize);
        cv::morphologyEx(image, result, cv::MORPH_GRADIENT, kernel);
        return result;
    }

    cv::Mat fillHoles(const cv::Mat& mask) {
        cv::Mat filled = mask.clone();
        cv::Mat floodFillMask = cv::Mat::zeros(mask.rows + 2, mask.cols + 2, CV_8U);
        cv::floodFill(filled, floodFillMask, cv::Point(0, 0), cv::Scalar(255));
        cv::bitwise_not(filled, filled);
        return (mask | filled);
    }

    cv::Mat topHat(const cv::Mat& image, cv::Size kernelSize) {
        cv::Mat result;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSize);
        cv::morphologyEx(image, result, cv::MORPH_TOPHAT, kernel);
        return result;
    }

    cv::Mat blackHat(const cv::Mat& image, cv::Size kernelSize) {
        cv::Mat result;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSize);
        cv::morphologyEx(image, result, cv::MORPH_BLACKHAT, kernel);
        return result;
    }

    cv::Mat skeleton(const cv::Mat& mask) {
        cv::Mat skel = cv::Mat::zeros(mask.size(), CV_8U);
        cv::Mat temp, eroded;
        cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
        
        cv::Mat img = mask.clone();
        bool done = false;
        
        while (!done) {
            cv::erode(img, eroded, element);
            cv::dilate(eroded, temp, element);
            cv::subtract(img, temp, temp);
            cv::bitwise_or(skel, temp, skel);
            eroded.copyTo(img);
            
            done = (cv::countNonZero(img) == 0);
        }
        
        return skel;
    }

    cv::Mat hitOrMiss(const cv::Mat& image, const cv::Mat& kernel1, const cv::Mat& kernel2) {
        cv::Mat result;
        cv::morphologyEx(image, result, cv::MORPH_HITMISS, kernel1);
        return result;
    }

}