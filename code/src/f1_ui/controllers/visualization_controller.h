#ifndef VISUALIZATION_CONTROLLER_H
#define VISUALIZATION_CONTROLLER_H

#include <QObject>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <opencv2/opencv.hpp>
#include <vector>
#include "f4_segmentation/segmentation.h"

// Forward declaration
struct SliceContext;

/**
 * @brief Controller for final visualization of multi-organ segmentation
 * 
 * Handles the compositing of multiple organ masks (lungs, bones, aorta)
 * with configurable visualization styles and opacity.
 */
class VisualizationController : public QObject
{
    Q_OBJECT

public:
    explicit VisualizationController(QObject* parent = nullptr);
    ~VisualizationController() = default;

    // Context and widget configuration
    void setSliceContext(SliceContext* context);
    void setWidgets(
        // Checkboxes for organ layers
        QCheckBox* showLungs,
        QCheckBox* showBones,
        QCheckBox* showSoftTissue,
        // Style controls
        QRadioButton* styleFill,
        QRadioButton* styleContour,
        QSlider* opacity,
        // Display label
        QLabel* finalView
    );

    // Main operations
    void updateVisualization();

signals:
    void statusMessageChanged(const QString& message);

private:
    // Context
    SliceContext* sliceContext = nullptr;

    // Widget pointers - Checkboxes
    QCheckBox* chkShowLungs = nullptr;
    QCheckBox* chkShowBones = nullptr;
    QCheckBox* chkShowSoftTissue = nullptr;

    // Widget pointers - Style controls
    QRadioButton* radStyleFill = nullptr;
    QRadioButton* radStyleContour = nullptr;
    QSlider* sliderOpacity = nullptr;

    // Widget pointers - Display
    QLabel* lblFinalView = nullptr;

    // Helper methods
    void renderOrganLayer(cv::Mat& imgColor, cv::Mat& overlay, cv::Mat& finalResult,
                         const std::vector<Segmentation::SegmentedRegion>& regions,
                         const cv::Scalar& color, bool useFillStyle, double alpha);
};

#endif // VISUALIZATION_CONTROLLER_H
