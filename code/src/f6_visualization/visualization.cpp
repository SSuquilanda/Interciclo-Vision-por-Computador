#include "visualization.h"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

namespace Visualization {

cv::Mat drawHistogram(const cv::Mat& image) {
    // Calcular el histograma
    cv::Mat hist;
    int histSize = 256;    // 256 bins (0-255)
    float range[] = { 0, 256 };
    const float* histRange = { range };
    cv::calcHist(&image, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, true, false);

    // Crear la imagen para mostrar el histograma
    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound((double)hist_w / histSize);
    cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0, 0, 0));

    // Normalizar el histograma para que quepa en la imagen
    cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

    // Dibujar las barras
    for (int i = 1; i < histSize; i++) {
        cv::line(histImage,
            cv::Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
            cv::Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))),
            cv::Scalar(255, 255, 255), 2, 8, 0);
    }
    return histImage;
}

void showImage(const std::string& windowName, const cv::Mat& image) {
    cv::imshow(windowName, image);
}

void showImageWithHistogram(const cv::Mat& image, const std::string& baseWindowName) {
    // Mostrar la imagen
    showImage(baseWindowName, image);
    
    // Calcular y mostrar el histograma
    cv::Mat histImage = drawHistogram(image);
    showImage(baseWindowName + " - Histograma", histImage);
}

void waitAndClose(const std::string& message) {
    std::cout << message << "\n";
    cv::waitKey(0);
    cv::destroyAllWindows();
}

} // namespace Visualization
