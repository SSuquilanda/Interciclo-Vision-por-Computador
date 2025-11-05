#include <iostream>
#include <opencv2/opencv.hpp>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkGDCMImageIO.h>
#include <itkOpenCVImageBridge.h>

using namespace std;
using namespace cv;

// Definir tipos de imagen ITK
using PixelType = float;
using ImageType = itk::Image<PixelType, 2>;

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    cout << "\n";
    cout << "╔═══════════════════════════════════════════════════════════╗\n";
    cout << "║     Proyecto Interciclo - Visión por Computador          ║\n";
    cout << "║           Análisis de Imágenes CT (DICOM)                ║\n";
    cout << "║              OpenCV + ITK Integration                     ║\n";
    cout << "╚═══════════════════════════════════════════════════════════╝\n";
    cout << "\n";
    
    // Verificar versiones de bibliotecas
    cout << "Información de bibliotecas:\n";
    cout << "   OpenCV version: " << CV_VERSION << "\n";
    cout << "   ITK version: " << ITK_VERSION_MAJOR << "." 
         << ITK_VERSION_MINOR << "." << ITK_VERSION_PATCH << "\n";
    cout << "\n";
    
    // Probar OpenCV
    cout << "Probando OpenCV...\n";
    Mat test_image = Mat::zeros(Size(640, 480), CV_8UC3);
    putText(test_image, "OpenCV funciona!", Point(50, 240), 
            FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 255, 0), 2);
    
    cout << "  OpenCV funcionando correctamente\n";
    cout << "   Tamaño de imagen de prueba: " << test_image.cols 
         << "x" << test_image.rows << "\n";
    cout << "\n";
    
    // Probar ITK
    cout << "Probando ITK...\n";
    ImageType::Pointer itkImage = ImageType::New();
    ImageType::SizeType size;
    size[0] = 512;
    size[1] = 512;
    
    ImageType::RegionType region;
    region.SetSize(size);
    itkImage->SetRegions(region);
    itkImage->Allocate();
    
    cout << "  ITK funcionando correctamente\n";
    cout << "   Imagen ITK creada: " << size[0] << "x" << size[1] << "\n";
    cout << "\n";
    
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    cout << "¡Todas las bibliotecas están funcionando correctamente!\n";
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    cout << "\n";
    cout << "Siguiente paso: Agregar código para procesar imágenes DICOM\n";
    cout << "\n";
    
    // Opcional: Mostrar la imagen de prueba
    // imshow("Test OpenCV", test_image);
    // waitKey(0);
    
    return 0;
}
