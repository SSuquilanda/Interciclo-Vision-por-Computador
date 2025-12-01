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

        try {
            // 1. Convertir a Float32 y normalizar [0,1]
            cv::Mat inputFloat;
            int originalType = noisyImage.type();
            
            if (noisyImage.type() == CV_8U) {
                noisyImage.convertTo(inputFloat, CV_32F, 1.0 / 255.0);
            } else if (noisyImage.type() == CV_16S || noisyImage.type() == CV_16U) {
                // Para imágenes CT (16-bit), normalizar a [0, 1]
                double minVal, maxVal;
                cv::minMaxLoc(noisyImage, &minVal, &maxVal);
                noisyImage.convertTo(inputFloat, CV_32F, 1.0/(maxVal - minVal), -minVal/(maxVal - minVal));
            } else {
                inputFloat = noisyImage.clone();
            }
            
            // 2. Asegurar single-channel
            if (inputFloat.channels() > 1) {
                cv::cvtColor(inputFloat, inputFloat, cv::COLOR_BGR2GRAY);
            }

            // 3. Crear Blob (NCHW: 1 x 1 x H x W)
            cv::Mat blob = cv::dnn::blobFromImage(
                inputFloat,
                1.0,                    // NO re-escalar (ya está en [0,1])
                inputFloat.size(),      
                cv::Scalar(0),          // NO restar mean
                false,                  
                false                   
            );
            
            // 4. Inferencia
            net.setInput(blob);
            cv::Mat outputBlob = net.forward();

            // 5. CORRECCIÓN CRÍTICA: Extraer imagen correctamente del blob de SALIDA
            // El blob tiene formato NCHW (1 x 1 x H x W)
            // Necesitamos extraer el canal [0][0]
            
            cv::Mat denoisedFloat;
            
            // Acceso directo a los datos del blob
            const int* dims = outputBlob.size.p;
            int height = dims[2];
            int width = dims[3];
            
            // Crear Mat desde los datos del blob
            denoisedFloat = cv::Mat(height, width, CV_32F, outputBlob.ptr<float>(0, 0));
            denoisedFloat = denoisedFloat.clone(); // Hacer copia profunda
            
            // 6. Clamp a [0, 1] para evitar valores fuera de rango
            cv::max(denoisedFloat, 0.0, denoisedFloat);
            cv::min(denoisedFloat, 1.0, denoisedFloat);

            // 7. Convertir de vuelta al tipo original
            cv::Mat result;
            
            if (originalType == CV_8U) {
                denoisedFloat.convertTo(result, CV_8U, 255.0);
            } else if (originalType == CV_16S || originalType == CV_16U) {
                double minVal, maxVal;
                cv::minMaxLoc(noisyImage, &minVal, &maxVal);
                denoisedFloat.convertTo(result, originalType, (maxVal - minVal), minVal);
            } else {
                result = denoisedFloat;
            }
            
            return result;
            
        } catch (const std::exception& e) {
            std::cerr << "[Excepción en DnCNN denoise]: " << e.what() << std::endl;
            return noisyImage.clone();
        }
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