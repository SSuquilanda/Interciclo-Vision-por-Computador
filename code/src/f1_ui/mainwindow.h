#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QSlider>
#include <QScrollArea>
#include <QPixmap>
#include <QImage>
#include <QTableWidget>
#include <QHeaderView>

#include <opencv2/core.hpp>
#include <string>
#include <vector>
#include <memory>

// Forward declarations
namespace Denoising {
    class DnCNNDenoiser;
}

#include "../f4_segmentation/segmentation.h"

// Estructura para manejar el estado del slice actual
struct SliceContext {
    // Imágenes en cada etapa del pipeline
    cv::Mat originalRaw;      // 16-bit DICOM original
    cv::Mat preprocessed;     // Resultado de F3 (Filtros tradicionales)
    cv::Mat preprocessedDnCNN; // Resultado de F3 (Red neuronal DnCNN)
    cv::Mat segmentationMask; // Resultado de F4 (Máscaras)
    cv::Mat segmentationOriginal; // Copia de segmentación antes de morfología
    cv::Mat finalOverlay;     // Resultado visual a color
    
    // Almacenamiento de regiones segmentadas por tipo de órgano
    std::vector<Segmentation::SegmentedRegion> pulmonesRegions;
    std::vector<Segmentation::SegmentedRegion> huesosRegions;
    std::vector<Segmentation::SegmentedRegion> aortaRegions;
    
    // Métricas
    double executionTimeMs = 0.0;
    double snrValue = 0.0;
    double processingTimeMs = 0.0;  // Tiempo total del pipeline
    
    // Flags de estado
    bool needsUpdate = true;  // Para evitar recálculos innecesarios
    bool isValid = false;     // Si el slice se cargó correctamente
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slots para acciones del menú
    void onOpenDataset();
    void onExportSlices();
    void onAbout();
    void onExit();

    // Slots para las pestañas
    void onTabChanged(int index);
    
    // Slots para navegación de slices
    void onSliceChanged(int sliceIndex);
    void onSpinBoxChanged(int value);
    
    // Slots para procesamiento
    void onProcessCurrentSlice();
    
    // Slots para preprocesamiento
    void onFilterChanged();
    void onPresetLungs();
    void onPresetBones();
    void onPresetSoftTissue();
    void onResetFilters();
    void onCompareWithDnCNN();
    
    // Slots para segmentación
    void onSegmentationChanged();
    void onSegPresetLungs();
    void onSegPresetBones();
    void onSegPresetAorta();
    void onResetSegmentation();
    
    // Slots para morfología
    void onMorphologyChanged();
    void onMorphPresetLungs();
    void onMorphPresetBones();
    void onMorphPresetAorta();
    void onResetMorphology();
    
    // Slots para visualización
    void onVisualizationChanged();
    void onSaveClicked();

private:
    // Métodos de inicialización
    void setupUI();
    void createMenuBar();
    void createToolBar();
    void createStatusBar();
    void createTabs();
    void createSliceNavigator();
    
    // Creación de pestañas individuales
    QWidget* createWelcomeTab();
    QWidget* createIOTab();
    QWidget* createPreprocessingTab();
    QWidget* createSegmentationTab();
    QWidget* createMorphologyTab();
    QWidget* createVisualizationTab();
    QWidget* createMetricsTab();
    
    // Métodos de procesamiento
    void loadDatasetFiles(const QString& dirPath);
    void loadSlice(int sliceIndex);
    void processCurrentSlice();
    void updateDisplay();
    void applyPreprocessing();
    void applySegmentation();
    void applyMorphology();
    void updateVisualization();
    void updateMetrics();
    
    // Métodos auxiliares
    QImage cvMatToQImage(const cv::Mat& mat);
    void displayImage(const cv::Mat& image);
    void updatePreprocessingDisplay();
    void updateSegmentationDisplay();
    void updateMorphologyDisplay();

    // Widgets principales
    QTabWidget *tabWidget;
    QStatusBar *statusBar;
    QToolBar *toolBar;
    
    // Acciones del menú
    QAction *actionOpenDataset;
    QAction *actionExportSlices;
    QAction *actionExit;
    QAction *actionAbout;
    
    // Widgets de información
    QLabel *lblStatus;
    QProgressBar *progressBar;
    QTextEdit *logOutput;
    
    // Widgets de navegación por slices
    QWidget *sliceNavigatorWidget;
    QSlider *sliceSlider;
    QSpinBox *sliceSpinBox;
    QLabel *sliceCountLabel;
    
    // Widgets de visualización
    QLabel *imageDisplayLabel;
    QScrollArea *imageScrollArea;
    
    // Widgets de preprocesamiento
    QLabel *imageBeforeLabel;
    QLabel *imageAfterLabel;
    QScrollArea *scrollBeforeArea;
    QScrollArea *scrollAfterArea;
    
    // Controles de filtros
    QCheckBox *checkGaussian;
    QSlider *sliderGaussianKernel;
    QLabel *lblGaussianValue;
    
    QCheckBox *checkMedian;
    QSlider *sliderMedianKernel;
    QLabel *lblMedianValue;
    
    QCheckBox *checkBilateral;
    QSlider *sliderBilateralD;
    QSlider *sliderBilateralSigma;
    QLabel *lblBilateralDValue;
    QLabel *lblBilateralSigmaValue;
    
    QCheckBox *checkCLAHE;
    QSlider *sliderCLAHEClip;
    QSlider *sliderCLAHETile;
    QLabel *lblCLAHEClipValue;
    QLabel *lblCLAHETileValue;
    
    QPushButton *btnPresetLungs;
    QPushButton *btnPresetBones;
    QPushButton *btnPresetSoftTissue;
    QPushButton *btnResetFilters;
    
    // Controles de DnCNN
    QCheckBox *checkDnCNN;
    QLabel *lblDnCNNStatus;
    QPushButton *btnCompareWithDnCNN;
    
    // Widgets de segmentación
    QLabel *imageSegBeforeLabel;
    QLabel *imageSegAfterLabel;
    QScrollArea *scrollSegBeforeArea;
    QScrollArea *scrollSegAfterArea;
    
    // Controles de segmentación
    QSlider *sliderMinHU;
    QSlider *sliderMaxHU;
    QSlider *sliderMinArea;
    QSlider *sliderMaxArea;
    QLabel *lblMinHUValue;
    QLabel *lblMaxHUValue;
    QLabel *lblMinAreaValue;
    QLabel *lblMaxAreaValue;
    
    QCheckBox *checkShowContours;
    QCheckBox *checkShowOverlay;
    QCheckBox *checkShowLabels;
    QCheckBox *checkFilterBorder;
    
    QPushButton *btnSegPresetLungs;
    QPushButton *btnSegPresetBones;
    QPushButton *btnSegPresetAorta;
    QPushButton *btnResetSegmentation;
    
    // Morphology tab widgets
    QLabel *imageMorphBeforeLabel;
    QLabel *imageMorphAfterLabel;
    QScrollArea *scrollMorphBeforeArea;
    QScrollArea *scrollMorphAfterArea;
    
    // Controles de morfología
    QCheckBox *checkErode;
    QCheckBox *checkDilate;
    QCheckBox *checkOpening;
    QCheckBox *checkClosing;
    QCheckBox *checkGradient;
    QCheckBox *checkFillHoles;
    QCheckBox *checkRemoveBorder;
    
    QSlider *sliderErodeKernel;
    QSlider *sliderDilateKernel;
    QSlider *sliderOpeningKernel;
    QSlider *sliderClosingKernel;
    QSlider *sliderGradientKernel;
    QSlider *sliderErodeIter;
    QSlider *sliderDilateIter;
    
    QComboBox *comboKernelShape;
    
    QLabel *lblErodeKernelValue;
    QLabel *lblDilateKernelValue;
    QLabel *lblOpeningKernelValue;
    QLabel *lblClosingKernelValue;
    QLabel *lblGradientKernelValue;
    QLabel *lblErodeIterValue;
    QLabel *lblDilateIterValue;
    
    QPushButton *btnMorphPresetLungs;
    QPushButton *btnMorphPresetBones;
    QPushButton *btnMorphPresetAorta;
    QPushButton *btnResetMorphology;
    
    // Visualization tab widgets
    QLabel *lblFinalView;
    QScrollArea *scrollFinalView;
    QCheckBox *chkShowLungs;
    QCheckBox *chkShowBones;
    QCheckBox *chkShowSoftTissue;
    QRadioButton *radStyleFill;
    QRadioButton *radStyleContour;
    QSlider *sliderOpacity;
    QLabel *lblOpacityValue;
    QPushButton *btnSaveFinal;
    
    // Metrics tab widgets
    QTableWidget *tableMetrics;
    QLabel *lblHistogramROI;
    QLabel *lblProcessTime;
    QLabel *lblMemoryUsage;
    
    // Variables de estado
    QString currentDatasetPath;
    bool datasetLoaded;
    
    // Datos del dataset
    std::vector<std::string> dicomFiles;  // Lista de archivos DICOM
    int currentSliceIndex;
    
    // Contexto del slice actual
    SliceContext sliceContext;
    
    // Red neuronal DnCNN
    std::unique_ptr<Denoising::DnCNNDenoiser> dncnnDenoiser;
};

#endif // MAINWINDOW_H
