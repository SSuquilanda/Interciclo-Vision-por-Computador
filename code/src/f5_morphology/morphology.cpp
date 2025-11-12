#include "morphology.h"
#include <iostream>
#include <algorithm>

namespace Morphology {

// CREACIÓN DE ELEMENTOS ESTRUCTURANTES

cv::Mat createStructuringElement(StructuringElementShape shape, 
                                  cv::Size size,
                                  cv::Point anchor) {
    int morphShape;
    
    switch (shape) {
        case MORPH_RECT:
            morphShape = cv::MORPH_RECT;
            break;
        case MORPH_ELLIPSE:
            morphShape = cv::MORPH_ELLIPSE;
            break;
        case MORPH_CROSS:
            morphShape = cv::MORPH_CROSS;
            break;
        default:
            morphShape = cv::MORPH_ELLIPSE;
    }
    
    return cv::getStructuringElement(morphShape, size, anchor);
}

cv::Mat createCustomKernel(const cv::Mat& kernel) {
    return kernel.clone();
}

// OPERACIONES MORFOLÓGICAS BÁSICAS

cv::Mat erode(const cv::Mat& image, 
              cv::Size kernelSize,
              StructuringElementShape shape,
              int iterations) {
    cv::Mat result;
    cv::Mat kernel = createStructuringElement(shape, kernelSize);
    cv::erode(image, result, kernel, cv::Point(-1, -1), iterations);
    return result;
}

cv::Mat dilate(const cv::Mat& image, 
               cv::Size kernelSize,
               StructuringElementShape shape,
               int iterations) {
    cv::Mat result;
    cv::Mat kernel = createStructuringElement(shape, kernelSize);
    cv::dilate(image, result, kernel, cv::Point(-1, -1), iterations);
    return result;
}

cv::Mat opening(const cv::Mat& image, 
                cv::Size kernelSize,
                StructuringElementShape shape) {
    cv::Mat result;
    cv::Mat kernel = createStructuringElement(shape, kernelSize);
    cv::morphologyEx(image, result, cv::MORPH_OPEN, kernel);
    return result;
}

cv::Mat closing(const cv::Mat& image, 
                cv::Size kernelSize,
                StructuringElementShape shape) {
    cv::Mat result;
    cv::Mat kernel = createStructuringElement(shape, kernelSize);
    cv::morphologyEx(image, result, cv::MORPH_CLOSE, kernel);
    return result;
}

// OPERACIONES MORFOLÓGICAS AVANZADAS

cv::Mat morphologicalGradient(const cv::Mat& image, 
                               cv::Size kernelSize,
                               StructuringElementShape shape) {
    cv::Mat result;
    cv::Mat kernel = createStructuringElement(shape, kernelSize);
    cv::morphologyEx(image, result, cv::MORPH_GRADIENT, kernel);
    return result;
}

cv::Mat topHat(const cv::Mat& image, 
               cv::Size kernelSize,
               StructuringElementShape shape) {
    cv::Mat result;
    cv::Mat kernel = createStructuringElement(shape, kernelSize);
    cv::morphologyEx(image, result, cv::MORPH_TOPHAT, kernel);
    return result;
}

cv::Mat blackHat(const cv::Mat& image, 
                 cv::Size kernelSize,
                 StructuringElementShape shape) {
    cv::Mat result;
    cv::Mat kernel = createStructuringElement(shape, kernelSize);
    cv::morphologyEx(image, result, cv::MORPH_BLACKHAT, kernel);
    return result;
}

cv::Mat hitOrMiss(const cv::Mat& image, 
                  const cv::Mat& kernel1, 
                  const cv::Mat& kernel2) {
    cv::Mat result;
    cv::Mat kernel = kernel1.clone();
    cv::morphologyEx(image, result, cv::MORPH_HITMISS, kernel);
    return result;
}

// ESQUELETIZACIÓN Y ADELGAZAMIENTO

cv::Mat skeletonize(const cv::Mat& image) {
    // TODO: Implementar algoritmo de esqueletización completo
    // Por ahora usar adelgazamiento iterativo
    return thinning(image);
}

cv::Mat thinning(const cv::Mat& image, int maxIterations) {
    // TODO: Implementar algoritmo de Zhang-Suen o similar
    // Placeholder: erosión simple
    
    cv::Mat result = image.clone();
    cv::Mat skel = cv::Mat::zeros(image.size(), CV_8U);
    cv::Mat temp;
    cv::Mat element = createStructuringElement(MORPH_CROSS, cv::Size(3, 3));
    
    int iterations = 0;
    bool done = false;
    
    while (!done && (maxIterations < 0 || iterations < maxIterations)) {
        cv::erode(result, temp, element);
        cv::dilate(temp, temp, element);
        cv::subtract(result, temp, temp);
        cv::bitwise_or(skel, temp, skel);
        cv::erode(result, result, element);
        
        double minVal, maxVal;
        cv::minMaxLoc(result, &minVal, &maxVal);
        done = (maxVal == 0);
        iterations++;
    }
    
    return skel;
}

// RECONSTRUCCIÓN MORFOLÓGICA

cv::Mat morphologicalReconstruction(const cv::Mat& marker, 
                                     const cv::Mat& mask,
                                     int maxIterations) {
    // TODO: Implementar reconstrucción morfológica geodésica
    
    cv::Mat result = marker.clone();
    cv::Mat prev = cv::Mat::zeros(marker.size(), marker.type());
    cv::Mat element = createStructuringElement(MORPH_CROSS, cv::Size(3, 3));
    
    int iterations = 0;
    
    while (cv::countNonZero(result != prev) > 0 && 
           (maxIterations < 0 || iterations < maxIterations)) {
        result.copyTo(prev);
        cv::dilate(result, result, element);
        cv::min(result, mask, result);
        iterations++;
    }
    
    return result;
}

cv::Mat fillHoles(const cv::Mat& image) {
    // Crear marcador invertido desde los bordes
    cv::Mat marker = cv::Mat::zeros(image.size(), image.type());
    
    // Marcar los bordes
    marker.row(0).setTo(255);
    marker.row(marker.rows - 1).setTo(255);
    marker.col(0).setTo(255);
    marker.col(marker.cols - 1).setTo(255);
    
    // Invertir la máscara
    cv::Mat mask;
    cv::bitwise_not(image, mask);
    
    // Reconstrucción
    cv::Mat filled = morphologicalReconstruction(marker, mask);
    
    // Invertir resultado
    cv::bitwise_not(filled, filled);
    
    return filled;
}

cv::Mat clearBorder(const cv::Mat& image) {
    // Crear marcador con los bordes
    cv::Mat marker = cv::Mat::zeros(image.size(), image.type());
    
    // Copiar bordes de la imagen original
    image.row(0).copyTo(marker.row(0));
    image.row(image.rows - 1).copyTo(marker.row(marker.rows - 1));
    image.col(0).copyTo(marker.col(0));
    image.col(image.cols - 1).copyTo(marker.col(marker.cols - 1));
    
    // Reconstrucción de objetos conectados a los bordes
    cv::Mat borderObjects = morphologicalReconstruction(marker, image);
    
    // Restar objetos de borde de la imagen original
    cv::Mat result;
    cv::subtract(image, borderObjects, result);
    
    return result;
}

// REFINAMIENTO DE SEGMENTACIONES

cv::Mat cleanMask(const cv::Mat& mask, 
                  const MorphParams& params,
                  int minObjectSize) {
    // 1. Apertura para eliminar ruido
    cv::Mat cleaned = opening(mask, params.kernelSize, params.shape);
    
    // 2. Cierre para rellenar huecos pequeños
    cleaned = closing(cleaned, params.kernelSize, params.shape);
    
    // 3. Eliminar componentes pequeños
    cv::Mat labels, stats, centroids;
    int nLabels = cv::connectedComponentsWithStats(cleaned, labels, stats, centroids);
    
    cv::Mat result = cv::Mat::zeros(mask.size(), CV_8U);
    
    for (int i = 1; i < nLabels; i++) {
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area >= minObjectSize) {
            result.setTo(255, labels == i);
        }
    }
    
    return result;
}

cv::Mat smoothContours(const cv::Mat& mask, cv::Size kernelSize) {
    // Aplicar cierre seguido de apertura
    cv::Mat smoothed = closing(mask, kernelSize, MORPH_ELLIPSE);
    smoothed = opening(smoothed, kernelSize, MORPH_ELLIPSE);
    return smoothed;
}

cv::Mat separateObjects(const cv::Mat& mask, cv::Size erosionSize) {
    // Erosión para separar
    cv::Mat eroded = erode(mask, erosionSize, MORPH_ELLIPSE, 1);
    
    // Reconstrucción para recuperar tamaños originales
    cv::Mat separated = morphologicalReconstruction(eroded, mask);
    
    return separated;
}

cv::Mat refineSegmentation(const cv::Mat& mask, 
                           const MorphParams& params,
                           int minObjectSize,
                           bool removeBorderObjects) {
    // 1. Limpieza inicial
    cv::Mat refined = cleanMask(mask, params, minObjectSize);
    
    // 2. Relleno de huecos
    refined = fillHoles(refined);
    
    // 3. Suavizado de contornos
    refined = smoothContours(refined, params.kernelSize);
    
    // 4. Eliminar objetos de borde si se solicita
    if (removeBorderObjects) {
        refined = clearBorder(refined);
    }
    
    return refined;
}

// OPERACIONES CON MÁSCARAS MÚLTIPLES

cv::Mat intersectMasks(const std::vector<cv::Mat>& masks) {
    if (masks.empty()) {
        return cv::Mat();
    }
    
    cv::Mat result = masks[0].clone();
    
    for (size_t i = 1; i < masks.size(); i++) {
        cv::bitwise_and(result, masks[i], result);
    }
    
    return result;
}

cv::Mat unionMasks(const std::vector<cv::Mat>& masks) {
    if (masks.empty()) {
        return cv::Mat();
    }
    
    cv::Mat result = masks[0].clone();
    
    for (size_t i = 1; i < masks.size(); i++) {
        cv::bitwise_or(result, masks[i], result);
    }
    
    return result;
}

cv::Mat subtractMasks(const cv::Mat& mask1, const cv::Mat& mask2) {
    cv::Mat result;
    cv::subtract(mask1, mask2, result);
    return result;
}

// ANÁLISIS MORFOMÉTRICO

std::vector<double> calculateAreas(const cv::Mat& mask) {
    std::vector<double> areas;
    
    cv::Mat labels, stats, centroids;
    int nLabels = cv::connectedComponentsWithStats(mask, labels, stats, centroids);
    
    for (int i = 1; i < nLabels; i++) {
        double area = stats.at<int>(i, cv::CC_STAT_AREA);
        areas.push_back(area);
    }
    
    return areas;
}

std::vector<double> calculatePerimeters(const cv::Mat& mask) {
    std::vector<double> perimeters;
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask.clone(), contours, cv::RETR_EXTERNAL, 
                     cv::CHAIN_APPROX_NONE);
    
    for (const auto& contour : contours) {
        double perimeter = cv::arcLength(contour, true);
        perimeters.push_back(perimeter);
    }
    
    return perimeters;
}

std::vector<double> calculateCircularity(const cv::Mat& mask) {
    std::vector<double> circularities;
    
    auto areas = calculateAreas(mask);
    auto perimeters = calculatePerimeters(mask);
    
    for (size_t i = 0; i < areas.size() && i < perimeters.size(); i++) {
        if (perimeters[i] > 0) {
            double circularity = 4.0 * CV_PI * areas[i] / (perimeters[i] * perimeters[i]);
            circularities.push_back(circularity);
        } else {
            circularities.push_back(0.0);
        }
    }
    
    return circularities;
}

// UTILIDADES

cv::Mat invertMask(const cv::Mat& mask) {
    cv::Mat inverted;
    cv::bitwise_not(mask, inverted);
    return inverted;
}

std::vector<std::vector<cv::Point>> maskToContours(const cv::Mat& mask) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask.clone(), contours, cv::RETR_EXTERNAL, 
                     cv::CHAIN_APPROX_SIMPLE);
    return contours;
}

void visualizeMorphOperation(const cv::Mat& original, 
                              const cv::Mat& processed,
                              const std::string& windowName) {
    // Crear visualización lado a lado
    cv::Mat comparison;
    
    if (original.channels() == 1 && processed.channels() == 1) {
        cv::hconcat(original, processed, comparison);
    } else {
        cv::Mat origGray, procGray;
        if (original.channels() == 3) {
            cv::cvtColor(original, origGray, cv::COLOR_BGR2GRAY);
        } else {
            origGray = original;
        }
        if (processed.channels() == 3) {
            cv::cvtColor(processed, procGray, cv::COLOR_BGR2GRAY);
        } else {
            procGray = processed;
        }
        cv::hconcat(origGray, procGray, comparison);
    }
    
    cv::imshow(windowName, comparison);
    cv::waitKey(0);
    cv::destroyWindow(windowName);
}

} // namespace Morphology
