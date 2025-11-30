#ifndef MORPHOLOGY_CONTROLLER_H
#define MORPHOLOGY_CONTROLLER_H

#include <QObject>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <opencv2/opencv.hpp>

// Forward declaration
struct SliceContext;

/**
 * @brief Controller for morphological operations
 * 
 * Handles all morphological transformations including erosion, dilation,
 * opening, closing, gradient, hole filling, and border removal.
 */
class MorphologyController : public QObject
{
    Q_OBJECT

public:
    explicit MorphologyController(QObject* parent = nullptr);
    ~MorphologyController() = default;

    // Context and widget configuration
    void setSliceContext(SliceContext* context);
    void setWidgets(
        // Checkboxes for operations
        QCheckBox* erode, QCheckBox* dilate,
        QCheckBox* opening, QCheckBox* closing,
        QCheckBox* gradient, QCheckBox* fillHoles,
        QCheckBox* removeBorder,
        // Sliders for kernel sizes and iterations
        QSlider* erodeKernel, QSlider* erodeIter,
        QSlider* dilateKernel, QSlider* dilateIter,
        QSlider* openingKernel, QSlider* closingKernel,
        QSlider* gradientKernel,
        // Combo box for kernel shape
        QComboBox* kernelShape,
        // Image labels
        QLabel* beforeLabel, QLabel* afterLabel
    );

    // Main operations
    void applyMorphology();
    void updateDisplay();

    // Presets
    void applyPresetLungs();
    void applyPresetBones();
    void applyPresetAorta();
    void resetMorphology();

signals:
    void statusMessageChanged(const QString& message);

private:
    // Context
    SliceContext* sliceContext = nullptr;

    // Widget pointers - Checkboxes
    QCheckBox* checkErode = nullptr;
    QCheckBox* checkDilate = nullptr;
    QCheckBox* checkOpening = nullptr;
    QCheckBox* checkClosing = nullptr;
    QCheckBox* checkGradient = nullptr;
    QCheckBox* checkFillHoles = nullptr;
    QCheckBox* checkRemoveBorder = nullptr;

    // Widget pointers - Sliders
    QSlider* sliderErodeKernel = nullptr;
    QSlider* sliderErodeIter = nullptr;
    QSlider* sliderDilateKernel = nullptr;
    QSlider* sliderDilateIter = nullptr;
    QSlider* sliderOpeningKernel = nullptr;
    QSlider* sliderClosingKernel = nullptr;
    QSlider* sliderGradientKernel = nullptr;

    // Widget pointers - Others
    QComboBox* comboKernelShape = nullptr;

    // Widget pointers - Image display
    QLabel* imageMorphBeforeLabel = nullptr;
    QLabel* imageMorphAfterLabel = nullptr;
};

#endif // MORPHOLOGY_CONTROLLER_H
