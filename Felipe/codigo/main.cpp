#include <iostream>
#include <opencv2/opencv.hpp>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkOpenCVImageBridge.h>

using namespace std;
using namespace cv;

// Definir tipos de imagen ITK
using PixelType = float;
using ImageType = itk::Image<PixelType, 2>;

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    cout << "╔═══════════════════════════════════════════════════════════╗" << endl;
    cout << "║           Práctica Interciclo - Vision por Computador    ║" << endl;
    cout << "║                   ITK + OpenCV                            ║" << endl;
    cout << "╚═══════════════════════════════════════════════════════════╝" << endl;
    cout << endl;
    
    // Tu código aquí
    
    
    
    return 0;
}
