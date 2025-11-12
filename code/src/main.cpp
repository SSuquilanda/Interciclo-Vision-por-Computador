#include <iostream>
#include <string>

// Modulos del Proyecto
#include "f2_io/dicom_reader.h"
#include "utils/itk_opencv_bridge.h"
#include "f6_visualization/visualization.h"

// Macros para la ruta del proyecto (definida en CMakeLists.txt)
#define GET_STR(x) #x
#define GET_PROJECT_SOURCE_DIR(x) GET_STR(x)
std::string projectPath = GET_PROJECT_SOURCE_DIR(PROJECT_SOURCE_DIR);


int main() {
    std::cout << "Fase 2: Exploración de imagen DICOM\n";

    // Definir Ruta de Imagen
    std::string inFileName = projectPath +
        "/../data/L291_qd/"
        "L291_QD_3_1.CT.0003.0001.2015.12.23.17.49.43.831724.127634491.IMA";

    try {
        // Leer imagen DICOM
        DicomIO::ImagePointer itkImage = DicomIO::readDicomImage(inFileName);
        
        // Lector para extraer metadata
        using ReaderType = itk::ImageFileReader<DicomIO::ImageType>;
        auto reader = ReaderType::New();
        reader->SetFileName(inFileName);
        reader->Update();
        
        // Extraer y mostrar metadata
        auto metadata = DicomIO::extractMetadata(reader);
        DicomIO::displayImageInfo(itkImage, metadata);
        
        // Mostrar estadisticas
        DicomIO::displayImageStatistics(itkImage);
        
        // Convertir a OpenCV
        cv::Mat cvImage = Bridge::itkToOpenCV(itkImage);
        
        // Normalizar a 8-bit para visualizacion
        cv::Mat cvImage8bit = Bridge::normalize16to8bit(cvImage);
        
        // Visualizar imagen e histograma
        Visualization::showImageWithHistogram(cvImage8bit, "Fase 2: Imagen DICOM");
        Visualization::waitAndClose("Mostrando imagen e histograma. Presiona ESC para salir.");
        
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
