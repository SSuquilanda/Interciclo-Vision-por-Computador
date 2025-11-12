#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <iomanip>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

// Modulos del Proyecto
#include "f2_io/dicom_reader.h"
#include "utils/itk_opencv_bridge.h"
#include "f6_visualization/visualization.h"

#define GET_STR(x) #x
#define GET_PROJECT_SOURCE_DIR(x) GET_STR(x)
std::string projectPath = GET_PROJECT_SOURCE_DIR(PROJECT_SOURCE_DIR);

namespace fs = std::filesystem;

// Estructura para almacenar información de slice
struct SliceInfo {
    std::string filePath;
    int sliceNumber;
    cv::Mat image;
};

// Función para extraer número de slice del nombre de archivo
int extractSliceNumber(const std::string& filename) {
    size_t pos = filename.find(".CT.");
    if (pos != std::string::npos) {
        size_t start = pos + 4; 
        size_t end = filename.find(".", start);
        if (end != std::string::npos) {
            std::string numStr = filename.substr(start, end - start);
            try {
                return std::stoi(numStr);
            } catch (...) {
                return -1;
            }
        }
    }
    return -1;
}

// Función para obtener todos los archivos DICOM de un directorio
std::vector<SliceInfo> getDicomFiles(const std::string& directory) {
    std::vector<SliceInfo> slices;
    
    std::cout << "\nEscaneando directorio: " << directory << std::endl;
    
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find(".IMA") != std::string::npos) {
                SliceInfo info;
                info.filePath = entry.path().string();
                info.sliceNumber = extractSliceNumber(filename);
                slices.push_back(info);
            }
        }
    }
    
    // Ordenar por número de slice
    std::sort(slices.begin(), slices.end(), 
              [](const SliceInfo& a, const SliceInfo& b) {
                  return a.sliceNumber < b.sliceNumber;
              });
    
    std::cout << "Encontrados " << slices.size() << " slices DICOM" << std::endl;
    return slices;
}

// Función para exportar slice como PNG
bool exportSlice(const cv::Mat& image, const std::string& outputPath, int sliceNumber) {
    std::ostringstream filename;
    filename << outputPath << "/slice_" << std::setfill('0') << std::setw(4) << sliceNumber << ".png";
    
    return cv::imwrite(filename.str(), image);
}

// Función para seleccionar slices representativos
std::vector<int> selectRepresentativeSlices(int totalSlices, int numToShow = 5) {
    std::vector<int> indices;
    
    if (totalSlices <= numToShow) {
        // Si hay pocos slices, mostrar todos
        for (int i = 0; i < totalSlices; i++) {
            indices.push_back(i);
        }
    } else {
        // Distribuir uniformemente
        indices.push_back(0);  // Primero
        indices.push_back(totalSlices / 4);  // 25%
        indices.push_back(totalSlices / 2);  // 50%
        indices.push_back(3 * totalSlices / 4);  // 75%
        indices.push_back(totalSlices - 1);  // Último
    }
    
    return indices;
}

int main(int argc, char* argv[]) {
    std::cout << "EXPORTADOR DE SLICES DICOM\n";

    // Configuración de rutas
    std::string inputDir = projectPath + "/../data/L291_qd/";
    std::string outputDir = projectPath + "/../data/exported_slices/";
    
    // Permitir especificar directorio por línea de comandos
    if (argc >= 2) {
        std::string arg = argv[1];
        if (arg == "fd" || arg == "FD") {
            inputDir = projectPath + "/../data/L291_fd/";
            outputDir = projectPath + "/../data/exported_slices_fd/";
            std::cout << "Modo: Full Dose (FD)" << std::endl;
        } else if (arg == "qd" || arg == "QD") {
            std::cout << "Modo: Quarter Dose (QD)" << std::endl;
        }
    } else {
        std::cout << "Modo por defecto: Quarter Dose (QD)" << std::endl;
        std::cout << "Uso: ./MyApp [fd|qd] para cambiar de dataset" << std::endl;
    }

    try {
        // Crear directorio de salida si no existe
        if (!fs::exists(outputDir)) {
            fs::create_directories(outputDir);
            std::cout << "Directorio de salida creado: " << outputDir << std::endl;
        }
        
        // Obtener lista de archivos DICOM
        std::vector<SliceInfo> slices = getDicomFiles(inputDir);
        
        if (slices.empty()) {
            std::cerr << "No se encontraron archivos DICOM en: " << inputDir << std::endl;
            return EXIT_FAILURE;
        }
        
        // Procesar y exportar todos los slices
        std::cout << "\nProcesando y exportando slices..." << std::endl;
        
        int progressInterval = std::max(1, static_cast<int>(slices.size()) / 20);
        
        for (size_t i = 0; i < slices.size(); i++) {
            try {
                // Leer imagen DICOM
                DicomIO::ImagePointer itkImage = DicomIO::readDicomImage(slices[i].filePath);
                
                // Convertir a OpenCV
                cv::Mat cvImage = Bridge::itkToOpenCV(itkImage);
                
                // Normalizar a 8-bit para exportación
                cv::Mat cvImage8bit = Bridge::normalize16to8bit(cvImage);
                
                // Almacenar para visualización posterior
                slices[i].image = cvImage8bit.clone();
                
                // Exportar como PNG con número secuencial
                if (!exportSlice(cvImage8bit, outputDir, static_cast<int>(i + 1))) {
                    std::cerr << "Error exportando slice " << (i + 1) << std::endl;
                }
                
                // Mostrar progreso
                if (i % progressInterval == 0 || i == slices.size() - 1) {
                    int percent = static_cast<int>((i + 1) * 100 / slices.size());
                    std::cout << "  Progreso: " << std::setw(3) << percent << "% "
                              << "(" << (i + 1) << "/" << slices.size() << ") "
                              << "Original DICOM: " << fs::path(slices[i].filePath).filename().string() << std::endl;
                }
                
            } catch (const std::exception& ex) {
                std::cerr << "Error procesando archivo: " 
                          << fs::path(slices[i].filePath).filename().string() 
                          << " - " << ex.what() << std::endl;
            }
        }
        
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout << "Exportación completada!" << std::endl;
        std::cout << "Slices guardados en: " << outputDir << std::endl;
        
        // Seleccionar slices representativos para mostrar
        std::vector<int> indicesToShow = selectRepresentativeSlices(slices.size());
        
        std::cout << "\nMostrando " << indicesToShow.size() 
                  << " slices representativos..." << std::endl;
        std::cout << "   (Presiona cualquier tecla para avanzar, ESC para salir)" << std::endl;
        
        // Mostrar slices seleccionados
        for (int idx : indicesToShow) {
            if (idx >= 0 && idx < static_cast<int>(slices.size())) {
                std::ostringstream windowName;
                windowName << "Slice #" << slices[idx].sliceNumber 
                          << " (" << (idx + 1) << "/" << slices.size() << ")";
                
                std::cout << "\nMostrando: " << windowName.str() << std::endl;
                
                // Mostrar con histograma
                Visualization::showImageWithHistogram(slices[idx].image, windowName.str());
                
                int key = cv::waitKey(0);
                if (key == 27) {  // ESC
                    std::cout << "Visualización cancelada por usuario" << std::endl;
                    break;
                }
                cv::destroyAllWindows();
            }
        }
        
        // Resumen final
        std::cout << "   RESUMEN\n";
        std::cout << "Total de slices procesados: " << slices.size() << std::endl;
        std::cout << "Archivos exportados: slice_0001.png - slice_" 
                  << std::setfill('0') << std::setw(4) << slices.size() << ".png" << std::endl;
        std::cout << "Directorio de salida: " << outputDir << std::endl;
        
    } catch (const std::exception& ex) {
        std::cerr << "Error fatal: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
