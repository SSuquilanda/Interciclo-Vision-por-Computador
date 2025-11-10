#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include "opencv2/core.hpp"
#include <string>

namespace Visualization {

// Calcula y dibuja un histograma para una imagen de 8 bits
cv::Mat drawHistogram(const cv::Mat& image);

// Muestra una imagen en una ventana con nombre específico
void showImage(const std::string& windowName, const cv::Mat& image);

// Muestra una imagen junto con su histograma en ventanas separadas
void showImageWithHistogram(const cv::Mat& image, const std::string& baseWindowName = "Image");

// Espera una tecla y cierra todas las ventanas
void waitAndClose(const std::string& message = "Presiona ESC para salir.");

} // namespace Visualization

#endif // VISUALIZATION_H
