#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QListWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <filesystem>

// Backend Modules
#include "../core/dicom_reader.h"
#include "../core/itk_opencv_bridge.h"
#include "../core/preprocessing.h"
#include "../core/segmentation.h"
#include "../core/morphology.h"

// Info del slice actual
struct SliceContext {
    cv::Mat originalRaw;      // 16-bit
    cv::Mat original8bit;     // 8-bit
    cv::Mat preprocessed;
    cv::Mat mask;
    cv::Mat morphologyMask;
    
    int sliceIndex = 0;
    double pixelSpacing = 1.0;
    
    void clear() {
        originalRaw.release();
        original8bit.release();
        preprocessed.release();
        mask.release();
        morphologyMask.release();
        sliceIndex = 0;
        pixelSpacing = 1.0;
    }
    
    bool hasData() const {
        return !originalRaw.empty() && !original8bit.empty();
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // I/O
    void onLoadDataset();
    void onSliceChanged(int value);
    void onFileListItemClicked(QListWidgetItem* item);
    
    // Preprocesamiento
    void onApplyPreprocessing();
    
    // Segmentación
    void onSegmentLungs();
    void onSegmentBones();
    void onSegmentAorta();
    void onClearSegmentation();
    
    // Morfología
    void onApplyMorphology();
    
    // Visualización
    void onUpdateVisualization();
    void onResetVisualization();
    
    // Métricas
    void onCalculateMetrics();
    void onLoadGroundTruthClicked();
    void onExportImages();

private:
    // Tabs
    QTabWidget* tabWidget;
    
    // Tab I/O
    QWidget* tabIO;
    QLabel* labelOriginalImage;
    QPushButton* btnLoadDataset;
    QSlider* sliceSlider;
    QLabel* labelSliceInfo;
    QListWidget* fileListWidget;
    
    // Tab Preprocesamiento
    QWidget* tabPreprocessing;
    QLabel* labelOriginalPreview;
    QLabel* labelPreprocessedImage;
    QCheckBox* checkUseDnCNN;
    QCheckBox* checkUseGaussian;
    QCheckBox* checkUseMedian;
    QCheckBox* checkUseBilateral;
    QCheckBox* checkUseHistogramEq;
    QCheckBox* checkUseCLAHE;
    QSlider* sliderGaussianKernel;
    QLabel* labelGaussianKernelValue;
    QSlider* sliderMedianKernel;
    QLabel* labelMedianKernelValue;
    QSlider* sliderBilateralD;
    QLabel* labelBilateralDValue;
    QSlider* sliderBilateralSigmaColor;
    QLabel* labelBilateralSigmaColorValue;
    QPushButton* btnApplyPreprocessing;
    QTextEdit* textPreprocessingLog;
    
    // Tab Segmentación
    QWidget* tabSegmentation;
    QLabel* labelSegmentationInput;
    QLabel* labelSegmentedImage;
    QPushButton* btnSegmentLungs;
    QPushButton* btnSegmentBones;
    QPushButton* btnSegmentAorta;
    QPushButton* btnClearSegmentation;
    QCheckBox* checkUseCustomHU;
    QSpinBox* spinMinHU;
    QSpinBox* spinMaxHU;
    QLabel* labelMinHU;
    QLabel* labelMaxHU;
    QCheckBox* checkUsePreprocessed;
    QTextEdit* textSegmentationLog;
    
    std::vector<Segmentation::SegmentedRegion> currentSegmentedRegions;
    
    // Tab Morfología
    QWidget* tabMorphology;
    QLabel* labelMorphologyInput;
    QLabel* labelMorphologyOutput;
    QComboBox* comboMorphologyOperation;
    QSlider* sliderMorphKernelSize;
    QLabel* labelMorphKernelValue;
    QSlider* sliderIterations;
    QLabel* labelIterationsValue;
    QPushButton* btnApplyMorphology;
    QPushButton* btnFillHoles;
    QTextEdit* textMorphologyLog;
    
    // Tab Visualización
    QWidget* tabVisualization;
    QLabel* labelVisualizationImage;
    QCheckBox* checkShowOverlay;
    QCheckBox* checkShowContours;
    QCheckBox* checkShowLabels;
    QSlider* sliderOverlayOpacity;
    QLabel* labelOpacityValue;
    QPushButton* btnUpdateVisualization;
    QPushButton* btnResetVisualization;
    
    // Tab Métricas
    QWidget* tabMetrics;
    QPushButton* btnCalculateMetrics;
    QPushButton* btnLoadGroundTruth;
    QTableWidget* tableMetrics;
    QLabel* labelHistogram;
    QLabel* labelPerformanceInfo;
    QLabel* labelMemoryInfo;
    
    // Estado
    SliceContext currentSlice;
    std::vector<std::string> datasetFiles;
    std::string currentDatasetPath;
    
    Preprocessing::DnCNNDenoiser dncnnDenoiser;
    bool dncnnModelLoaded;
    
    // Funciones auxiliares
    void setupUI();
    void setupTabIO();
    void setupTabPreprocessing();
    void setupTabSegmentation();
    void setupTabMorphology();
    void setupTabVisualization();
    void setupTabMetrics();
    
    void loadSlice(int index);
    void updateImageDisplay(QLabel* label, const cv::Mat& image);
    QImage cvMatToQImage(const cv::Mat& mat);
    
    void logMessage(QTextEdit* textEdit, const QString& message);
    void clearLogs();
};

#endif // MAINWINDOW_H

