#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include <vector>
#include <string>

namespace Segmentation {

// ============================================================================
// ESTRUCTURAS DE DATOS
// ============================================================================

/**
 * @brief Estructura para almacenar información de una región segmentada
 */
struct SegmentedRegion {
    cv::Mat mask;                   // Máscara binaria de la región
    cv::Rect boundingBox;           // Caja delimitadora
    double area;                    // Área en píxeles
    cv::Point2d centroid;           // Centro de masa
    std::string label; 
    double meanHU;             // Valor medio de Hounsfield
    cv::Scalar color;               // Color para visualización
};

/**
 * @brief Parámetros para segmentación específica de órganos
 */
struct SegmentationParams {
    int minHU;                      // Valor HU mínimo (para umbralización)
    int maxHU;                      // Valor HU máximo
    int minArea;                    // Área mínima en píxeles
    int maxArea;                    // Área máxima en píxeles
    cv::Scalar visualColor;         // Color para visualización
};

// ============================================================================
// UMBRALIZACIÓN (THRESHOLDING)
// ============================================================================

/**
 * @brief Aplica umbralización global usando método de Otsu
 * @param image Imagen en escala de grises
 * @return Máscara binaria
 */
cv::Mat thresholdOtsu(const cv::Mat& image);


/**
 * @brief Aplica umbralización manual con un valor específico
 * @param image Imagen en escala de grises
 * @param threshold Valor de umbral
 * @param maxValue Valor máximo de salida (default: 255)
 * @return Máscara binaria
 */
cv::Mat thresholdManual(const cv::Mat& image, double threshold, double maxValue = 255.0);

/**
 * @brief Aplica umbralización por rango (para valores HU específicos)
 * @param image Imagen de entrada
 * @param minVal Valor mínimo del rango
 * @param maxVal Valor máximo del rango
 * @return Máscara binaria
 */
cv::Mat thresholdByRange(const cv::Mat& image, double minVal, double maxVal);

/**
 * @brief Aplica umbralización adaptativa
 * @param image Imagen en escala de grises
 * @param blockSize Tamaño del vecindario (debe ser impar)
 * @param C Constante sustraída de la media ponderada
 * @return Máscara binaria
 */
cv::Mat thresholdAdaptive(const cv::Mat& image, int blockSize = 11, double C = 2.0);

// ============================================================================
// SEGMENTACIÓN AVANZADA
// ============================================================================

/**
 * @brief Aplica segmentación por Watershed
 * @param image Imagen de entrada
 * @param markers Marcadores iniciales (regiones semilla)
 * @return Imagen con regiones segmentadas etiquetadas
 */
cv::Mat segmentWatershed(const cv::Mat& image, cv::Mat& markers);

/**
 * @brief Aplica K-means clustering para segmentar
 * @param image Imagen de entrada
 * @param K Número de clusters
 * @param attempts Número de intentos (default: 3)
 * @return Imagen segmentada
 */
cv::Mat segmentKMeans(const cv::Mat& image, int K, int attempts = 3);

/**
 * @brief Segmentación por crecimiento de regiones (Region Growing)
 * @param image Imagen de entrada
 * @param seedPoints Puntos semilla iniciales
 * @param threshold Umbral de similitud
 * @return Máscara binaria de la región crecida
 */
cv::Mat segmentRegionGrowing(const cv::Mat& image, 
                              const std::vector<cv::Point>& seedPoints, 
                              double threshold);

/**
 * @brief Encuentra componentes conectados en una imagen binaria
 * @param binaryImage Imagen binaria
 * @param minArea Área mínima para considerar un componente
 * @return Vector con información de cada componente
 */
std::vector<SegmentedRegion> findConnectedComponents(const cv::Mat& binaryImage, 
                                                      int minArea = 100);

// ============================================================================
// SEGMENTACIÓN ESPECÍFICA PARA CT (A IMPLEMENTAR SEGÚN ÓRGANO)
// ============================================================================

/**
 * @brief Segmenta pulmones usando valores HU típicos (-1000 a -400)
 * @param image Imagen CT preprocesada
 * @param params Parámetros de segmentación
 * @return Vector con regiones de pulmones (normalmente 2)
 */
std::vector<SegmentedRegion> segmentLungs(const cv::Mat& image, 
                                          const SegmentationParams& params);

/**
 * @brief Segmenta el corazón usando valores HU típicos (0 a 100)
 * @param image Imagen CT preprocesada
 * @param params Parámetros de segmentación
 * @return Región del corazón
 */
SegmentedRegion segmentHeart(const cv::Mat& image, 
                              const SegmentationParams& params);

/**
 * @brief Segmenta estructuras óseas usando valores HU altos (>200)
 * @param image Imagen CT preprocesada
 * @param params Parámetros de segmentación
 * @return Vector con regiones óseas
 */
std::vector<SegmentedRegion> segmentBones(const cv::Mat& image, 
                                          const SegmentationParams& params);

/**
 * @brief Segmenta un órgano genérico basado en parámetros HU
 * @param image Imagen CT preprocesada
 * @param params Parámetros de segmentación
 * @param organName Nombre del órgano para la etiqueta
 * @return Regiones segmentadas
 */
std::vector<SegmentedRegion> segmentOrgan(const cv::Mat& image, 
                                          const SegmentationParams& params,
                                          const std::string& organName);

// ============================================================================
// REFINAMIENTO Y POST-PROCESAMIENTO
// ============================================================================

/**
 * @brief Filtra regiones por área mínima y máxima
 * @param regions Vector de regiones
 * @param minArea Área mínima
 * @param maxArea Área máxima
 * @return Vector filtrado de regiones
 */
std::vector<SegmentedRegion> filterRegionsByArea(const std::vector<SegmentedRegion>& regions,
                                                  double minArea, double maxArea);

/**
 * @brief Filtra regiones por posición (ej: solo lado izquierdo)
 * @param regions Vector de regiones
 * @param roi Región de interés donde deben estar las regiones
 * @return Vector filtrado de regiones
 */
std::vector<SegmentedRegion> filterRegionsByPosition(const std::vector<SegmentedRegion>& regions,
                                                      const cv::Rect& roi);

/**
 * @brief Combina múltiples máscaras en una sola
 * @param masks Vector de máscaras binarias
 * @return Máscara combinada
 */
cv::Mat combineMasks(const std::vector<cv::Mat>& masks);

// ============================================================================
// VISUALIZACIÓN
// ============================================================================

/**
 * @brief Aplica colormap a una máscara para visualización
 * @param mask Máscara binaria
 * @param colormapType Tipo de colormap (default: COLORMAP_JET)
 * @return Imagen con colormap aplicado
 */
cv::Mat applyColorMap(const cv::Mat& mask, int colormapType = cv::COLORMAP_JET);

/**
 * @brief Superpone máscaras de color sobre la imagen original
 * @param image Imagen original
 * @param regions Vector de regiones segmentadas
 * @param alpha Transparencia de las máscaras (0.0 a 1.0, default: 0.5)
 * @return Imagen con máscaras superpuestas
 */
cv::Mat overlaySegmentations(const cv::Mat& image, 
                              const std::vector<SegmentedRegion>& regions,
                              double alpha = 0.5);

/**
 * @brief Dibuja contornos de las regiones segmentadas
 * @param image Imagen de salida
 * @param regions Vector de regiones
 * @param thickness Grosor de los contornos (default: 2)
 * @return Imagen con contornos dibujados
 */
cv::Mat drawSegmentationContours(const cv::Mat& image, 
                                  const std::vector<SegmentedRegion>& regions,
                                  int thickness = 2);

// ============================================================================
// UTILIDADES
// ============================================================================

/**
 * @brief Obtiene parámetros predeterminados para segmentación de pulmones
 * @return Parámetros configurados para pulmones
 */
SegmentationParams getDefaultLungParams();

/**
 * @brief Obtiene parámetros predeterminados para segmentación del corazón
 * @return Parámetros configurados para corazón
 */
SegmentationParams getDefaultHeartParams();

/**
 * @brief Obtiene parámetros predeterminados para segmentación de huesos
 * @return Parámetros configurados para estructuras óseas
 */
SegmentationParams getDefaultBoneParams();

/**
 * @brief Guarda las máscaras de segmentación en archivos
 * @param regions Vector de regiones
 * @param outputPath Ruta base para guardar
 */
void saveSegmentationMasks(const std::vector<SegmentedRegion>& regions, 
                            const std::string& outputPath);

// ════════════════════════════════════════════════════════════
// SEGMENTACIÓN DE ARTERIAS (NUEVA SECCIÓN)
// ════════════════════════════════════════════════════════════

/**
 * @brief Tipo de arteria detectada
 */
enum class ArteriaType {
    DESCONOCIDA,
    AORTA_ASCENDENTE,
    AORTA_DESCENDENTE,
    ARCO_AORTICO,
    PULMONAR_PRINCIPAL,
    PULMONAR_DERECHA,
    PULMONAR_IZQUIERDA
};

/**
 * @brief Estructura para almacenar información de arteria detectada
 */
struct ArteriaRegion {
    ArteriaType tipo;
    cv::Mat mask;              // Máscara binaria
    cv::Rect boundingBox;
    double area;
    double circularidad;       // 0-1 (1 = círculo perfecto)
    double meanHU;
    cv::Point2f centroid;
    cv::Scalar color;
    std::string label;
};

/**
 * @brief Parámetros para segmentación de arterias
 */
struct ArteriaParams {
    double minHU = 150.0;
    double maxHU = 600.0;
    double minArea = 50.0;
    double maxArea = 5000.0;    
    double minCircularidad = 0.2;
    double maxCircularidad = 1.0;
};

/**
 * @brief Segmenta arterias pulmonares y aorta en imagen CT
 * @param imageHU Imagen en valores Hounsfield (16-bit signed)
 * @param params Parámetros de segmentación
 * @return Vector con arterias detectadas y clasificadas
 */
std::vector<ArteriaRegion> segmentArterias(const cv::Mat& imageHU, 
                                           const ArteriaParams& params = ArteriaParams());

/**
 * @brief Calcula circularidad de una región
 * @param area Área de la región
 * @param perimeter Perímetro de la región
 * @return Circularidad (0-1, donde 1 es círculo perfecto)
 */
double calculateCircularity(double area, double perimeter);

/**
 * @brief Clasifica tipo de arteria según forma y posición
 * @param region Región segmentada
 * @param circularidad Circularidad calculada
 * @param imageSize Tamaño de la imagen completa
 * @return Tipo de arteria detectada
 */
ArteriaType classifyArteria(const SegmentedRegion& region, 
                            double circularidad,
                            const cv::Size& imageSize);

/**
 * @brief Obtiene nombre legible del tipo de arteria
 */
std::string getArteriaTypeName(ArteriaType tipo);

/**
 * @brief Obtiene color asignado al tipo de arteria
 */
cv::Scalar getArteriaTypeColor(ArteriaType tipo);

} // namespace Segmentation

#endif // SEGMENTATION_H
