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
        std::string label;      // Nombre
        cv::Scalar color;       // Color para visualización (BGR)
    };

    // Funciones de segmentación

    // Segmenta los pulmones (aire)
    // Umbral (-1000 a -400), filtro de bordes, selección de los 2 mayores.
    std::vector<SegmentedRegion> segmentLungs(const cv::Mat& image);

    // Segmenta estructuras óseas y las clasifica
    // Umbral (200 a 3000), clasificación por posición (Columna, Esternón, Costillas).
    std::vector<SegmentedRegion> segmentBones(const cv::Mat& image);

    // Segmenta la Aorta  Arterias Pulmonares
    // Umbral (30 a 120), filtros geométricos (central, anterior), morfología de conexión.
    std::vector<SegmentedRegion> segmentAorta(const cv::Mat& image);

    // Versiones con umbrales HU personalizados (avanzado)
    std::vector<SegmentedRegion> segmentLungsCustom(const cv::Mat& image, int minHU, int maxHU);
    std::vector<SegmentedRegion> segmentBonesCustom(const cv::Mat& image, int minHU, int maxHU);
    std::vector<SegmentedRegion> segmentAortaCustom(const cv::Mat& image, int minHU, int maxHU);

    // Utilidades
    
    // Aplica umbralización por rango (inRange)
    cv::Mat thresholdByRange(const cv::Mat& image, double minVal, double maxVal);
    
    // Encuentra componentes conectados y devuelve vector de regiones
    std::vector<SegmentedRegion> findConnectedComponents(const cv::Mat& binaryMask, int minArea);

}

#endif // SEGMENTATION_H