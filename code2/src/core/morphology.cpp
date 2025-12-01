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
        // Algoritmo de floodfill para rellenar huecos internos
        cv::Mat filled = mask.clone();
        cv::Mat floodFillMask = cv::Mat::zeros(mask.rows + 2, mask.cols + 2, CV_8U);
        
        // Invertir y rellenar desde el borde
        cv::floodFill(filled, floodFillMask, cv::Point(0, 0), cv::Scalar(255));
        
        // Invertir de nuevo para obtener los huecos llenos
        cv::bitwise_not(filled, filled);
        
        // Combinar con la máscara original
        return (mask | filled);
    }

}