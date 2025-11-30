#include "morphology_controller.h"
#include "f1_ui/mainwindow.h"
#include "f1_ui/helpers/qt_opencv_helpers.h"
#include "f5_morphology/morphology.h"
#include <QPixmap>
#include <iostream>

MorphologyController::MorphologyController(QObject* parent)
    : QObject(parent)
{
}

void MorphologyController::setSliceContext(SliceContext* context)
{
    sliceContext = context;
}

void MorphologyController::setWidgets(
    QCheckBox* erode, QCheckBox* dilate,
    QCheckBox* opening, QCheckBox* closing,
    QCheckBox* gradient, QCheckBox* fillHoles,
    QCheckBox* removeBorder,
    QSlider* erodeKernel, QSlider* erodeIter,
    QSlider* dilateKernel, QSlider* dilateIter,
    QSlider* openingKernel, QSlider* closingKernel,
    QSlider* gradientKernel,
    QComboBox* kernelShape,
    QLabel* beforeLabel, QLabel* afterLabel)
{
    // Checkboxes
    checkErode = erode;
    checkDilate = dilate;
    checkOpening = opening;
    checkClosing = closing;
    checkGradient = gradient;
    checkFillHoles = fillHoles;
    checkRemoveBorder = removeBorder;

    // Sliders
    sliderErodeKernel = erodeKernel;
    sliderErodeIter = erodeIter;
    sliderDilateKernel = dilateKernel;
    sliderDilateIter = dilateIter;
    sliderOpeningKernel = openingKernel;
    sliderClosingKernel = closingKernel;
    sliderGradientKernel = gradientKernel;

    // Others
    comboKernelShape = kernelShape;

    // Image display
    imageMorphBeforeLabel = beforeLabel;
    imageMorphAfterLabel = afterLabel;
}

void MorphologyController::applyMorphology()
{
    if (!sliceContext || !sliceContext->isValid) {
        return;
    }

    // CRITICAL: Always start from the ORIGINAL segmentation, not the modified one
    cv::Mat workingImage;
    
    if (!sliceContext->segmentationOriginal.empty()) {
        // IMPORTANT: Use the ORIGINAL segmentation, not the modified one
        workingImage = sliceContext->segmentationOriginal.clone();
    } else {
        std::cerr << "No hay máscara de segmentación original para aplicar morfología" << std::endl;
        return;
    }
    
    // Ensure it's binary 8-bit
    if (workingImage.type() != CV_8U) {
        workingImage.convertTo(workingImage, CV_8U);
    }
    
    // If it has multiple channels, convert to grayscale
    if (workingImage.channels() > 1) {
        cv::cvtColor(workingImage, workingImage, cv::COLOR_BGR2GRAY);
    }
    
    // Ensure it's binary (0 or 255)
    cv::threshold(workingImage, workingImage, 10, 255, cv::THRESH_BINARY);

    // Get kernel shape
    int shapeIdx = comboKernelShape ? comboKernelShape->currentIndex() : 0;
    Morphology::StructuringElementShape shape = static_cast<Morphology::StructuringElementShape>(shapeIdx);

    cv::Mat result = workingImage.clone();

    // Apply operations in order
    
    // 1. Erosion
    if (checkErode && checkErode->isChecked()) {
        int kernelSize = sliderErodeKernel ? sliderErodeKernel->value() : 3;
        if (kernelSize % 2 == 0) kernelSize++;
        int iterations = sliderErodeIter ? sliderErodeIter->value() : 1;
        result = Morphology::erode(result, cv::Size(kernelSize, kernelSize), shape, iterations);
    }

    // 2. Dilation
    if (checkDilate && checkDilate->isChecked()) {
        int kernelSize = sliderDilateKernel ? sliderDilateKernel->value() : 3;
        if (kernelSize % 2 == 0) kernelSize++;
        int iterations = sliderDilateIter ? sliderDilateIter->value() : 1;
        result = Morphology::dilate(result, cv::Size(kernelSize, kernelSize), shape, iterations);
    }

    // 3. Opening
    if (checkOpening && checkOpening->isChecked()) {
        int kernelSize = sliderOpeningKernel ? sliderOpeningKernel->value() : 5;
        if (kernelSize % 2 == 0) kernelSize++;
        result = Morphology::opening(result, cv::Size(kernelSize, kernelSize), shape);
    }

    // 4. Closing
    if (checkClosing && checkClosing->isChecked()) {
        int kernelSize = sliderClosingKernel ? sliderClosingKernel->value() : 9;
        if (kernelSize % 2 == 0) kernelSize++;
        result = Morphology::closing(result, cv::Size(kernelSize, kernelSize), shape);
    }

    // 5. Morphological gradient
    if (checkGradient && checkGradient->isChecked()) {
        int kernelSize = sliderGradientKernel ? sliderGradientKernel->value() : 3;
        if (kernelSize % 2 == 0) kernelSize++;
        result = Morphology::morphologicalGradient(result, cv::Size(kernelSize, kernelSize), shape);
    }

    // 6. Fill holes
    if (checkFillHoles && checkFillHoles->isChecked()) {
        result = Morphology::fillHoles(result);
    }

    // 7. Remove borders
    if (checkRemoveBorder && checkRemoveBorder->isChecked()) {
        result = Morphology::clearBorder(result);
    }

    // Save result in sliceContext
    sliceContext->segmentationMask = result;
    
    // Create color visualization
    cv::Mat colorResult;
    cv::cvtColor(result, colorResult, cv::COLOR_GRAY2BGR);
    sliceContext->finalOverlay = colorResult;

    // Update display
    updateDisplay();
}

void MorphologyController::updateDisplay()
{
    if (!imageMorphBeforeLabel || !imageMorphAfterLabel) {
        return;
    }

    // BEFORE: Show original segmentation (before morphology)
    if (!sliceContext->segmentationOriginal.empty()) {
        cv::Mat beforeDisplay;
        if (sliceContext->segmentationOriginal.channels() == 1) {
            cv::cvtColor(sliceContext->segmentationOriginal, beforeDisplay, cv::COLOR_GRAY2BGR);
        } else {
            beforeDisplay = sliceContext->segmentationOriginal.clone();
        }
        
        QImage qimgBefore = UIHelpers::cvMatToQImage(beforeDisplay);
        if (!qimgBefore.isNull()) {
            QPixmap pixmapBefore = QPixmap::fromImage(qimgBefore);
            imageMorphBeforeLabel->setPixmap(pixmapBefore);
            imageMorphBeforeLabel->resize(pixmapBefore.size());
        }
    }

    // AFTER: Show morphological result
    if (!sliceContext->segmentationMask.empty()) {
        cv::Mat afterDisplay;
        if (sliceContext->segmentationMask.channels() == 1) {
            cv::cvtColor(sliceContext->segmentationMask, afterDisplay, cv::COLOR_GRAY2BGR);
        } else {
            afterDisplay = sliceContext->segmentationMask.clone();
        }
        
        QImage qimgAfter = UIHelpers::cvMatToQImage(afterDisplay);
        if (!qimgAfter.isNull()) {
            QPixmap pixmapAfter = QPixmap::fromImage(qimgAfter);
            imageMorphAfterLabel->setPixmap(pixmapAfter);
            imageMorphAfterLabel->resize(pixmapAfter.size());
        }
    }
}

void MorphologyController::applyPresetLungs()
{
    if (!checkOpening || !checkClosing || !checkFillHoles) {
        return;
    }

    // Deactivate all operations
    if (checkErode) checkErode->setChecked(false);
    if (checkDilate) checkDilate->setChecked(false);
    if (checkGradient) checkGradient->setChecked(false);
    if (checkRemoveBorder) checkRemoveBorder->setChecked(false);

    // Optimal configuration for lungs
    // Opening to remove small protrusions
    checkOpening->setChecked(true);
    if (sliderOpeningKernel) sliderOpeningKernel->setValue(5);

    // Closing to close small gaps
    checkClosing->setChecked(true);
    if (sliderClosingKernel) sliderClosingKernel->setValue(9);

    // Fill holes to fill internal gaps
    checkFillHoles->setChecked(true);

    // Elliptical kernel
    if (comboKernelShape) comboKernelShape->setCurrentIndex(0); // Ellipse

    // Apply changes
    applyMorphology();
    
    emit statusMessageChanged("Preset 'Pulmones Óptimo' aplicado: Opening(5) + Closing(9) + FillHoles");
}

void MorphologyController::applyPresetBones()
{
    if (!checkOpening || !checkErode || !checkRemoveBorder) {
        return;
    }

    // Deactivate unnecessary operations
    if (checkDilate) checkDilate->setChecked(false);
    if (checkClosing) checkClosing->setChecked(false);
    if (checkGradient) checkGradient->setChecked(false);
    if (checkFillHoles) checkFillHoles->setChecked(false);

    // Optimal configuration for bones
    // Soft erosion to remove noise
    if (checkErode) {
        checkErode->setChecked(true);
        if (sliderErodeKernel) sliderErodeKernel->setValue(3);
        if (sliderErodeIter) sliderErodeIter->setValue(1);
    }

    // Opening to clean
    checkOpening->setChecked(true);
    if (sliderOpeningKernel) sliderOpeningKernel->setValue(3);

    // Remove structures touching borders
    checkRemoveBorder->setChecked(true);

    // Rectangular kernel
    if (comboKernelShape) comboKernelShape->setCurrentIndex(1); // Rectangle

    // Apply changes
    applyMorphology();
    
    emit statusMessageChanged("Preset 'Huesos Óptimo' aplicado: Erode(3x1) + Opening(3) + RemoveBorder");
}

void MorphologyController::applyPresetAorta()
{
    if (!checkClosing || !checkDilate) {
        return;
    }

    // Deactivate unnecessary operations
    if (checkErode) checkErode->setChecked(false);
    if (checkOpening) checkOpening->setChecked(false);
    if (checkGradient) checkGradient->setChecked(false);
    if (checkFillHoles) checkFillHoles->setChecked(false);
    if (checkRemoveBorder) checkRemoveBorder->setChecked(false);

    // Optimal configuration for aorta (based on pipeline_aorta.cpp)
    // Closing to join aorta fragments
    checkClosing->setChecked(true);
    if (sliderClosingKernel) sliderClosingKernel->setValue(5);

    // Dilation to slightly expand the mask
    if (checkDilate) {
        checkDilate->setChecked(true);
        if (sliderDilateKernel) sliderDilateKernel->setValue(5);
        if (sliderDilateIter) sliderDilateIter->setValue(1);
    }

    // Elliptical kernel (appropriate for circular structures like arteries)
    if (comboKernelShape) comboKernelShape->setCurrentIndex(0); // Ellipse

    // Apply changes
    applyMorphology();
    
    emit statusMessageChanged("🩸 Preset 'Aorta Óptimo' aplicado: Closing(5) + Dilate(5x1)");
}

void MorphologyController::resetMorphology()
{
    // Deactivate all operations
    if (checkErode) checkErode->setChecked(false);
    if (checkDilate) checkDilate->setChecked(false);
    if (checkOpening) checkOpening->setChecked(false);
    if (checkClosing) checkClosing->setChecked(false);
    if (checkGradient) checkGradient->setChecked(false);
    if (checkFillHoles) checkFillHoles->setChecked(false);
    if (checkRemoveBorder) checkRemoveBorder->setChecked(false);

    // Reset slider values
    if (sliderErodeKernel) sliderErodeKernel->setValue(3);
    if (sliderDilateKernel) sliderDilateKernel->setValue(3);
    if (sliderOpeningKernel) sliderOpeningKernel->setValue(5);
    if (sliderClosingKernel) sliderClosingKernel->setValue(9);
    if (sliderGradientKernel) sliderGradientKernel->setValue(3);
    if (sliderErodeIter) sliderErodeIter->setValue(1);
    if (sliderDilateIter) sliderDilateIter->setValue(1);

    // Default elliptical kernel
    if (comboKernelShape) comboKernelShape->setCurrentIndex(0);

    // Apply changes
    applyMorphology();
    
    emit statusMessageChanged("Morfología reseteada a valores por defecto");
}
