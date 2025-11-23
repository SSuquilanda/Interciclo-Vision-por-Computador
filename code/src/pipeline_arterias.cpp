#include <iostream>
#include <string>
#include <vector>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "f2_io/dicom_reader.h"
#include "utils/itk_opencv_bridge.h"
#include "f4_segmentation/segmentation.h"
#include "f5_morphology/morphology.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <ruta_archivo.IMA>" << std::endl;
        return -1;
    }

    std::string dicomPath = argv[1];

    try {
        std::cout << "\n╔═══════════════════════════════════════════════╗" << std::endl;
        std::cout << "║  PIPELINE: ARTERIAS PULMONARES (PULPO) 🫀   ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════════╝\n" << std::endl;

        // 1. LECTURA
        std::cout << "→ Leyendo DICOM..." << std::endl;
        DicomIO::ImagePointer itkImage = DicomIO::readDicomImage(dicomPath);
        DicomIO::displayImageStatistics(itkImage);
        
        // 2. CONVERSIÓN
        cv::Mat imageHU_16bit = Bridge::itkToOpenCV(itkImage);
        if (imageHU_16bit.empty()) {
            throw std::runtime_error("Conversión ITK->OpenCV falló");
        }
        std::cout << "  Dimensiones: " << imageHU_16bit.cols << "x" << imageHU_16bit.rows << std::endl;

        cv::Point2d imgCenter(imageHU_16bit.cols / 2.0, imageHU_16bit.rows / 2.0);

        // 3. SEGMENTACIÓN CONSERVADORA DE ARTERIAS
        std::cout << "\n→ Buscando arterias pulmonares..." << std::endl;
        
        std::vector<Segmentation::SegmentedRegion> finalArteries;
        
        // Estrategia única: contraste moderado + filtros anatómicos estrictos
        std::cout << "\n  Buscando estructuras con contraste (30-120 HU)..." << std::endl;
        Segmentation::SegmentationParams arteryParams;
        arteryParams.minHU = 30;
        arteryParams.maxHU = 120;
        arteryParams.minArea = 200;
        arteryParams.maxArea = 8000; // Limitar tamaño máximo
        arteryParams.visualColor = cv::Scalar(0, 255, 0);
        
        auto candidates = Segmentation::segmentOrgan(imageHU_16bit, arteryParams, "Arteria");
        std::cout << "     Detectadas: " << candidates.size() << " regiones candidatas" << std::endl;

        // 4. FILTROS ANATÓMICOS ESTRICTOS
        std::cout << "\n→ Aplicando filtros anatómicos..." << std::endl;
        
        std::vector<Segmentation::SegmentedRegion> filteredArteries;
        
        for (const auto& region : candidates) {
            // Calcular posición relativa
            double distX = std::abs(region.centroid.x - imgCenter.x);
            double distY = region.centroid.y - imgCenter.y;
            double distTotal = cv::norm(region.centroid - imgCenter);
            
            // Criterios anatómicos del mediastino anterior-superior
            bool esCentral = (distX < 70);        // Muy cerca del eje central
            bool esAnterior = (distY < 20);       // Parte anterior/superior de la imagen
            bool esMediano = (distTotal < 100);   // Distancia total limitada
            bool tamanioOk = (region.area >= 300 && region.area <= 5000);
            
            if (esCentral && esAnterior && esMediano && tamanioOk) {
                filteredArteries.push_back(region);
                std::cout << "  ✓ Candidato válido - Área: " << region.area 
                         << " | Distancia: " << distTotal << std::endl;
            }
        }
        
        std::cout << "  Candidatos válidos tras filtros: " << filteredArteries.size() << std::endl;

        // 5. PROCESAMIENTO MORFOLÓGICO MODERADO
        if (!filteredArteries.empty()) {
            std::cout << "\n→ Refinando morfología..." << std::endl;
            
            cv::Mat combinedMask = cv::Mat::zeros(imageHU_16bit.size(), CV_8U);
            for (const auto& r : filteredArteries) {
                cv::bitwise_or(combinedMask, r.mask, combinedMask);
            }
            
            // Morfología moderada para conectar ramas sin sobre-expandir
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
            cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);
            cv::dilate(combinedMask, combinedMask, kernel);
            
            // Extraer componente más grande y central
            auto finalComponents = Segmentation::findConnectedComponents(combinedMask, 500);
            
            if (!finalComponents.empty()) {
                // Ordenar por área (mayor primero)
                std::sort(finalComponents.begin(), finalComponents.end(),
                    [](const auto& a, const auto& b) { return a.area > b.area; });
                
                // Tomar solo el componente más grande y central
                auto& largest = finalComponents[0];
                double dist = cv::norm(largest.centroid - imgCenter);
                
                if (dist < 120.0) {
                    largest.label = "Arterias Pulmonares";
                    largest.color = cv::Scalar(0, 255, 0);
                    finalArteries.push_back(largest);
                    std::cout << "  ✓ Arteria principal detectada - Área: " << largest.area << " px²" << std::endl;
                }
            }
        }

        // 6. VISUALIZACIÓN
        std::cout << "\n→ Generando visualización..." << std::endl;
        
        cv::Mat image8bit = Bridge::normalize16to8bit(imageHU_16bit);
        cv::Mat imageColor = Bridge::convertToColor(image8bit);
        cv::Mat result = imageColor.clone();
        
        if (!finalArteries.empty()) {
            // Overlay semi-transparente
            cv::Mat overlay = imageColor.clone();
            for (const auto& artery : finalArteries) {
                overlay.setTo(cv::Scalar(0, 255, 0), artery.mask);
            }
            cv::addWeighted(imageColor, 0.6, overlay, 0.4, 0, result);
            
            // Contornos
            for (const auto& artery : finalArteries) {
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(artery.mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
                cv::drawContours(result, contours, -1, cv::Scalar(0, 255, 0), 3);
                
                std::cout << "  ✓ Arteria detectada - Área: " << artery.area << " px²" << std::endl;
            }
            
            cv::putText(result, "VERDE: Arterias Pulmonares", 
                       cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        } else {
            std::cout << "  ⚠ No se detectaron arterias pulmonares definidas" << std::endl;
            std::cout << "     Razones posibles:" << std::endl;
            std::cout << "     - Imagen sin medio de contraste IV" << std::endl;
            std::cout << "     - Fase de contraste incorrecta" << std::endl;
            std::cout << "     - Corte anatómico no muestra bifurcación" << std::endl;
            
            cv::putText(result, "SIN CONTRASTE DETECTABLE", 
                       cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
        }

        cv::imshow("Imagen Original", image8bit);
        cv::imshow("ARTERIAS PULMONARES", result);
        
        std::cout << "\n╔═══════════════════════════════════════════════╗" << std::endl;
        std::cout << "║          SEGMENTACIÓN COMPLETADA              ║" << std::endl;
        std::cout << "║  " << finalArteries.size() << " estructuras arteriales detectadas       ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════════╝\n" << std::endl;
        
        std::cout << "Presiona cualquier tecla para salir..." << std::endl;
        cv::waitKey(0);

    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERROR: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
