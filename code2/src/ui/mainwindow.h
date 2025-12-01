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

/**
 * @brief Estructura para mantener el estado del slice actual
 * Evita variables globales y centraliza el contexto de procesamiento
 */
struct SliceContext {
    cv::Mat originalRaw;      // 16-bit (Para cálculos HU)
    cv::Mat original8bit;     // 8-bit (Para visualización)
    cv::Mat preprocessed;     // Salida de Preprocessing
    cv::Mat mask;             // Salida de Segmentation
    cv::Mat morphologyMask;   // Salida de Morphology
    
    // Metadatos
    int sliceIndex = 0;
    double pixelSpacing = 1.0;
    
    // Método para resetear el contexto
    void clear() {
        originalRaw.release();
        original8bit.release();
        preprocessed.release();
        mask.release();
        morphologyMask.release();
        sliceIndex = 0;
        pixelSpacing = 1.0;
    }
    
    // Verificar si hay datos cargados
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
    // === PESTAÑA 1: I/O Dataset ===
    void onLoadDataset();
    void onSliceChanged(int value);
    void onFileListItemClicked(QListWidgetItem* item);
    
    // === PESTAÑA 2: Preprocesamiento ===
    void onApplyPreprocessing();
    
    // === PESTAÑA 3: Segmentación ===
    void onSegmentLungs();
    void onSegmentBones();
    void onSegmentAorta();
    void onClearSegmentation();
    
    // === PESTAÑA 4: Morfología ===
    void onApplyMorphology();
    
    // === PESTAÑA 5: Visualización ===
    void onUpdateVisualization();
    void onResetVisualization();
    
    // === PESTAÑA 6: Métricas ===
    void onCalculateMetrics();

private:
    // === WIDGETS PRINCIPALES ===
    QTabWidget* tabWidget;
    
    // === PESTAÑA 1: I/O Dataset ===
    QWidget* tabIO;
    QLabel* labelOriginalImage;
    QPushButton* btnLoadDataset;
    QSlider* sliceSlider;
    QLabel* labelSliceInfo;
    QListWidget* fileListWidget;
    
    // === PESTAÑA 2: Preprocesamiento ===
    QWidget* tabPreprocessing;
    QLabel* labelOriginalPreview;        // Imagen Original (Izquierda)
    QLabel* labelPreprocessedImage;      // Imagen Procesada (Derecha)
    QCheckBox* checkUseDnCNN;
    QCheckBox* checkUseGaussian;
    QCheckBox* checkUseMedian;
    QCheckBox* checkUseCLAHE;
    QSlider* sliderGaussianKernel;
    QLabel* labelGaussianKernelValue;
    QSlider* sliderMedianKernel;
    QLabel* labelMedianKernelValue;
    QPushButton* btnApplyPreprocessing;
    QTextEdit* textPreprocessingLog;
    
    // === PESTAÑA 3: Segmentación ===
    QWidget* tabSegmentation;
    QLabel* labelSegmentationInput;     // Imagen Preprocesada o Original (Izquierda)
    QLabel* labelSegmentedImage;        // Máscara de Segmentación (Derecha)
    QPushButton* btnSegmentLungs;
    QPushButton* btnSegmentBones;
    QPushButton* btnSegmentAorta;
    QPushButton* btnClearSegmentation;
    QSpinBox* spinMinHU;
    QSpinBox* spinMaxHU;
    QLabel* labelMinHU;
    QLabel* labelMaxHU;
    QCheckBox* checkUsePreprocessed;
    QTextEdit* textSegmentationLog;
    
    // Almacenar regiones segmentadas actuales
    std::vector<Segmentation::SegmentedRegion> currentSegmentedRegions;
    
    // === PESTAÑA 4: Morfología ===
    QWidget* tabMorphology;
    QLabel* labelMorphologyInput;       // Máscara de entrada (Izquierda)
    QLabel* labelMorphologyOutput;      // Máscara refinada (Derecha)
    QComboBox* comboMorphologyOperation;
    QSlider* sliderMorphKernelSize;
    QLabel* labelMorphKernelValue;
    QSlider* sliderIterations;
    QLabel* labelIterationsValue;
    QPushButton* btnApplyMorphology;
    QPushButton* btnFillHoles;
    QTextEdit* textMorphologyLog;
    
    // === PESTAÑA 5: Visualización ===
    QWidget* tabVisualization;
    QLabel* labelVisualizationImage;
    QCheckBox* checkShowOverlay;
    QCheckBox* checkShowContours;
    QCheckBox* checkShowLabels;
    QSlider* sliderOverlayOpacity;
    QLabel* labelOpacityValue;
    QPushButton* btnUpdateVisualization;
    QPushButton* btnResetVisualization;
    
    // === PESTAÑA 6: Métricas ===
    QWidget* tabMetrics;
    QPushButton* btnCalculateMetrics;
    QTableWidget* tableMetrics;
    QLabel* labelHistogram;
    QLabel* labelPerformanceInfo;
    QLabel* labelMemoryInfo;
    
    // === ESTADO DE LA APLICACIÓN ===
    SliceContext currentSlice;
    std::vector<std::string> datasetFiles;
    std::string currentDatasetPath;
    
    // DnCNN Denoiser (instancia persistente)
    Preprocessing::DnCNNDenoiser dncnnDenoiser;
    bool dncnnModelLoaded;
    
    // === MÉTODOS AUXILIARES ===
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
