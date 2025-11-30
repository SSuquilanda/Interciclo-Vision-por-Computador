#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

namespace Segmentation {

    // Estructura de datos unificada para una región segmentada
    struct SegmentedRegion {
        cv::Mat mask;           // Máscara binaria (0/255)
        cv::Rect boundingBox;   // Caja delimitadora
        double area;            // Área en píxeles
        cv::Point2d centroid;   // Centroide (X, Y)
        std::string label;      // Nombre (ej. "Pulmón Derecho", "Costilla")
        cv::Scalar color;       // Color para visualización (BGR)
    };

    // --- FUNCIONES PRINCIPALES DE SEGMENTACIÓN (POR ÓRGANO) ---

    /**
     * @brief Segmenta los pulmones (aire)
     * Lógica: Umbral (-1000 a -400), filtro de bordes, selección de los 2 mayores.
     */
    std::vector<SegmentedRegion> segmentLungs(const cv::Mat& image);

    /**
     * @brief Segmenta estructuras óseas y las clasifica
     * Lógica: Umbral (200 a 3000), clasificación por posición (Columna, Esternón, Costillas).
     * Incluye corrección para ignorar calcificaciones centrales.
     */
    std::vector<SegmentedRegion> segmentBones(const cv::Mat& image);

    /**
     * @brief Segmenta la Aorta / Arterias Pulmonares
     * Lógica: Umbral (30 a 120), filtros geométricos (central, anterior), morfología de conexión.
     */
    std::vector<SegmentedRegion> segmentAorta(const cv::Mat& image);


    // --- UTILIDADES (Usadas internamente y disponibles si se requieren) ---
    
    // Aplica umbralización por rango (inRange)
    cv::Mat thresholdByRange(const cv::Mat& image, double minVal, double maxVal);
    
    // Encuentra componentes conectados y devuelve vector de regiones
    std::vector<SegmentedRegion> findConnectedComponents(const cv::Mat& binaryMask, int minArea);

}

#endif // SEGMENTATION_H