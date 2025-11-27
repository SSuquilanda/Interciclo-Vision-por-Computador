#include "denoising.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <chrono>
#include <cmath>

namespace Denoising {

// ============================================================================
// CLASE DnCNNDenoiser
// ============================================================================

DnCNNDenoiser::DnCNNDenoiser() : modelLoaded(false) {}

bool DnCNNDenoiser::loadModel(const std::string& onnxPath) {
    try {
        std::cout << "🔄 Cargando modelo DnCNN: " << onnxPath << std::endl;
        
        // Cargar modelo ONNX con OpenCV DNN
        net = cv::dnn::readNetFromONNX(onnxPath);
        
        if (net.empty()) {
            std::cerr << "❌ Error: No se pudo cargar el modelo ONNX" << std::endl;
            return false;
        }
        
        // Configurar backend (CPU por defecto)
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        
        modelLoaded = true;
        modelPath = onnxPath;
        
        std::cout << "✅ Modelo DnCNN cargado exitosamente" << std::endl;
        return true;
        
    } catch (const cv::Exception& e) {
        std::cerr << "❌ Excepción OpenCV: " << e.what() << std::endl;
        modelLoaded = false;
        return false;
    }
}

cv::Mat DnCNNDenoiser::denoise(const cv::Mat& noisyImage) {
    if (!modelLoaded) {
        std::cerr << "⚠ Advertencia: Modelo no cargado, devolviendo imagen original" << std::endl;
        return noisyImage.clone();
    }
    
    try {
        // 1. Convertir a float [0, 1]
        cv::Mat inputFloat;
        int originalType = noisyImage.type();
        
        if (noisyImage.type() == CV_8U) {
            noisyImage.convertTo(inputFloat, CV_32F, 1.0/255.0);
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
        
        // 3. Crear blob (NCHW: 1 x 1 x H x W)
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
        cv::Mat outputBlob = net.forward();  // ← Aquí está la salida del modelo
        
        // 5. CORRECCIÓN CRÍTICA: Extraer imagen del blob de SALIDA
        // El blob tiene formato NCHW (1 x 1 x H x W)
        // Necesitamos extraer el canal [0][0]
        
        cv::Mat denoisedFloat;
        
        // Método 1: Acceso directo a los datos
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
        
        // 8. Verificación de debug (opcional, comentar en producción)
        /*
        cv::Mat diff;
        cv::absdiff(noisyImage, result, diff);
        double maxDiff = 0;
        cv::minMaxLoc(diff, nullptr, &maxDiff);
        std::cout << "   [DEBUG] Diferencia máxima: " << maxDiff << std::endl;
        */
        
        return result;
        
    } catch (const cv::Exception& e) {
        std::cerr << "❌ Error en denoising: " << e.what() << std::endl;
        return noisyImage.clone();
    }
}

std::string DnCNNDenoiser::getInfo() const {
    if (!modelLoaded) {
        return "❌ Modelo no cargado";
    }
    return "✅ DnCNN | Path: " + modelPath + " | Backend: CPU";
}

// ============================================================================
// FUNCIONES DE COMPARACIÓN
// ============================================================================

DenoisingComparison compareWithAndWithoutDenoising(
    const cv::Mat& noisyImage, 
    DnCNNDenoiser& denoiser) {
    
    DenoisingComparison result;
    result.original = noisyImage.clone();
    
    if (!denoiser.isLoaded()) {
        std::cerr << "❌ Error: Modelo no cargado" << std::endl;
        result.denoised = noisyImage.clone();
        return result;
    }
    
    // Medir tiempo de procesamiento
    auto start = std::chrono::high_resolution_clock::now();
    
    result.denoised = denoiser.denoise(noisyImage);
    
    auto end = std::chrono::high_resolution_clock::now();
    result.processingTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Calcular métricas
    result.psnr = calculatePSNR(noisyImage, result.denoised);
    result.snr = calculateSNR(result.denoised);
    
    std::cout << "\n📊 Comparación Denoising:" << std::endl;
    std::cout << "   PSNR: " << result.psnr << " dB" << std::endl;
    std::cout << "   SNR: " << result.snr << " dB" << std::endl;
    std::cout << "   Tiempo: " << result.processingTime << " ms\n" << std::endl;
    
    return result;
}

double calculatePSNR(const cv::Mat& img1, const cv::Mat& img2) {
    cv::Mat diff;
    cv::absdiff(img1, img2, diff);
    diff.convertTo(diff, CV_32F);
    diff = diff.mul(diff);
    
    cv::Scalar s = cv::sum(diff);
    double sse = s.val[0];
    
    if (sse <= 1e-10) {
        return 100.0; // Imágenes idénticas
    }
    
    double mse = sse / (double)(img1.total() * img1.channels());
    
    double maxPixel = 255.0;
    if (img1.depth() == CV_16S || img1.depth() == CV_16U) {
        double minVal, maxVal;
        cv::minMaxLoc(img1, &minVal, &maxVal);
        maxPixel = maxVal;
    }
    
    double psnr = 10.0 * log10((maxPixel * maxPixel) / mse);
    
    return psnr;
}

double calculateSNR(const cv::Mat& image) {
    cv::Scalar mean, stddev;
    cv::meanStdDev(image, mean, stddev);
    
    double signal = mean[0];
    double noise = stddev[0];
    
    if (noise < 1e-10) {
        return 100.0;
    }
    
    double snr = 10.0 * log10((signal * signal) / (noise * noise));
    
    return snr;
}

cv::Mat visualizeComparison(const DenoisingComparison& comparison) {
    // Convertir a 8-bit para visualización
    cv::Mat original8, denoised8;
    
    if (comparison.original.type() != CV_8U) {
        cv::normalize(comparison.original, original8, 0, 255, cv::NORM_MINMAX, CV_8U);
        cv::normalize(comparison.denoised, denoised8, 0, 255, cv::NORM_MINMAX, CV_8U);
    } else {
        original8 = comparison.original.clone();
        denoised8 = comparison.denoised.clone();
    }
    
    // Concatenar horizontalmente
    cv::Mat combined;
    cv::hconcat(original8, denoised8, combined);
    
    // Convertir a color para agregar texto
    cv::Mat combinedColor;
    cv::cvtColor(combined, combinedColor, cv::COLOR_GRAY2BGR);
    
    // Agregar etiquetas
    cv::putText(combinedColor, "SIN DENOISING", 
               cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 
               0.8, cv::Scalar(0, 0, 255), 2);
    
    cv::putText(combinedColor, "CON DENOISING (DnCNN)", 
               cv::Point(original8.cols + 10, 30), cv::FONT_HERSHEY_SIMPLEX, 
               0.8, cv::Scalar(0, 255, 0), 2);
    
    // Agregar métricas
    std::string psnrText = "PSNR: " + std::to_string(comparison.psnr).substr(0, 5) + " dB";
    cv::putText(combinedColor, psnrText, 
               cv::Point(10, combinedColor.rows - 40), cv::FONT_HERSHEY_SIMPLEX, 
               0.6, cv::Scalar(255, 255, 0), 2);
    
    std::string timeText = "Tiempo: " + std::to_string(comparison.processingTime).substr(0, 6) + " ms";
    cv::putText(combinedColor, timeText, 
               cv::Point(10, combinedColor.rows - 10), cv::FONT_HERSHEY_SIMPLEX, 
               0.6, cv::Scalar(255, 255, 0), 2);
    
    return combinedColor;
}

} // namespace Denoising