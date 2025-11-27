#ifndef DENOISING_H
#define DENOISING_H

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <string>

namespace Denoising {

/**
 * @brief Clase para aplicar denoising con red neuronal DnCNN
 */
class DnCNNDenoiser {
private:
    cv::dnn::Net net;
    bool modelLoaded;
    std::string modelPath;
    
public:
    DnCNNDenoiser();
    
    /**
     * @brief Cargar modelo ONNX
     * @param onnxPath Ruta al archivo .onnx
     * @return true si carga exitosa
     */
    bool loadModel(const std::string& onnxPath);
    
    /**
     * @brief Aplicar denoising a imagen CT
     * @param noisyImage Imagen con ruido (CV_8U o CV_16S)
     * @return Imagen sin ruido (mismo tipo que entrada)
     */
    cv::Mat denoise(const cv::Mat& noisyImage);
    
    /**
     * @brief Verificar si modelo está cargado
     */
    bool isLoaded() const { return modelLoaded; }
    
    /**
     * @brief Obtener información del modelo
     */
    std::string getInfo() const;
};

/**
 * @brief Estructura para comparar resultados con/sin denoising
 */
struct DenoisingComparison {
    cv::Mat original;           // Imagen original con ruido
    cv::Mat denoised;           // Imagen después de denoising
    double psnr;                // Peak Signal-to-Noise Ratio (dB)
    double snr;                 // Signal-to-Noise Ratio (dB)
    double processingTime;      // Tiempo de procesamiento (ms)
    
    DenoisingComparison() : psnr(0.0), snr(0.0), processingTime(0.0) {}
};

/**
 * @brief Comparar imagen con y sin denoising
 * @param noisyImage Imagen con ruido
 * @param denoiser Denoiser con modelo cargado
 * @return Estructura con resultados de comparación
 */
DenoisingComparison compareWithAndWithoutDenoising(
    const cv::Mat& noisyImage, 
    DnCNNDenoiser& denoiser
);

/**
 * @brief Calcular PSNR entre dos imágenes
 * @param img1 Primera imagen
 * @param img2 Segunda imagen
 * @return PSNR en dB (mayor es mejor)
 */
double calculatePSNR(const cv::Mat& img1, const cv::Mat& img2);

/**
 * @brief Calcular SNR de una imagen
 * @param image Imagen
 * @return SNR en dB
 */
double calculateSNR(const cv::Mat& image);

/**
 * @brief Visualizar comparación lado a lado
 * @param comparison Resultado de comparación
 * @return Imagen concatenada con métricas
 */
cv::Mat visualizeComparison(const DenoisingComparison& comparison);

} // namespace Denoising

#endif // DENOISING_H