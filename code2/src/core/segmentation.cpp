#include "segmentation.h"
#include "morphology.h"
#include <algorithm>
#include <iostream>

namespace Segmentation {

// Funciones auxiliares

cv::Mat thresholdByRange(const cv::Mat& image, double minVal, double maxVal) {
    cv::Mat mask;
    cv::inRange(image, cv::Scalar(minVal), cv::Scalar(maxVal), mask);
    return mask;
}

std::vector<SegmentedRegion> findConnectedComponents(const cv::Mat& binaryMask, int minArea) {
    std::vector<SegmentedRegion> regions;
    
    cv::Mat labels, stats, centroids;
    int nLabels = cv::connectedComponentsWithStats(binaryMask, labels, stats, centroids, 8, CV_32S);
    
    // empezar en 1 porque 0 es el fondo
    for (int i = 1; i < nLabels; i++) {
        double area = stats.at<int>(i, cv::CC_STAT_AREA);
        
        if (area >= minArea) {
            SegmentedRegion region;
            region.mask = (labels == i);
            region.mask.setTo(255, region.mask); 
            
            region.area = area;
            region.boundingBox = cv::Rect(
                stats.at<int>(i, cv::CC_STAT_LEFT),
                stats.at<int>(i, cv::CC_STAT_TOP),
                stats.at<int>(i, cv::CC_STAT_WIDTH),
                stats.at<int>(i, cv::CC_STAT_HEIGHT)
            );
            region.centroid = cv::Point2d(
                centroids.at<double>(i, 0),
                centroids.at<double>(i, 1)
            );
            
            regions.push_back(region);
        }
    }
    return regions;
}

static bool touchesBorder(const cv::Mat& mask, const cv::Rect& bbox) {
    return (bbox.x <= 1 || bbox.y <= 1 || 
            (bbox.x + bbox.width) >= mask.cols - 1 || 
            (bbox.y + bbox.height) >= mask.rows - 1);
}


// Segmentación de pulmones

std::vector<SegmentedRegion> segmentLungs(const cv::Mat& image) {
    std::vector<SegmentedRegion> finalLungs;
    
    // rango de aire: -1000 a -400 HU
    cv::Mat mask = thresholdByRange(image, -1000, -400);
    
    auto candidates = findConnectedComponents(mask, 1000);
    
    // quitar el aire externo que toca bordes
    std::vector<SegmentedRegion> validCandidates;
    for (const auto& reg : candidates) {
        if (!touchesBorder(mask, reg.boundingBox)) {
            validCandidates.push_back(reg);
        }
    }
    
    // tomar los 2 más grandes
    std::sort(validCandidates.begin(), validCandidates.end(), 
              [](const auto& a, const auto& b) { return a.area > b.area; });
    
    for (size_t i = 0; i < std::min(size_t(2), validCandidates.size()); ++i) {
        auto lung = validCandidates[i];
        lung.label = (lung.centroid.x < image.cols / 2) ? "Pulmon Derecho" : "Pulmon Izquierdo";
        lung.color = cv::Scalar(255, 0, 0);
        finalLungs.push_back(lung);
    }
    
    return finalLungs;
}

// Segmentación de huesos

std::vector<SegmentedRegion> segmentBones(const cv::Mat& image) {
    std::vector<SegmentedRegion> boneRegions;
    cv::Point2d imgCenter(image.cols / 2.0, image.rows / 2.0);

    // rango de hueso: 200 a 3000 HU
    cv::Mat mask = thresholdByRange(image, 200, 3000);

    auto components = findConnectedComponents(mask, 80);

    for (auto& region : components) {
        // limpiar ruido
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::morphologyEx(region.mask, region.mask, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(region.mask, region.mask, cv::MORPH_CLOSE, kernel);

        region.area = cv::countNonZero(region.mask);
        if (region.area < 80) continue;

        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = region.centroid.y - imgCenter.y;
        double distTotal = cv::norm(region.centroid - imgCenter);
        double aspectRatio = (region.boundingBox.height > 0) ? 
                             (double)region.boundingBox.width / (double)region.boundingBox.height : 0.0;

        // ignorar artefactos pequeños en el centro
        if (distX < 50 && distTotal < 80 && region.area < 500) {
            continue;
        }

        // clasificar por ubicación
        if (distX < 60 && distY > 40 && region.area > 150) {
            region.label = "Columna Vertebral";
            region.color = cv::Scalar(0, 0, 255);
            boneRegions.push_back(region);
        }
        else if (distX < 60 && distY < -20 && region.area > 200) {
            region.label = "Esternon";
            region.color = cv::Scalar(0, 100, 255);
            boneRegions.push_back(region);
        }
        else if (distX > 60 || aspectRatio > 2.0) {
            region.label = "Costilla";
            region.color = cv::Scalar(0, 200, 255);
            boneRegions.push_back(region);
        }
        else if (region.area > 300) {
            if (distTotal > 60) { 
                region.label = "Hueso (Otro)";
                region.color = cv::Scalar(100, 100, 100);
                boneRegions.push_back(region);
            }
        }
    }
    return boneRegions;
}

// Segmentación de aorta

std::vector<SegmentedRegion> segmentAorta(const cv::Mat& image) {
    std::vector<SegmentedRegion> aortaRegions;
    cv::Point2d imgCenter(image.cols / 2.0, image.rows / 2.0);

    // rango de vasos con contraste: 30 a 120 HU
    cv::Mat mask = thresholdByRange(image, 30, 120);

    auto candidates = findConnectedComponents(mask, 200);

    // filtrar por ubicación
    std::vector<SegmentedRegion> filteredArteries;
    for (const auto& region : candidates) {
        if (region.area > 8000) continue;

        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = region.centroid.y - imgCenter.y;
        double distTotal = cv::norm(region.centroid - imgCenter);
        
        bool esCentral = (distX < 70);
        bool esAnterior = (distY < 20);
        bool esMediano = (distTotal < 100);
        
        if (esCentral && esAnterior && esMediano) {
            filteredArteries.push_back(region);
        }
    }

    // juntar fragmentos
    if (!filteredArteries.empty()) {
        cv::Mat combinedMask = cv::Mat::zeros(image.size(), CV_8U);
        for (const auto& r : filteredArteries) {
            cv::bitwise_or(combinedMask, r.mask, combinedMask);
        }
        
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);
        cv::dilate(combinedMask, combinedMask, kernel);
        
        auto finalComponents = findConnectedComponents(combinedMask, 200);
        
        if (!finalComponents.empty()) {
            std::sort(finalComponents.begin(), finalComponents.end(),
                [](const auto& a, const auto& b) { return a.area > b.area; });
            
            auto& largest = finalComponents[0];
            double dist = cv::norm(largest.centroid - imgCenter);
            
            if (dist < 120.0) {
                largest.label = "Aorta / Arterias";
                largest.color = cv::Scalar(0, 255, 0);
                aortaRegions.push_back(largest);
            }
        }
    }
    
    return aortaRegions;
}

// Versiones con umbrales personalizados

std::vector<SegmentedRegion> segmentLungsCustom(const cv::Mat& image, int minHU, int maxHU) {
    std::vector<SegmentedRegion> finalLungs;
    
    cv::Mat mask = thresholdByRange(image, minHU, maxHU);
    
    auto candidates = findConnectedComponents(mask, 1000);
    
    std::vector<SegmentedRegion> validCandidates;
    for (const auto& reg : candidates) {
        if (!touchesBorder(mask, reg.boundingBox)) {
            validCandidates.push_back(reg);
        }
    }
    
    std::sort(validCandidates.begin(), validCandidates.end(), 
              [](const auto& a, const auto& b) { return a.area > b.area; });
    
    for (size_t i = 0; i < std::min(size_t(2), validCandidates.size()); ++i) {
        auto lung = validCandidates[i];
        lung.label = (lung.centroid.x < image.cols / 2) ? "Pulmon Derecho" : "Pulmon Izquierdo";
        lung.color = cv::Scalar(255, 0, 0);
        finalLungs.push_back(lung);
    }
    
    return finalLungs;
}

std::vector<SegmentedRegion> segmentBonesCustom(const cv::Mat& image, int minHU, int maxHU) {
    std::vector<SegmentedRegion> boneRegions;
    cv::Point2d imgCenter(image.cols / 2.0, image.rows / 2.0);

    cv::Mat mask = thresholdByRange(image, minHU, maxHU);

    // procesar el esternón primero
    auto prelimComponents = findConnectedComponents(mask, 50);
    
    std::vector<cv::Mat> sternumFragments;
    cv::Mat sternumMask = cv::Mat::zeros(mask.size(), CV_8U);
    
    for (const auto& comp : prelimComponents) {
        double distX = std::abs(comp.centroid.x - imgCenter.x);
        double distY = comp.centroid.y - imgCenter.y;
        
        // buscar fragmentos del esternón
        if (distX < 70.0 && distY < -50.0 && comp.area < 800) {
            cv::bitwise_or(sternumMask, comp.mask, sternumMask);
        }
    }
    
    // juntar fragmentos del esternón si hay
    if (cv::countNonZero(sternumMask) > 0) {
        cv::Mat kernelClose = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11));
        cv::morphologyEx(sternumMask, sternumMask, cv::MORPH_CLOSE, kernelClose);
        
        // limpiar zona anterior y agregar esternón procesado
        cv::Rect cleanROI(imgCenter.x - 80, 0, 160, imgCenter.y - 40);
        cleanROI = cleanROI & cv::Rect(0, 0, mask.cols, mask.rows);
        mask(cleanROI).setTo(0);
        
        cv::bitwise_or(mask, sternumMask, mask);
    }

    auto components = findConnectedComponents(mask, 80);

    for (auto& region : components) {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::morphologyEx(region.mask, region.mask, cv::MORPH_OPEN, kernel);
        
        // no cerrar el esternón otra vez
        bool esSternon = (std::abs(region.centroid.x - imgCenter.x) < 60.0 && 
                          region.centroid.y < imgCenter.y - 50.0);
        if (!esSternon) {
            cv::morphologyEx(region.mask, region.mask, cv::MORPH_CLOSE, kernel);
        }

        region.area = cv::countNonZero(region.mask);
        if (region.area < 80) continue;

        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = region.centroid.y - imgCenter.y;
        double distTotal = cv::norm(region.centroid - imgCenter);

        // quitar ruido cerca del centro
        if (distX < 50 && distTotal < 80 && region.area < 500) {
            continue;
        }
        
        // quitar fragmentos pequeños alrededor de la columna
        if (distX < 60 && distY > -50 && distY < 100 && region.area < 250) {
            continue;
        }

        // clasificar
        if (distX < 50.0 && region.area > 300) {
            region.label = "Columna";
            region.color = cv::Scalar(0, 255, 255);
        } else if (distX < 80.0 && distY < -50.0) {
            region.label = "Esternon";
            region.color = cv::Scalar(0, 165, 255);
        } else {
            region.label = "Costilla";
            region.color = cv::Scalar(0, 128, 255);
        }

        boneRegions.push_back(region);
    }

    return boneRegions;
}

std::vector<SegmentedRegion> segmentAortaCustom(const cv::Mat& image, int minHU, int maxHU) {
    std::vector<SegmentedRegion> aortaRegions;
    cv::Point2d imgCenter(image.cols / 2.0, image.rows / 2.0);

    cv::Mat mask = thresholdByRange(image, minHU, maxHU);

    auto components = findConnectedComponents(mask, 30);

    // filtrar por zona anatómica
    std::vector<SegmentedRegion> filteredArteries;
    for (auto& region : components) {
        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = imgCenter.y - region.centroid.y;

        if (distX < 120.0 && distY > -50.0 && region.area >= 30 && region.area <= 2000) {
            filteredArteries.push_back(region);
        }
    }

    // juntar candidatos
    if (!filteredArteries.empty()) {
        cv::Mat combinedMask = cv::Mat::zeros(image.size(), CV_8U);
        for (const auto& r : filteredArteries) {
            cv::bitwise_or(combinedMask, r.mask, combinedMask);
        }
        
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);
        cv::dilate(combinedMask, combinedMask, kernel);
        
        auto finalComponents = findConnectedComponents(combinedMask, 200);
        
        if (!finalComponents.empty()) {
            std::sort(finalComponents.begin(), finalComponents.end(),
                [](const auto& a, const auto& b) { return a.area > b.area; });
            
            auto& largest = finalComponents[0];
            double dist = cv::norm(largest.centroid - imgCenter);
            
            if (dist < 120.0) {
                largest.label = "Aorta / Arterias";
                largest.color = cv::Scalar(0, 255, 0);
                aortaRegions.push_back(largest);
            }
        }
    }
    
    return aortaRegions;
}

} // namespace Segmentation
