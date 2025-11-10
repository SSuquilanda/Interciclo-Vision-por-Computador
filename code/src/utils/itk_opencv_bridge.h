#ifndef ITK_OPENCV_BRIDGE_H
#define ITK_OPENCV_BRIDGE_H

#include "itkImage.h"
#include "itkOpenCVImageBridge.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

namespace Bridge {

// Tipos de imagen ITK
const unsigned int Dimension = 2;
using PixelType = short;
using ImageType = itk::Image<PixelType, Dimension>;

// Convierte una imagen ITK a cv::Mat
cv::Mat itkToOpenCV(ImageType::Pointer itkImage);

// Normaliza una imagen de 16-bit a 8-bit para visualización
cv::Mat normalize16to8bit(const cv::Mat& image);

// Convierte una imagen a formato de 3 canales (BGR) para overlay
cv::Mat convertToColor(const cv::Mat& image);

} // namespace Bridge

#endif // ITK_OPENCV_BRIDGE_H
