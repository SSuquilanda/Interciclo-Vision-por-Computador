#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <memory>
#include <curl/curl.h>

namespace Preprocessing {

    // Filtros comunes
    cv::Mat convertToGrayscale(const cv::Mat& image);
    cv::Mat normalizeImage(const cv::Mat& image); 
    cv::Mat applyGaussianFilter(const cv::Mat& image, int kernelSize = 3, double sigma = 0);
    cv::Mat applyMedianFilter(const cv::Mat& image, int kernelSize = 3);
    cv::Mat applyBilateralFilter(const cv::Mat& image, int d = 9, double sigmaColor = 75, double sigmaSpace = 75);
    cv::Mat applyCLAHE(const cv::Mat& image, double clipLimit = 2.0, cv::Size tileGridSize = cv::Size(8, 8));

    // Inteligencia Artificial (Clase DnCNN)
    class DnCNNDenoiser {
    private:
        cv::dnn::Net net;
        bool modelLoaded;
        std::string flaskServerUrl;
        bool useFlaskServer;
        
        cv::Mat denoiseViaFlask(const cv::Mat& noisyImage);
        cv::Mat denoiseViaOpenCV(const cv::Mat& noisyImage);
        
    public:
        DnCNNDenoiser();
        
        // Cargar el modelo .onnx (para fallback)
        bool loadModel(const std::string& onnxPath);
        
        void setFlaskServer(const std::string& url);
        cv::Mat denoise(const cv::Mat& noisyImage);
        
        bool isLoaded() const { return modelLoaded || useFlaskServer; }
    };

    // Aplicar DnCNN fácilmente
    cv::Mat applyDnCNN(const cv::Mat& noisyImage, const std::string& modelPath);

    // Métricas
    double calculatePSNR(const cv::Mat& original, const cv::Mat& processed);
    double calculateSNR(const cv::Mat& image);

}

#endif // PREPROCESSING_H