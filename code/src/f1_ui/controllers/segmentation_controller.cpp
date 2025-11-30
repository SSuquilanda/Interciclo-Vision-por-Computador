#include "segmentation_controller.h"
#include "f1_ui/mainwindow.h"
#include "f1_ui/helpers/qt_opencv_helpers.h"
#include "f4_segmentation/segmentation.h"
#include "f5_morphology/morphology.h"
#include "utils/itk_opencv_bridge.h"
#include <QPixmap>
#include <algorithm>

SegmentationController::SegmentationController(QObject* parent)
    : QObject(parent)
{
}

void SegmentationController::setSliceContext(SliceContext* context)
{
    sliceContext = context;
}

void SegmentationController::setWidgets(
    QSlider* minHU, QSlider* maxHU,
    QSlider* minArea, QSlider* maxArea,
    QLabel* minHUVal, QLabel* maxHUVal,
    QLabel* minAreaVal, QLabel* maxAreaVal,
    QCheckBox* showOverlay, QCheckBox* showContours,
    QCheckBox* showLabels, QCheckBox* filterBorder,
    QLabel* beforeLabel, QLabel* afterLabel)
{
    // Sliders
    sliderMinHU = minHU;
    sliderMaxHU = maxHU;
    sliderMinArea = minArea;
    sliderMaxArea = maxArea;

    // Value labels
    lblMinHUValue = minHUVal;
    lblMaxHUValue = maxHUVal;
    lblMinAreaValue = minAreaVal;
    lblMaxAreaValue = maxAreaVal;

    // Checkboxes
    checkShowOverlay = showOverlay;
    checkShowContours = showContours;
    checkShowLabels = showLabels;
    checkFilterBorder = filterBorder;

    // Image display
    imageSegBeforeLabel = beforeLabel;
    imageSegAfterLabel = afterLabel;
}

void SegmentationController::applySegmentation()
{
    if (!sliceContext || !sliceContext->isValid || sliceContext->originalRaw.empty()) {
        return;
    }

    // Use preprocessed image if available, otherwise normalized original
    cv::Mat sourceImage;
    if (!sliceContext->preprocessed.empty()) {
        sourceImage = sliceContext->preprocessed.clone();
    } else {
        sourceImage = Bridge::normalize16to8bit(sliceContext->originalRaw);
    }

    // Work with original image in HU for correct thresholding
    cv::Mat imageForSegmentation = sliceContext->originalRaw.clone();

    // Get segmentation parameters
    int minHU = sliderMinHU->value();
    int maxHU = sliderMaxHU->value();
    int minArea = sliderMinArea->value();
    int maxArea = sliderMaxArea->value();

    // Create segmentation parameters
    Segmentation::SegmentationParams params;
    params.minHU = minHU;
    params.maxHU = maxHU;
    params.minArea = minArea;
    params.maxArea = maxArea;
    params.visualColor = cv::Scalar(255, 0, 0); // Blue by default

    // Segment
    auto regions = Segmentation::segmentOrgan(imageForSegmentation, params, "Region");

    // Filter regions touching border if activated
    if (checkFilterBorder && checkFilterBorder->isChecked()) {
        std::vector<Segmentation::SegmentedRegion> filteredRegions;
        for (const auto& region : regions) {
            bool touchesBorder = (region.boundingBox.x <= 1 ||
                                 region.boundingBox.y <= 1 ||
                                 (region.boundingBox.x + region.boundingBox.width) >= imageForSegmentation.cols - 1 ||
                                 (region.boundingBox.y + region.boundingBox.height) >= imageForSegmentation.rows - 1);

            if (!touchesBorder) {
                filteredRegions.push_back(region);
            }
        }
        regions = filteredRegions;
    }

    // Sort by area (largest first)
    std::sort(regions.begin(), regions.end(),
              [](const Segmentation::SegmentedRegion& a, const Segmentation::SegmentedRegion& b) {
                  return a.area > b.area;
              });

    // Determine organ type based on HU range
    bool isLungs = (minHU <= -400 && maxHU <= -100);  // Air/lungs: [-1000, -400]
    bool isAorta = (minHU >= 20 && maxHU <= 150);     // Vessels with contrast: [30, 120]
    bool isBones = (minHU >= 150);                     // Bones: [200, 1000]

    // ANATOMICAL FILTERING FOR AORTA (as in pipeline_aorta.cpp)
    if (isAorta && !regions.empty()) {
        filterAortaRegions(regions, imageForSegmentation);
    }

    // Limit number of regions according to organ type (only if NOT aorta, already filtered above)
    if (!isAorta) {
        size_t maxRegions = isLungs ? 2 : 20; // Lungs: 2, Bones: multiple
        if (regions.size() > maxRegions) {
            regions.resize(maxRegions);
        }
    }

    // Assign labels and colors
    assignLabelsAndColors(regions, isLungs, isAorta, isBones);

    // Store regions in corresponding vector according to organ type
    if (isLungs) {
        sliceContext->pulmonesRegions = regions;
    } else if (isAorta) {
        sliceContext->aortaRegions = regions;
    } else if (isBones) {
        sliceContext->huesosRegions = regions;
    }

    // Apply morphological refinement to masks
    applyMorphologicalRefinement(regions);

    // Create visualization
    cv::Mat imageColor = createVisualization(sourceImage, regions);

    // Create binary mask combining all regions
    cv::Mat combinedMask = cv::Mat::zeros(sourceImage.size(), CV_8U);
    for (const auto& region : regions) {
        combinedMask |= region.mask;
    }

    // Save result in context
    sliceContext->segmentationMask = combinedMask;
    sliceContext->segmentationOriginal = combinedMask.clone(); // Copy for morphology
    sliceContext->finalOverlay = imageColor;

    // Update display
    updateDisplay();
}

void SegmentationController::filterAortaRegions(std::vector<Segmentation::SegmentedRegion>& regions, const cv::Mat& image)
{
    cv::Point2d imgCenter(image.cols / 2.0, image.rows / 2.0);

    std::vector<Segmentation::SegmentedRegion> filteredAorta;
    for (const auto& region : regions) {
        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = region.centroid.y - imgCenter.y;
        double distTotal = cv::norm(region.centroid - imgCenter);

        // Anatomical filters (from pipeline_aorta.cpp):
        bool isCentral = (distX < 70);      // Must be near horizontal center
        bool isAnterior = (distY < 20);     // Must be in anterior part (above center)
        bool isMedian = (distTotal < 100);  // Not too far from center
        bool sizeOk = (region.area >= 300 && region.area <= 5000); // Appropriate size

        if (isCentral && isAnterior && isMedian && sizeOk) {
            filteredAorta.push_back(region);
        }
    }

    // If we found candidates, apply morphological processing and keep the largest
    if (!filteredAorta.empty()) {
        // Combine masks
        cv::Mat combinedMask = cv::Mat::zeros(image.size(), CV_8U);
        for (const auto& r : filteredAorta) {
            cv::bitwise_or(combinedMask, r.mask, combinedMask);
        }

        // Morphology: closing + dilation
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);
        cv::dilate(combinedMask, combinedMask, kernel);

        // Re-segment connected components
        auto finalComponents = Segmentation::findConnectedComponents(combinedMask, 500);

        if (!finalComponents.empty()) {
            // Sort by area and take the largest
            std::sort(finalComponents.begin(), finalComponents.end(),
                [](const auto& a, const auto& b) { return a.area > b.area; });

            auto& largest = finalComponents[0];
            double dist = cv::norm(largest.centroid - imgCenter);

            // Verify that the largest is near the center
            if (dist < 120.0) {
                regions.clear();
                regions.push_back(largest);
            } else {
                regions.clear(); // No valid aorta found
            }
        }
    } else {
        regions.clear(); // No regions met anatomical criteria
    }
}

void SegmentationController::applyMorphologicalRefinement(std::vector<Segmentation::SegmentedRegion>& regions)
{
    for (auto& region : regions) {
        // Opening to smooth edges
        region.mask = Morphology::opening(region.mask, cv::Size(5, 5));
        // Closing to fill holes
        region.mask = Morphology::closing(region.mask, cv::Size(9, 9));
        // Fill all internal holes
        region.mask = Morphology::fillHoles(region.mask);
    }
}

void SegmentationController::assignLabelsAndColors(std::vector<Segmentation::SegmentedRegion>& regions,
                                                   bool isLungs, bool isAorta, bool isBones)
{
    for (size_t i = 0; i < regions.size(); i++) {
        if (isLungs) {
            regions[i].label = (i == 0) ? "Pulmon Derecho" : "Pulmon Izquierdo";
            regions[i].color = cv::Scalar(255, 0, 0); // Blue
        } else if (isAorta) {
            regions[i].label = "Aorta"; // Only 1 structure after anatomical filtering
            regions[i].color = cv::Scalar(0, 0, 255); // Red
        } else if (isBones) {
            regions[i].label = "Hueso_" + std::to_string(i+1);
            regions[i].color = cv::Scalar(0, 255, 0); // Green
        } else {
            regions[i].label = "Region_" + std::to_string(i+1);
            regions[i].color = cv::Scalar(255, 255, 0); // Cyan
        }
    }
}

cv::Mat SegmentationController::createVisualization(const cv::Mat& sourceImage,
                                                   const std::vector<Segmentation::SegmentedRegion>& regions)
{
    // Create color image
    cv::Mat imageColor;
    if (sourceImage.channels() == 1) {
        cv::cvtColor(sourceImage, imageColor, cv::COLOR_GRAY2BGR);
    } else {
        imageColor = sourceImage.clone();
    }

    // Create overlay if activated
    if (checkShowOverlay && checkShowOverlay->isChecked()) {
        cv::Mat overlay = imageColor.clone();
        for (const auto& region : regions) {
            overlay.setTo(region.color, region.mask);
        }
        cv::addWeighted(imageColor, 0.7, overlay, 0.3, 0, imageColor);
    }

    // Draw contours if activated
    if (checkShowContours && checkShowContours->isChecked()) {
        for (const auto& region : regions) {
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(region.mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::drawContours(imageColor, contours, -1, region.color, 3);
        }
    }

    // Draw labels if activated
    if (checkShowLabels && checkShowLabels->isChecked()) {
        for (const auto& region : regions) {
            cv::putText(imageColor, region.label,
                       cv::Point(region.boundingBox.x, region.boundingBox.y - 10),
                       cv::FONT_HERSHEY_SIMPLEX, 0.8, region.color, 2);
        }
    }

    return imageColor;
}

void SegmentationController::updateDisplay()
{
    if (!sliceContext || !sliceContext->isValid || sliceContext->originalRaw.empty()) {
        return;
    }

    // BEFORE image (preprocessed or normalized)
    cv::Mat imageBefore;
    if (!sliceContext->preprocessed.empty()) {
        imageBefore = sliceContext->preprocessed;
    } else {
        imageBefore = Bridge::normalize16to8bit(sliceContext->originalRaw);
    }

    QImage qimgBefore = UIHelpers::cvMatToQImage(imageBefore);
    if (!qimgBefore.isNull() && imageSegBeforeLabel) {
        QPixmap pixmapBefore = QPixmap::fromImage(qimgBefore);
        imageSegBeforeLabel->setPixmap(pixmapBefore);
        imageSegBeforeLabel->resize(pixmapBefore.size());
    }

    // AFTER image (segmented)
    if (!sliceContext->finalOverlay.empty() && imageSegAfterLabel) {
        QImage qimgAfter = UIHelpers::cvMatToQImage(sliceContext->finalOverlay);
        if (!qimgAfter.isNull()) {
            QPixmap pixmapAfter = QPixmap::fromImage(qimgAfter);
            imageSegAfterLabel->setPixmap(pixmapAfter);
            imageSegAfterLabel->resize(pixmapAfter.size());
        }
    }
}

void SegmentationController::applyPresetLungs()
{
    // Optimized preset for lungs (based on pipeline_pulmones.cpp)

    // Configure HU thresholds for air/lungs
    if (sliderMinHU) sliderMinHU->setValue(-1000);
    if (sliderMaxHU) sliderMaxHU->setValue(-400);

    // Configure area filters
    if (sliderMinArea) sliderMinArea->setValue(5000);   // Filter small regions
    if (sliderMaxArea) sliderMaxArea->setValue(200000); // Upper limit

    // Activate optimal visualization options
    if (checkShowContours) checkShowContours->setChecked(true);
    if (checkShowOverlay) checkShowOverlay->setChecked(true);
    if (checkShowLabels) checkShowLabels->setChecked(true);
    if (checkFilterBorder) checkFilterBorder->setChecked(true); // Remove external air

    // Apply changes
    applySegmentation();

    emit statusMessageChanged("Preset 'Pulmones Óptimo' aplicado a segmentación");
}

void SegmentationController::applyPresetBones()
{
    // Optimized preset for bones

    // Configure HU thresholds for bones (range 200-1000 HU)
    if (sliderMinHU) sliderMinHU->setValue(200);
    if (sliderMaxHU) sliderMaxHU->setValue(1000);

    // Configure area filters for bones
    if (sliderMinArea) sliderMinArea->setValue(1000);
    if (sliderMaxArea) sliderMaxArea->setValue(50000);

    // Activate visualization options
    if (checkShowContours) checkShowContours->setChecked(true);
    if (checkShowOverlay) checkShowOverlay->setChecked(true);
    if (checkShowLabels) checkShowLabels->setChecked(false);

    // Deactivate border filtering (we want to detect ribs and vertebrae at edges)
    if (checkFilterBorder) checkFilterBorder->setChecked(false);

    // Apply changes
    applySegmentation();

    emit statusMessageChanged("🦴 Preset 'Huesos Óptimo' aplicado: HU[200,1000], Área[1000,50000]");
}

void SegmentationController::applyPresetAorta()
{
    // Optimized preset for aorta (based on pipeline_aorta.cpp)

    // Configure HU thresholds for arteries (range 30-120 HU)
    if (sliderMinHU) sliderMinHU->setValue(30);
    if (sliderMaxHU) sliderMaxHU->setValue(120);

    // Configure area filters for aorta
    if (sliderMinArea) sliderMinArea->setValue(200);
    if (sliderMaxArea) sliderMaxArea->setValue(8000);

    // Activate visualization options
    if (checkShowContours) checkShowContours->setChecked(true);
    if (checkShowOverlay) checkShowOverlay->setChecked(true);
    if (checkShowLabels) checkShowLabels->setChecked(false); // No labels for arteries
    if (checkFilterBorder) checkFilterBorder->setChecked(false); // Don't filter border for aorta

    // Apply changes
    applySegmentation();

    emit statusMessageChanged("🩸 Preset 'Aorta Óptimo' aplicado: HU[30,120], Área[200,8000]");
}

void SegmentationController::resetSegmentation()
{
    // Reset HU thresholds
    if (sliderMinHU) sliderMinHU->setValue(-1000);
    if (sliderMaxHU) sliderMaxHU->setValue(-400);

    // Reset area filters
    if (sliderMinArea) sliderMinArea->setValue(1000);
    if (sliderMaxArea) sliderMaxArea->setValue(200000);

    // Reset visualization options
    if (checkShowContours) checkShowContours->setChecked(true);
    if (checkShowOverlay) checkShowOverlay->setChecked(true);
    if (checkShowLabels) checkShowLabels->setChecked(true);
    if (checkFilterBorder) checkFilterBorder->setChecked(true);

    // Apply changes
    applySegmentation();

    emit statusMessageChanged("Segmentación reseteada a valores por defecto");
}
