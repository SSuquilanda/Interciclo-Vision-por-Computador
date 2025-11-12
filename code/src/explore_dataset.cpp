#include <iostream>
#include <string>
#include <vector>

// Módulos del Proyecto
#include "f2_io/dicom_reader.h"
#include "f2_io/dataset_explorer.h"
#include "utils/itk_opencv_bridge.h"
#include "f6_visualization/visualization.h"

// OpenCV
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

// Macros para la ruta del proyecto
#define GET_STR(x) #x
#define GET_PROJECT_SOURCE_DIR(x) GET_STR(x)
std::string projectPath = GET_PROJECT_SOURCE_DIR(PROJECT_SOURCE_DIR);


void showComparisonSideBySide(const cv::Mat& fdImage, const cv::Mat& qdImage, 
                               const DatasetExplorer::DoseComparison& comparison) {
    // Normalizar ambas imágenes
    cv::Mat fd8bit = Bridge::normalize16to8bit(fdImage);
    cv::Mat qd8bit = Bridge::normalize16to8bit(qdImage);
    
    // Crear imagen combinada lado a lado
    cv::Mat combined(fd8bit.rows, fd8bit.cols * 2, CV_8UC1);
    fd8bit.copyTo(combined(cv::Rect(0, 0, fd8bit.cols, fd8bit.rows)));
    qd8bit.copyTo(combined(cv::Rect(fd8bit.cols, 0, qd8bit.cols, qd8bit.rows)));
    
    // Agregar línea divisoria
    cv::line(combined, cv::Point(fd8bit.cols, 0), 
             cv::Point(fd8bit.cols, fd8bit.rows), cv::Scalar(255), 2);
    
    // Agregar texto
    cv::putText(combined, "Full Dose", cv::Point(20, 30), 
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255), 2);
    cv::putText(combined, "Quarter Dose", cv::Point(fd8bit.cols + 20, 30), 
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255), 2);
    
    // Agregar información estadística
    std::string info = "Slice " + std::to_string(comparison.fullDose.sliceNumber);
    cv::putText(combined, info, cv::Point(20, combined.rows - 80), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255), 2);
    
    info = "PSNR: " + std::to_string(static_cast<int>(comparison.psnr)) + " dB";
    cv::putText(combined, info, cv::Point(20, combined.rows - 50), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255), 2);
    
    info = "SNR FD: " + std::to_string(static_cast<int>(comparison.fullDose.snr));
    cv::putText(combined, info, cv::Point(20, combined.rows - 20), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255), 2);
    
    info = "SNR QD: " + std::to_string(static_cast<int>(comparison.quarterDose.snr));
    cv::putText(combined, info, cv::Point(fd8bit.cols + 20, combined.rows - 20), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255), 2);
    
    cv::imshow("Comparación: Full Dose vs Quarter Dose", combined);
}


int main() {
    std::cout << "FASE 2.2: EXPLORACIÓN DEL DATASET CT\n";
    std::cout << "Full Dose vs Quarter Dose Comparison\n";
    
    // Definir rutas de las carpetas
    std::string fdFolder = projectPath + "/../data/L291_fd";
    std::string qdFolder = projectPath + "/../data/L291_qd";
    
    std::cout << "Cargando archivos DICOM...\n";
    
    // Obtener listas de archivos
    auto fdFiles = DatasetExplorer::getDicomFileList(fdFolder);
    auto qdFiles = DatasetExplorer::getDicomFileList(qdFolder);
    
    if (fdFiles.empty() || qdFiles.empty()) {
        std::cerr << "Error: No se encontraron archivos DICOM en las carpetas.\n";
        return EXIT_FAILURE;
    }
    
    if (fdFiles.size() != qdFiles.size()) {
        std::cerr << "Advertencia: Número diferente de archivos FD (" 
                  << fdFiles.size() << ") vs QD (" << qdFiles.size() << ")\n";
    }

    std::cout << "Encontrados " << fdFiles.size() << " slices Full Dose\n";
    std::cout << "Encontrados " << qdFiles.size() << " slices Quarter Dose\n\n";
    
    // Preguntar al usuario qué hacer
    std::cout << "Seleccione una opción:\n";
    std::cout << "  1. Análisis completo del dataset (puede tomar varios minutos)\n";
    std::cout << "  2. Análisis rápido (cada 10 slices)\n";
    std::cout << "  3. Visualizar slices específicos\n";
    std::cout << "Opción: ";
    
    int option;
    std::cin >> option;
    
    std::vector<DatasetExplorer::DoseComparison> comparisons;
    int step = (option == 2) ? 10 : 1;
    
    if (option == 1 || option == 2) {
        std::cout << "\nProcesando slices...\n";
        
        size_t numSlices = std::min(fdFiles.size(), qdFiles.size());
        
        for (size_t i = 0; i < numSlices; i += step) {
            // Mostrar progreso
            if (i % 50 == 0) {
                std::cout << "  Procesando slice " << (i + 1) << "/" << numSlices << "...\r" << std::flush;
            }
            
            try {
                // Leer imágenes
                auto fdImage = DicomIO::readDicomImage(fdFiles[i]);
                auto qdImage = DicomIO::readDicomImage(qdFiles[i]);
                
                // Calcular estadísticas
                auto fdInfo = DatasetExplorer::calculateSliceStats(fdImage, fdFiles[i], i + 1);
                auto qdInfo = DatasetExplorer::calculateSliceStats(qdImage, qdFiles[i], i + 1);
                
                // Comparar
                auto comparison = DatasetExplorer::compareSlices(fdImage, qdImage, fdInfo, qdInfo);
                comparisons.push_back(comparison);
                
            } catch (const std::exception& ex) {
                std::cerr << "\nError procesando slice " << (i + 1) << ": " << ex.what() << "\n";
            }
        }

        std::cout << "\nAnálisis completado!\n\n";
        // Mostrar estadísticas resumidas
        DatasetExplorer::displaySummaryStatistics(comparisons);
        
        // Identificar slices representativos
        std::cout << "Identificando slices representativos...\n";
        auto representativeIndices = DatasetExplorer::identifyRepresentativeSlices(comparisons, 5);

        std::cout << "Slices representativos identificados:\n";
        for (int idx : representativeIndices) {
            if (idx >= 0 && idx < static_cast<int>(comparisons.size())) {
                std::cout << "  - Slice " << comparisons[idx].fullDose.sliceNumber 
                          << " (PSNR: " << comparisons[idx].psnr << " dB)\n";
            }
        }
        
        // Guardar reporte
        std::string reportPath = projectPath + "/../data/dataset_comparison_report.csv";
        DatasetExplorer::saveComparisonReport(comparisons, reportPath);
        
        // Preguntar si desea visualizar slices representativos
        std::cout << "\n¿Desea visualizar los slices representativos? (s/n): ";
        char answer;
        std::cin >> answer;
        
        if (answer == 's' || answer == 'S') {
            for (int idx : representativeIndices) {
                if (idx >= 0 && idx < static_cast<int>(comparisons.size())) {
                    int fileIdx = comparisons[idx].fullDose.sliceNumber - 1;
                    
                    auto fdImage = DicomIO::readDicomImage(fdFiles[fileIdx]);
                    auto qdImage = DicomIO::readDicomImage(qdFiles[fileIdx]);
                    
                    cv::Mat fdMat = Bridge::itkToOpenCV(fdImage);
                    cv::Mat qdMat = Bridge::itkToOpenCV(qdImage);
                    
                    showComparisonSideBySide(fdMat, qdMat, comparisons[idx]);
                    
                    std::cout << "\nSlice " << comparisons[idx].fullDose.sliceNumber 
                              << " - Presiona cualquier tecla para continuar, ESC para salir...\n";
                    
                    int key = cv::waitKey(0);
                    if (key == 27) break;  // ESC
                }
            }
        }
        
    } else if (option == 3) {
        // Visualización interactiva
        std::cout << "\n--- Visualización Interactiva ---\n";
        std::cout << "Número de slices disponibles: " << fdFiles.size() << "\n";
        
        int currentSlice = fdFiles.size() / 2;  // Empezar en el medio
        bool running = true;
        
        while (running) {
            std::cout << "\nVisualizando slice " << (currentSlice + 1) << "/" << fdFiles.size() << "\n";
            
            try {
                auto fdImage = DicomIO::readDicomImage(fdFiles[currentSlice]);
                auto qdImage = DicomIO::readDicomImage(qdFiles[currentSlice]);
                
                auto fdInfo = DatasetExplorer::calculateSliceStats(fdImage, fdFiles[currentSlice], currentSlice + 1);
                auto qdInfo = DatasetExplorer::calculateSliceStats(qdImage, qdFiles[currentSlice], currentSlice + 1);
                auto comparison = DatasetExplorer::compareSlices(fdImage, qdImage, fdInfo, qdInfo);
                
                cv::Mat fdMat = Bridge::itkToOpenCV(fdImage);
                cv::Mat qdMat = Bridge::itkToOpenCV(qdImage);
                
                showComparisonSideBySide(fdMat, qdMat, comparison);
                
                std::cout << "Controles: [→] Siguiente | [←] Anterior | [ESC] Salir\n";
                
                int key = cv::waitKey(0);
                
                if (key == 27) {  // ESC
                    running = false;
                } else if (key == 83 || key == 2555904) {  // Flecha derecha
                    currentSlice = std::min(currentSlice + 1, static_cast<int>(fdFiles.size()) - 1);
                } else if (key == 81 || key == 2424832) {  // Flecha izquierda
                    currentSlice = std::max(currentSlice - 1, 0);
                }
                
            } catch (const std::exception& ex) {
                std::cerr << "Error: " << ex.what() << std::endl;
                running = false;
            }
        }
    }
    
    cv::destroyAllWindows();
    std::cout << "\nExploración del dataset completada!\n";
    
    return EXIT_SUCCESS;
}
