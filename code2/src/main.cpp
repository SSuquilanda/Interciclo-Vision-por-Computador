#include <iostream>
#include <string>
#include <vector>
#include <filesystem> // C++17

// OpenCV
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// TU CORE MODULAR (Asegúrate de que las rutas sean correctas)
#include "core/dicom_reader.h"
#include "core/itk_opencv_bridge.h"
#include "core/preprocessing.h"
#include "core/segmentation.h"
#include "core/morphology.h"

// Función auxiliar para dibujar resultados
void drawOverlay(cv::Mat& displayImage, const std::vector<Segmentation::SegmentedRegion>& regions) {
    for (const auto& region : regions) {
        // 1. Dibujar máscara semi-transparente
        cv::Mat overlay = displayImage.clone();
        overlay.setTo(region.color, region.mask);
        cv::addWeighted(displayImage, 0.7, overlay, 0.3, 0, displayImage);

        // 2. Dibujar contornos sólidos
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(region.mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::drawContours(displayImage, contours, -1, region.color, 2);

        // 3. Etiqueta
        cv::putText(displayImage, region.label, region.centroid, 
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
        cv::putText(displayImage, region.label, region.centroid, 
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, region.color, 1);
    }
}

int main(int argc, char* argv[]) {
    // 0. VALIDACIÓN DE ARGUMENTOS
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <ruta_al_archivo_DICOM>" << std::endl;
        return -1;
    }
    std::string dicomPath = argv[1];

    try {
        std::cout << "=== INICIANDO TEST DE PIPELINES MÉDICOS ===\n" << std::endl;

        // ---------------------------------------------------------
        // FASE 1 & 2: LECTURA Y CONVERSIÓN
        // ---------------------------------------------------------
        std::cout << "[1/6] Leyendo DICOM..." << std::endl;
        auto itkImage = DicomIO::readDicomImage(dicomPath);
        cv::Mat imgRaw = Bridge::itkToOpenCV(itkImage); // 16-bit original
        cv::Mat img8bit = Bridge::normalize16to8bit(imgRaw); // 8-bit para visualización
        
        cv::imshow("1. Original Raw", img8bit);

        // ---------------------------------------------------------
        // FASE 3: PREPROCESAMIENTO (IA + Filtros)
        // ---------------------------------------------------------
        std::cout << "[2/6] Aplicando Preprocesamiento..." << std::endl;
        
        // A. Intentar cargar Red Neuronal
        std::string modelPath = "../models/dncnn_grayscale.onnx";
        cv::Mat imgPreprocessed;
        
        if (std::filesystem::exists(modelPath)) {
            std::cout << "   -> Aplicando Red Neuronal DnCNN..." << std::endl;
            imgPreprocessed = Preprocessing::applyDnCNN(img8bit, modelPath);
        } else {
            std::cerr << "   [!] Modelo no encontrado. Usando Filtro Gaussiano..." << std::endl;
            imgPreprocessed = Preprocessing::applyGaussianFilter(img8bit, 3);
        }
        
        // B. Mejora de Contraste (Opcional, ayuda a la segmentación)
        // imgPreprocessed = Preprocessing::applyCLAHE(imgPreprocessed);

        cv::imshow("2. Preprocesada (Input para Pipelines)", imgPreprocessed);

        // ---------------------------------------------------------
        // FASE 4 & 5: PIPELINES POR ÓRGANO
        // ---------------------------------------------------------
        cv::Mat visualizacionFinal = Bridge::convertToColor(img8bit);

        // --- PIPELINE A: PULMONES ---
        std::cout << "[3/6] Ejecutando Pipeline: PULMONES..." << std::endl;
        // 1. Segmentar (Threshold + Selección)
        // Nota: Pasamos la imagen de 16-bit si queremos usar umbrales HU exactos (-1000), 
        // o la preprocesada de 8-bit si ajustamos los umbrales. 
        // Tu función segmentLungs usa HU (-1000), así que necesita imgRaw (16-bit).
        // Pero para beneficiarse del DnCNN, idealmente deberíamos aplicar DnCNN a la de 16-bit o adaptar umbrales.
        // Por ahora, usemos imgRaw para respetar tu lógica de HU.
        auto lungs = Segmentation::segmentLungs(imgRaw); 
        
        // 2. Morfología Específica (Limpieza)
        std::cout << "   -> Refinando máscaras de pulmones..." << std::endl;
        for (auto& lung : lungs) {
            lung.mask = Morphology::opening(lung.mask, cv::Size(5, 5));
            lung.mask = Morphology::closing(lung.mask, cv::Size(7, 7));
            lung.mask = Morphology::fillHoles(lung.mask);
        }
        drawOverlay(visualizacionFinal, lungs);

        // --- PIPELINE B: HUESOS ---
        std::cout << "[4/6] Ejecutando Pipeline: HUESOS..." << std::endl;
        // segmentBones ya incluye su propia morfología interna y lógica de HU
        auto bones = Segmentation::segmentBones(imgRaw);
        drawOverlay(visualizacionFinal, bones);

        // --- PIPELINE C: AORTA ---
        std::cout << "[5/6] Ejecutando Pipeline: AORTA..." << std::endl;
        // La Aorta se beneficia mucho del denoising.
        // Si tu función segmentAorta usa umbrales bajos (30-120), quizás requiera imgRaw.
        // Si fue diseñada para la imagen de 8-bit limpia, usa imgPreprocessed.
        // Asumiremos imgRaw por consistencia de HU.
        auto aorta = Segmentation::segmentAorta(imgRaw);
        drawOverlay(visualizacionFinal, aorta);

        // ---------------------------------------------------------
        // FASE 6: VISUALIZACIÓN FINAL
        // ---------------------------------------------------------
        std::cout << "[6/6] Generando Visualización..." << std::endl;
        
        cv::imshow("3. Resultado Final Integrado", visualizacionFinal);
        
        std::cout << "\n=== PRUEBA COMPLETADA EXITOSAMENTE ===" << std::endl;
        std::cout << "Presiona cualquier tecla en las ventanas para salir." << std::endl;
        
        cv::waitKey(0);

    } catch (const std::exception& ex) {
        std::cerr << "\n[ERROR CRÍTICO]: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}