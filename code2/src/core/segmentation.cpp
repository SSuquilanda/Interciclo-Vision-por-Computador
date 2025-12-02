#include "segmentation.h"
#include "morphology.h" // Asegúrate de tener este archivo en src/core/
#include <algorithm>
#include <iostream>

namespace Segmentation {

// Funciones auxiliares

cv::Mat thresholdByRange(const cv::Mat& image, double minVal, double maxVal) {
    cv::Mat mask;
    // inRange maneja rangos inclusivos.
    cv::inRange(image, cv::Scalar(minVal), cv::Scalar(maxVal), mask);
    return mask;
}

std::vector<SegmentedRegion> findConnectedComponents(const cv::Mat& binaryMask, int minArea) {
    std::vector<SegmentedRegion> regions;
    
    cv::Mat labels, stats, centroids;
    // Conectividad 8 para agrupar mejor
    int nLabels = cv::connectedComponentsWithStats(binaryMask, labels, stats, centroids, 8, CV_32S);
    
    // Iterar desde 1 (0 es el fondo)
    for (int i = 1; i < nLabels; i++) {
        double area = stats.at<int>(i, cv::CC_STAT_AREA);
        
        if (area >= minArea) {
            SegmentedRegion region;
            region.mask = (labels == i); // Crear máscara individual
            // Convertir a 255 para visualización correcta
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

// Función auxiliar privada para Pulmones
static bool touchesBorder(const cv::Mat& mask, const cv::Rect& bbox) {
    return (bbox.x <= 1 || bbox.y <= 1 || 
            (bbox.x + bbox.width) >= mask.cols - 1 || 
            (bbox.y + bbox.height) >= mask.rows - 1);
}


// Pipelines de Segmentación

// Pulmones
std::vector<SegmentedRegion> segmentLungs(const cv::Mat& image) {
    std::vector<SegmentedRegion> finalLungs;
    
    // Umbralización (Aire: -1000 a -400 HU)
    cv::Mat mask = thresholdByRange(image, -1000, -400);
    
    // Obtener candidatos (Area mínima 1000 para ignorar ruido pequeño)
    auto candidates = findConnectedComponents(mask, 1000);
    
    // Filtrado: Eliminar aire externo (que toca los bordes)
    std::vector<SegmentedRegion> validCandidates;
    for (const auto& reg : candidates) {
        if (!touchesBorder(mask, reg.boundingBox)) {
            validCandidates.push_back(reg);
        }
    }
    
    // Selección: Tomar los 2 más grandes (Pulmón Izq y Der)
    std::sort(validCandidates.begin(), validCandidates.end(), 
              [](const auto& a, const auto& b) { return a.area > b.area; });
    
    for (size_t i = 0; i < std::min(size_t(2), validCandidates.size()); ++i) {
        auto lung = validCandidates[i];
        // Etiquetar
        lung.label = (lung.centroid.x < image.cols / 2) ? "Pulmon Derecho" : "Pulmon Izquierdo";
        lung.color = cv::Scalar(255, 0, 0); // Azul (BGR)
        finalLungs.push_back(lung);
    }
    
    return finalLungs;
}

// Huesos
std::vector<SegmentedRegion> segmentBones(const cv::Mat& image) {
    std::vector<SegmentedRegion> boneRegions;
    cv::Point2d imgCenter(image.cols / 2.0, image.rows / 2.0);

    // Umbralización (Hueso: 200 a 3000 HU)
    cv::Mat mask = thresholdByRange(image, 200, 3000);

    // Componentes Conectados (Area min 80)
    auto components = findConnectedComponents(mask, 80);

    // Clasificación Anatómica
    for (auto& region : components) {
        // Morfología local para limpiar hueso
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::morphologyEx(region.mask, region.mask, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(region.mask, region.mask, cv::MORPH_CLOSE, kernel);

        // Recalcular área real tras limpieza
        region.area = cv::countNonZero(region.mask);
        if (region.area < 80) continue;

        // Métricas geométricas
        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = region.centroid.y - imgCenter.y; // + abajo, - arriba
        double distTotal = cv::norm(region.centroid - imgCenter);
        double aspectRatio = (region.boundingBox.height > 0) ? 
                             (double)region.boundingBox.width / (double)region.boundingBox.height : 0.0;

        // FILTRO: Eliminar ruido central (arteria aorta calcificada, tejido blando denso)
        // Si está muy centrado y es pequeño, probablemente es ruido, no hueso estructural
        if (distX < 50 && distTotal < 80 && region.area < 500) {
            continue; // Ignorar artefactos centrales pequeños
        }

        // Lógica de Clasificación
        if (distX < 60 && distY > 40 && region.area > 150) {
            region.label = "Columna Vertebral";
            region.color = cv::Scalar(0, 0, 255); // Rojo
            boneRegions.push_back(region);
        }
        else if (distX < 60 && distY < -20 && region.area > 200) {
            region.label = "Esternon";
            region.color = cv::Scalar(0, 100, 255); // Naranja
            boneRegions.push_back(region);
        }
        else if (distX > 60 || aspectRatio > 2.0) {
            region.label = "Costilla";
            region.color = cv::Scalar(0, 200, 255); // Amarillo
            boneRegions.push_back(region);
        }
        else if (region.area > 300) {
            if (distTotal > 60) { 
                region.label = "Hueso (Otro)";
                region.color = cv::Scalar(100, 100, 100); // Gris
                boneRegions.push_back(region);
            }
        }
    }
    return boneRegions;
}

// Aorta
std::vector<SegmentedRegion> segmentAorta(const cv::Mat& image) {
    std::vector<SegmentedRegion> aortaRegions;
    cv::Point2d imgCenter(image.cols / 2.0, image.rows / 2.0);

    // Umbralización (Vasos con contraste: 30 a 120 HU)
    cv::Mat mask = thresholdByRange(image, 30, 120);

    // Candidatos iniciales
    auto candidates = findConnectedComponents(mask, 200); // minArea 200

    // Filtros Anatómicos (Lógica de tu compañera)
    std::vector<SegmentedRegion> filteredArteries;
    for (const auto& region : candidates) {
        if (region.area > 8000) continue; // maxArea

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

    // Procesamiento Morfológico Conjunto
    if (!filteredArteries.empty()) {
        // Unir candidatos en una sola máscara
        cv::Mat combinedMask = cv::Mat::zeros(image.size(), CV_8U);
        for (const auto& r : filteredArteries) {
            cv::bitwise_or(combinedMask, r.mask, combinedMask);
        }
        
        // Cierre + Dilatación para conectar fragmentos
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);
        cv::dilate(combinedMask, combinedMask, kernel);
        
        // Buscar el componente final más grande y centrado
        auto finalComponents = findConnectedComponents(combinedMask, 200);
        
        if (!finalComponents.empty()) {
            std::sort(finalComponents.begin(), finalComponents.end(),
                [](const auto& a, const auto& b) { return a.area > b.area; });
            
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

// Umbrales personalizados

std::vector<SegmentedRegion> segmentLungsCustom(const cv::Mat& image, int minHU, int maxHU) {
    std::vector<SegmentedRegion> finalLungs;
    
    // Umbralización con rangos personalizados
    cv::Mat mask = thresholdByRange(image, minHU, maxHU);
    
    // Obtener candidatos (Area mínima 1000 para ignorar ruido pequeño)
    auto candidates = findConnectedComponents(mask, 1000);
    
    // eliminar aire externo
    std::vector<SegmentedRegion> validCandidates;
    for (const auto& reg : candidates) {
        if (!touchesBorder(mask, reg.boundingBox)) {
            validCandidates.push_back(reg);
        }
    }
    
    // tomar los dos mas grandes
    std::sort(validCandidates.begin(), validCandidates.end(), 
              [](const auto& a, const auto& b) { return a.area > b.area; });
    
    for (size_t i = 0; i < std::min(size_t(2), validCandidates.size()); ++i) {
        auto lung = validCandidates[i];
        // Etiquetar
        lung.label = (lung.centroid.x < image.cols / 2) ? "Pulmon Derecho" : "Pulmon Izquierdo";
        lung.color = cv::Scalar(255, 0, 0); // Azul (BGR)
        finalLungs.push_back(lung);
    }
    
    return finalLungs;
}

std::vector<SegmentedRegion> segmentBonesCustom(const cv::Mat& image, int minHU, int maxHU) {
    std::vector<SegmentedRegion> boneRegions;
    cv::Point2d imgCenter(image.cols / 2.0, image.rows / 2.0);

    // Umbralización con rangos personalizados
    cv::Mat mask = thresholdByRange(image, minHU, maxHU);

    // Procesamiento morfológico especial para el esternón
    // Primero obtener componentes para identificar candidatos a esternón
    auto prelimComponents = findConnectedComponents(mask, 50);
    
    // Identificar fragmentos del esternón (región anterior-superior-central)
    std::vector<cv::Mat> sternumFragments;
    cv::Mat sternumMask = cv::Mat::zeros(mask.size(), CV_8U);
    
    for (const auto& comp : prelimComponents) {
        double distX = std::abs(comp.centroid.x - imgCenter.x);
        double distY = comp.centroid.y - imgCenter.y;
        
        // Criterios para esternón: anterior (distY < -50), central (distX < 70), pequeño/mediano (área < 800)
        if (distX < 70.0 && distY < -50.0 && comp.area < 800) {
            cv::bitwise_or(sternumMask, comp.mask, sternumMask);
        }
    }
    
    // Aplicar cierre SOLO si hay fragmentos de esternón detectados
    if (cv::countNonZero(sternumMask) > 0) {
        cv::Mat kernelClose = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11));
        cv::morphologyEx(sternumMask, sternumMask, cv::MORPH_CLOSE, kernelClose);
        
        // Reemplazar región del esternón en la máscara original
        // Limpiar región anterior-superior en máscara original
        cv::Rect cleanROI(imgCenter.x - 80, 0, 160, imgCenter.y - 40);
        cleanROI = cleanROI & cv::Rect(0, 0, mask.cols, mask.rows);
        mask(cleanROI).setTo(0);
        
        // Agregar esternón procesado
        cv::bitwise_or(mask, sternumMask, mask);
    }

    // Componentes Conectados
    auto components = findConnectedComponents(mask, 80);

    // Clasificación Anatómica
    for (auto& region : components) {
        // Morfología local para limpiar hueso
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::morphologyEx(region.mask, region.mask, cv::MORPH_OPEN, kernel);
        
        // No aplicar cierre adicional al esternón ya que fue procesado antes
        bool esSternon = (std::abs(region.centroid.x - imgCenter.x) < 60.0 && 
                          region.centroid.y < imgCenter.y - 50.0);
        if (!esSternon) {
            cv::morphologyEx(region.mask, region.mask, cv::MORPH_CLOSE, kernel);
        }

        // Recalcular área real tras limpieza
        region.area = cv::countNonZero(region.mask);
        if (region.area < 80) continue;

        // Métricas geométricas
        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = region.centroid.y - imgCenter.y; // + abajo, - arriba
        double distTotal = cv::norm(region.centroid - imgCenter);

        // FILTRO MEJORADO: Eliminar ruido pequeño cerca de la columna
        // Ruido típico: fragmentos pequeños (<200px) muy cerca del centro (<50px) y cerca de la columna verticalmente
        if (distX < 50 && distTotal < 80 && region.area < 500) {
            continue; // Eliminar artefactos centrales pequeños
        }
        
        // NUEVO: Eliminar fragmentos pequeños alrededor de la región de la columna
        // Si está muy cerca del centro horizontal (<60px), abajo del centro (distY > -50), 
        // y es pequeño (<250px), probablemente es ruido de la columna
        if (distX < 60 && distY > -50 && distY < 100 && region.area < 250) {
            continue; // Eliminar ruido alrededor de la columna vertebral
        }

        // Clasificación por posición y geometría
        if (distX < 50.0 && region.area > 300) {
            region.label = "Columna";
            region.color = cv::Scalar(0, 255, 255); // Amarillo
        } else if (distX < 80.0 && distY < -50.0) {  // Relajar criterios para esternón cerrado
            region.label = "Esternon";
            region.color = cv::Scalar(0, 165, 255); // Naranja
        } else {
            region.label = "Costilla";
            region.color = cv::Scalar(0, 128, 255); // Naranja Claro
        }

        boneRegions.push_back(region);
    }

    return boneRegions;
}

std::vector<SegmentedRegion> segmentAortaCustom(const cv::Mat& image, int minHU, int maxHU) {
    std::vector<SegmentedRegion> aortaRegions;
    cv::Point2d imgCenter(image.cols / 2.0, image.rows / 2.0);

    // Umbralización con rangos personalizados
    cv::Mat mask = thresholdByRange(image, minHU, maxHU);

    // Componentes Conectados
    auto components = findConnectedComponents(mask, 30);

    // Filtrado Anatómico (arterias suelen estar centradas y en zona media-alta)
    std::vector<SegmentedRegion> filteredArteries;
    for (auto& region : components) {
        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = imgCenter.y - region.centroid.y; // + arriba, - abajo

        // Criterios: zona central (distX < 120), anterior (distY > -50), tamaño 30-2000
        if (distX < 120.0 && distY > -50.0 && region.area >= 30 && region.area <= 2000) {
            filteredArteries.push_back(region);
        }
    }

    // Procesamiento Morfológico Conjunto
    if (!filteredArteries.empty()) {
        // Unir candidatos en una sola máscara
        cv::Mat combinedMask = cv::Mat::zeros(image.size(), CV_8U);
        for (const auto& r : filteredArteries) {
            cv::bitwise_or(combinedMask, r.mask, combinedMask);
        }
        
        // Cierre + Dilatación para conectar fragmentos
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);
        cv::dilate(combinedMask, combinedMask, kernel);
        
        // Buscar el componente final más grande y centrado
        auto finalComponents = findConnectedComponents(combinedMask, 200);
        
        if (!finalComponents.empty()) {
            std::sort(finalComponents.begin(), finalComponents.end(),
                [](const auto& a, const auto& b) { return a.area > b.area; });
            
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