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
        std::cout << "\n╔═══════════════════════════════════════════╗" << std::endl;
        std::cout << "║   PIPELINE ESPECIALIZADO: HUESOS 🦴      ║" << std::endl;
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

        cv::Point2d imgCenter(imageHU_16bit.cols / 2.0, imageHU_16bit.rows / 2.0);

        // 3. SEGMENTACIÓN DE HUESOS
        std::cout << "\n→ Segmentando estructuras óseas..." << std::endl;
        
        // Parámetros para hueso cortical y esponjoso
        Segmentation::SegmentationParams boneParams;
        boneParams.minHU = 200;    // Hueso denso
        boneParams.maxHU = 3000;   // Rango completo de hueso
        boneParams.minArea = 80;   // Aumentado para reducir fragmentos pequeños
        boneParams.maxArea = 100000;
        boneParams.visualColor = cv::Scalar(0, 0, 255); // Rojo
        
        auto boneRegions = Segmentation::segmentOrgan(imageHU_16bit, boneParams, "Hueso");
        std::cout << "  Total regiones óseas detectadas: " << boneRegions.size() << std::endl;

        // 4. CLASIFICACIÓN DE HUESOS CON CRITERIOS MEJORADOS
        std::cout << "\n→ Clasificando estructuras óseas..." << std::endl;
        
        std::vector<Segmentation::SegmentedRegion> costillas;
        std::vector<Segmentation::SegmentedRegion> columna;
        std::vector<Segmentation::SegmentedRegion> esternon;
        std::vector<Segmentation::SegmentedRegion> otros;
        
        for (const auto& region : boneRegions) {
            if (region.area < 80) continue;
            
            Segmentation::SegmentedRegion bone = region;
            double distX = std::abs(region.centroid.x - imgCenter.x);
            double distY = region.centroid.y - imgCenter.y;
            double distTotal = cv::norm(region.centroid - imgCenter); // CORRECCIÓN: Calculamos distancia total al centro
            
            // Calcular aspect ratio (relación ancho/alto) del bounding box
            double width = region.boundingBox.width;
            double height = region.boundingBox.height;
            double aspectRatio = (height > 0) ? (width / height) : 1.0;
            
            // COLUMNA: Posterior (parte inferior) y muy central horizontalmente
            if (distX < 60 && distY > 40 && region.area > 150) {
                bone.label = "Columna Vertebral";
                bone.color = cv::Scalar(0, 0, 255); // Rojo
                columna.push_back(bone);
            }
            // ESTERNÓN: Anterior (parte superior) y central
            else if (distX < 60 && distY < -20 && region.area > 200) {
                bone.label = "Esternon";
                bone.color = cv::Scalar(0, 100, 255); // Rojo claro (Naranja)
                esternon.push_back(bone);
            }
            // COSTILLAS: Laterales y generalmente alargadas
            else if (distX > 60 || aspectRatio > 2.0) {
                bone.label = "Costilla";
                bone.color = cv::Scalar(0, 200, 255); // Amarillo/Naranja
                costillas.push_back(bone);
            }
            // OTROS: Fragmentos no clasificados
            // CORRECCIÓN: Aumentamos área mínima y FILTRAMOS EL CENTRO
            else if (region.area > 300) { 
                // Si está muy cerca del centro (distTotal < 60), es probable que sea calcificación de aorta/corazón.
                // Solo aceptamos "otros huesos" si están lejos del centro.
                if (distTotal > 60) {
                    bone.label = "Hueso (Otro)";
                    bone.color = cv::Scalar(100, 100, 100); // CORRECCIÓN: Gris (para diferenciar del rojo)
                    otros.push_back(bone);
                }
            }
        }
        
        std::cout << "  ✓ Costillas: " << costillas.size() << std::endl;
        std::cout << "  ✓ Columna vertebral: " << columna.size() << " segmentos" << std::endl;
        std::cout << "  ✓ Esternón: " << esternon.size() << std::endl;
        std::cout << "  ✓ Otros: " << otros.size() << std::endl;

        // 5. REFINAMIENTO
        std::cout << "\n→ Refinando máscaras..." << std::endl;
        
        auto refinar = [](std::vector<Segmentation::SegmentedRegion>& bones) {
            for (auto& bone : bones) {
                // Apertura suave para eliminar ruido
                bone.mask = Morphology::opening(bone.mask, cv::Size(3, 3));
                // Cierre para conectar fragmentos
                bone.mask = Morphology::closing(bone.mask, cv::Size(3, 3));
            }
        };
        
        refinar(costillas);
        refinar(columna);
        refinar(esternon);
        refinar(otros);

        // 6. VISUALIZACIÓN CON OVERLAY RELLENO
        std::cout << "\n→ Generando visualización..." << std::endl;
        
        cv::Mat image8bit = Bridge::normalize16to8bit(imageHU_16bit);
        cv::Mat imageColor = Bridge::convertToColor(image8bit);
        
        // Combinar todas las regiones
        std::vector<Segmentation::SegmentedRegion> allBones;
        allBones.insert(allBones.end(), columna.begin(), columna.end());
        allBones.insert(allBones.end(), esternon.begin(), esternon.end());
        allBones.insert(allBones.end(), costillas.begin(), costillas.end());
        allBones.insert(allBones.end(), otros.begin(), otros.end());
        
        // Crear overlay con máscaras RELLENAS
        cv::Mat result = imageColor.clone();
        cv::Mat overlay = result.clone();
        
        // Rellenar cada región con su color
        for (const auto& bone : allBones) {
            overlay.setTo(bone.color, bone.mask);
        }
        
        // Combinar: 70% imagen + 30% overlay coloreado
        cv::addWeighted(result, 0.7, overlay, 0.3, 0, result);
        
        // Dibujar contornos GRUESOS encima para definición
        for (const auto& bone : allBones) {
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(bone.mask.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::drawContours(result, contours, -1, bone.color, 2);
        }
        
        // Leyenda
        int y = 30;
        cv::putText(result, "ROJO: Columna", cv::Point(10, y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
        y += 25;
        cv::putText(result, "NARANJA: Costillas", cv::Point(10, y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 200, 255), 2);
        y += 25;
        cv::putText(result, "ROJO CLARO: Esternon", cv::Point(10, y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 100, 255), 2);
        y += 25;
        cv::putText(result, "GRIS: Otros", cv::Point(10, y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(100, 100, 100), 2); // CORRECCIÓN

        cv::imshow("Imagen Original", image8bit);
        cv::imshow("HUESOS Segmentados", result);
        
        std::cout << "\n╔═══════════════════════════════════════════╗" << std::endl;
        std::cout << "║          SEGMENTACIÓN COMPLETADA          ║" << std::endl;
        std::cout << "║  " << allBones.size() << " estructuras óseas identificadas     ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════╝\n" << std::endl;
        
        std::cout << "Presiona cualquier tecla para salir..." << std::endl;
        cv::waitKey(0);

    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERROR: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}