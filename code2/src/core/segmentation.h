#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

namespace Segmentation {

    // Región segmentada con su máscara y propiedades
    struct SegmentedRegion {
        cv::Mat mask;
        cv::Rect boundingBox;
        double area;
        cv::Point2d centroid;
        std::string label;
        cv::Scalar color;
    };

    // Segmenta los pulmones usando umbral de aire
    std::vector<SegmentedRegion> segmentLungs(const cv::Mat& image);

    // Segmenta huesos y los clasifica por posición
    std::vector<SegmentedRegion> segmentBones(const cv::Mat& image);

    // Segmenta la aorta y arterias principales
    std::vector<SegmentedRegion> segmentAorta(const cv::Mat& image);

    // Versiones con umbrales HU personalizados
    std::vector<SegmentedRegion> segmentLungsCustom(const cv::Mat& image, int minHU, int maxHU);
    std::vector<SegmentedRegion> segmentBonesCustom(const cv::Mat& image, int minHU, int maxHU);
    std::vector<SegmentedRegion> segmentAortaCustom(const cv::Mat& image, int minHU, int maxHU);

    // Umbraliza imagen por rango de valores
    cv::Mat thresholdByRange(const cv::Mat& image, double minVal, double maxVal);
    
    // Extrae componentes conectados de una máscara binaria
    std::vector<SegmentedRegion> findConnectedComponents(const cv::Mat& binaryMask, int minArea);

}

#endif // SEGMENTATION_H
