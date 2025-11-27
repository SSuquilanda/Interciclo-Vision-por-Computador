#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "denoising.h"

namespace Preprocessing {

// CONVERSIÓN Y NORMALIZACIÓN

/**
 * @brief Convierte una imagen a escala de grises si no lo está
 * @param image Imagen de entrada (BGR o escala de grises)
 * @return Imagen en escala de grises
 */
cv::Mat convertToGrayscale(const cv::Mat& image);

/**
 * @brief Normaliza la imagen estirando el rango de intensidades (stretching lineal)
 * @param image Imagen de entrada
 * @param minVal Valor mínimo de salida (default: 0)
 * @param maxVal Valor máximo de salida (default: 255)
 * @return Imagen normalizada
 */
cv::Mat normalizeImage(const cv::Mat& image, double minVal = 0.0, double maxVal = 255.0);

// ECUALIZACIÓN DE HISTOGRAMA

/**
 * @brief Aplica ecualización de histograma clásica
 * @param image Imagen en escala de grises
 * @return Imagen ecualizada
 */
cv::Mat equalizeHistogram(const cv::Mat& image);

/**
 * @brief Aplica CLAHE (Contrast Limited Adaptive Histogram Equalization)
 * @param image Imagen en escala de grises
 * @param clipLimit Límite de contraste (default: 2.0)
 * @param tileGridSize Tamaño de la cuadrícula de tiles (default: 8x8)
 * @return Imagen con CLAHE aplicado
 */
cv::Mat applyCLAHE(const cv::Mat& image, double clipLimit = 2.0, 
                    cv::Size tileGridSize = cv::Size(8, 8));

// FILTROS DE REDUCCIÓN DE RUIDO

/**
 * @brief Aplica filtro Gaussiano para suavizar la imagen
 * @param image Imagen de entrada
 * @param kernelSize Tamaño del kernel (debe ser impar, default: 5)
 * @param sigma Desviación estándar (default: 0 = auto)
 * @return Imagen suavizada
 */
cv::Mat applyGaussianBlur(const cv::Mat& image, int kernelSize = 5, double sigma = 0.0);

/**
 * @brief Aplica filtro de la mediana
 * @param image Imagen de entrada
 * @param kernelSize Tamaño del kernel (debe ser impar, default: 5)
 * @return Imagen con ruido reducido
 */
cv::Mat applyMedianFilter(const cv::Mat& image, int kernelSize = 5);

/**
 * @brief Aplica filtro de la media
 * @param image Imagen de entrada
 * @param kernelSize Tamaño del kernel (default: 5)
 * @return Imagen promediada
 */
cv::Mat applyMeanFilter(const cv::Mat& image, int kernelSize = 5);

/**
 * @brief Aplica filtro bilateral (preserva bordes)
 * @param image Imagen de entrada
 * @param d Diámetro del vecindario (default: 9)
 * @param sigmaColor Filtro sigma en el espacio de color (default: 75)
 * @param sigmaSpace Filtro sigma en el espacio de coordenadas (default: 75)
 * @return Imagen suavizada con bordes preservados
 */
cv::Mat applyBilateralFilter(const cv::Mat& image, int d = 9, 
                              double sigmaColor = 75.0, double sigmaSpace = 75.0);

// DETECCIÓN Y REALCE DE BORDES

/**
 * @brief Aplica detector de bordes Canny
 * @param image Imagen en escala de grises
 * @param lowThreshold Umbral bajo (default: 50)
 * @param highThreshold Umbral alto (default: 150)
 * @param apertureSize Tamaño de apertura Sobel (default: 3)
 * @return Imagen binaria con bordes detectados
 */
cv::Mat detectEdgesCanny(const cv::Mat& image, double lowThreshold = 50.0, 
                          double highThreshold = 150.0, int apertureSize = 3);

/**
 * @brief Aplica detector de bordes Sobel
 * @param image Imagen en escala de grises
 * @param dx Orden de la derivada en x (default: 1)
 * @param dy Orden de la derivada en y (default: 0)
 * @param kernelSize Tamaño del kernel Sobel (default: 3)
 * @return Imagen con gradientes detectados
 */
cv::Mat detectEdgesSobel(const cv::Mat& image, int dx = 1, int dy = 0, int kernelSize = 3);

/**
 * @brief Aplica filtro Laplaciano para realce de bordes
 * @param image Imagen de entrada
 * @param kernelSize Tamaño del kernel (default: 3)
 * @return Imagen con bordes realzados
 */
cv::Mat applyLaplacian(const cv::Mat& image, int kernelSize = 3);

// PIPELINE DE PREPROCESAMIENTO COMPLETO

/**
 * @brief Aplica un pipeline completo de preprocesamiento para imágenes CT
 * @param image Imagen de entrada
 * @param useCLAHE Si usar CLAHE en lugar de ecualización clásica (default: true)
 * @param useDenoising Si aplicar reducción de ruido (default: true)
 * @return Imagen preprocesada lista para segmentación
 */
cv::Mat preprocessCTImage(const cv::Mat& image, bool useCLAHE = true, 
                          bool useDenoising = true);

/**
 * @brief Compara múltiples técnicas de preprocesamiento lado a lado
 * @param image Imagen de entrada
 * @return Vector con pares <nombre_técnica, imagen_resultado>
 */
std::vector<std::pair<std::string, cv::Mat>> comparePreprocessingTechniques(const cv::Mat& image);

/**
 * @brief Pipeline completo con opción de denoising con red neuronal
 * @param image Imagen de entrada
 * @param useCLAHE Usar CLAHE para ecualización
 * @param denoiser Puntero a denoiser (nullptr = no usar)
 * @return Imagen preprocesada
 */
cv::Mat preprocessCTImageWithDenoising(const cv::Mat& image, 
                                        bool useCLAHE,
                                        Denoising::DnCNNDenoiser* denoiser);

} // namespace Preprocessing

#endif // PREPROCESSING_H
