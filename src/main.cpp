#include <iostream>

// --- Cabeceras de ITK ---
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h" // Para escalar la imagen para verla

// --- Cabecera del PUENTE ---
#include "itkOpenCVImageBridge.h"

// --- Cabeceras de OpenCV ---
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"


int main() {
    // --- 1. Definir tipos de imagen en ITK ---
    // Las imágenes DICOM suelen ser 16-bit (short). Ajusta esto si es necesario.
    const unsigned int Dimension = 2; // Estamos leyendo un slice 2D
    using PixelType = short; // píxel de 16-bit
    using ImageType = itk::Image<PixelType, Dimension>;

    // -----------------------------------------------------------------
    // ¡¡CAMBIO CLAVE AQUÍ!!
    // -----------------------------------------------------------------
    // Le decimos al programa que busque la imagen en la carpeta 'data/'
    // El "../" significa "subir un nivel" (desde build/Release hasta la raíz)
    std::string inFileName = "../../data/L506_QD_3_1.CT.0003.0100.2015.12.22.20.45.42.541197.358793121.IMA";

    // --- 2. Leer la imagen con ITK ---
    using ReaderType = itk::ImageFileReader<ImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inFileName);

    std::cout << "Leyendo imagen con ITK: " << inFileName << std::endl;

    try {
        reader->Update();
    }
    catch (const itk::ExceptionObject& ex) {
        std::cerr << "Excepcion al leer el archivo: " << std::endl;
        std::cerr << ex << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer itkImage = reader->GetOutput();

    // --- 3. Convertir de ITK a cv::Mat usando el Bridge ---
    std::cout << "Convirtiendo imagen a cv::Mat..." << std::endl;
    cv::Mat cvImage;
    try {
        // ¡La función mágica!
        cvImage = itk::OpenCVImageBridge::ITKImageToCVMat(itkImage, false);
    }
    catch (const itk::ExceptionObject& ex) {
        std::cerr << "Excepcion al convertir la imagen: " << std::endl;
        std::cerr << ex << std::endl;
        return EXIT_FAILURE;
    }

    // --- 4. Procesar y Mostrar con OpenCV ---
    // Normalizamos la imagen de 16-bit (0-65535) a 8-bit (0-255)
    cv::Mat cvImage8bit;
    cv::normalize(cvImage, cvImage8bit, 0, 255, cv::NORM_MINMAX, CV_8U);

    std::cout << "Mostrando imagen con OpenCV. Presiona ESC para salir." << std::endl;

    cv::imshow("Imagen ITK leida y mostrada con OpenCV", cvImage8bit);

    // Espera a que el usuario presione ESC
    while (cv::waitKey(0) != 27) {}

    return EXIT_SUCCESS;
}