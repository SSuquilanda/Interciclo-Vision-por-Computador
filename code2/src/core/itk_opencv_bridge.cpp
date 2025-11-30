#include "itk_opencv_bridge.h"
#include <iostream>

namespace Bridge {

cv::Mat itkToOpenCV(ImageType::Pointer itkImage) {
    std::cout << "--- Puente ITK -> OpenCV ---\n";
    std::cout << "Convirtiendo imagen a cv::Mat...\n";
    
    cv::Mat cvImage;
    try {
        cvImage = itk::OpenCVImageBridge::ITKImageToCVMat<ImageType>(itkImage.GetPointer(), false);
    }
    catch (const itk::ExceptionObject& ex) {
        throw std::runtime_error("Excepcion al convertir la imagen: " + std::string(ex.GetDescription()));
    }
    
    std::cout << "Conversion exitosa.\n";
    std::cout << "  Tipo OpenCV: " << cvImage.type() << "\n";
    std::cout << "  Dimensiones: " << cvImage.cols << "x" << cvImage.rows << "\n\n";
    
    return cvImage;
}

cv::Mat normalize16to8bit(const cv::Mat& image) {
    cv::Mat normalized;
    cv::normalize(image, normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
    return normalized;
}

cv::Mat convertToColor(const cv::Mat& image) {
    cv::Mat colorImage;
    if (image.channels() == 1) {
        cv::cvtColor(image, colorImage, cv::COLOR_GRAY2BGR);
    } else {
        colorImage = image.clone();
    }
    return colorImage;
}

} // namespace Bridge
