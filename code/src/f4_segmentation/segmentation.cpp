#include "segmentation.h"
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <algorithm>

namespace Segmentation {

// UMBRALIZACIÓN (THRESHOLDING)

cv::Mat thresholdOtsu(const cv::Mat& image) {
    cv::Mat binary;
    cv::threshold(image, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    return binary;
}

cv::Mat thresholdManual(const cv::Mat& image, double threshold, double maxValue) {
    cv::Mat binary;
    cv::threshold(image, binary, threshold, maxValue, cv::THRESH_BINARY);
    return binary;
}

cv::Mat thresholdByRange(const cv::Mat& image, double minVal, double maxVal) {
    cv::Mat mask;
    cv::inRange(image, cv::Scalar(minVal), cv::Scalar(maxVal), mask);
    return mask;
}

cv::Mat thresholdAdaptive(const cv::Mat& image, int blockSize, double C) {
    cv::Mat binary;
    
    // Asegurar que blockSize sea impar
    if (blockSize % 2 == 0) {
        blockSize++;
    }
    
    cv::adaptiveThreshold(image, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                          cv::THRESH_BINARY, blockSize, C);
    return binary;
}

// SEGMENTACIÓN AVANZADA

cv::Mat segmentWatershed(const cv::Mat& image, cv::Mat& markers) {
    // TODO: Implementar Watershed cuando se necesite
    // Requiere marcadores previos (regiones semilla)
    
    cv::Mat result = image.clone();
    
    // Aplicar watershed
    cv::watershed(result, markers);
    
    return markers;
}

cv::Mat segmentKMeans(const cv::Mat& image, int K, int attempts) {
    // TODO: Implementar K-means cuando se necesite
    
    cv::Mat samples(image.rows * image.cols, image.channels(), CV_32F);
    
    // Convertir imagen a vector de muestras
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            if (image.channels() == 1) {
                samples.at<float>(y + x * image.rows, 0) = image.at<uchar>(y, x);
            } else {
                for (int z = 0; z < image.channels(); z++) {
                    samples.at<float>(y + x * image.rows, z) = 
                        image.at<cv::Vec3b>(y, x)[z];
                }
            }
        }
    }
    
    cv::Mat labels;
    cv::Mat centers;
    cv::kmeans(samples, K, labels, 
               cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0),
               attempts, cv::KMEANS_PP_CENTERS, centers);
    
    // Reconstruir imagen segmentada
    cv::Mat segmented(image.size(), image.type());
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            int cluster_idx = labels.at<int>(y + x * image.rows, 0);
            if (image.channels() == 1) {
                segmented.at<uchar>(y, x) = centers.at<float>(cluster_idx, 0);
            } else {
                for (int z = 0; z < image.channels(); z++) {
                    segmented.at<cv::Vec3b>(y, x)[z] = centers.at<float>(cluster_idx, z);
                }
            }
        }
    }
    
    return segmented;
}

cv::Mat segmentRegionGrowing(const cv::Mat& image, 
                              const std::vector<cv::Point>& seedPoints, 
                              double threshold) {
    // TODO: Implementar Region Growing cuando se necesite
    // Algoritmo básico de crecimiento de regiones
    
    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8U);
    
    // Placeholder: marcar puntos semilla
    for (const auto& point : seedPoints) {
        if (point.x >= 0 && point.x < image.cols && 
            point.y >= 0 && point.y < image.rows) {
            mask.at<uchar>(point) = 255;
        }
    }
    
    return mask;
}

std::vector<SegmentedRegion> findConnectedComponents(const cv::Mat& binaryImage, 
                                                      int minArea) {
    std::vector<SegmentedRegion> regions;
    
    // Encontrar componentes conectados
    cv::Mat labels, stats, centroids;
    int nLabels = cv::connectedComponentsWithStats(binaryImage, labels, stats, centroids);
    
    // Procesar cada componente (saltar el fondo que es label 0)
    for (int i = 1; i < nLabels; i++) {
        double area = stats.at<int>(i, cv::CC_STAT_AREA);
        
        if (area >= minArea) {
            SegmentedRegion region;
            
            // Crear máscara para este componente
            region.mask = (labels == i);
            
            // Bounding box
            region.boundingBox = cv::Rect(
                stats.at<int>(i, cv::CC_STAT_LEFT),
                stats.at<int>(i, cv::CC_STAT_TOP),
                stats.at<int>(i, cv::CC_STAT_WIDTH),
                stats.at<int>(i, cv::CC_STAT_HEIGHT)
            );
            
            // Área
            region.area = area;
            
            // Centroide
            region.centroid = cv::Point2d(
                centroids.at<double>(i, 0),
                centroids.at<double>(i, 1)
            );
            
            regions.push_back(region);
        }
    }
    
    return regions;
}

// SEGMENTACIÓN ESPECÍFICA PARA CT (A IMPLEMENTAR SEGÚN ÓRGANO)

std::vector<SegmentedRegion> segmentLungs(const cv::Mat& image, 
                                          const SegmentationParams& params) {
    // TODO: Implementar segmentación específica de pulmones
    // Usar valores HU típicos: -1000 a -400
    
    std::vector<SegmentedRegion> lungRegions;
    
    // Placeholder: umbralización por rango HU
    cv::Mat mask = thresholdByRange(image, params.minHU, params.maxHU);
    
    // Encontrar componentes conectados
    lungRegions = findConnectedComponents(mask, params.minArea);
    
    // Asignar etiquetas y colores
    for (size_t i = 0; i < lungRegions.size() && i < 2; i++) {
        lungRegions[i].label = (i == 0) ? "Pulmón Derecho" : "Pulmón Izquierdo";
        lungRegions[i].color = params.visualColor;
    }
    
    return lungRegions;
}

SegmentedRegion segmentHeart(const cv::Mat& image, 
                              const SegmentationParams& params) {
    // TODO: Implementar segmentación específica del corazón
    // Usar valores HU típicos: 0 a 100
    
    SegmentedRegion heart;
    
    // Placeholder: umbralización por rango HU
    cv::Mat mask = thresholdByRange(image, params.minHU, params.maxHU);
    
    // Encontrar el componente más grande cerca del centro
    auto regions = findConnectedComponents(mask, params.minArea);
    
    if (!regions.empty()) {
        // Tomar la región más grande o la más cercana al centro
        heart = regions[0];
        heart.label = "Corazón";
        heart.color = params.visualColor;
    }
    
    return heart;
}

std::vector<SegmentedRegion> segmentBones(const cv::Mat& image, 
                                          const SegmentationParams& params) {
    // TODO: Implementar segmentación específica de huesos
    // Usar valores HU altos: >200
    
    std::vector<SegmentedRegion> boneRegions;
    
    // Placeholder: umbralización por rango HU
    cv::Mat mask = thresholdByRange(image, params.minHU, params.maxHU);
    
    // Encontrar componentes conectados
    boneRegions = findConnectedComponents(mask, params.minArea);
    
    // Asignar etiquetas y colores
    for (size_t i = 0; i < boneRegions.size(); i++) {
        boneRegions[i].label = "Estructura Ósea " + std::to_string(i + 1);
        boneRegions[i].color = params.visualColor;
    }
    
    return boneRegions;
}

std::vector<SegmentedRegion> segmentOrgan(const cv::Mat& image, 
                                          const SegmentationParams& params,
                                          const std::string& organName) {
    // TODO: Implementar segmentación genérica basada en parámetros
    
    std::vector<SegmentedRegion> regions;
    
    cv::Mat mask = thresholdByRange(image, params.minHU, params.maxHU);
    regions = findConnectedComponents(mask, params.minArea);
    
    // Filtrar por área máxima si está definido
    if (params.maxArea > 0) {
        regions = filterRegionsByArea(regions, params.minArea, params.maxArea);
    }
    
    // Asignar etiquetas y colores
    for (size_t i = 0; i < regions.size(); i++) {
        regions[i].label = organName + " " + std::to_string(i + 1);
        regions[i].color = params.visualColor;
    }
    
    return regions;
}

// REFINAMIENTO Y POST-PROCESAMIENTO

std::vector<SegmentedRegion> filterRegionsByArea(const std::vector<SegmentedRegion>& regions,
                                                  double minArea, double maxArea) {
    std::vector<SegmentedRegion> filtered;
    
    for (const auto& region : regions) {
        if (region.area >= minArea && (maxArea <= 0 || region.area <= maxArea)) {
            filtered.push_back(region);
        }
    }
    
    return filtered;
}

std::vector<SegmentedRegion> filterRegionsByPosition(const std::vector<SegmentedRegion>& regions,
                                                      const cv::Rect& roi) {
    std::vector<SegmentedRegion> filtered;
    
    for (const auto& region : regions) {
        // Verificar si el centroide está dentro del ROI
        if (roi.contains(region.centroid)) {
            filtered.push_back(region);
        }
    }
    
    return filtered;
}

cv::Mat combineMasks(const std::vector<cv::Mat>& masks) {
    if (masks.empty()) {
        return cv::Mat();
    }
    
    cv::Mat combined = cv::Mat::zeros(masks[0].size(), CV_8U);
    
    for (const auto& mask : masks) {
        cv::bitwise_or(combined, mask, combined);
    }
    
    return combined;
}

// VISUALIZACIÓN

cv::Mat applyColorMap(const cv::Mat& mask, int colormapType) {
    cv::Mat colored;
    cv::applyColorMap(mask, colored, colormapType);
    return colored;
}

cv::Mat overlaySegmentations(const cv::Mat& image, 
                              const std::vector<SegmentedRegion>& regions,
                              double alpha) {
    cv::Mat overlay;
    
    // Convertir a BGR si es necesario
    if (image.channels() == 1) {
        cv::cvtColor(image, overlay, cv::COLOR_GRAY2BGR);
    } else {
        overlay = image.clone();
    }
    
    // Superponer cada región
    for (const auto& region : regions) {
        cv::Mat colorMask = cv::Mat::zeros(overlay.size(), overlay.type());
        colorMask.setTo(region.color, region.mask);
        cv::addWeighted(overlay, 1.0, colorMask, alpha, 0, overlay);
    }
    
    return overlay;
}

cv::Mat drawSegmentationContours(const cv::Mat& image, 
                                  const std::vector<SegmentedRegion>& regions,
                                  int thickness) {
    cv::Mat output;
    
    // Convertir a BGR si es necesario
    if (image.channels() == 1) {
        cv::cvtColor(image, output, cv::COLOR_GRAY2BGR);
    } else {
        output = image.clone();
    }
    
    // Dibujar contornos de cada región
    for (const auto& region : regions) {
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(region.mask.clone(), contours, cv::RETR_EXTERNAL, 
                         cv::CHAIN_APPROX_SIMPLE);
        cv::drawContours(output, contours, -1, region.color, thickness);
    }
    
    return output;
}

// UTILIDADES

SegmentationParams getDefaultLungParams() {
    SegmentationParams params;
    params.minHU = -1000;           // Aire en pulmones
    params.maxHU = -400;            // Tejido pulmonar
    params.minArea = 1000;          // Área mínima en píxeles
    params.maxArea = 100000;        // Área máxima
    params.visualColor = cv::Scalar(255, 0, 0);  // Azul
    return params;
}

SegmentationParams getDefaultHeartParams() {
    SegmentationParams params;
    params.minHU = 0;               // Tejido blando
    params.maxHU = 100;             // Músculo cardíaco
    params.minArea = 2000;          // Área mínima
    params.maxArea = 50000;         // Área máxima
    params.visualColor = cv::Scalar(0, 0, 255);  // Rojo
    return params;
}

SegmentationParams getDefaultBoneParams() {
    SegmentationParams params;
    params.minHU = 200;             // Hueso trabecular
    params.maxHU = 3000;            // Hueso cortical
    params.minArea = 500;           // Área mínima
    params.maxArea = -1;            // Sin límite superior
    params.visualColor = cv::Scalar(0, 255, 0);  // Verde
    return params;
}

void saveSegmentationMasks(const std::vector<SegmentedRegion>& regions, 
                            const std::string& outputPath) {
    // TODO: Implementar guardado de máscaras
    
    for (size_t i = 0; i < regions.size(); i++) {
        std::string filename = outputPath + "_" + regions[i].label + ".png";
        cv::imwrite(filename, regions[i].mask);
    }
}

// ... (includes y funciones anteriores)

std::vector<SegmentedRegion> segmentAorta(const cv::Mat& image) {
    std::vector<SegmentedRegion> aortaRegions;
    cv::Point2d imgCenter(image.cols / 2.0, image.rows / 2.0);

    // 1. Umbralización (Rango de contraste de vasos: 30 a 120 HU)
    // Nota: Si usas la imagen preprocesada (8-bit), estos valores podrían necesitar ajuste.
    // Si usas originalRaw (16-bit), estos valores son exactos.
    cv::Mat mask = thresholdByRange(image, 30, 120);

    // 2. Segmentación inicial (Candidatos)
    SegmentationParams params;
    params.minArea = 200;
    params.maxArea = 8000;
    params.visualColor = cv::Scalar(0, 255, 0); // Verde
    
    // Reutilizamos findConnectedComponents para obtener candidatos básicos
    auto candidates = findConnectedComponents(mask, params.minArea);

    // 3. Filtros Anatómicos (La lógica de tu compañera)
    std::vector<SegmentedRegion> filteredArteries;
    for (const auto& region : candidates) {
        if (region.area > params.maxArea) continue;

        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = region.centroid.y - imgCenter.y; // + abajo, - arriba
        double distTotal = cv::norm(region.centroid - imgCenter);
        
        // Criterios: Central, Anterior (arriba del centro), y tamaño adecuado
        bool esCentral = (distX < 70);
        bool esAnterior = (distY < 20); 
        bool esMediano = (distTotal < 100);
        
        if (esCentral && esAnterior && esMediano) {
            filteredArteries.push_back(region);
        }
    }

    // 4. Procesamiento Morfológico y Selección Final
    if (!filteredArteries.empty()) {
        // Combinar candidatos en una sola máscara para morfología
        cv::Mat combinedMask = cv::Mat::zeros(image.size(), CV_8U);
        for (const auto& r : filteredArteries) {
            cv::bitwise_or(combinedMask, r.mask, combinedMask);
        }
        
        // Cierre + Dilatación (conecta fragmentos de la aorta)
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);
        cv::dilate(combinedMask, combinedMask, kernel);
        
        // Buscar el componente final más grande tras la morfología
        auto finalComponents = findConnectedComponents(combinedMask, 200);
        
        if (!finalComponents.empty()) {
            // Ordenar por área (mayor a menor)
            std::sort(finalComponents.begin(), finalComponents.end(),
                [](const auto& a, const auto& b) { return a.area > b.area; });
            
            // Tomar el más grande que siga estando centrado
            auto& largest = finalComponents[0];
            double dist = cv::norm(largest.centroid - imgCenter);
            
            if (dist < 120.0) {
                largest.label = "Aorta / Arterias";
                largest.color = cv::Scalar(0, 255, 0); // Verde
                aortaRegions.push_back(largest);
            }
        }
    }
    
    return aortaRegions;
    }

} // namespace Segmentation