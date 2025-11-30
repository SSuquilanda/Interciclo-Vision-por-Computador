#ifndef SEGMENTATION_CONTROLLER_H
#define SEGMENTATION_CONTROLLER_H

#include <QObject>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <vector>
#include "f4_segmentation/segmentation.h"

// Forward declaration
struct SliceContext;

/**
 * @brief Controller for segmentation operations
 * 
 * Handles all segmentation logic including organ detection,
 * anatomical filtering, and visualization options.
 */
class SegmentationController : public QObject
{
    Q_OBJECT

public:
    explicit SegmentationController(QObject* parent = nullptr);
    ~SegmentationController() = default;

    // Context and widget configuration
    void setSliceContext(SliceContext* context);
    void setWidgets(
        // Sliders
        QSlider* minHU, QSlider* maxHU,
        QSlider* minArea, QSlider* maxArea,
        // Labels
        QLabel* minHUVal, QLabel* maxHUVal,
        QLabel* minAreaVal, QLabel* maxAreaVal,
        // Checkboxes
        QCheckBox* showOverlay, QCheckBox* showContours,
        QCheckBox* showLabels, QCheckBox* filterBorder,
        // Image labels
        QLabel* beforeLabel, QLabel* afterLabel
    );

    // Main operations
    void applySegmentation();
    void updateDisplay();

    // Presets
    void applyPresetLungs();
    void applyPresetBones();
    void applyPresetAorta();
    void resetSegmentation();

signals:
    void statusMessageChanged(const QString& message);

private:
    // Context
    SliceContext* sliceContext = nullptr;

    // Widget pointers - Sliders
    QSlider* sliderMinHU = nullptr;
    QSlider* sliderMaxHU = nullptr;
    QSlider* sliderMinArea = nullptr;
    QSlider* sliderMaxArea = nullptr;

    // Widget pointers - Value labels
    QLabel* lblMinHUValue = nullptr;
    QLabel* lblMaxHUValue = nullptr;
    QLabel* lblMinAreaValue = nullptr;
    QLabel* lblMaxAreaValue = nullptr;

    // Widget pointers - Checkboxes
    QCheckBox* checkShowOverlay = nullptr;
    QCheckBox* checkShowContours = nullptr;
    QCheckBox* checkShowLabels = nullptr;
    QCheckBox* checkFilterBorder = nullptr;

    // Widget pointers - Image display
    QLabel* imageSegBeforeLabel = nullptr;
    QLabel* imageSegAfterLabel = nullptr;

    // Helper methods
    void filterAortaRegions(std::vector<Segmentation::SegmentedRegion>& regions, const cv::Mat& image);
    void applyMorphologicalRefinement(std::vector<Segmentation::SegmentedRegion>& regions);
    void assignLabelsAndColors(std::vector<Segmentation::SegmentedRegion>& regions, 
                               bool isLungs, bool isAorta, bool isBones);
    cv::Mat createVisualization(const cv::Mat& sourceImage, 
                               const std::vector<Segmentation::SegmentedRegion>& regions);
};

#endif // SEGMENTATION_CONTROLLER_H
