#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "f2_io/dicom_reader.h"
#include "utils/itk_opencv_bridge.h"
#include "f4_segmentation/segmentation.h"
#include "f5_morphology/morphology.h"

bool tocaElBorde(const cv::Mat& mask, const cv::Rect& boundingBox) {
    return (boundingBox.x <= 1 || boundingBox.y <= 1 || 
            (boundingBox.x + boundingBox.width) >= mask.cols - 1 || 
            (boundingBox.y + boundingBox.height) >= mask.rows - 1);
}

bool compararPorArea(const Segmentation::SegmentedRegion& a, const Segmentation::SegmentedRegion& b) {
    return a.area > b.area;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <ruta_archivo.IMA>" << std::endl;
        return -1;
    }

    std::string dicomPath = argv[1];

    try {
        std::cout << "\n╔═══════════════════════════════════════════╗" << std::endl;
        std::cout << "║   PIPELINE ESPECIALIZADO: PULMONES 🫁    ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════╝\n" << std::endl;

        // 1. LECTURA
        std::cout << "→ Leyendo DICOM..." << std::endl;
        DicomIO::ImagePointer itkImage = DicomIO::readDicomImage(dicomPath);
        
        // 2. CONVERSIÓN
        cv::Mat imageHU_16bit = Bridge::itkToOpenCV(itkImage);
        if (imageHU_16bit.empty()) {
            throw std::runtime_error("Conversión ITK->OpenCV falló");
        }
        std::cout << "  Dimensiones: " << imageHU_16bit.cols << "x" << imageHU_16bit.rows << std::endl;

        // 3. SEGMENTACIÓN DE AIRE (PULMONES)
        std::cout << "\n→ Segmentando aire (pulmones)..." << std::endl;
        
        // Parámetros optimizados para pulmones
        Segmentation::SegmentationParams lungParams;
        lungParams.minHU = -1000;  // Aire máximo
        lungParams.maxHU = -400;   // Hasta tejido pulmonar
        lungParams.minArea = 1000; // Filtrar ruido pequeño
        lungParams.maxArea = 200000;
        lungParams.visualColor = cv::Scalar(255, 0, 0); // Azul
        
        auto airRegions = Segmentation::segmentOrgan(imageHU_16bit, lungParams, "Aire");
        std::cout << "  Total regiones de aire detectadas: " << airRegions.size() << std::endl;

        // 4. FILTRADO INTELIGENTE
        std::cout << "\n→ Filtrando pulmones..." << std::endl;
        std::vector<Segmentation::SegmentedRegion> finalLungs;
        
        // Eliminar aire exterior (toca bordes)
        std::vector<Segmentation::SegmentedRegion> candidatos;
        for (const auto& region : airRegions) {
            if (!tocaElBorde(region.mask, region.boundingBox) && region.area > 5000) {
                candidatos.push_back(region);
            }
        }
        
        // Ordenar por área (mayor a menor)
        std::sort(candidatos.begin(), candidatos.end(), compararPorArea);
        
        // Los 2 más grandes son los pulmones
        for (size_t i = 0; i < std::min(size_t(2), candidatos.size()); ++i) {
            Segmentation::SegmentedRegion lung = candidatos[i];
            lung.label = (i == 0) ? "Pulmon Derecho" : "Pulmon Izquierdo";
            lung.color = cv::Scalar(255, 0, 0); // Azul
            finalLungs.push_back(lung);
            std::cout << "  ✓ " << lung.label << " - Área: " << lung.area << " px²" << std::endl;
        }

        // 5. REFINAMIENTO MORFOLÓGICO
        std::cout << "\n→ Refinando máscaras..." << std::endl;
        for (auto& lung : finalLungs) {
            // Apertura: eliminar pequeños puntos blancos
            lung.mask = Morphology::opening(lung.mask, cv::Size(5, 5));
            
            // Cierre: rellenar vasos sanguíneos y bronquios
            lung.mask = Morphology::closing(lung.mask, cv::Size(9, 9));
            
            // Rellenar todos los huecos internos
            lung.mask = Morphology::fillHoles(lung.mask);
            
            std::cout << "  ✓ Refinado completado" << std::endl;
        }

        // 6. VISUALIZACIÓN
        std::cout << "\n→ Generando visualización..." << std::endl;
        
        cv::Mat image8bit = Bridge::normalize16to8bit(imageHU_16bit);
        cv::Mat imageColor = Bridge::convertToColor(image8bit);
        
        // Dibujar contornos gruesos
        cv::Mat result = Segmentation::drawSegmentationContours(imageColor, finalLungs, 3);
        
        // Crear overlay semi-transparente
        cv::Mat overlay = imageColor.clone();
        for (const auto& lung : finalLungs) {
            // Rellenar con color semi-transparente
            overlay.setTo(lung.color, lung.mask);
        }
        cv::addWeighted(imageColor, 0.7, overlay, 0.3, 0, result);
        
        // Dibujar contornos encima
        for (const auto& lung : finalLungs) {
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(lung.mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::drawContours(result, contours, -1, lung.color, 3);
            
            // Etiqueta
            cv::putText(result, lung.label, 
                       cv::Point(lung.boundingBox.x, lung.boundingBox.y - 10),
                       cv::FONT_HERSHEY_SIMPLEX, 0.8, lung.color, 2);
        }

        // Mostrar
        cv::imshow("Imagen Original", image8bit);
        cv::imshow("PULMONES Segmentados", result);
        
        std::cout << "\n╔═══════════════════════════════════════════╗" << std::endl;
        std::cout << "║          SEGMENTACIÓN COMPLETADA          ║" << std::endl;
        std::cout << "║  " << finalLungs.size() << " pulmones identificados                ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════╝\n" << std::endl;
        
        std::cout << "Presiona cualquier tecla para salir..." << std::endl;
        cv::waitKey(0);

    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERROR: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
