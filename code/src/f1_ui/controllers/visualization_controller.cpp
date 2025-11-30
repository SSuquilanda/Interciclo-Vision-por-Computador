#include "visualization_controller.h"
#include "f1_ui/mainwindow.h"
#include "f1_ui/helpers/qt_opencv_helpers.h"
#include "f4_segmentation/segmentation.h"
#include "utils/itk_opencv_bridge.h"
#include <QPixmap>

VisualizationController::VisualizationController(QObject* parent)
    : QObject(parent)
{
}

void VisualizationController::setSliceContext(SliceContext* context)
{
    sliceContext = context;
}

void VisualizationController::setWidgets(
    QCheckBox* showLungs,
    QCheckBox* showBones,
    QCheckBox* showSoftTissue,
    QRadioButton* styleFill,
    QRadioButton* styleContour,
    QSlider* opacity,
    QLabel* finalView)
{
    // Checkboxes
    chkShowLungs = showLungs;
    chkShowBones = showBones;
    chkShowSoftTissue = showSoftTissue;

    // Style controls
    radStyleFill = styleFill;
    radStyleContour = styleContour;
    sliderOpacity = opacity;

    // Display
    lblFinalView = finalView;
}

void VisualizationController::updateVisualization()
{
    if (!sliceContext || !sliceContext->isValid || sliceContext->originalRaw.empty()) {
        if (lblFinalView) {
            lblFinalView->setText("No hay imagen cargada");
        }
        return;
    }

    // 1. Convert original 16-bit image to 8-bit BGR
    cv::Mat imgNormalized = Bridge::normalize16to8bit(sliceContext->originalRaw);
    cv::Mat imgColor;
    if (imgNormalized.channels() == 1) {
        cv::cvtColor(imgNormalized, imgColor, cv::COLOR_GRAY2BGR);
    } else {
        imgColor = imgNormalized.clone();
    }

    // 2. Create overlay by cloning base image
    cv::Mat overlay = imgColor.clone();
    cv::Mat finalResult = imgColor.clone();

    // Determine visualization style
    bool useFillStyle = radStyleFill && radStyleFill->isChecked();
    
    // Get opacity (0-100 -> 0.0-1.0)
    double alpha = (sliderOpacity ? sliderOpacity->value() : 40) / 100.0;

    // 3. Apply organ layers from stored vectors
    
    // LAYER: Lungs (Blue)
    if (chkShowLungs && chkShowLungs->isChecked() && 
        !sliceContext->pulmonesRegions.empty()) {
        renderOrganLayer(imgColor, overlay, finalResult, 
                        sliceContext->pulmonesRegions,
                        cv::Scalar(255, 0, 0), // BGR: Blue
                        useFillStyle, alpha);
    }

    // LAYER: Bones (Green)
    if (chkShowBones && chkShowBones->isChecked() && chkShowBones->isEnabled() &&
        !sliceContext->huesosRegions.empty()) {
        renderOrganLayer(imgColor, overlay, finalResult,
                        sliceContext->huesosRegions,
                        cv::Scalar(0, 255, 0), // BGR: Green
                        useFillStyle, alpha);
    }

    // LAYER: Aorta/Soft Tissue (Red)
    if (chkShowSoftTissue && chkShowSoftTissue->isChecked() && chkShowSoftTissue->isEnabled() &&
        !sliceContext->aortaRegions.empty()) {
        renderOrganLayer(imgColor, overlay, finalResult,
                        sliceContext->aortaRegions,
                        cv::Scalar(0, 0, 255), // BGR: Red
                        useFillStyle, alpha);
    }

    // 4. Display result in QLabel
    if (lblFinalView) {
        QImage qimg = UIHelpers::cvMatToQImage(finalResult);
        if (!qimg.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(qimg);
            lblFinalView->setPixmap(pixmap);
            lblFinalView->resize(pixmap.size());
        }
    }
}

void VisualizationController::renderOrganLayer(cv::Mat& imgColor, cv::Mat& overlay, cv::Mat& finalResult,
                                              const std::vector<Segmentation::SegmentedRegion>& regions,
                                              const cv::Scalar& color, bool useFillStyle, double alpha)
{
    // Create mask combining all regions
    cv::Mat combinedMask = cv::Mat::zeros(imgColor.size(), CV_8U);
    for (const auto& region : regions) {
        combinedMask |= region.mask;
    }
    
    if (useFillStyle) {
        // Solid fill with blend
        overlay.setTo(color, combinedMask);
        cv::addWeighted(imgColor, 1.0 - alpha, overlay, alpha, 0, finalResult);
        imgColor = finalResult.clone(); // Accumulate for next layer
    } else {
        // Contours only
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(combinedMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::drawContours(finalResult, contours, -1, color, 3);
    }
}
