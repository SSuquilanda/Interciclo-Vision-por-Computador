#include "preprocessing.h"
#include <iostream>
#include <vector>

namespace Preprocessing {

// CONVERSIÓN Y NORMALIZACIÓN

cv::Mat convertToGrayscale(const cv::Mat& image) {
    cv::Mat gray;
    
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    return gray;
}

cv::Mat normalizeImage(const cv::Mat& image, double minVal, double maxVal) {
    cv::Mat normalized;
    cv::normalize(image, normalized, minVal, maxVal, cv::NORM_MINMAX);
    return normalized;
}

// ECUALIZACIÓN DE HISTOGRAMA

cv::Mat equalizeHistogram(const cv::Mat& image) {
    cv::Mat equalized;
    
    // Verificar que la imagen esté en escala de grises
    cv::Mat gray = convertToGrayscale(image);
    
    cv::equalizeHist(gray, equalized);
    
    return equalized;
}

cv::Mat applyCLAHE(const cv::Mat& image, double clipLimit, cv::Size tileGridSize) {
    cv::Mat clahe_result;
    
    // Verificar que la imagen esté en escala de grises
    cv::Mat gray = convertToGrayscale(image);
    
    // Crear objeto CLAHE
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileGridSize);
    clahe->apply(gray, clahe_result);
    
    return clahe_result;
}

// FILTROS DE REDUCCIÓN DE RUIDO

cv::Mat applyGaussianBlur(const cv::Mat& image, int kernelSize, double sigma) {
    cv::Mat blurred;
    
    // Asegurar que el kernel sea impar
    if (kernelSize % 2 == 0) {
        kernelSize++;
    }
    
    cv::GaussianBlur(image, blurred, cv::Size(kernelSize, kernelSize), sigma);
    
    return blurred;
}

cv::Mat applyMedianFilter(const cv::Mat& image, int kernelSize) {
    cv::Mat filtered;
    
    // Asegurar que el kernel sea impar
    if (kernelSize % 2 == 0) {
        kernelSize++;
    }
    
    cv::medianBlur(image, filtered, kernelSize);
    
    return filtered;
}

cv::Mat applyMeanFilter(const cv::Mat& image, int kernelSize) {
    cv::Mat filtered;
    
    cv::blur(image, filtered, cv::Size(kernelSize, kernelSize));
    
    return filtered;
}

cv::Mat applyBilateralFilter(const cv::Mat& image, int d, 
                              double sigmaColor, double sigmaSpace) {
    cv::Mat filtered;
    
    cv::bilateralFilter(image, filtered, d, sigmaColor, sigmaSpace);
    
    return filtered;
}

// DETECCIÓN Y REALCE DE BORDES

cv::Mat detectEdgesCanny(const cv::Mat& image, double lowThreshold, 
                          double highThreshold, int apertureSize) {
    cv::Mat edges;
    
    // Verificar que la imagen esté en escala de grises
    cv::Mat gray = convertToGrayscale(image);
    
    cv::Canny(gray, edges, lowThreshold, highThreshold, apertureSize);
    
    return edges;
}

cv::Mat detectEdgesSobel(const cv::Mat& image, int dx, int dy, int kernelSize) {
    cv::Mat sobel, abs_sobel;
    
    // Verificar que la imagen esté en escala de grises
    cv::Mat gray = convertToGrayscale(image);
    
    // Aplicar Sobel
    cv::Sobel(gray, sobel, CV_16S, dx, dy, kernelSize);
    
    // Convertir a valor absoluto y escalar a 8 bits
    cv::convertScaleAbs(sobel, abs_sobel);
    
    return abs_sobel;
}

cv::Mat applyLaplacian(const cv::Mat& image, int kernelSize) {
    cv::Mat laplacian, abs_laplacian;
    
    // Verificar que la imagen esté en escala de grises
    cv::Mat gray = convertToGrayscale(image);
    
    // Aplicar Laplaciano
    cv::Laplacian(gray, laplacian, CV_16S, kernelSize);
    
    // Convertir a valor absoluto y escalar a 8 bits
    cv::convertScaleAbs(laplacian, abs_laplacian);
    
    return abs_laplacian;
}

// PIPELINE DE PREPROCESAMIENTO COMPLETO

cv::Mat preprocessCTImage(const cv::Mat& image, bool useCLAHE, bool useDenoising) {
    // TODO: Implementar pipeline completo según necesidades específicas
    // Este será el método principal que combine las técnicas apropiadas
    
    cv::Mat processed = image.clone();
    
    // 1. Convertir a escala de grises
    processed = convertToGrayscale(processed);
    
    // 2. Aplicar ecualización (CLAHE o clásica)
    if (useCLAHE) {
        processed = applyCLAHE(processed, 2.0, cv::Size(8, 8));
    } else {
        processed = equalizeHistogram(processed);
    }
    
    // 3. Reducción de ruido (opcional)
    if (useDenoising) {
        processed = applyGaussianBlur(processed, 5, 0);
    }
    
    return processed;
}

std::vector<std::pair<std::string, cv::Mat>> comparePreprocessingTechniques(const cv::Mat& image) {
    // TODO: Implementar comparación de múltiples técnicas
    // Útil para determinar qué método funciona mejor
    
    std::vector<std::pair<std::string, cv::Mat>> results;
    
    cv::Mat gray = convertToGrayscale(image);
    
    // Agregar imagen original
    results.push_back({"Original", gray});
    
    // Agregar diferentes técnicas
    results.push_back({"Equalizado", equalizeHistogram(gray)});
    results.push_back({"CLAHE", applyCLAHE(gray)});
    results.push_back({"Gaussian Blur", applyGaussianBlur(gray)});
    results.push_back({"Median Filter", applyMedianFilter(gray)});
    results.push_back({"Bilateral Filter", applyBilateralFilter(gray)});
    results.push_back({"Canny Edges", detectEdgesCanny(gray)});
    results.push_back({"Sobel Edges", detectEdgesSobel(gray)});
    
    return results;
}

} // namespace Preprocessing
