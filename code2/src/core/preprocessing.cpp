#include "preprocessing.h"
#include <opencv2/dnn.hpp> // Importante para la IA
#include <iostream>
#include <fstream>

namespace Preprocessing {

    cv::Mat convertToGrayscale(const cv::Mat& image) {
        cv::Mat gray;
        if (image.channels() == 3) cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        else gray = image.clone();
        return gray;
    }

    cv::Mat normalizeImage(const cv::Mat& image) {
        cv::Mat norm;
        cv::normalize(image, norm, 0, 255, cv::NORM_MINMAX, CV_8U);
        return norm;
    }

    cv::Mat applyGaussianFilter(const cv::Mat& image, int kernelSize, double sigma) {
        cv::Mat result;
        if (kernelSize % 2 == 0) kernelSize++; // Asegurar impar
        cv::GaussianBlur(image, result, cv::Size(kernelSize, kernelSize), sigma);
        return result;
    }

    cv::Mat applyMedianFilter(const cv::Mat& image, int kernelSize) {
        cv::Mat result;
        if (kernelSize % 2 == 0) kernelSize++;
        cv::medianBlur(image, result, kernelSize);
        return result;
    }

    cv::Mat applyBilateralFilter(const cv::Mat& image, int d, double sigmaColor, double sigmaSpace) {
        cv::Mat result;
        cv::bilateralFilter(image, result, d, sigmaColor, sigmaSpace);
        return result;
    }

    cv::Mat applyCLAHE(const cv::Mat& image, double clipLimit, cv::Size tileGridSize) {
        cv::Mat result;
        auto clahe = cv::createCLAHE(clipLimit, tileGridSize);
        clahe->apply(image, result);
        return result;
    }

   // ==========================================
    // IMPLEMENTACIÓN DnCNN (RED NEURONAL)
    // ==========================================

    DnCNNDenoiser::DnCNNDenoiser() : modelLoaded(false) {}

    bool DnCNNDenoiser::loadModel(const std::string& onnxPath) {
        try {
            std::ifstream f(onnxPath.c_str());
            if (!f.good()) {
                std::cerr << "[Error] Archivo ONNX no encontrado: " << onnxPath << std::endl;
                return false;
            }

            net = cv::dnn::readNetFromONNX(onnxPath);
            if (net.empty()) return false;

            // Intentar usar aceleración si está disponible
            net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
            
            modelLoaded = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[Excepción DnCNN]: " << e.what() << std::endl;
            modelLoaded = false;
            return false;
        }
    }

    cv::Mat DnCNNDenoiser::denoise(const cv::Mat& noisyImage) {
        if (!modelLoaded) return noisyImage.clone();

        // Preprocesar: Convertir a Float32 y normalizar [0,1]
        cv::Mat imgFloat;
        noisyImage.convertTo(imgFloat, CV_32F, 1.0 / 255.0);

        // Crear Blob
        cv::Mat blob = cv::dnn::blobFromImage(imgFloat);
        net.setInput(blob);

        // Inferencia
        cv::Mat output = net.forward();

        // Postprocesar (DnCNN predice el ruido residual)
        std::vector<cv::Mat> outImages;
        cv::dnn::imagesFromBlob(output, outImages);
        cv::Mat predictedNoise = outImages[0];
        
        // Imagen Limpia = Original - Ruido
        cv::Mat cleanImage = imgFloat - predictedNoise;

        // Regresar a 8-bit [0,255]
        cv::Mat result;
        cleanImage.convertTo(result, CV_8U, 255.0);
        
        return result;
    }

    // ==========================================
    // FUNCIÓN HELPER PARA USAR DnCNN FÁCILMENTE
    // ==========================================

    cv::Mat applyDnCNN(const cv::Mat& noisyImage, const std::string& modelPath) {
        static DnCNNDenoiser denoiser;
        static std::string lastModelPath = "";
        
        // Cargar el modelo solo si es diferente o no está cargado
        if (!denoiser.isLoaded() || lastModelPath != modelPath) {
            if (!denoiser.loadModel(modelPath)) {
                std::cerr << "[Error] No se pudo cargar DnCNN. Retornando imagen original." << std::endl;
                return noisyImage.clone();
            }
            lastModelPath = modelPath;
        }
        
        return denoiser.denoise(noisyImage);
    }

    // ==========================================
    // MÉTRICAS (PSNR / SNR)
    // ==========================================

    double calculatePSNR(const cv::Mat& I1, const cv::Mat& I2) {
        cv::Mat s1;
        cv::absdiff(I1, I2, s1);       // |I1 - I2|
        s1.convertTo(s1, CV_32F);      // No podemos elevar al cuadrado en 8-bit
        s1 = s1.mul(s1);               // (I1 - I2)^2

        cv::Scalar s = cv::sum(s1);    // Suma de elementos
        double sse = s.val[0] + s.val[1] + s.val[2]; // Suma de canales

        if(sse <= 1e-10) return 0; // Son iguales

        double mse = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }

    double calculateSNR(const cv::Mat& image) {
        cv::Scalar mean, stddev;
        cv::meanStdDev(image, mean, stddev);
        // SNR = Media / Desviación Estándar
        if (stddev.val[0] == 0) return 0;
        return 20.0 * log10(mean.val[0] / stddev.val[0]);
    }

}