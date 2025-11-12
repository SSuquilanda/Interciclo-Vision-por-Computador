#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <iomanip>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

// ITK para lectura de series DICOM
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkExtractImageFilter.h>

// Módulos del Proyecto
#include "utils/itk_opencv_bridge.h"
#include "f6_visualization/visualization.h"

#define GET_STR(x) #x
#define GET_PROJECT_SOURCE_DIR(x) GET_STR(x)
std::string projectPath = GET_PROJECT_SOURCE_DIR(PROJECT_SOURCE_DIR);

namespace fs = std::filesystem;

// Tipos de imagen ITK
using PixelType = signed short;
using ImageType3D = itk::Image<PixelType, 3>;
using ImageType2D = itk::Image<PixelType, 2>;
using ReaderType = itk::ImageSeriesReader<ImageType3D>;
using ImageIOType = itk::GDCMImageIO;
using NamesGeneratorType = itk::GDCMSeriesFileNames;
using ExtractFilterType = itk::ExtractImageFilter<ImageType3D, ImageType2D>;

// Enumeración para las vistas
enum class ViewOrientation {
    AXIAL,    // Cortes XY (transversal)
    CORONAL,  // Cortes XZ (frontal)
    SAGITTAL  // Cortes YZ (lateral)
};

// Función para cargar volumen DICOM 3D
ImageType3D::Pointer loadDicomVolume(const std::string& directory) {
    std::cout << "CARGANDO VOLUMEN DICOM 3D" << std::endl;
    
    // Configurar generador de nombres de archivos DICOM
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
    nameGenerator->SetUseSeriesDetails(true);
    nameGenerator->SetDirectory(directory);
    
    try {
        // Obtener series UIDs
        using SeriesIdContainer = std::vector<std::string>;
        const SeriesIdContainer& seriesUID = nameGenerator->GetSeriesUIDs();
        
        if (seriesUID.empty()) {
            throw std::runtime_error("No se encontraron series DICOM en el directorio");
        }
        
        std::cout << "Series DICOM encontradas: " << seriesUID.size() << std::endl;
        
        // Usar la primera serie
        std::string seriesIdentifier = seriesUID.begin()->c_str();
        std::cout << "Usando serie: " << seriesIdentifier << std::endl;
        
        // Obtener nombres de archivos de la serie
        using FileNamesContainer = std::vector<std::string>;
        FileNamesContainer fileNames = nameGenerator->GetFileNames(seriesIdentifier);
        
        std::cout << "Archivos DICOM en la serie: " << fileNames.size() << std::endl;
        
        // Configurar lector de series
        ReaderType::Pointer reader = ReaderType::New();
        ImageIOType::Pointer dicomIO = ImageIOType::New();
        
        reader->SetImageIO(dicomIO);
        reader->SetFileNames(fileNames);
        
        std::cout << "Leyendo volumen 3D..." << std::endl;
        reader->Update();
        
        ImageType3D::Pointer volume = reader->GetOutput();
        ImageType3D::RegionType region = volume->GetLargestPossibleRegion();
        ImageType3D::SizeType size = region.GetSize();
        
        std::cout << "Volumen cargado exitosamente!" << std::endl;
        std::cout << "Dimensiones del volumen:" << std::endl;
        std::cout << "  - X (ancho):  " << size[0] << " píxeles" << std::endl;
        std::cout << "  - Y (alto):   " << size[1] << " píxeles" << std::endl;
        std::cout << "  - Z (slices): " << size[2] << " slices" << std::endl;
        
        return volume;
        
    } catch (const itk::ExceptionObject& ex) {
        std::cerr << "Error ITK: " << ex << std::endl;
        throw;
    }
}

// Función para extraer un slice 2D del volumen 3D
ImageType2D::Pointer extractSlice(ImageType3D::Pointer volume, 
                                   ViewOrientation orientation, 
                                   int sliceIndex) {
    ImageType3D::RegionType inputRegion = volume->GetLargestPossibleRegion();
    ImageType3D::SizeType size = inputRegion.GetSize();
    ImageType3D::IndexType start = inputRegion.GetIndex();
    
    // Configurar región de extracción según la orientación
    ImageType3D::SizeType extractSize = size;
    ImageType3D::IndexType extractIndex = start;
    
    switch (orientation) {
        case ViewOrientation::AXIAL:
            // Extraer slice en plano XY (índice Z fijo)
            extractSize[2] = 0;  // Colapsar dimensión Z
            extractIndex[2] = sliceIndex;
            break;
            
        case ViewOrientation::CORONAL:
            // Extraer slice en plano XZ (índice Y fijo)
            extractSize[1] = 0;  // Colapsar dimensión Y
            extractIndex[1] = sliceIndex;
            break;
            
        case ViewOrientation::SAGITTAL:
            // Extraer slice en plano YZ (índice X fijo)
            extractSize[0] = 0;  // Colapsar dimensión X
            extractIndex[0] = sliceIndex;
            break;
    }
    
    ImageType3D::RegionType desiredRegion(extractIndex, extractSize);
    
    // Extraer slice
    ExtractFilterType::Pointer extractor = ExtractFilterType::New();
    extractor->SetExtractionRegion(desiredRegion);
    extractor->SetInput(volume);
    extractor->SetDirectionCollapseToIdentity();
    extractor->Update();
    
    return extractor->GetOutput();
}

// Función para obtener el nombre de la orientación
std::string getOrientationName(ViewOrientation orientation) {
    switch (orientation) {
        case ViewOrientation::AXIAL:    return "axial";
        case ViewOrientation::CORONAL:  return "coronal";
        case ViewOrientation::SAGITTAL: return "sagital";
        default: return "unknown";
    }
}

// Función para exportar slices de una orientación específica
void exportOrientation(ImageType3D::Pointer volume, 
                      ViewOrientation orientation,
                      const std::string& baseOutputDir,
                      bool showSamples = true) {
    
    std::string orientationName = getOrientationName(orientation);
    std::string outputDir = baseOutputDir + "/" + orientationName;
    
    std::cout << "\nEXPORTANDO VISTA: " << orientationName << std::endl;
    
    // Crear directorio de salida
    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    }
    
    // Obtener dimensiones del volumen
    ImageType3D::RegionType region = volume->GetLargestPossibleRegion();
    ImageType3D::SizeType size = region.GetSize();
    
    // Determinar número de slices según la orientación
    int numSlices = 0;
    switch (orientation) {
        case ViewOrientation::AXIAL:    numSlices = size[2]; break;
        case ViewOrientation::CORONAL:  numSlices = size[1]; break;
        case ViewOrientation::SAGITTAL: numSlices = size[0]; break;
    }
    
    std::cout << "Total de slices a exportar: " << numSlices << std::endl;
    
    // Vector para almacenar imágenes de muestra
    std::vector<cv::Mat> sampleImages;
    std::vector<int> sampleIndices;
    
    // Calcular índices de muestras (0%, 25%, 50%, 75%, 100%)
    if (showSamples && numSlices > 0) {
        sampleIndices = {
            0,
            numSlices / 4,
            numSlices / 2,
            3 * numSlices / 4,
            numSlices - 1
        };
    }
    
    int progressInterval = std::max(1, numSlices / 20);
    
    // Exportar cada slice
    for (int i = 0; i < numSlices; i++) {
        try {
            // Extraer slice 2D
            ImageType2D::Pointer slice2D = extractSlice(volume, orientation, i);
            
            // Convertir ITK 2D a OpenCV
            cv::Mat cvImage = Bridge::itkToOpenCV(slice2D);
            
            // Normalizar a 8-bit
            cv::Mat cvImage8bit = Bridge::normalize16to8bit(cvImage);
            
            // Guardar imagen para muestras si corresponde
            bool isSample = std::find(sampleIndices.begin(), sampleIndices.end(), i) != sampleIndices.end();
            if (isSample && showSamples) {
                sampleImages.push_back(cvImage8bit.clone());
            }
            
            // Exportar como PNG
            std::ostringstream filename;
            filename << outputDir << "/slice_" << std::setfill('0') << std::setw(4) << (i + 1) << ".png";
            
            if (!cv::imwrite(filename.str(), cvImage8bit)) {
                std::cerr << "Error exportando slice " << (i + 1) << std::endl;
            }
            
            // Mostrar progreso
            if (i % progressInterval == 0 || i == numSlices - 1) {
                int percent = static_cast<int>((i + 1) * 100 / numSlices);
                std::cout << "  Progreso: " << std::setw(3) << percent << "% "
                          << "(" << (i + 1) << "/" << numSlices << ")" << std::endl;
            }
            
        } catch (const std::exception& ex) {
            std::cerr << "Error procesando slice " << (i + 1) << ": " << ex.what() << std::endl;
        }
    }
    
    std::cout << "Exportación de vista " << orientationName << " completada!" << std::endl;
    std::cout << "  Archivos guardados en: " << outputDir << std::endl;
    
    // Mostrar muestras si están disponibles
    if (showSamples && !sampleImages.empty()) {
        std::cout << "\nMostrando " << sampleImages.size() << " slices representativos..." << std::endl;
        std::cout << "   (Presiona cualquier tecla para avanzar, ESC para salir)" << std::endl;
        
        for (size_t i = 0; i < sampleImages.size(); i++) {
            std::ostringstream windowName;
            windowName << "Vista " << orientationName << " - Slice " 
                      << (sampleIndices[i] + 1) << "/" << numSlices;
            
            Visualization::showImageWithHistogram(sampleImages[i], windowName.str());
            
            int key = cv::waitKey(0);
            if (key == 27) {  // ESC
                std::cout << "Visualización cancelada por usuario" << std::endl;
                break;
            }
            cv::destroyAllWindows();
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "\nEXPORTADOR DE SLICES DICOM - 3 VISTAS" << std::endl;
    std::cout << "Axial | Coronal | Sagital" << std::endl;
    
    // Configuración de rutas
    std::string inputDir = projectPath + "/../data/L291_qd/";
    std::string outputDir = projectPath + "/../data/exported_slices_3views/";
    std::string modalityName = "Quarter Dose (QD)";
    
    // Permitir especificar directorio por línea de comandos
    if (argc >= 2) {
        std::string arg = argv[1];
        if (arg == "fd" || arg == "FD") {
            inputDir = projectPath + "/../data/L291_fd/";
            outputDir = projectPath + "/../data/exported_slices_3views_fd/";
            modalityName = "Full Dose (FD)";
        }
    } else {
        std::cout << "\nUso: ./ExportSlices3Views [fd|qd]" << std::endl;
        std::cout << "  fd - Full Dose" << std::endl;
        std::cout << "  qd - Quarter Dose (por defecto)" << std::endl;
    }
    
    std::cout << "\nModalidad: " << modalityName << std::endl;
    std::cout << "Directorio de entrada: " << inputDir << std::endl;
    std::cout << "Directorio de salida: " << outputDir << std::endl;
    
    try {
        // Verificar que el directorio de entrada existe
        if (!fs::exists(inputDir)) {
            throw std::runtime_error("El directorio de entrada no existe: " + inputDir);
        }
        
        // Crear directorio de salida base
        if (!fs::exists(outputDir)) {
            fs::create_directories(outputDir);
        }
        
        // Cargar volumen DICOM 3D
        ImageType3D::Pointer volume = loadDicomVolume(inputDir);
        
        // Preguntar si desea ver muestras
        std::cout << "\n¿Desea visualizar muestras de cada orientación? (s/n): ";
        char response;
        std::cin >> response;
        bool showSamples = (response == 's' || response == 'S');
        
        // Exportar las tres orientaciones
        exportOrientation(volume, ViewOrientation::AXIAL, outputDir, showSamples);
        exportOrientation(volume, ViewOrientation::CORONAL, outputDir, showSamples);
        exportOrientation(volume, ViewOrientation::SAGITTAL, outputDir, showSamples);
        
        // Resumen final
        std::cout << "\nRESUMEN DE EXPORTACIÓN" << std::endl;
        
        ImageType3D::RegionType region = volume->GetLargestPossibleRegion();
        ImageType3D::SizeType size = region.GetSize();
        
        std::cout << "\nModalidad: " << modalityName << std::endl;
        std::cout << "Dimensiones del volumen: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;
        
        std::cout << "\nSlices exportados por orientación:" << std::endl;
        std::cout << "  Axial (XY):    " << size[2] << " slices → " << outputDir << "/axial/" << std::endl;
        std::cout << "  Coronal (XZ):  " << size[1] << " slices → " << outputDir << "/coronal/" << std::endl;
        std::cout << "  Sagital (YZ):  " << size[0] << " slices → " << outputDir << "/sagital/" << std::endl;
        
        int totalSlices = size[0] + size[1] + size[2];
        std::cout << "\nTotal de imágenes generadas: " << totalSlices << " archivos PNG" << std::endl;
        
        std::cout << "\nExportación completada exitosamente!" << std::endl;
        
    } catch (const std::exception& ex) {
        std::cerr << "\n✗ Error fatal: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
