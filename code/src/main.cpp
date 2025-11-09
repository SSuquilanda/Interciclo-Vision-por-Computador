#include <iostream>
#include <iomanip> // Para std::setprecision

// --- Cabeceras de ITK ---
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkGDCMImageIOFactory.h" // Para registrar el lector DICOM
#include "itkGDCMImageIO.h"       // Para acceder a la Metadata
#include "itkMetaDataObject.h"   // Para la Metadata
#include "itkStatisticsImageFilter.h" // Para Min, Max, Media, etc.
#include "itkOpenCVImageBridge.h"  // El "puente"
#include <filesystem>
namespace fs = std::filesystem;


// --- Cabeceras de OpenCV ---
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

// --- Macros para la ruta del proyecto (definida en CMakeLists.txt) ---
#define GET_STR(x) #x
#define GET_PROJECT_SOURCE_DIR(x) GET_STR(x)
std::string projectPath = GET_PROJECT_SOURCE_DIR(PROJECT_SOURCE_DIR);


/**
 * @brief Dibuja un histograma simple para una imagen de 8 bits
 * @param image Imagen de 8 bits (CV_8U)
 * @return Una imagen (cv::Mat) que contiene el histograma
 */
cv::Mat drawHistogram(const cv::Mat& image) {
    // 1. Calcular el histograma
    cv::Mat hist;
    int histSize = 256;    // 256 bins (0-255)
    float range[] = { 0, 256 };
    const float* histRange = { range };
    cv::calcHist(&image, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, true, false);

    // 2. Crear la imagen para mostrar el histograma
    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound((double)hist_w / histSize);
    cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0, 0, 0));

    // 3. Normalizar el histograma para que quepa en la imagen
    cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

    // 4. Dibujar las barras
    for (int i = 1; i < histSize; i++) {
        cv::line(histImage,
            cv::Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
            cv::Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))),
            cv::Scalar(255, 255, 255), 2, 8, 0);
    }
    return histImage;
}


int main() {
    // --- 1. Registrar Fábricas (¡Crítico para leer DICOM!) ---
    itk::GDCMImageIOFactory::RegisterOneFactory();

    // --- 2. Definir Tipos de Imagen ---
    const unsigned int Dimension = 2;
    // ¡IMPORTANTE! Los valores HU son signed short (16-bit), no float.
    using PixelType = short;
    using ImageType = itk::Image<PixelType, Dimension>;

    // --- 3. Definir Ruta de Imagen ---
    // Usamos la variable de CMake para encontrar la carpeta 'data'
    std::string inFileName = projectPath +
        "/../data/CT_low_dose_reconstruction_dataset/Original Data/"
        "Quarter Dose/3mm Slice Thickness/Soft Kernel (B30)/L291/"
        "L291_QD_3_1.CT.0003.0001.2015.12.23.17.49.43.831724.127634491.IMA";

    // --- 4. Configurar Lector ITK ---
    using ReaderType = itk::ImageFileReader<ImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inFileName);

    // --- Verificar existencia de archivo antes de leer ---
    std::cout << "Ruta absoluta del archivo DICOM:\n  " << inFileName << "\n";

    if (!fs::exists(inFileName)) {
        std::cerr << "❌ Archivo no encontrado, revisa la ruta.\n";
        return 1;
    }


    // --- Intentar leer el archivo con manejo de errores ---
    try {
        reader->Update(); // Forzar la lectura del archivo
    }
    catch (itk::ExceptionObject& ex) {
        std::cerr << "❌ Error al leer el archivo DICOM:\n" << ex << std::endl;
        return 1;
    }

    ImageType::Pointer itkImage = reader->GetOutput();


    std::cout << "╔═══════════════════════════════════════════════════════╗\n";
    std::cout << "║       FASE 2: EXPLORACION DE IMAGEN DICOM             ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════╝\n";
    std::cout << "Leyendo imagen: " << inFileName << "\n\n";

    // --- 5. Tarea 2.1: Mostrar Metadata DICOM ---
    std::cout << "--- 2.1 Metadata DICOM (Ejemplos) ---\n";
    itk::MetaDataDictionary& dictionary = reader->GetImageIO()->GetMetaDataDictionary();

    std::string patientNameTag = "0010|0010";
    std::string patientIDTag = "0010|0020";
    std::string studyDescriptionTag = "0008|1030";
    std::string patientName, patientID, studyDescription;

    itk::ExposeMetaData<std::string>(dictionary, patientNameTag, patientName);
    itk::ExposeMetaData<std::string>(dictionary, patientIDTag, patientID);
    itk::ExposeMetaData<std::string>(dictionary, studyDescriptionTag, studyDescription);

    std::cout << "  Paciente: " << patientName << "\n";
    std::cout << "  ID Paciente: " << patientID << "\n";
    std::cout << "  Estudio: " << studyDescription << "\n\n";

    // --- 6. Tarea 2.1 y 2.2: Mostrar Estadísticas ---
    std::cout << "--- 2.1 y 2.2 Estadisticas de Imagen ---\n";
    using StatsFilterType = itk::StatisticsImageFilter<ImageType>;
    auto statsFilter = StatsFilterType::New();
    statsFilter->SetInput(itkImage);
    statsFilter->Update();

    std::cout << std::fixed << std::setprecision(2); // Formatear a 2 decimales
    std::cout << "  Dimensiones: " << itkImage->GetLargestPossibleRegion().GetSize() << "\n";
    std::cout << "  Rango (Min HU): " << statsFilter->GetMinimum() << "\n";
    std::cout << "  Rango (Max HU): " << statsFilter->GetMaximum() << "\n";
    std::cout << "  Media (Promedio HU): " << statsFilter->GetMean() << "\n";
    std::cout << "  Desv. Estandar (Ruido): " << statsFilter->GetSigma() << "\n\n";

    // --- 7. Tarea 2.1: Convertir a OpenCV ---
    std::cout << "--- 2.1 Puente ITK -> OpenCV ---\n";
    std::cout << "Convirtiendo imagen a cv::Mat...\n";
    cv::Mat cvImage;
    try {
        cvImage = itk::OpenCVImageBridge::ITKImageToCVMat<ImageType>(itkImage.GetPointer(), false);
    }
    catch (const itk::ExceptionObject& ex) {
        std::cerr << "Excepcion al convertir la imagen: " << std::endl;
        std::cerr << ex << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Conversion exitosa.\n\n";

    // --- 8. Tarea 2.2: Visualizar Imagen e Histograma ---

    // Normalizar la imagen de 16-bit (ej. -1000 a 2000) a 8-bit (0-255) para poder verla
    cv::Mat cvImage8bit;
    cv::normalize(cvImage, cvImage8bit, 0, 255, cv::NORM_MINMAX, CV_8U);

    // Calcular y dibujar el histograma de la imagen de 8 bits
    cv::Mat histImage = drawHistogram(cvImage8bit);

    std::cout << "Mostrando imagen e histograma. Presiona ESC para salir.\n";
    cv::imshow("Fase 2: Imagen DICOM (Normalizada 8-bit)", cvImage8bit);
    cv::imshow("Fase 2: Histograma", histImage);
    cv::waitKey(0);

    return EXIT_SUCCESS;
}