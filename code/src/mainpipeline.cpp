#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // Para std::sort
#include <opencv2/highgui.hpp> // Para imshow y waitKey
#include <opencv2/imgproc.hpp> // Para cvtColor

// --- Headers del proyecto ---
#include "f2_io/dicom_reader.h"
#include "utils/itk_opencv_bridge.h"
#include "f4_segmentation/segmentation.h"
#include "f5_morphology/morphology.h"
#include "f3_preprocessing/preprocessing.h"


bool compararPorArea(const Segmentation::SegmentedRegion& a, const Segmentation::SegmentedRegion& b) {
    return a.area > b.area;
}

bool tocaElBorde(const cv::Mat& mask, const cv::Rect& boundingBox) {
    // Si la caja del contorno toca x=0, y=0, o el ancho/alto máximo
    return (boundingBox.x <= 1 || boundingBox.y <= 1 || 
            (boundingBox.x + boundingBox.width) >= mask.cols - 1 || 
            (boundingBox.y + boundingBox.height) >= mask.rows - 1);
}

double distanciaAlCentro(const cv::Point2d& centroide, const cv::Size& imageSize) {
    cv::Point2d centroImagen(imageSize.width / 2.0, imageSize.height / 2.0);
    return cv::norm(centroide - centroImagen);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <ruta_al_archivo.IMA>" << std::endl;
        return -1;
    }

    std::string dicomPath = argv[1];

    try {
        // --- 1. LECTURA (Usando dicom_reader.cpp) ---
        std::cout << "=== 1. LECTURA DE IMAGEN DICOM ===" << std::endl;
        DicomIO::ImagePointer itkImage = DicomIO::readDicomImage(dicomPath);
        DicomIO::displayImageStatistics(itkImage); // Muestra Min/Max HU

        // --- 2. PUENTE ITK -> OpenCV (Usando itk_opencv_bridge.cpp) ---
        std::cout << "\n=== 2. CONVERSIÓN ITK -> OpenCV ===" << std::endl;
        cv::Mat imageHU_16bit = Bridge::itkToOpenCV(itkImage);
        
        if (imageHU_16bit.empty()) {
            throw std::runtime_error("La conversión de ITK a OpenCV falló.");
        }
        std::cout << "Conversión exitosa. Dimensiones: " << imageHU_16bit.cols << "x" << imageHU_16bit.rows << std::endl;

        // --- 3. SEGMENTACIÓN (Basada en consulta médica) ---
        std::cout << "\n=== 3. SEGMENTACIÓN POR RANGOS HU (Criterio Médico) ===" << std::endl;

        // Cargar parámetros de segmentación
        auto boneParams = Segmentation::getDefaultBoneParams();
        auto lungParams = Segmentation::getDefaultLungParams(); // Ventana de Pulmón (Aire: -1000 a -400 HU)
        auto softTissueParams = Segmentation::getDefaultHeartParams(); // Tejido Blando (0 a 100 HU)
        
        // Parámetros personalizados para arterias pulmonares (con contraste son más brillantes)
        Segmentation::SegmentationParams arteryParams;
        arteryParams.minHU = 100;  // Sangre con contraste
        arteryParams.maxHU = 300;  // Rango alto para capturar vasos con contraste
        arteryParams.minArea = 50;
        arteryParams.maxArea = 100000;
        arteryParams.visualColor = cv::Scalar(0, 255, 0); // Verde
        
        // Ejecutar segmentación
        std::cout << "Segmentando Huesos (Costillas, Columna, Esternón)..." << std::endl;
        auto boneRegions = Segmentation::segmentOrgan(imageHU_16bit, boneParams, "Hueso");
        std::cout << "  -> " << boneRegions.size() << " regiones encontradas" << std::endl;
        
        std::cout << "Segmentando Aire (Pulmones y Tráquea)..." << std::endl;
        auto airRegions = Segmentation::segmentOrgan(imageHU_16bit, lungParams, "Aire");
        std::cout << "  -> " << airRegions.size() << " regiones de aire encontradas" << std::endl;
        
        std::cout << "Segmentando Arterias Pulmonares (El Pulpo)..." << std::endl;
        auto arteryRegions = Segmentation::segmentOrgan(imageHU_16bit, arteryParams, "ArteriaPulmonar");
        std::cout << "  -> " << arteryRegions.size() << " regiones vasculares encontradas" << std::endl;
        
        std::cout << "Segmentando Tejido Blando (Corazón/Mediastino)..." << std::endl;
        auto softTissueRegions = Segmentation::segmentOrgan(imageHU_16bit, softTissueParams, "TejidoBlando");
        std::cout << "  -> " << softTissueRegions.size() << " regiones encontradas" << std::endl;

        /// --- 4. LÓGICA DE SELECCIÓN AVANZADA (Refinada) ---
        std::cout << "\n=== 4. FILTRADO Y SELECCIÓN DE REGIONES ===" << std::endl;
        
        std::vector<Segmentation::SegmentedRegion> finalLungRegions;
        std::vector<Segmentation::SegmentedRegion> tracheaRegions;
        std::vector<Segmentation::SegmentedRegion> finalArteries;
        std::vector<Segmentation::SegmentedRegion> finalBones;

        cv::Point2d imgCenter(imageHU_16bit.cols / 2.0, imageHU_16bit.rows / 2.0);

        // A) FILTRAR AIRE (Pulmones y Tráquea)
        // Los pulmones son las 2 regiones de aire más grandes que NO tocan los bordes
        std::vector<Segmentation::SegmentedRegion> candidatosPulmones;
        
        for (const auto& region : airRegions) {
            // Ignorar regiones que tocan el borde (aire exterior)
            if (tocaElBorde(region.mask, region.boundingBox)) {
                continue; 
            }
            candidatosPulmones.push_back(region);
        }
        
        // Ordenar por área (mayor a menor)
        std::sort(candidatosPulmones.begin(), candidatosPulmones.end(), compararPorArea);
        
        // Los 2 más grandes son los pulmones
        for (size_t i = 0; i < std::min(size_t(2), candidatosPulmones.size()); ++i) {
            Segmentation::SegmentedRegion lung = candidatosPulmones[i];
            lung.label = "Pulmon_" + std::to_string(i + 1);
            lung.color = cv::Scalar(255, 0, 0); // Azul para pulmones
            finalLungRegions.push_back(lung);
        }
        
        // Buscar tráquea: región pequeña y central entre los pulmones
        if (candidatosPulmones.size() > 2) {
            for (size_t i = 2; i < candidatosPulmones.size(); ++i) {
                const auto& region = candidatosPulmones[i];
                double dist = distanciaAlCentro(region.centroid, cv::Size(imageHU_16bit.cols, imageHU_16bit.rows));
                
                // Tráquea: área pequeña (100-2000) y muy central
                if (region.area >= 100 && region.area < 2000 && dist < 100.0) {
                    Segmentation::SegmentedRegion trachea = region;
                    trachea.label = "Traquea";
                    trachea.color = cv::Scalar(255, 100, 0); // Azul claro
                    tracheaRegions.push_back(trachea);
                    break;
                }
            }
        }
        
        std::cout << "  -> " << finalLungRegions.size() << " pulmones identificados" << std::endl;
        std::cout << "  -> " << tracheaRegions.size() << " tráquea identificada" << std::endl;

        // B) FILTRAR ARTERIAS PULMONARES (El Pulpo)
        // Las arterias pulmonares salen del centro (corazón) hacia los pulmones
        // Estrategia: Buscar regiones vasculares centrales y conectadas
        std::cout << "Procesando arterias pulmonares..." << std::endl;
        
        if (!arteryRegions.empty()) {
            // Crear una máscara unificada de todas las arterias detectadas
            cv::Mat arteryMask = cv::Mat::zeros(imageHU_16bit.size(), CV_8U);
            for(const auto& r : arteryRegions) {
                cv::bitwise_or(arteryMask, r.mask, arteryMask);
            }
            
            // Aplicar cierre morfológico para conectar ramas del árbol arterial
            cv::Mat kernelConnect = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9));
            cv::morphologyEx(arteryMask, arteryMask, cv::MORPH_CLOSE, kernelConnect);
            
            // Encontrar componentes conectados
            std::vector<Segmentation::SegmentedRegion> arterialTrees = 
                Segmentation::findConnectedComponents(arteryMask, 200); // Min 200 px
            
            // Seleccionar las regiones centrales (entre pulmones)
            for (const auto& tree : arterialTrees) {
                double dist = cv::norm(tree.centroid - imgCenter);
                
                // Las arterias pulmonares están en la región mediastinal central
                // Área razonable y posición central
                if (tree.area > 300 && dist < 100.0) {
                    Segmentation::SegmentedRegion artery = tree;
                    artery.label = "Arterias Pulmonares (Pulpo)";
                    artery.color = cv::Scalar(0, 255, 0); // Verde para arterias
                    finalArteries.push_back(artery);
                }
            }
            
            if (!finalArteries.empty()) {
                std::cout << "  -> Arterias pulmonares identificadas: " << finalArteries.size() << " estructuras" << std::endl;
                for (const auto& artery : finalArteries) {
                    std::cout << "     Área: " << artery.area << " px²" << std::endl;
                }
            } else {
                std::cout << "  -> No se identificaron arterias pulmonares (puede que la imagen no tenga contraste)" << std::endl;
            }
        } else {
            std::cout << "  -> No se detectaron estructuras vasculares (imagen sin contraste)" << std::endl;
        }

        // C) FILTRAR HUESOS (Costillas, Columna, Esternón)
        // Identificar las principales estructuras óseas
        std::cout << "Procesando huesos..." << std::endl;
        
        // Separar huesos por ubicación
        std::vector<Segmentation::SegmentedRegion> costas;
        std::vector<Segmentation::SegmentedRegion> columna;
        std::vector<Segmentation::SegmentedRegion> esternon;
        
        for (const auto& region : boneRegions) {
            if (region.area < 50) continue; // Ignorar fragmentos muy pequeños
            
            Segmentation::SegmentedRegion bone = region;
            bone.color = cv::Scalar(0, 0, 255); // Rojo para huesos
            
            // Clasificar por posición
            double distX = std::abs(region.centroid.x - imgCenter.x);
            double distY = std::abs(region.centroid.y - imgCenter.y);
            
            // Esternón: frontal y central
            if (distX < 60 && region.centroid.y < imgCenter.y - 50) {
                bone.label = "Esternon";
                esternon.push_back(bone);
            }
            // Columna vertebral: posterior y central
            else if (distX < 60 && region.centroid.y > imgCenter.y + 30) {
                bone.label = "Columna";
                columna.push_back(bone);
            }
            // Costillas: laterales
            else {
                bone.label = "Costilla";
                costas.push_back(bone);
            }
            
            finalBones.push_back(bone);
        }
        
        std::cout << "  -> Huesos identificados: " << finalBones.size() << " regiones" << std::endl;
        std::cout << "     · Costillas: " << costas.size() << std::endl;
        std::cout << "     · Columna: " << columna.size() << std::endl;
        std::cout << "     · Esternón: " << esternon.size() << std::endl;

        // --- 5. REFINAMIENTO (Usando morphology.cpp) ---
        std::cout << "\n=== 5. REFINAMIENTO MORFOLÓGICO ===" << std::endl;
        Morphology::MorphParams morphParams;
        morphParams.kernelSize = cv::Size(5, 5);
        morphParams.shape = Morphology::MORPH_ELLIPSE;

        // Refinar PULMONES
        std::cout << "Refinando Pulmones..." << std::endl;
        for(auto& region : finalLungRegions) {
            // Apertura para suavizar bordes y eliminar ruido
            region.mask = Morphology::opening(region.mask, cv::Size(5, 5));
            // Cierre para rellenar huecos pequeños (vasos sanguíneos)
            region.mask = Morphology::closing(region.mask, cv::Size(7, 7));
            // Rellenar huecos internos completamente
            region.mask = Morphology::fillHoles(region.mask);
        }
        
        // Refinar TRÁQUEA
        std::cout << "Refinando Tráquea..." << std::endl;
        for(auto& region : tracheaRegions) {
            region.mask = Morphology::opening(region.mask, cv::Size(3, 3));
            region.mask = Morphology::fillHoles(region.mask);
        }

        // Refinar ARTERIAS PULMONARES
        std::cout << "Refinando Arterias Pulmonares..." << std::endl;
        for(auto& region : finalArteries) {
            // Dilatación para conectar ramas
            cv::Mat kernelDilate = Morphology::createStructuringElement(Morphology::MORPH_ELLIPSE, cv::Size(7, 7));
            cv::dilate(region.mask, region.mask, kernelDilate);
            // Cierre para suavizar
            region.mask = Morphology::closing(region.mask, cv::Size(5, 5));
            // Rellenar huecos
            region.mask = Morphology::fillHoles(region.mask);
        }
        
        // Refinar HUESOS
        std::cout << "Refinando Huesos..." << std::endl;
        for(auto& region : finalBones) {
            region.mask = Morphology::opening(region.mask, cv::Size(3, 3));
        }
        
        std::cout << "Refinamiento completado." << std::endl;

        // --- 6. VISUALIZACIÓN (Usando OpenCV y bridge) ---
        std::cout << "\n=== 6. VISUALIZACIÓN ===" << std::endl;
        
        // Normalizar la imagen HU de 16 bits a 8 bits (0-255) para poder verla
        cv::Mat imageToShow_8bit = Bridge::normalize16to8bit(imageHU_16bit);
        
        // Convertir a color para dibujar contornos
        cv::Mat imageColor = Bridge::convertToColor(imageToShow_8bit);

        // Combinar todas las regiones FINALES seleccionadas para dibujar
        std::vector<Segmentation::SegmentedRegion> allRegions;
        allRegions.insert(allRegions.end(), finalLungRegions.begin(), finalLungRegions.end());
        allRegions.insert(allRegions.end(), tracheaRegions.begin(), tracheaRegions.end());
        allRegions.insert(allRegions.end(), finalArteries.begin(), finalArteries.end());
        allRegions.insert(allRegions.end(), finalBones.begin(), finalBones.end());

        std::cout << "\n=== RESUMEN DE SEGMENTACIÓN ===" << std::endl;
        std::cout << "Total de regiones visualizadas: " << allRegions.size() << std::endl;
        std::cout << "  🫁 Pulmones: " << finalLungRegions.size() << " (AZUL)" << std::endl;
        std::cout << "  🫁 Tráquea: " << tracheaRegions.size() << " (AZUL CLARO)" << std::endl;
        std::cout << "  🫀 Arterias Pulmonares: " << finalArteries.size() << " (VERDE)" << std::endl;
        std::cout << "  🦴 Huesos: " << finalBones.size() << " (ROJO)" << std::endl;

        // Dibujar los contornos finales (de segmentation.cpp)
        cv::Mat finalResult = Segmentation::drawSegmentationContours(imageColor, allRegions);

        // Mostrar el resultado
        cv::imshow("Imagen Original (Normalizada)", imageToShow_8bit);
        cv::imshow("Segmentacion: Pulmones + Huesos + Arterias", finalResult);
        
        std::cout << "\n=== PIPELINE COMPLETADO ===" << std::endl;
        std::cout << "Estructuras identificadas:" << std::endl;
        std::cout << "  · AZUL    = Pulmones" << std::endl;
        std::cout << "  · ROJO    = Huesos (costillas, columna, esternón)" << std::endl;
        std::cout << "  · VERDE   = Arterias Pulmonares (el 'pulpo')" << std::endl;
        std::cout << "\nPresiona cualquier tecla en las ventanas para salir." << std::endl;
        cv::waitKey(0); // Espera a que el usuario presione una tecla

    } catch (const std::exception& e) {
        std::cerr << "\n=== ERROR FATAL ===" << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}