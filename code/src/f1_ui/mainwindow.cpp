#include "f1_ui/mainwindow.h"
#include "f2_io/dicom_reader.h"
#include "utils/itk_opencv_bridge.h"
#include "f3_preprocessing/preprocessing.h"
#include "f3_preprocessing/denoising.h"
#include "f4_segmentation/segmentation.h"
#include "f5_morphology/morphology.h"

#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QLineEdit>
#include <QDir>
#include <QMenu>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , tabWidget(nullptr)
    , statusBar(nullptr)
    , toolBar(nullptr)
    , actionOpenDataset(nullptr)
    , actionExportSlices(nullptr)
    , actionExit(nullptr)
    , actionAbout(nullptr)
    , lblStatus(nullptr)
    , progressBar(nullptr)
    , logOutput(nullptr)
    , sliceNavigatorWidget(nullptr)
    , sliceSlider(nullptr)
    , sliceSpinBox(nullptr)
    , sliceCountLabel(nullptr)
    , imageDisplayLabel(nullptr)
    , imageScrollArea(nullptr)
    , imageBeforeLabel(nullptr)
    , imageAfterLabel(nullptr)
    , scrollBeforeArea(nullptr)
    , scrollAfterArea(nullptr)
    , checkGaussian(nullptr)
    , sliderGaussianKernel(nullptr)
    , lblGaussianValue(nullptr)
    , checkMedian(nullptr)
    , sliderMedianKernel(nullptr)
    , lblMedianValue(nullptr)
    , checkBilateral(nullptr)
    , sliderBilateralD(nullptr)
    , sliderBilateralSigma(nullptr)
    , lblBilateralDValue(nullptr)
    , lblBilateralSigmaValue(nullptr)
    , checkCLAHE(nullptr)
    , sliderCLAHEClip(nullptr)
    , sliderCLAHETile(nullptr)
    , lblCLAHEClipValue(nullptr)
    , lblCLAHETileValue(nullptr)
    , btnPresetLungs(nullptr)
    , btnPresetBones(nullptr)
    , btnPresetSoftTissue(nullptr)
    , btnResetFilters(nullptr)
    , checkDnCNN(nullptr)
    , lblDnCNNStatus(nullptr)
    , btnCompareWithDnCNN(nullptr)
    , imageSegBeforeLabel(nullptr)
    , imageSegAfterLabel(nullptr)
    , scrollSegBeforeArea(nullptr)
    , scrollSegAfterArea(nullptr)
    , sliderMinHU(nullptr)
    , sliderMaxHU(nullptr)
    , sliderMinArea(nullptr)
    , sliderMaxArea(nullptr)
    , lblMinHUValue(nullptr)
    , lblMaxHUValue(nullptr)
    , lblMinAreaValue(nullptr)
    , lblMaxAreaValue(nullptr)
    , checkShowContours(nullptr)
    , checkShowOverlay(nullptr)
    , checkShowLabels(nullptr)
    , checkFilterBorder(nullptr)
    , btnSegPresetLungs(nullptr)
    , btnSegPresetBones(nullptr)
    , btnSegPresetAorta(nullptr)
    , btnResetSegmentation(nullptr)
    , imageMorphBeforeLabel(nullptr)
    , imageMorphAfterLabel(nullptr)
    , scrollMorphBeforeArea(nullptr)
    , scrollMorphAfterArea(nullptr)
    , checkErode(nullptr)
    , checkDilate(nullptr)
    , checkOpening(nullptr)
    , checkClosing(nullptr)
    , checkGradient(nullptr)
    , checkFillHoles(nullptr)
    , checkRemoveBorder(nullptr)
    , sliderErodeKernel(nullptr)
    , sliderDilateKernel(nullptr)
    , sliderOpeningKernel(nullptr)
    , sliderClosingKernel(nullptr)
    , sliderGradientKernel(nullptr)
    , sliderErodeIter(nullptr)
    , sliderDilateIter(nullptr)
    , comboKernelShape(nullptr)
    , lblErodeKernelValue(nullptr)
    , lblDilateKernelValue(nullptr)
    , lblOpeningKernelValue(nullptr)
    , lblClosingKernelValue(nullptr)
    , lblGradientKernelValue(nullptr)
    , lblErodeIterValue(nullptr)
    , lblDilateIterValue(nullptr)
    , btnMorphPresetLungs(nullptr)
    , btnMorphPresetBones(nullptr)
    , btnMorphPresetAorta(nullptr)
    , btnResetMorphology(nullptr)
    , lblFinalView(nullptr)
    , scrollFinalView(nullptr)
    , chkShowLungs(nullptr)
    , chkShowBones(nullptr)
    , chkShowSoftTissue(nullptr)
    , radStyleFill(nullptr)
    , radStyleContour(nullptr)
    , sliderOpacity(nullptr)
    , lblOpacityValue(nullptr)
    , btnSaveFinal(nullptr)
    , tableMetrics(nullptr)
    , lblHistogramROI(nullptr)
    , lblProcessTime(nullptr)
    , lblMemoryUsage(nullptr)
    , datasetLoaded(false)
    , currentSliceIndex(0)
{
    setupUI();
    
    // Inicializar DnCNN Denoiser
    dncnnDenoiser = std::make_unique<Denoising::DnCNNDenoiser>();
    std::string modelPath = "../models/dncnn_grayscale.onnx";
    
    if (dncnnDenoiser->loadModel(modelPath)) {
        if (lblDnCNNStatus) {
            lblDnCNNStatus->setText("Estado: Modelo cargado");
            lblDnCNNStatus->setStyleSheet("QLabel { color: green; font-weight: bold; padding: 5px; }");
        }
        if (btnCompareWithDnCNN) {
            btnCompareWithDnCNN->setEnabled(true);
        }
        std::cout << "✓ Modelo DnCNN cargado exitosamente desde: " << modelPath << std::endl;
    } else {
        if (lblDnCNNStatus) {
            lblDnCNNStatus->setText("Estado: Modelo no encontrado");
            lblDnCNNStatus->setStyleSheet("QLabel { color: red; font-weight: bold; padding: 5px; }");
        }
        if (checkDnCNN) {
            checkDnCNN->setEnabled(false);
        }
        std::cerr << "✗ No se pudo cargar el modelo DnCNN desde: " << modelPath << std::endl;
    }
    
    // Configurar tamaño inicial (80% de la pantalla)
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = screenGeometry.width() * 0.8;
    int height = screenGeometry.height() * 0.8;
    resize(width, height);
    
    // Centrar ventana
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            screenGeometry
        )
    );
}

MainWindow::~MainWindow()
{
    // Los widgets Qt se limpian automáticamente por el sistema de padres
}

void MainWindow::setupUI()
{
    setWindowTitle("Proyecto Visión por Computador - CT Low Dose Analysis");
    
    createMenuBar();
    createToolBar();
    createTabs();
    createSliceNavigator();
    createStatusBar();
}

void MainWindow::createMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // Menú Archivo
    QMenu *fileMenu = menuBar->addMenu("&Archivo");
    
    actionOpenDataset = new QAction("&Abrir Dataset...", this);
    actionOpenDataset->setShortcut(QKeySequence::Open);
    actionOpenDataset->setStatusTip("Abrir un dataset DICOM");
    connect(actionOpenDataset, &QAction::triggered, this, &MainWindow::onOpenDataset);
    fileMenu->addAction(actionOpenDataset);
    
    actionExportSlices = new QAction("&Exportar Slices...", this);
    actionExportSlices->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    actionExportSlices->setStatusTip("Exportar slices como imágenes PNG");
    actionExportSlices->setEnabled(false);
    connect(actionExportSlices, &QAction::triggered, this, &MainWindow::onExportSlices);
    fileMenu->addAction(actionExportSlices);
    
    fileMenu->addSeparator();
    
    actionExit = new QAction("&Salir", this);
    actionExit->setShortcut(QKeySequence::Quit);
    actionExit->setStatusTip("Salir de la aplicación");
    connect(actionExit, &QAction::triggered, this, &MainWindow::onExit);
    fileMenu->addAction(actionExit);
    
    // Menú Ayuda
    QMenu *helpMenu = menuBar->addMenu("&Ayuda");
    
    actionAbout = new QAction("&Acerca de...", this);
    actionAbout->setStatusTip("Información sobre la aplicación");
    connect(actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(actionAbout);
}

void MainWindow::createToolBar()
{
    toolBar = addToolBar("Barra de Herramientas Principal");
    toolBar->setMovable(false);
    
    toolBar->addAction(actionOpenDataset);
    toolBar->addAction(actionExportSlices);
    toolBar->addSeparator();
}

void MainWindow::createStatusBar()
{
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    
    lblStatus = new QLabel("Listo");
    statusBar->addWidget(lblStatus, 1);
    
    progressBar = new QProgressBar();
    progressBar->setVisible(false);
    progressBar->setMaximumWidth(200);
    statusBar->addPermanentWidget(progressBar);
}

void MainWindow::createTabs()
{
    tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::North);
    tabWidget->setMovable(false);
    
    // Crear todas las pestañas
    tabWidget->addTab(createWelcomeTab(), "Inicio");
    tabWidget->addTab(createIOTab(), "I/O Dataset");
    tabWidget->addTab(createPreprocessingTab(), "Preprocesamiento");
    tabWidget->addTab(createSegmentationTab(), "Segmentación");
    tabWidget->addTab(createMorphologyTab(), "Morfología");
    tabWidget->addTab(createVisualizationTab(), "Visualización");
    tabWidget->addTab(createMetricsTab(), "Métricas");
    
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    
    setCentralWidget(tabWidget);
}

void MainWindow::createSliceNavigator()
{
    // Crear widget contenedor para el navegador de slices
    sliceNavigatorWidget = new QWidget(this);
    QHBoxLayout *navLayout = new QHBoxLayout(sliceNavigatorWidget);
    navLayout->setContentsMargins(10, 5, 10, 5);
    
    // Etiqueta de información
    QLabel *lblSlice = new QLabel("Slice:");
    navLayout->addWidget(lblSlice);
    
    // SpinBox para entrada directa
    sliceSpinBox = new QSpinBox();
    sliceSpinBox->setMinimum(0);
    sliceSpinBox->setMaximum(0);
    sliceSpinBox->setValue(0);
    sliceSpinBox->setEnabled(false);
    sliceSpinBox->setMinimumWidth(80);
    connect(sliceSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MainWindow::onSpinBoxChanged);
    navLayout->addWidget(sliceSpinBox);
    
    // Slider para navegación rápida
    sliceSlider = new QSlider(Qt::Horizontal);
    sliceSlider->setMinimum(0);
    sliceSlider->setMaximum(0);
    sliceSlider->setValue(0);
    sliceSlider->setEnabled(false);
    connect(sliceSlider, &QSlider::valueChanged, this, &MainWindow::onSliceChanged);
    navLayout->addWidget(sliceSlider, 1);
    
    // Contador de slices
    sliceCountLabel = new QLabel("0 / 0");
    sliceCountLabel->setMinimumWidth(80);
    sliceCountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    navLayout->addWidget(sliceCountLabel);
    
    // Agregar al layout principal como toolbar en la parte inferior
    QToolBar *bottomToolbar = new QToolBar("Navegación");
    bottomToolbar->addWidget(sliceNavigatorWidget);
    bottomToolbar->setMovable(false);
    addToolBar(Qt::BottomToolBarArea, bottomToolbar);
}

QWidget* MainWindow::createWelcomeTab()
{
    QWidget *welcomeWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(welcomeWidget);
    layout->setSpacing(20);
    
    // Título
    QLabel *titleLabel = new QLabel("<h1>Proyecto de Visión por Computador</h1>");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // Subtítulo
    QLabel *subtitleLabel = new QLabel("<h2>Análisis de CT Low Dose Reconstruction Dataset</h2>");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #666;");
    layout->addWidget(subtitleLabel);
    
    layout->addSpacing(30);
    
    // Información del proyecto
    QGroupBox *infoGroup = new QGroupBox("Información del Proyecto");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    
    QLabel *infoText = new QLabel(
        "<b>Dataset:</b> L291 CT Low Dose Reconstruction<br>"
        "<b>Modalidades:</b> Full Dose (FD) y Quarter Dose (QD)<br>"
        "<b>Slices por serie:</b> 343<br>"
        "<b>Resolución:</b> 512x512 píxeles<br><br>"
        "<b>Fases del proyecto:</b><br>"
        "• Fase 1: Interfaz de Usuario (Qt6)<br>"
        "• Fase 2: Entrada/Salida de Datos (DICOM/PNG)<br>"
        "• Fase 3: Preprocesamiento (Filtros y mejoras)<br>"
        "• Fase 4: Segmentación (Umbralización y contornos)<br>"
        "• Fase 5: Morfología (Operaciones morfológicas)<br>"
        "• Fase 6: Visualización (Renderizado 3D)<br>"
        "• Fase 7: Investigación (Análisis avanzado)<br>"
        "• Fase 8: Métricas (Evaluación y comparación)"
    );
    infoText->setWordWrap(true);
    infoLayout->addWidget(infoText);
    
    layout->addWidget(infoGroup);
    
    layout->addSpacing(20);
    
    // Botones de inicio rápido
    QGroupBox *quickStartGroup = new QGroupBox("Inicio Rápido");
    QVBoxLayout *quickStartLayout = new QVBoxLayout(quickStartGroup);
    
    QPushButton *btnOpenDataset = new QPushButton("Abrir Dataset DICOM");
    btnOpenDataset->setMinimumHeight(40);
    connect(btnOpenDataset, &QPushButton::clicked, this, &MainWindow::onOpenDataset);
    quickStartLayout->addWidget(btnOpenDataset);
    
    QPushButton *btnViewGuide = new QPushButton("Ver Guía de Uso");
    btnViewGuide->setMinimumHeight(40);
    btnViewGuide->setEnabled(false); // Por ahora deshabilitado
    quickStartLayout->addWidget(btnViewGuide);
    
    layout->addWidget(quickStartGroup);
    
    layout->addStretch();
    
    return welcomeWidget;
}

QWidget* MainWindow::createIOTab()
{
    QWidget *ioWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(ioWidget);
    
    // Panel izquierdo: Controles
    QWidget *controlPanel = new QWidget();
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlPanel->setMaximumWidth(400);
    
    // Sección de carga de dataset
    QGroupBox *loadGroup = new QGroupBox("Cargar Dataset");
    QVBoxLayout *loadLayout = new QVBoxLayout(loadGroup);
    
    QPushButton *btnLoadDataset = new QPushButton("Abrir Carpeta DICOM...");
    btnLoadDataset->setMinimumHeight(40);
    connect(btnLoadDataset, &QPushButton::clicked, this, &MainWindow::onOpenDataset);
    loadLayout->addWidget(btnLoadDataset);
    
    controlLayout->addWidget(loadGroup);
    
    // Sección de información del dataset
    QGroupBox *infoGroup = new QGroupBox("Información del Dataset");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    
    logOutput = new QTextEdit();
    logOutput->setReadOnly(true);
    logOutput->setPlaceholderText("La información del dataset aparecerá aquí...");
    logOutput->setMaximumHeight(150);
    infoLayout->addWidget(logOutput);
    
    controlLayout->addWidget(infoGroup);
    
    // Sección de exportación
    QGroupBox *exportGroup = new QGroupBox("Exportar Slices");
    QVBoxLayout *exportLayout = new QVBoxLayout(exportGroup);
    
    QHBoxLayout *exportOptionsLayout = new QHBoxLayout();
    QLabel *lblFormat = new QLabel("Formato:");
    QComboBox *cmbFormat = new QComboBox();
    cmbFormat->addItems({"PNG", "JPG", "BMP", "TIFF"});
    exportOptionsLayout->addWidget(lblFormat);
    exportOptionsLayout->addWidget(cmbFormat);
    exportOptionsLayout->addStretch();
    exportLayout->addLayout(exportOptionsLayout);
    
    QPushButton *btnExport = new QPushButton("Exportar Todos los Slices");
    btnExport->setMinimumHeight(35);
    btnExport->setEnabled(false);
    exportLayout->addWidget(btnExport);
    
    controlLayout->addWidget(exportGroup);
    controlLayout->addStretch();
    
    mainLayout->addWidget(controlPanel);
    
    // Panel derecho: Visualización
    QGroupBox *viewGroup = new QGroupBox("Vista del Slice Actual");
    QVBoxLayout *viewLayout = new QVBoxLayout(viewGroup);
    
    // ScrollArea para la imagen
    imageScrollArea = new QScrollArea();
    imageScrollArea->setWidgetResizable(true);
    imageScrollArea->setAlignment(Qt::AlignCenter);
    
    // Label para mostrar la imagen
    imageDisplayLabel = new QLabel("No hay dataset cargado");
    imageDisplayLabel->setAlignment(Qt::AlignCenter);
    imageDisplayLabel->setMinimumSize(512, 512);
    imageDisplayLabel->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    
    imageScrollArea->setWidget(imageDisplayLabel);
    viewLayout->addWidget(imageScrollArea);
    
    mainLayout->addWidget(viewGroup, 1);
    
    return ioWidget;
}

QWidget* MainWindow::createPreprocessingTab()
{
    QWidget *preprocessWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(preprocessWidget);
    
    // ========== PANEL IZQUIERDO: CONTROLES ==========
    QWidget *controlPanel = new QWidget();
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlPanel->setMaximumWidth(350);
    
    // --- Sección de Presets ---
    QGroupBox *presetGroup = new QGroupBox("Presets Optimizados");
    QVBoxLayout *presetLayout = new QVBoxLayout(presetGroup);
    
    btnPresetLungs = new QPushButton("🫁 Pulmones Óptimo");
    btnPresetLungs->setMinimumHeight(35);
    btnPresetLungs->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; }");
    connect(btnPresetLungs, &QPushButton::clicked, this, &MainWindow::onPresetLungs);
    presetLayout->addWidget(btnPresetLungs);
    
    btnPresetBones = new QPushButton("🦴 Huesos Óptimo");
    btnPresetBones->setMinimumHeight(35);
    btnPresetBones->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    connect(btnPresetBones, &QPushButton::clicked, this, &MainWindow::onPresetBones);
    presetLayout->addWidget(btnPresetBones);
    
    btnPresetSoftTissue = new QPushButton("🩸 Aorta Óptimo");
    btnPresetSoftTissue->setMinimumHeight(35);
    btnPresetSoftTissue->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
    connect(btnPresetSoftTissue, &QPushButton::clicked, this, &MainWindow::onPresetSoftTissue);
    presetLayout->addWidget(btnPresetSoftTissue);
    
    btnResetFilters = new QPushButton("↺ Resetear Filtros");
    btnResetFilters->setMinimumHeight(30);
    connect(btnResetFilters, &QPushButton::clicked, this, &MainWindow::onResetFilters);
    presetLayout->addWidget(btnResetFilters);
    
    controlLayout->addWidget(presetGroup);
    
    // --- Sección de Filtros ---
    QGroupBox *filtersGroup = new QGroupBox("Filtros de Preprocesamiento");
    QVBoxLayout *filtersLayout = new QVBoxLayout(filtersGroup);
    
    // Filtro Gaussiano
    checkGaussian = new QCheckBox("Filtro Gaussiano");
    connect(checkGaussian, &QCheckBox::stateChanged, this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(checkGaussian);
    
    QHBoxLayout *gaussLayout = new QHBoxLayout();
    gaussLayout->addWidget(new QLabel("Kernel:"));
    sliderGaussianKernel = new QSlider(Qt::Horizontal);
    sliderGaussianKernel->setMinimum(1);
    sliderGaussianKernel->setMaximum(15);
    sliderGaussianKernel->setValue(5);
    sliderGaussianKernel->setSingleStep(2);
    connect(sliderGaussianKernel, &QSlider::valueChanged, this, &MainWindow::onFilterChanged);
    gaussLayout->addWidget(sliderGaussianKernel);
    lblGaussianValue = new QLabel("5");
    lblGaussianValue->setMinimumWidth(30);
    gaussLayout->addWidget(lblGaussianValue);
    filtersLayout->addLayout(gaussLayout);
    filtersLayout->addSpacing(10);
    
    // Filtro Mediana
    checkMedian = new QCheckBox("Filtro Mediana");
    connect(checkMedian, &QCheckBox::stateChanged, this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(checkMedian);
    
    QHBoxLayout *medianLayout = new QHBoxLayout();
    medianLayout->addWidget(new QLabel("Kernel:"));
    sliderMedianKernel = new QSlider(Qt::Horizontal);
    sliderMedianKernel->setMinimum(1);
    sliderMedianKernel->setMaximum(15);
    sliderMedianKernel->setValue(5);
    sliderMedianKernel->setSingleStep(2);
    connect(sliderMedianKernel, &QSlider::valueChanged, this, &MainWindow::onFilterChanged);
    medianLayout->addWidget(sliderMedianKernel);
    lblMedianValue = new QLabel("5");
    lblMedianValue->setMinimumWidth(30);
    medianLayout->addWidget(lblMedianValue);
    filtersLayout->addLayout(medianLayout);
    filtersLayout->addSpacing(10);
    
    // Filtro Bilateral
    checkBilateral = new QCheckBox("Filtro Bilateral (preserva bordes)");
    connect(checkBilateral, &QCheckBox::stateChanged, this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(checkBilateral);
    
    QHBoxLayout *bilateralDLayout = new QHBoxLayout();
    bilateralDLayout->addWidget(new QLabel("Diámetro:"));
    sliderBilateralD = new QSlider(Qt::Horizontal);
    sliderBilateralD->setMinimum(3);
    sliderBilateralD->setMaximum(15);
    sliderBilateralD->setValue(9);
    connect(sliderBilateralD, &QSlider::valueChanged, this, &MainWindow::onFilterChanged);
    bilateralDLayout->addWidget(sliderBilateralD);
    lblBilateralDValue = new QLabel("9");
    lblBilateralDValue->setMinimumWidth(30);
    bilateralDLayout->addWidget(lblBilateralDValue);
    filtersLayout->addLayout(bilateralDLayout);
    
    QHBoxLayout *bilateralSigmaLayout = new QHBoxLayout();
    bilateralSigmaLayout->addWidget(new QLabel("Sigma:"));
    sliderBilateralSigma = new QSlider(Qt::Horizontal);
    sliderBilateralSigma->setMinimum(10);
    sliderBilateralSigma->setMaximum(150);
    sliderBilateralSigma->setValue(75);
    connect(sliderBilateralSigma, &QSlider::valueChanged, this, &MainWindow::onFilterChanged);
    bilateralSigmaLayout->addWidget(sliderBilateralSigma);
    lblBilateralSigmaValue = new QLabel("75");
    lblBilateralSigmaValue->setMinimumWidth(30);
    bilateralSigmaLayout->addWidget(lblBilateralSigmaValue);
    filtersLayout->addLayout(bilateralSigmaLayout);
    filtersLayout->addSpacing(10);
    
    // CLAHE (Mejora de Contraste)
    checkCLAHE = new QCheckBox("CLAHE (Mejora de Contraste)");
    connect(checkCLAHE, &QCheckBox::stateChanged, this, &MainWindow::onFilterChanged);
    filtersLayout->addWidget(checkCLAHE);
    
    QHBoxLayout *claheClipLayout = new QHBoxLayout();
    claheClipLayout->addWidget(new QLabel("Clip Limit:"));
    sliderCLAHEClip = new QSlider(Qt::Horizontal);
    sliderCLAHEClip->setMinimum(10);
    sliderCLAHEClip->setMaximum(50);
    sliderCLAHEClip->setValue(20);
    connect(sliderCLAHEClip, &QSlider::valueChanged, this, &MainWindow::onFilterChanged);
    claheClipLayout->addWidget(sliderCLAHEClip);
    lblCLAHEClipValue = new QLabel("2.0");
    lblCLAHEClipValue->setMinimumWidth(40);
    claheClipLayout->addWidget(lblCLAHEClipValue);
    filtersLayout->addLayout(claheClipLayout);
    
    QHBoxLayout *claheTileLayout = new QHBoxLayout();
    claheTileLayout->addWidget(new QLabel("Tile Size:"));
    sliderCLAHETile = new QSlider(Qt::Horizontal);
    sliderCLAHETile->setMinimum(4);
    sliderCLAHETile->setMaximum(16);
    sliderCLAHETile->setValue(8);
    connect(sliderCLAHETile, &QSlider::valueChanged, this, &MainWindow::onFilterChanged);
    claheTileLayout->addWidget(sliderCLAHETile);
    lblCLAHETileValue = new QLabel("8");
    lblCLAHETileValue->setMinimumWidth(40);
    claheTileLayout->addWidget(lblCLAHETileValue);
    filtersLayout->addLayout(claheTileLayout);
    
    controlLayout->addWidget(filtersGroup);
    
    // ========== GRUPO: RED NEURONAL DnCNN ==========
    QGroupBox *dncnnGroup = new QGroupBox("Red Neuronal DnCNN (Denoising)");
    QVBoxLayout *dncnnLayout = new QVBoxLayout(dncnnGroup);
    
    checkDnCNN = new QCheckBox("Usar DnCNN en lugar de filtros tradicionales");
    connect(checkDnCNN, &QCheckBox::stateChanged, this, &MainWindow::onFilterChanged);
    dncnnLayout->addWidget(checkDnCNN);
    
    lblDnCNNStatus = new QLabel("Estado: Modelo no cargado");
    lblDnCNNStatus->setStyleSheet("QLabel { color: red; font-weight: bold; padding: 5px; }");
    dncnnLayout->addWidget(lblDnCNNStatus);
    
    QLabel *infoLabel = new QLabel(
        "DnCNN es una red neuronal convolucional para reducción de ruido.<br>"
        "Compara el resultado con los filtros tradicionales en la vista previa."
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { font-size: 9pt; color: #666; }");
    dncnnLayout->addWidget(infoLabel);
    
    btnCompareWithDnCNN = new QPushButton("Ver Comparación Detallada");
    btnCompareWithDnCNN->setEnabled(false);
    connect(btnCompareWithDnCNN, &QPushButton::clicked, this, &MainWindow::onCompareWithDnCNN);
    dncnnLayout->addWidget(btnCompareWithDnCNN);
    
    controlLayout->addWidget(dncnnGroup);
    controlLayout->addStretch();
    
    mainLayout->addWidget(controlPanel);
    
    // ========== PANEL DERECHO: VISUALIZACIÓN BEFORE/AFTER ==========
    QWidget *viewPanel = new QWidget();
    QVBoxLayout *viewLayout = new QVBoxLayout(viewPanel);
    
    // Contenedor para las dos imágenes lado a lado
    QWidget *imagesContainer = new QWidget();
    QHBoxLayout *imagesLayout = new QHBoxLayout(imagesContainer);
    
    // Imagen ANTES (Izquierda)
    QGroupBox *beforeGroup = new QGroupBox("Imagen Original");
    QVBoxLayout *beforeLayout = new QVBoxLayout(beforeGroup);
    
    scrollBeforeArea = new QScrollArea();
    scrollBeforeArea->setWidgetResizable(true);
    scrollBeforeArea->setAlignment(Qt::AlignCenter);
    
    imageBeforeLabel = new QLabel("Cargue un dataset para ver la imagen");
    imageBeforeLabel->setAlignment(Qt::AlignCenter);
    imageBeforeLabel->setMinimumSize(400, 400);
    imageBeforeLabel->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    
    scrollBeforeArea->setWidget(imageBeforeLabel);
    beforeLayout->addWidget(scrollBeforeArea);
    imagesLayout->addWidget(beforeGroup);
    
    // Imagen DESPUÉS (Derecha)
    QGroupBox *afterGroup = new QGroupBox("Imagen Procesada");
    QVBoxLayout *afterLayout = new QVBoxLayout(afterGroup);
    
    scrollAfterArea = new QScrollArea();
    scrollAfterArea->setWidgetResizable(true);
    scrollAfterArea->setAlignment(Qt::AlignCenter);
    
    imageAfterLabel = new QLabel("Los filtros aparecerán aquí");
    imageAfterLabel->setAlignment(Qt::AlignCenter);
    imageAfterLabel->setMinimumSize(400, 400);
    imageAfterLabel->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    
    scrollAfterArea->setWidget(imageAfterLabel);
    afterLayout->addWidget(scrollAfterArea);
    imagesLayout->addWidget(afterGroup);
    
    viewLayout->addWidget(imagesContainer);
    
    mainLayout->addWidget(viewPanel, 1);
    
    return preprocessWidget;
}

QWidget* MainWindow::createSegmentationTab()
{
    QWidget *segmentWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(segmentWidget);
    
    // ========== PANEL IZQUIERDO: CONTROLES ==========
    QWidget *controlPanel = new QWidget();
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlPanel->setMaximumWidth(350);
    
    // --- Sección de Presets ---
    QGroupBox *presetGroup = new QGroupBox("Presets de Segmentación");
    QVBoxLayout *presetLayout = new QVBoxLayout(presetGroup);
    
    btnSegPresetLungs = new QPushButton("dddd Pulmones Óptimo");
    btnSegPresetLungs->setMinimumHeight(35);
    btnSegPresetLungs->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; }");
    connect(btnSegPresetLungs, &QPushButton::clicked, this, &MainWindow::onSegPresetLungs);
    presetLayout->addWidget(btnSegPresetLungs);
    
    btnSegPresetBones = new QPushButton("🦴 Huesos Óptimo");
    btnSegPresetBones->setMinimumHeight(35);
    btnSegPresetBones->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    connect(btnSegPresetBones, &QPushButton::clicked, this, &MainWindow::onSegPresetBones);
    presetLayout->addWidget(btnSegPresetBones);
    
    btnSegPresetAorta = new QPushButton("🩸 Aorta Óptimo");
    btnSegPresetAorta->setMinimumHeight(35);
    btnSegPresetAorta->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
    connect(btnSegPresetAorta, &QPushButton::clicked, this, &MainWindow::onSegPresetAorta);
    presetLayout->addWidget(btnSegPresetAorta);
    
    btnResetSegmentation = new QPushButton("↺ Resetear Segmentación");
    btnResetSegmentation->setMinimumHeight(30);
    connect(btnResetSegmentation, &QPushButton::clicked, this, &MainWindow::onResetSegmentation);
    presetLayout->addWidget(btnResetSegmentation);
    
    controlLayout->addWidget(presetGroup);
    
    // --- Sección de Umbrales HU ---
    QGroupBox *thresholdGroup = new QGroupBox("Umbrales de Unidades Hounsfield (HU)");
    QVBoxLayout *thresholdLayout = new QVBoxLayout(thresholdGroup);
    
    // HU Mínimo
    QHBoxLayout *minHULayout = new QHBoxLayout();
    minHULayout->addWidget(new QLabel("HU Mín:"));
    sliderMinHU = new QSlider(Qt::Horizontal);
    sliderMinHU->setMinimum(-1000);
    sliderMinHU->setMaximum(1000);
    sliderMinHU->setValue(-1000);
    connect(sliderMinHU, &QSlider::valueChanged, this, &MainWindow::onSegmentationChanged);
    minHULayout->addWidget(sliderMinHU);
    lblMinHUValue = new QLabel("-1000");
    lblMinHUValue->setMinimumWidth(50);
    minHULayout->addWidget(lblMinHUValue);
    thresholdLayout->addLayout(minHULayout);
    
    // HU Máximo
    QHBoxLayout *maxHULayout = new QHBoxLayout();
    maxHULayout->addWidget(new QLabel("HU Máx:"));
    sliderMaxHU = new QSlider(Qt::Horizontal);
    sliderMaxHU->setMinimum(-1000);
    sliderMaxHU->setMaximum(1000);
    sliderMaxHU->setValue(-400);
    connect(sliderMaxHU, &QSlider::valueChanged, this, &MainWindow::onSegmentationChanged);
    maxHULayout->addWidget(sliderMaxHU);
    lblMaxHUValue = new QLabel("-400");
    lblMaxHUValue->setMinimumWidth(50);
    maxHULayout->addWidget(lblMaxHUValue);
    thresholdLayout->addLayout(maxHULayout);
    
    QLabel *huInfo = new QLabel(
        "<small><b>Referencia HU:</b><br>"
        "• Aire/Pulmones: -1000 a -400<br>"
        "• Tejido blando: -100 a 100<br>"
        "• Huesos: 200 a 1000</small>"
    );
    huInfo->setWordWrap(true);
    huInfo->setStyleSheet("QLabel { color: #666; padding: 5px; }");
    thresholdLayout->addWidget(huInfo);
    
    controlLayout->addWidget(thresholdGroup);
    
    // --- Sección de Filtros de Área ---
    QGroupBox *areaGroup = new QGroupBox("Filtros de Área (píxeles)");
    QVBoxLayout *areaLayout = new QVBoxLayout(areaGroup);
    
    // Área Mínima
    QHBoxLayout *minAreaLayout = new QHBoxLayout();
    minAreaLayout->addWidget(new QLabel("Área Mín:"));
    sliderMinArea = new QSlider(Qt::Horizontal);
    sliderMinArea->setMinimum(0);
    sliderMinArea->setMaximum(10000);
    sliderMinArea->setValue(1000);
    sliderMinArea->setSingleStep(100);
    connect(sliderMinArea, &QSlider::valueChanged, this, &MainWindow::onSegmentationChanged);
    minAreaLayout->addWidget(sliderMinArea);
    lblMinAreaValue = new QLabel("1000");
    lblMinAreaValue->setMinimumWidth(60);
    minAreaLayout->addWidget(lblMinAreaValue);
    areaLayout->addLayout(minAreaLayout);
    
    // Área Máxima
    QHBoxLayout *maxAreaLayout = new QHBoxLayout();
    maxAreaLayout->addWidget(new QLabel("Área Máx:"));
    sliderMaxArea = new QSlider(Qt::Horizontal);
    sliderMaxArea->setMinimum(1000);
    sliderMaxArea->setMaximum(300000);
    sliderMaxArea->setValue(200000);
    sliderMaxArea->setSingleStep(1000);
    connect(sliderMaxArea, &QSlider::valueChanged, this, &MainWindow::onSegmentationChanged);
    maxAreaLayout->addWidget(sliderMaxArea);
    lblMaxAreaValue = new QLabel("200000");
    lblMaxAreaValue->setMinimumWidth(60);
    maxAreaLayout->addWidget(lblMaxAreaValue);
    areaLayout->addLayout(maxAreaLayout);
    
    controlLayout->addWidget(areaGroup);
    
    // --- Opciones de Visualización ---
    QGroupBox *vizGroup = new QGroupBox("Opciones de Visualización");
    QVBoxLayout *vizLayout = new QVBoxLayout(vizGroup);
    
    checkShowContours = new QCheckBox("Mostrar Contornos");
    checkShowContours->setChecked(true);
    connect(checkShowContours, &QCheckBox::toggled, this, &MainWindow::onSegmentationChanged);
    vizLayout->addWidget(checkShowContours);
    
    checkShowOverlay = new QCheckBox("Mostrar Overlay Semi-transparente");
    checkShowOverlay->setChecked(true);
    connect(checkShowOverlay, &QCheckBox::toggled, this, &MainWindow::onSegmentationChanged);
    vizLayout->addWidget(checkShowOverlay);
    
    checkShowLabels = new QCheckBox("Mostrar Etiquetas");
    checkShowLabels->setChecked(true);
    connect(checkShowLabels, &QCheckBox::toggled, this, &MainWindow::onSegmentationChanged);
    vizLayout->addWidget(checkShowLabels);
    
    checkFilterBorder = new QCheckBox("Filtrar Regiones en Borde");
    checkFilterBorder->setChecked(true);
    checkFilterBorder->setToolTip("Elimina regiones que tocan los bordes de la imagen (aire exterior)");
    connect(checkFilterBorder, &QCheckBox::toggled, this, &MainWindow::onSegmentationChanged);
    vizLayout->addWidget(checkFilterBorder);
    
    controlLayout->addWidget(vizGroup);
    controlLayout->addStretch();
    
    mainLayout->addWidget(controlPanel);
    
    // ========== PANEL DERECHO: VISUALIZACIÓN BEFORE/AFTER ==========
    QWidget *viewPanel = new QWidget();
    QVBoxLayout *viewLayout = new QVBoxLayout(viewPanel);
    
    // Contenedor para las dos imágenes lado a lado
    QWidget *imagesContainer = new QWidget();
    QHBoxLayout *imagesLayout = new QHBoxLayout(imagesContainer);
    
    // Imagen ANTES (Izquierda) - Imagen Preprocesada
    QGroupBox *beforeGroup = new QGroupBox("Imagen Preprocesada");
    QVBoxLayout *beforeLayout = new QVBoxLayout(beforeGroup);
    
    scrollSegBeforeArea = new QScrollArea();
    scrollSegBeforeArea->setWidgetResizable(true);
    scrollSegBeforeArea->setAlignment(Qt::AlignCenter);
    
    imageSegBeforeLabel = new QLabel("La imagen preprocesada aparecerá aquí");
    imageSegBeforeLabel->setAlignment(Qt::AlignCenter);
    imageSegBeforeLabel->setMinimumSize(400, 400);
    imageSegBeforeLabel->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    
    scrollSegBeforeArea->setWidget(imageSegBeforeLabel);
    beforeLayout->addWidget(scrollSegBeforeArea);
    imagesLayout->addWidget(beforeGroup);
    
    // Imagen DESPUÉS (Derecha) - Segmentación
    QGroupBox *afterGroup = new QGroupBox("Resultado de Segmentación");
    QVBoxLayout *afterLayout = new QVBoxLayout(afterGroup);
    
    scrollSegAfterArea = new QScrollArea();
    scrollSegAfterArea->setWidgetResizable(true);
    scrollSegAfterArea->setAlignment(Qt::AlignCenter);
    
    imageSegAfterLabel = new QLabel("La segmentación aparecerá aquí");
    imageSegAfterLabel->setAlignment(Qt::AlignCenter);
    imageSegAfterLabel->setMinimumSize(400, 400);
    imageSegAfterLabel->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    
    scrollSegAfterArea->setWidget(imageSegAfterLabel);
    afterLayout->addWidget(scrollSegAfterArea);
    imagesLayout->addWidget(afterGroup);
    
    viewLayout->addWidget(imagesContainer);
    
    mainLayout->addWidget(viewPanel, 1);
    
    return segmentWidget;
}

QWidget* MainWindow::createMorphologyTab()
{
    QWidget *morphWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(morphWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // ========== LEFT PANEL: Controls ==========
    QWidget *controlPanel = new QWidget();
    controlPanel->setMinimumWidth(380);
    controlPanel->setMaximumWidth(380);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setAlignment(Qt::AlignTop);

    QLabel *titleLabel = new QLabel("<b>Operaciones Morfológicas</b>");
    titleLabel->setAlignment(Qt::AlignCenter);
    controlLayout->addWidget(titleLabel);

    // Preset buttons
    QHBoxLayout *presetLayout = new QHBoxLayout();
    btnMorphPresetLungs = new QPushButton("dddd Pulmones Óptimo");
    btnMorphPresetLungs->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; }");
    
    btnMorphPresetBones = new QPushButton("🦴 Huesos Óptimo");
    btnMorphPresetBones->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    
    btnMorphPresetAorta = new QPushButton("🩸 Aorta Óptimo");
    btnMorphPresetAorta->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
    
    btnResetMorphology = new QPushButton("↺ Resetear");
    
    presetLayout->addWidget(btnMorphPresetLungs);
    presetLayout->addWidget(btnMorphPresetBones);
    presetLayout->addWidget(btnMorphPresetAorta);
    presetLayout->addWidget(btnResetMorphology);
    controlLayout->addLayout(presetLayout);

    connect(btnMorphPresetLungs, &QPushButton::clicked, this, &MainWindow::onMorphPresetLungs);
    connect(btnMorphPresetBones, &QPushButton::clicked, this, &MainWindow::onMorphPresetBones);
    connect(btnMorphPresetAorta, &QPushButton::clicked, this, &MainWindow::onMorphPresetAorta);
    connect(btnResetMorphology, &QPushButton::clicked, this, &MainWindow::onResetMorphology);

    controlLayout->addSpacing(10);

    // Kernel shape selector
    QHBoxLayout *shapeLayout = new QHBoxLayout();
    QLabel *labelShape = new QLabel("Forma del Kernel:");
    comboKernelShape = new QComboBox();
    comboKernelShape->addItem("Elipse", 0);
    comboKernelShape->addItem("Rectángulo", 1);
    comboKernelShape->addItem("Cruz", 2);
    shapeLayout->addWidget(labelShape);
    shapeLayout->addWidget(comboKernelShape);
    controlLayout->addLayout(shapeLayout);

    connect(comboKernelShape, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onMorphologyChanged);

    controlLayout->addSpacing(10);

    // Erosion controls
    checkErode = new QCheckBox("Erosión");
    controlLayout->addWidget(checkErode);
    
    QHBoxLayout *erodeKernelLayout = new QHBoxLayout();
    QLabel *labelErodeKernel = new QLabel("  Tamaño Kernel:");
    sliderErodeKernel = new QSlider(Qt::Horizontal);
    sliderErodeKernel->setRange(1, 15);
    sliderErodeKernel->setValue(3);
    lblErodeKernelValue = new QLabel("3");
    lblErodeKernelValue->setMinimumWidth(30);
    erodeKernelLayout->addWidget(labelErodeKernel);
    erodeKernelLayout->addWidget(sliderErodeKernel);
    erodeKernelLayout->addWidget(lblErodeKernelValue);
    controlLayout->addLayout(erodeKernelLayout);

    QHBoxLayout *erodeIterLayout = new QHBoxLayout();
    QLabel *labelErodeIter = new QLabel("  Iteraciones:");
    sliderErodeIter = new QSlider(Qt::Horizontal);
    sliderErodeIter->setRange(1, 10);
    sliderErodeIter->setValue(1);
    lblErodeIterValue = new QLabel("1");
    lblErodeIterValue->setMinimumWidth(30);
    erodeIterLayout->addWidget(labelErodeIter);
    erodeIterLayout->addWidget(sliderErodeIter);
    erodeIterLayout->addWidget(lblErodeIterValue);
    controlLayout->addLayout(erodeIterLayout);

    connect(checkErode, &QCheckBox::stateChanged, this, &MainWindow::onMorphologyChanged);
    connect(sliderErodeKernel, &QSlider::valueChanged, [this](int value) {
        if (value % 2 == 0) value++;
        sliderErodeKernel->blockSignals(true);
        sliderErodeKernel->setValue(value);
        sliderErodeKernel->blockSignals(false);
        lblErodeKernelValue->setText(QString::number(value));
        onMorphologyChanged();
    });
    connect(sliderErodeIter, &QSlider::valueChanged, [this](int value) {
        lblErodeIterValue->setText(QString::number(value));
        onMorphologyChanged();
    });

    controlLayout->addSpacing(5);

    // Dilation controls
    checkDilate = new QCheckBox("Dilatación");
    controlLayout->addWidget(checkDilate);
    
    QHBoxLayout *dilateKernelLayout = new QHBoxLayout();
    QLabel *labelDilateKernel = new QLabel("  Tamaño Kernel:");
    sliderDilateKernel = new QSlider(Qt::Horizontal);
    sliderDilateKernel->setRange(1, 15);
    sliderDilateKernel->setValue(3);
    lblDilateKernelValue = new QLabel("3");
    lblDilateKernelValue->setMinimumWidth(30);
    dilateKernelLayout->addWidget(labelDilateKernel);
    dilateKernelLayout->addWidget(sliderDilateKernel);
    dilateKernelLayout->addWidget(lblDilateKernelValue);
    controlLayout->addLayout(dilateKernelLayout);

    QHBoxLayout *dilateIterLayout = new QHBoxLayout();
    QLabel *labelDilateIter = new QLabel("  Iteraciones:");
    sliderDilateIter = new QSlider(Qt::Horizontal);
    sliderDilateIter->setRange(1, 10);
    sliderDilateIter->setValue(1);
    lblDilateIterValue = new QLabel("1");
    lblDilateIterValue->setMinimumWidth(30);
    dilateIterLayout->addWidget(labelDilateIter);
    dilateIterLayout->addWidget(sliderDilateIter);
    dilateIterLayout->addWidget(lblDilateIterValue);
    controlLayout->addLayout(dilateIterLayout);

    connect(checkDilate, &QCheckBox::stateChanged, this, &MainWindow::onMorphologyChanged);
    connect(sliderDilateKernel, &QSlider::valueChanged, [this](int value) {
        if (value % 2 == 0) value++;
        sliderDilateKernel->blockSignals(true);
        sliderDilateKernel->setValue(value);
        sliderDilateKernel->blockSignals(false);
        lblDilateKernelValue->setText(QString::number(value));
        onMorphologyChanged();
    });
    connect(sliderDilateIter, &QSlider::valueChanged, [this](int value) {
        lblDilateIterValue->setText(QString::number(value));
        onMorphologyChanged();
    });

    controlLayout->addSpacing(5);

    // Opening controls
    checkOpening = new QCheckBox("Apertura (Opening)");
    controlLayout->addWidget(checkOpening);
    
    QHBoxLayout *openingKernelLayout = new QHBoxLayout();
    QLabel *labelOpeningKernel = new QLabel("  Tamaño Kernel:");
    sliderOpeningKernel = new QSlider(Qt::Horizontal);
    sliderOpeningKernel->setRange(1, 15);
    sliderOpeningKernel->setValue(5);
    lblOpeningKernelValue = new QLabel("5");
    lblOpeningKernelValue->setMinimumWidth(30);
    openingKernelLayout->addWidget(labelOpeningKernel);
    openingKernelLayout->addWidget(sliderOpeningKernel);
    openingKernelLayout->addWidget(lblOpeningKernelValue);
    controlLayout->addLayout(openingKernelLayout);

    connect(checkOpening, &QCheckBox::stateChanged, this, &MainWindow::onMorphologyChanged);
    connect(sliderOpeningKernel, &QSlider::valueChanged, [this](int value) {
        if (value % 2 == 0) value++;
        sliderOpeningKernel->blockSignals(true);
        sliderOpeningKernel->setValue(value);
        sliderOpeningKernel->blockSignals(false);
        lblOpeningKernelValue->setText(QString::number(value));
        onMorphologyChanged();
    });

    controlLayout->addSpacing(5);

    // Closing controls
    checkClosing = new QCheckBox("Cierre (Closing)");
    controlLayout->addWidget(checkClosing);
    
    QHBoxLayout *closingKernelLayout = new QHBoxLayout();
    QLabel *labelClosingKernel = new QLabel("  Tamaño Kernel:");
    sliderClosingKernel = new QSlider(Qt::Horizontal);
    sliderClosingKernel->setRange(1, 15);
    sliderClosingKernel->setValue(9);
    lblClosingKernelValue = new QLabel("9");
    lblClosingKernelValue->setMinimumWidth(30);
    closingKernelLayout->addWidget(labelClosingKernel);
    closingKernelLayout->addWidget(sliderClosingKernel);
    closingKernelLayout->addWidget(lblClosingKernelValue);
    controlLayout->addLayout(closingKernelLayout);

    connect(checkClosing, &QCheckBox::stateChanged, this, &MainWindow::onMorphologyChanged);
    connect(sliderClosingKernel, &QSlider::valueChanged, [this](int value) {
        if (value % 2 == 0) value++;
        sliderClosingKernel->blockSignals(true);
        sliderClosingKernel->setValue(value);
        sliderClosingKernel->blockSignals(false);
        lblClosingKernelValue->setText(QString::number(value));
        onMorphologyChanged();
    });

    controlLayout->addSpacing(5);

    // Gradient controls
    checkGradient = new QCheckBox("Gradiente Morfológico");
    controlLayout->addWidget(checkGradient);
    
    QHBoxLayout *gradientKernelLayout = new QHBoxLayout();
    QLabel *labelGradientKernel = new QLabel("  Tamaño Kernel:");
    sliderGradientKernel = new QSlider(Qt::Horizontal);
    sliderGradientKernel->setRange(1, 15);
    sliderGradientKernel->setValue(3);
    lblGradientKernelValue = new QLabel("3");
    lblGradientKernelValue->setMinimumWidth(30);
    gradientKernelLayout->addWidget(labelGradientKernel);
    gradientKernelLayout->addWidget(sliderGradientKernel);
    gradientKernelLayout->addWidget(lblGradientKernelValue);
    controlLayout->addLayout(gradientKernelLayout);

    connect(checkGradient, &QCheckBox::stateChanged, this, &MainWindow::onMorphologyChanged);
    connect(sliderGradientKernel, &QSlider::valueChanged, [this](int value) {
        if (value % 2 == 0) value++;
        sliderGradientKernel->blockSignals(true);
        sliderGradientKernel->setValue(value);
        sliderGradientKernel->blockSignals(false);
        lblGradientKernelValue->setText(QString::number(value));
        onMorphologyChanged();
    });

    controlLayout->addSpacing(10);

    // Advanced operations
    QLabel *advancedLabel = new QLabel("<b>Operaciones Avanzadas</b>");
    advancedLabel->setAlignment(Qt::AlignCenter);
    controlLayout->addWidget(advancedLabel);

    checkFillHoles = new QCheckBox("Rellenar Huecos (Fill Holes)");
    checkRemoveBorder = new QCheckBox("Eliminar Bordes (Clear Border)");
    
    controlLayout->addWidget(checkFillHoles);
    controlLayout->addWidget(checkRemoveBorder);

    connect(checkFillHoles, &QCheckBox::stateChanged, this, &MainWindow::onMorphologyChanged);
    connect(checkRemoveBorder, &QCheckBox::stateChanged, this, &MainWindow::onMorphologyChanged);

    controlLayout->addStretch();
    mainLayout->addWidget(controlPanel);

    // ========== RIGHT PANEL: Before/After View ==========
    QWidget *viewPanel = new QWidget();
    QVBoxLayout *viewLayout = new QVBoxLayout(viewPanel);
    viewLayout->setSpacing(5);

    QLabel *viewTitle = new QLabel("<b>Vista: Antes / Después</b>");
    viewTitle->setAlignment(Qt::AlignCenter);
    viewLayout->addWidget(viewTitle);

    QHBoxLayout *imagesLayout = new QHBoxLayout();
    imagesLayout->setSpacing(10);

    // Left: Original (segmented)
    QVBoxLayout *originalLayout = new QVBoxLayout();
    QLabel *titleOriginal = new QLabel("<b>Segmentación Original</b>");
    titleOriginal->setAlignment(Qt::AlignCenter);
    originalLayout->addWidget(titleOriginal);
    
    scrollMorphBeforeArea = new QScrollArea();
    scrollMorphBeforeArea->setWidgetResizable(false);
    scrollMorphBeforeArea->setAlignment(Qt::AlignCenter);
    imageMorphBeforeLabel = new QLabel();
    imageMorphBeforeLabel->setAlignment(Qt::AlignCenter);
    imageMorphBeforeLabel->setText("Esperando segmentación...");
    scrollMorphBeforeArea->setWidget(imageMorphBeforeLabel);
    originalLayout->addWidget(scrollMorphBeforeArea);
    imagesLayout->addLayout(originalLayout);

    // Right: Morphology result
    QVBoxLayout *resultLayout = new QVBoxLayout();
    QLabel *titleResult = new QLabel("<b>Resultado Morfológico</b>");
    titleResult->setAlignment(Qt::AlignCenter);
    resultLayout->addWidget(titleResult);
    
    scrollMorphAfterArea = new QScrollArea();
    scrollMorphAfterArea->setWidgetResizable(false);
    scrollMorphAfterArea->setAlignment(Qt::AlignCenter);
    imageMorphAfterLabel = new QLabel();
    imageMorphAfterLabel->setAlignment(Qt::AlignCenter);
    imageMorphAfterLabel->setText("Resultado aparecerá aquí");
    scrollMorphAfterArea->setWidget(imageMorphAfterLabel);
    resultLayout->addWidget(scrollMorphAfterArea);
    imagesLayout->addLayout(resultLayout);

    viewLayout->addLayout(imagesLayout);
    mainLayout->addWidget(viewPanel, 1);

    return morphWidget;
}

QWidget* MainWindow::createVisualizationTab()
{
    QWidget *vizWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(vizWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // ========== LEFT PANEL: Controls ==========
    QWidget *controlPanel = new QWidget();
    controlPanel->setMinimumWidth(300);
    controlPanel->setMaximumWidth(300);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setAlignment(Qt::AlignTop);

    QLabel *titleLabel = new QLabel("<b>Visualización Final</b>");
    titleLabel->setAlignment(Qt::AlignCenter);
    controlLayout->addWidget(titleLabel);

    controlLayout->addSpacing(10);

    // Group: Capas de Órganos
    QGroupBox *grpLayers = new QGroupBox("Capas de Órganos");
    QVBoxLayout *layersLayout = new QVBoxLayout(grpLayers);

    chkShowLungs = new QCheckBox("Pulmones (Azul)");
    chkShowLungs->setChecked(true);
    layersLayout->addWidget(chkShowLungs);

    chkShowBones = new QCheckBox("Huesos (Verde)");
    chkShowBones->setChecked(false);
    chkShowBones->setEnabled(false); // Por ahora deshabilitado
    layersLayout->addWidget(chkShowBones);

    chkShowSoftTissue = new QCheckBox("🩸 Aorta (Rojo)");
    chkShowSoftTissue->setChecked(true);
    chkShowSoftTissue->setEnabled(true); // Habilitado para visualizar aorta
    layersLayout->addWidget(chkShowSoftTissue);

    connect(chkShowLungs, &QCheckBox::stateChanged, this, &MainWindow::onVisualizationChanged);
    connect(chkShowBones, &QCheckBox::stateChanged, this, &MainWindow::onVisualizationChanged);
    connect(chkShowSoftTissue, &QCheckBox::stateChanged, this, &MainWindow::onVisualizationChanged);

    controlLayout->addWidget(grpLayers);
    controlLayout->addSpacing(10);

    // Group: Apariencia
    QGroupBox *grpStyle = new QGroupBox("Apariencia");
    QVBoxLayout *styleLayout = new QVBoxLayout(grpStyle);

    radStyleFill = new QRadioButton("Relleno Sólido");
    radStyleFill->setChecked(true);
    styleLayout->addWidget(radStyleFill);

    radStyleContour = new QRadioButton("Solo Contornos");
    styleLayout->addWidget(radStyleContour);

    connect(radStyleFill, &QRadioButton::toggled, this, &MainWindow::onVisualizationChanged);
    connect(radStyleContour, &QRadioButton::toggled, this, &MainWindow::onVisualizationChanged);

    styleLayout->addSpacing(10);

    QLabel *lblOpacity = new QLabel("Opacidad del Color:");
    styleLayout->addWidget(lblOpacity);

    QHBoxLayout *opacityLayout = new QHBoxLayout();
    sliderOpacity = new QSlider(Qt::Horizontal);
    sliderOpacity->setRange(0, 100);
    sliderOpacity->setValue(40);
    lblOpacityValue = new QLabel("40%");
    lblOpacityValue->setMinimumWidth(40);
    opacityLayout->addWidget(sliderOpacity);
    opacityLayout->addWidget(lblOpacityValue);
    styleLayout->addLayout(opacityLayout);

    connect(sliderOpacity, &QSlider::valueChanged, [this](int value) {
        lblOpacityValue->setText(QString("%1%").arg(value));
        onVisualizationChanged();
    });

    controlLayout->addWidget(grpStyle);
    controlLayout->addSpacing(20);

    // Botón Guardar
    btnSaveFinal = new QPushButton("💾 Guardar Imagen Final");
    btnSaveFinal->setMinimumHeight(40);
    connect(btnSaveFinal, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    controlLayout->addWidget(btnSaveFinal);

    controlLayout->addStretch();
    mainLayout->addWidget(controlPanel);

    // ========== RIGHT PANEL: Viewer ==========
    QWidget *viewPanel = new QWidget();
    QVBoxLayout *viewLayout = new QVBoxLayout(viewPanel);
    viewLayout->setSpacing(5);

    QLabel *viewTitle = new QLabel("<b>Vista Final con Overlay</b>");
    viewTitle->setAlignment(Qt::AlignCenter);
    viewLayout->addWidget(viewTitle);

    scrollFinalView = new QScrollArea();
    scrollFinalView->setWidgetResizable(false);
    scrollFinalView->setAlignment(Qt::AlignCenter);
    scrollFinalView->setStyleSheet("QScrollArea { background-color: #2b2b2b; }");
    
    lblFinalView = new QLabel();
    lblFinalView->setAlignment(Qt::AlignCenter);
    lblFinalView->setText("Esperando segmentación y morfología...");
    lblFinalView->setStyleSheet("QLabel { color: white; }");
    scrollFinalView->setWidget(lblFinalView);
    
    viewLayout->addWidget(scrollFinalView);
    mainLayout->addWidget(viewPanel, 1);

    return vizWidget;
}

QWidget* MainWindow::createMetricsTab()
{
    QWidget *metricsWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(metricsWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Grupo 1: Análisis Cuantitativo
    QGroupBox *groupQuantitative = new QGroupBox("Análisis Cuantitativo");
    QVBoxLayout *quantLayout = new QVBoxLayout(groupQuantitative);
    
    tableMetrics = new QTableWidget();
    tableMetrics->setColumnCount(4);
    tableMetrics->setHorizontalHeaderLabels({"Estructura", "Área (px)", "Densidad Media (HU)", "Desviación (Ruido)"});
    tableMetrics->horizontalHeader()->setStretchLastSection(true);
    tableMetrics->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableMetrics->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableMetrics->setMinimumHeight(150);
    quantLayout->addWidget(tableMetrics);
    
    mainLayout->addWidget(groupQuantitative);
    
    // Grupo 2: Histograma de la ROI
    QGroupBox *groupHistogram = new QGroupBox("Histograma de la ROI");
    QVBoxLayout *histLayout = new QVBoxLayout(groupHistogram);
    
    lblHistogramROI = new QLabel();
    lblHistogramROI->setAlignment(Qt::AlignCenter);
    lblHistogramROI->setMinimumSize(512, 300);
    lblHistogramROI->setStyleSheet("QLabel { background-color: white; border: 1px solid #cccccc; }");
    lblHistogramROI->setText("El histograma se mostrará aquí después de la segmentación");
    histLayout->addWidget(lblHistogramROI);
    
    mainLayout->addWidget(groupHistogram);
    
    // Grupo 3: Rendimiento del Sistema
    QGroupBox *groupPerformance = new QGroupBox("Rendimiento del Sistema");
    QVBoxLayout *perfLayout = new QVBoxLayout(groupPerformance);
    
    lblProcessTime = new QLabel("Tiempo de Pipeline: 0 ms");
    lblProcessTime->setStyleSheet("QLabel { font-size: 12pt; padding: 5px; }");
    perfLayout->addWidget(lblProcessTime);
    
    lblMemoryUsage = new QLabel("Memoria Estimada: 0 MB");
    lblMemoryUsage->setStyleSheet("QLabel { font-size: 12pt; padding: 5px; }");
    perfLayout->addWidget(lblMemoryUsage);
    
    mainLayout->addWidget(groupPerformance);
    
    mainLayout->addStretch();
    
    return metricsWidget;
}

// Slots para acciones del menú
void MainWindow::onOpenDataset()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Seleccionar carpeta con archivos DICOM",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!dir.isEmpty()) {
        currentDatasetPath = dir;
        loadDatasetFiles(dir);
    }
}

void MainWindow::onExportSlices()
{
    if (!datasetLoaded) {
        QMessageBox::warning(
            this,
            "Advertencia",
            "Primero debe cargar un dataset."
        );
        return;
    }
    
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Seleccionar carpeta de destino para exportación",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!dir.isEmpty()) {
        QMessageBox::information(
            this,
            "Exportación",
            "Funcionalidad de exportación en desarrollo.\n"
            "Se exportarán los slices a: " + dir
        );
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(
        this,
        "Acerca de",
        "<h2>Proyecto Visión por Computador</h2>"
        "<p><b>Versión:</b> 1.0.0</p>"
        "<p><b>Dataset:</b> CT Low Dose Reconstruction L291</p>"
        "<p><b>Tecnologías:</b></p>"
        "<ul>"
        "<li>Qt 6.10.0</li>"
        "<li>OpenCV 4.10.0</li>"
        "<li>ITK 6.0.0</li>"
        "<li>C++17</li>"
        "</ul>"
        "<p>Aplicación para análisis y procesamiento de imágenes médicas DICOM.</p>"
    );
}

void MainWindow::onExit()
{
    QApplication::quit();
}

void MainWindow::onTabChanged(int index)
{
    lblStatus->setText("Pestaña activa: " + tabWidget->tabText(index));
    
    // Si hay un dataset cargado y cambiamos de pestaña, actualizar la vista
    if (datasetLoaded && sliceContext.isValid) {
        QString tabName = tabWidget->tabText(index);
        
        if (index == 2) {
            // Pestaña de Preprocesamiento
            applyPreprocessing();
        } else if (index == 3) {
            // Pestaña de Segmentación
            // Primero asegurarse de que el preprocesamiento esté aplicado
            if (sliceContext.preprocessed.empty()) {
                applyPreprocessing();
            }
            applySegmentation();
        } else if (index == 4) {
            // Pestaña de Morfología
            // Asegurarse de que haya segmentación
            if (sliceContext.segmentationMask.empty()) {
                if (sliceContext.preprocessed.empty()) {
                    applyPreprocessing();
                }
                applySegmentation();
            }
            // Actualizar display primero para mostrar la segmentación original
            updateMorphologyDisplay();
            // Luego aplicar morfología si hay operaciones activas
            if ((checkErode && checkErode->isChecked()) ||
                (checkDilate && checkDilate->isChecked()) ||
                (checkOpening && checkOpening->isChecked()) ||
                (checkClosing && checkClosing->isChecked()) ||
                (checkGradient && checkGradient->isChecked()) ||
                (checkFillHoles && checkFillHoles->isChecked()) ||
                (checkRemoveBorder && checkRemoveBorder->isChecked())) {
                applyMorphology();
            }
        } else if (index == 5) {
            // Pestaña de Visualización
            // Asegurarse de que todo el pipeline esté completo
            if (sliceContext.preprocessed.empty()) {
                applyPreprocessing();
            }
            if (sliceContext.segmentationMask.empty()) {
                applySegmentation();
            }
            // Actualizar visualización final
            updateVisualization();
        } else if (index == 6) {
            // Pestaña de Métricas
            // Asegurarse de que todo el pipeline esté completo
            if (sliceContext.preprocessed.empty()) {
                applyPreprocessing();
            }
            if (sliceContext.segmentationMask.empty()) {
                applySegmentation();
            }
            // Actualizar métricas
            updateMetrics();
        } else {
            sliceContext.needsUpdate = true;
            processCurrentSlice();
        }
    }
}

void MainWindow::onSliceChanged(int sliceIndex)
{
    if (!datasetLoaded || sliceIndex < 0 || sliceIndex >= static_cast<int>(dicomFiles.size())) {
        return;
    }
    
    currentSliceIndex = sliceIndex;
    
    // Actualizar el spinbox sin disparar su señal
    sliceSpinBox->blockSignals(true);
    sliceSpinBox->setValue(sliceIndex);
    sliceSpinBox->blockSignals(false);
    
    // Actualizar el contador
    sliceCountLabel->setText(QString("%1 / %2").arg(sliceIndex + 1).arg(dicomFiles.size()));
    
    // Cargar el nuevo slice
    loadSlice(sliceIndex);
}

void MainWindow::onSpinBoxChanged(int value)
{
    // Actualizar el slider sin disparar su señal
    sliceSlider->blockSignals(true);
    sliceSlider->setValue(value);
    sliceSlider->blockSignals(false);
    
    // Cargar el slice
    onSliceChanged(value);
}

void MainWindow::onProcessCurrentSlice()
{
    if (!datasetLoaded || !sliceContext.isValid) {
        return;
    }
    
    processCurrentSlice();
}

// Métodos de procesamiento
void MainWindow::loadDatasetFiles(const QString& dirPath)
{
    try {
        dicomFiles.clear();
        
        // Buscar todos los archivos .IMA o .dcm en el directorio
        fs::path dir(dirPath.toStdString());
        
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                
                if (ext == ".ima" || ext == ".dcm") {
                    dicomFiles.push_back(entry.path().string());
                }
            }
        }
        
        // Ordenar los archivos alfabéticamente
        std::sort(dicomFiles.begin(), dicomFiles.end());
        
        if (dicomFiles.empty()) {
            QMessageBox::warning(this, "Error", 
                "No se encontraron archivos DICOM (.IMA o .dcm) en el directorio seleccionado.");
            return;
        }
        
        // Actualizar UI
        datasetLoaded = true;
        actionExportSlices->setEnabled(true);
        
        // Configurar el slider y spinbox
        int maxIndex = static_cast<int>(dicomFiles.size()) - 1;
        sliceSlider->setMaximum(maxIndex);
        sliceSlider->setValue(0);
        sliceSlider->setEnabled(true);
        
        sliceSpinBox->setMaximum(maxIndex);
        sliceSpinBox->setValue(0);
        sliceSpinBox->setEnabled(true);
        
        sliceCountLabel->setText(QString("1 / %1").arg(dicomFiles.size()));
        
        // Actualizar log
        logOutput->clear();
        logOutput->append("<b>Dataset cargado exitosamente</b>");
        logOutput->append(QString("Ruta: %1").arg(dirPath));
        logOutput->append(QString("Archivos encontrados: %1 slices DICOM").arg(dicomFiles.size()));
        logOutput->append(QString("\nPrimer archivo: %1").arg(QString::fromStdString(
            fs::path(dicomFiles[0]).filename().string())));
        logOutput->append(QString("Último archivo: %1").arg(QString::fromStdString(
            fs::path(dicomFiles.back()).filename().string())));
        
        lblStatus->setText(QString("Dataset cargado: %1 slices").arg(dicomFiles.size()));
        
        // Cargar el primer slice
        currentSliceIndex = 0;
        loadSlice(0);
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", 
            QString("Error al cargar el dataset: %1").arg(e.what()));
    }
}

void MainWindow::loadSlice(int sliceIndex)
{
    if (sliceIndex < 0 || sliceIndex >= static_cast<int>(dicomFiles.size())) {
        return;
    }
    
    // Iniciar medición de tiempo
    cv::TickMeter timer;
    timer.start();
    
    try {
        // Leer el archivo DICOM usando ITK
        std::string dicomPath = dicomFiles[sliceIndex];
        DicomIO::ImagePointer itkImage = DicomIO::readDicomImage(dicomPath);
        
        // Convertir de ITK a OpenCV
        sliceContext.originalRaw = Bridge::itkToOpenCV(itkImage);
        
        if (sliceContext.originalRaw.empty()) {
            throw std::runtime_error("Error al convertir imagen ITK a OpenCV");
        }
        
        // Limpiar vectores de regiones al cargar nuevo slice
        sliceContext.pulmonesRegions.clear();
        sliceContext.huesosRegions.clear();
        sliceContext.aortaRegions.clear();
        
        sliceContext.isValid = true;
        sliceContext.needsUpdate = true;
        
        // Actualizar status
        std::string filename = fs::path(dicomPath).filename().string();
        lblStatus->setText(QString("Slice %1/%2: %3")
            .arg(sliceIndex + 1)
            .arg(dicomFiles.size())
            .arg(QString::fromStdString(filename)));
        
        // Procesar y visualizar según la pestaña activa
        int currentTab = tabWidget->currentIndex();
        if (currentTab == 2) { // Pestaña de Preprocesamiento
            applyPreprocessing();
        } else if (currentTab == 3) { // Pestaña de Segmentación
            if (sliceContext.preprocessed.empty()) {
                applyPreprocessing();
            }
            applySegmentation();
        } else if (currentTab == 4) { // Pestaña de Morfología
            if (sliceContext.preprocessed.empty()) {
                applyPreprocessing();
            }
            if (sliceContext.segmentationMask.empty()) {
                applySegmentation();
            }
            applyMorphology();
        } else if (currentTab == 5) { // Pestaña de Visualización
            if (sliceContext.preprocessed.empty()) {
                applyPreprocessing();
            }
            if (sliceContext.segmentationMask.empty()) {
                applySegmentation();
            }
            updateVisualization();
        } else if (currentTab == 6) { // Pestaña de Métricas
            if (sliceContext.preprocessed.empty()) {
                applyPreprocessing();
            }
            if (sliceContext.segmentationMask.empty()) {
                applySegmentation();
            }
            updateMetrics();
        } else {
            processCurrentSlice();
        }
        
        // Detener timer y guardar tiempo
        timer.stop();
        sliceContext.processingTimeMs = timer.getTimeMilli();
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error al cargar slice",
            QString("No se pudo cargar el slice: %1").arg(e.what()));
        sliceContext.isValid = false;
        sliceContext.processingTimeMs = 0.0;
    }
}

void MainWindow::processCurrentSlice()
{
    if (!sliceContext.isValid || !sliceContext.needsUpdate) {
        return;
    }
    
    // Por ahora solo mostramos la imagen original normalizada
    // En el futuro aquí irá la lógica de preprocesamiento y segmentación
    
    updateDisplay();
    sliceContext.needsUpdate = false;
}

void MainWindow::updateDisplay()
{
    if (!sliceContext.isValid || sliceContext.originalRaw.empty()) {
        return;
    }
    
    // Normalizar la imagen de 16-bit a 8-bit para visualización
    cv::Mat display8bit = Bridge::normalize16to8bit(sliceContext.originalRaw);
    
    // Mostrar la imagen
    displayImage(display8bit);
}

QImage MainWindow::cvMatToQImage(const cv::Mat& mat)
{
    if (mat.empty()) {
        return QImage();
    }
    
    // Convertir según el tipo de imagen
    if (mat.type() == CV_8UC1) {
        // Escala de grises
        return QImage(mat.data, mat.cols, mat.rows, 
                     static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
    } else if (mat.type() == CV_8UC3) {
        // BGR a RGB
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows,
                     static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
    }
    
    return QImage();
}

void MainWindow::displayImage(const cv::Mat& image)
{
    if (image.empty()) {
        imageDisplayLabel->setText("Error al cargar la imagen");
        return;
    }
    
    QImage qimg = cvMatToQImage(image);
    if (qimg.isNull()) {
        imageDisplayLabel->setText("Error al convertir la imagen");
        return;
    }
    
    // Mostrar la imagen en el label
    QPixmap pixmap = QPixmap::fromImage(qimg);
    imageDisplayLabel->setPixmap(pixmap);
    imageDisplayLabel->resize(pixmap.size());
}

// ========== MÉTODOS DE PREPROCESAMIENTO ==========

void MainWindow::applyPreprocessing()
{
    if (!sliceContext.isValid || sliceContext.originalRaw.empty()) {
        return;
    }
    
    // Partir de la imagen normalizada a 8-bit
    cv::Mat current = Bridge::normalize16to8bit(sliceContext.originalRaw);
    
    // DECISIÓN: ¿Usar DnCNN o filtros tradicionales?
    bool useDnCNN = checkDnCNN && checkDnCNN->isChecked() && 
                    dncnnDenoiser && dncnnDenoiser->isLoaded();
    
    if (useDnCNN) {
        // ===== OPCIÓN A: RED NEURONAL DnCNN =====
        std::cout << "Aplicando DnCNN para denoising..." << std::endl;
        
        cv::TickMeter timer;
        timer.start();
        
        sliceContext.preprocessedDnCNN = dncnnDenoiser->denoise(current);
        sliceContext.preprocessed = sliceContext.preprocessedDnCNN.clone();
        
        timer.stop();
        std::cout << "  Tiempo DnCNN: " << timer.getTimeMilli() << " ms" << std::endl;
        
    } else {
        // ===== OPCIÓN B: FILTROS TRADICIONALES =====
        
        // 1. Filtro Gaussiano
        if (checkGaussian && checkGaussian->isChecked()) {
            int kernelSize = sliderGaussianKernel->value();
            if (kernelSize % 2 == 0) kernelSize++;
            current = Preprocessing::applyGaussianBlur(current, kernelSize);
        }
        
        // 2. Filtro Mediana
        if (checkMedian && checkMedian->isChecked()) {
            int kernelSize = sliderMedianKernel->value();
            if (kernelSize % 2 == 0) kernelSize++;
            current = Preprocessing::applyMedianFilter(current, kernelSize);
        }
        
        // 3. Filtro Bilateral
        if (checkBilateral && checkBilateral->isChecked()) {
            int d = sliderBilateralD->value();
            double sigma = sliderBilateralSigma->value();
            current = Preprocessing::applyBilateralFilter(current, d, sigma, sigma);
        }
        
        // 4. CLAHE (mejora de contraste)
        if (checkCLAHE && checkCLAHE->isChecked()) {
            double clipLimit = sliderCLAHEClip->value() / 10.0;
            int tileSize = sliderCLAHETile->value();
            current = Preprocessing::applyCLAHE(current, clipLimit, cv::Size(tileSize, tileSize));
        }
        
        sliceContext.preprocessed = current;
    }
    
    // Actualizar la visualización
    updatePreprocessingDisplay();
}

void MainWindow::updatePreprocessingDisplay()
{
    if (!sliceContext.isValid || sliceContext.originalRaw.empty()) {
        return;
    }
    
    // Imagen ANTES (normalizada)
    cv::Mat imageBefore = Bridge::normalize16to8bit(sliceContext.originalRaw);
    QImage qimgBefore = cvMatToQImage(imageBefore);
    if (!qimgBefore.isNull() && imageBeforeLabel) {
        QPixmap pixmapBefore = QPixmap::fromImage(qimgBefore);
        imageBeforeLabel->setPixmap(pixmapBefore);
        imageBeforeLabel->resize(pixmapBefore.size());
    }
    
    // Imagen DESPUÉS (procesada)
    if (!sliceContext.preprocessed.empty() && imageAfterLabel) {
        QImage qimgAfter = cvMatToQImage(sliceContext.preprocessed);
        if (!qimgAfter.isNull()) {
            QPixmap pixmapAfter = QPixmap::fromImage(qimgAfter);
            imageAfterLabel->setPixmap(pixmapAfter);
            imageAfterLabel->resize(pixmapAfter.size());
        }
    }
}

void MainWindow::onFilterChanged()
{
    // Actualizar labels de valores
    if (lblGaussianValue) {
        int val = sliderGaussianKernel->value();
        if (val % 2 == 0) val++;
        lblGaussianValue->setText(QString::number(val));
    }
    
    if (lblMedianValue) {
        int val = sliderMedianKernel->value();
        if (val % 2 == 0) val++;
        lblMedianValue->setText(QString::number(val));
    }
    
    if (lblBilateralDValue) {
        lblBilateralDValue->setText(QString::number(sliderBilateralD->value()));
    }
    
    if (lblBilateralSigmaValue) {
        lblBilateralSigmaValue->setText(QString::number(sliderBilateralSigma->value()));
    }
    
    if (lblCLAHEClipValue) {
        double val = sliderCLAHEClip->value() / 10.0;
        lblCLAHEClipValue->setText(QString::number(val, 'f', 1));
    }
    
    if (lblCLAHETileValue) {
        lblCLAHETileValue->setText(QString::number(sliderCLAHETile->value()));
    }
    
    // Aplicar preprocesamiento si hay datos cargados
    if (datasetLoaded && sliceContext.isValid) {
        applyPreprocessing();
    }
}

void MainWindow::onPresetLungs()
{
    // Preset optimizado para pulmones (basado en pipeline_pulmones.cpp)
    // NOTA: pipeline_pulmones.cpp NO usa preprocesamiento, solo CLAHE para visualización
    
    bool prevState = blockSignals(true);
    
    // Desactivar todos los filtros primero
    if (checkGaussian) checkGaussian->setChecked(false);
    if (checkMedian) checkMedian->setChecked(false);
    if (checkBilateral) checkBilateral->setChecked(false);
    if (checkCLAHE) checkCLAHE->setChecked(false);
    if (checkDnCNN) checkDnCNN->setChecked(false);
    
    // Configuración óptima para pulmones:
    // - Solo CLAHE para mejorar contraste aire-tejido
    // - SIN bilateral: la segmentación por HU es suficientemente precisa
    
    if (checkCLAHE && sliderCLAHEClip && sliderCLAHETile) {
        checkCLAHE->setChecked(true);
        sliderCLAHEClip->setValue(20);  // 2.0 clip limit (moderado)
        sliderCLAHETile->setValue(8);   // 8x8 tiles
    }
    
    blockSignals(prevState);
    onFilterChanged();
    
    lblStatus->setText("dddd Preset 'Pulmones Óptimo' aplicado: CLAHE únicamente");
}

void MainWindow::onPresetBones()
{
    // Preset optimizado para huesos (basado en pipeline_huesos.cpp)
    
    bool prevState = blockSignals(true);
    
    // Desactivar todos los filtros primero
    if (checkGaussian) checkGaussian->setChecked(false);
    if (checkMedian) checkMedian->setChecked(false);
    if (checkBilateral) checkBilateral->setChecked(false);
    if (checkCLAHE) checkCLAHE->setChecked(false);
    if (checkDnCNN) checkDnCNN->setChecked(false);
    
    // Configuración óptima para huesos:
    // - Mediana para eliminar ruido sin perder bordes duros
    // - CLAHE fuerte para realzar estructuras óseas
    
    if (checkMedian && sliderMedianKernel) {
        checkMedian->setChecked(true);
        sliderMedianKernel->setValue(5); // Kernel moderado
    }
    
    if (checkCLAHE && sliderCLAHEClip && sliderCLAHETile) {
        checkCLAHE->setChecked(true);
        sliderCLAHEClip->setValue(30);  // 3.0 clip limit (fuerte para huesos)
        sliderCLAHETile->setValue(8);   // 8x8 tiles
    }
    
    blockSignals(prevState);
    onFilterChanged();
    
    lblStatus->setText("Preset 'Huesos Óptimo' aplicado: Mediana + CLAHE fuerte");
}

void MainWindow::onPresetSoftTissue()
{
    // Preset optimizado para aorta/arterias (basado en pipeline_aorta.cpp)
    // NOTA: pipeline_aorta.cpp usa DnCNN opcional, no filtros tradicionales
    
    bool prevState = blockSignals(true);
    
    // Desactivar todos los filtros primero
    if (checkGaussian) checkGaussian->setChecked(false);
    if (checkMedian) checkMedian->setChecked(false);
    if (checkBilateral) checkBilateral->setChecked(false);
    if (checkCLAHE) checkCLAHE->setChecked(false);
    if (checkDnCNN) checkDnCNN->setChecked(false);
    
    // Configuración óptima para arterias/aorta:
    // - Solo CLAHE muy suave para mejorar contraste vascular
    // - La segmentación por HU [30-120] es muy precisa para vasos con contraste
    // - Usar DnCNN manualmente si se necesita denoising
    
    if (checkCLAHE && sliderCLAHEClip && sliderCLAHETile) {
        checkCLAHE->setChecked(true);
        sliderCLAHEClip->setValue(15);  // 1.5 clip limit (suave)
        sliderCLAHETile->setValue(8);   // 8x8 tiles
    }
    
    blockSignals(prevState);
    onFilterChanged();
    
    lblStatus->setText("🩸 Preset 'Aorta Óptimo' aplicado: CLAHE suave únicamente");
}

void MainWindow::onResetFilters()
{
    // Bloquear señales
    bool prevState = blockSignals(true);
    
    // Desactivar todos los checkboxes
    if (checkGaussian) checkGaussian->setChecked(false);
    if (checkMedian) checkMedian->setChecked(false);
    if (checkBilateral) checkBilateral->setChecked(false);
    if (checkCLAHE) checkCLAHE->setChecked(false);
    
    // Resetear sliders a valores por defecto
    if (sliderGaussianKernel) sliderGaussianKernel->setValue(5);
    if (sliderMedianKernel) sliderMedianKernel->setValue(5);
    if (sliderBilateralD) sliderBilateralD->setValue(9);
    if (sliderBilateralSigma) sliderBilateralSigma->setValue(75);
    if (sliderCLAHEClip) sliderCLAHEClip->setValue(20);
    if (sliderCLAHETile) sliderCLAHETile->setValue(8);
    
    blockSignals(prevState);
    
    // Aplicar cambios
    onFilterChanged();
    
    lblStatus->setText("Filtros reseteados a valores por defecto");
}

void MainWindow::onCompareWithDnCNN()
{
    if (!sliceContext.isValid || sliceContext.originalRaw.empty()) {
        QMessageBox::warning(this, "Error", 
            "No hay imagen cargada. Por favor carga un dataset primero.");
        return;
    }
    
    if (!dncnnDenoiser || !dncnnDenoiser->isLoaded()) {
        QMessageBox::warning(this, "Error", 
            "El modelo DnCNN no está cargado.");
        return;
    }
    
    // Obtener imagen original normalizada
    cv::Mat original8bit = Bridge::normalize16to8bit(sliceContext.originalRaw);
    
    // Aplicar DnCNN
    Denoising::DenoisingComparison comparison = 
        Denoising::compareWithAndWithoutDenoising(original8bit, *dncnnDenoiser);
    
    // Crear visualización de comparación
    cv::Mat visualComparison = Denoising::visualizeComparison(comparison);
    
    // Mostrar en ventana modal
    QDialog *comparisonDialog = new QDialog(this);
    comparisonDialog->setWindowTitle("Comparación: DnCNN vs Original");
    comparisonDialog->resize(visualComparison.cols + 40, visualComparison.rows + 120);
    
    QVBoxLayout *layout = new QVBoxLayout(comparisonDialog);
    
    // Label con la imagen
    QLabel *imageLabel = new QLabel();
    QImage qImage = cvMatToQImage(visualComparison);
    imageLabel->setPixmap(QPixmap::fromImage(qImage));
    layout->addWidget(imageLabel);
    
    // Métricas
    QLabel *metricsLabel = new QLabel(
        QString("<b>Métricas de Denoising:</b><br>"
                "PSNR: %1 dB<br>"
                "SNR: %2 dB<br>"
                "Tiempo de procesamiento: %3 ms")
        .arg(QString::number(comparison.psnr, 'f', 2))
        .arg(QString::number(comparison.snr, 'f', 2))
        .arg(QString::number(comparison.processingTime, 'f', 2))
    );
    metricsLabel->setStyleSheet("QLabel { font-size: 11pt; padding: 10px; background-color: #f0f0f0; }");
    layout->addWidget(metricsLabel);
    
    // Botón cerrar
    QPushButton *closeBtn = new QPushButton("Cerrar");
    connect(closeBtn, &QPushButton::clicked, comparisonDialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    comparisonDialog->exec();
    
    lblStatus->setText(QString("Comparación DnCNN: PSNR=%1dB, SNR=%2dB")
                      .arg(comparison.psnr, 0, 'f', 1)
                      .arg(comparison.snr, 0, 'f', 1));
}

// ========== MÉTODOS DE SEGMENTACIÓN ==========

void MainWindow::applySegmentation()
{
    if (!sliceContext.isValid || sliceContext.originalRaw.empty()) {
        return;
    }
    
    // Usar la imagen preprocesada si existe, sino usar la original normalizada
    cv::Mat sourceImage;
    if (!sliceContext.preprocessed.empty()) {
        sourceImage = sliceContext.preprocessed.clone();
    } else {
        sourceImage = Bridge::normalize16to8bit(sliceContext.originalRaw);
    }
    
    // Necesitamos trabajar con la imagen original en HU para umbralización correcta
    // Pero mostraremos usando la preprocesada si existe
    cv::Mat imageForSegmentation = sliceContext.originalRaw.clone();
    
    // Obtener parámetros de segmentación
    int minHU = sliderMinHU->value();
    int maxHU = sliderMaxHU->value();
    int minArea = sliderMinArea->value();
    int maxArea = sliderMaxArea->value();
    
    // Crear parámetros de segmentación
    Segmentation::SegmentationParams params;
    params.minHU = minHU;
    params.maxHU = maxHU;
    params.minArea = minArea;
    params.maxArea = maxArea;
    params.visualColor = cv::Scalar(255, 0, 0); // Azul por defecto
    
    // Segmentar
    auto regions = Segmentation::segmentOrgan(imageForSegmentation, params, "Region");
    
    // Filtrar regiones que tocan el borde si está activado
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
    
    // Ordenar por área (mayor a menor)
    std::sort(regions.begin(), regions.end(), 
              [](const Segmentation::SegmentedRegion& a, const Segmentation::SegmentedRegion& b) {
                  return a.area > b.area;
              });
    
    // Determinar tipo de órgano según rango HU (usar variables ya declaradas)
    bool esPulmones = (minHU <= -400 && maxHU <= -100);  // Aire/pulmones: [-1000, -400]
    bool esAorta = (minHU >= 20 && maxHU <= 150);        // Vasos con contraste: [30, 120]
    bool esHuesos = (minHU >= 150);                      // Huesos: [200, 1000]
    
    // FILTRADO ANATÓMICO ESPECÍFICO PARA AORTA (como en pipeline_aorta.cpp)
    if (esAorta && !regions.empty()) {
        cv::Point2d imgCenter(imageForSegmentation.cols / 2.0, imageForSegmentation.rows / 2.0);
        
        std::vector<Segmentation::SegmentedRegion> filteredAorta;
        for (const auto& region : regions) {
            double distX = std::abs(region.centroid.x - imgCenter.x);
            double distY = region.centroid.y - imgCenter.y;
            double distTotal = cv::norm(region.centroid - imgCenter);
            
            // Filtros anatómicos (de pipeline_aorta.cpp):
            bool esCentral = (distX < 70);      // Debe estar cerca del centro horizontal
            bool esAnterior = (distY < 20);     // Debe estar en parte anterior (arriba del centro)
            bool esMediano = (distTotal < 100); // No muy lejos del centro
            bool tamanioOk = (region.area >= 300 && region.area <= 5000); // Tamaño apropiado
            
            if (esCentral && esAnterior && esMediano && tamanioOk) {
                filteredAorta.push_back(region);
            }
        }
        
        // Si encontramos candidatos, aplicar procesamiento morfológico y quedarnos con el más grande
        if (!filteredAorta.empty()) {
            // Combinar máscaras
            cv::Mat combinedMask = cv::Mat::zeros(imageForSegmentation.size(), CV_8U);
            for (const auto& r : filteredAorta) {
                cv::bitwise_or(combinedMask, r.mask, combinedMask);
            }
            
            // Morfología: cierre + dilatación
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
            cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);
            cv::dilate(combinedMask, combinedMask, kernel);
            
            // Re-segmentar componentes conectados
            auto finalComponents = Segmentation::findConnectedComponents(combinedMask, 500);
            
            if (!finalComponents.empty()) {
                // Ordenar por área y tomar el más grande
                std::sort(finalComponents.begin(), finalComponents.end(),
                    [](const auto& a, const auto& b) { return a.area > b.area; });
                
                auto& largest = finalComponents[0];
                double dist = cv::norm(largest.centroid - imgCenter);
                
                // Verificar que el más grande está cerca del centro
                if (dist < 120.0) {
                    regions.clear();
                    regions.push_back(largest);
                } else {
                    regions.clear(); // No se encontró aorta válida
                }
            }
        } else {
            regions.clear(); // No se encontraron regiones que cumplan criterios anatómicos
        }
    }
    
    // Limitar número de regiones según el órgano (solo si NO es aorta, ya filtrada arriba)
    if (!esAorta) {
        size_t maxRegions = esPulmones ? 2 : 20; // Pulmones: 2, Huesos: múltiples
        if (regions.size() > maxRegions) {
            regions.resize(maxRegions);
        }
    }
    
    // Asignar etiquetas y colores específicos
    // IMPORTANTE: Si ya existen regiones clasificadas de huesos (del pipeline especializado),
    // preservar esa clasificación en lugar de sobrescribirla con etiquetas genéricas
    bool preservarClasificacionHuesos = esHuesos && !sliceContext.huesosRegions.empty() &&
                                        (sliceContext.huesosRegions[0].label.find("Columna") != std::string::npos ||
                                         sliceContext.huesosRegions[0].label.find("Costilla") != std::string::npos);
    
    if (preservarClasificacionHuesos) {
        // Ya existe una clasificación anatómica detallada, mantenerla
        regions = sliceContext.huesosRegions;
    } else {
        // Asignar etiquetas genéricas para nueva segmentación
        for (size_t i = 0; i < regions.size(); i++) {
            if (esPulmones) {
                regions[i].label = (i == 0) ? "Pulmon Derecho" : "Pulmon Izquierdo";
                regions[i].color = cv::Scalar(255, 0, 0); // Azul
            } else if (esAorta) {
                regions[i].label = "Aorta"; // Solo 1 estructura después del filtrado anatómico
                regions[i].color = cv::Scalar(0, 0, 255); // Rojo
            } else if (esHuesos) {
                regions[i].label = "Hueso_" + std::to_string(i+1);
                regions[i].color = cv::Scalar(0, 255, 0); // Verde
            } else {
                regions[i].label = "Region_" + std::to_string(i+1);
                regions[i].color = cv::Scalar(255, 255, 0); // Cian
            }
        }
        
        // Guardar regiones en el vector correspondiente según el tipo de órgano
        if (esPulmones) {
            sliceContext.pulmonesRegions = regions;
        } else if (esAorta) {
            sliceContext.aortaRegions = regions;
        } else if (esHuesos) {
            sliceContext.huesosRegions = regions;
        }
    }
    
    // Crear imagen de visualización a color
    cv::Mat imageColor;
    if (sourceImage.channels() == 1) {
        cv::cvtColor(sourceImage, imageColor, cv::COLOR_GRAY2BGR);
    } else {
        imageColor = sourceImage.clone();
    }
    
    // Aplicar refinamiento morfológico a las máscaras
    for (auto& region : regions) {
        // Apertura para suavizar bordes
        region.mask = Morphology::opening(region.mask, cv::Size(5, 5));
        // Cierre para rellenar huecos
        region.mask = Morphology::closing(region.mask, cv::Size(9, 9));
        // Rellenar todos los huecos internos
        region.mask = Morphology::fillHoles(region.mask);
    }
    
    // Crear overlay si está activado
    if (checkShowOverlay && checkShowOverlay->isChecked()) {
        cv::Mat overlay = imageColor.clone();
        for (const auto& region : regions) {
            overlay.setTo(region.color, region.mask);
        }
        cv::addWeighted(imageColor, 0.7, overlay, 0.3, 0, imageColor);
    }
    
    // Dibujar contornos si está activado
    if (checkShowContours && checkShowContours->isChecked()) {
        for (const auto& region : regions) {
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(region.mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::drawContours(imageColor, contours, -1, region.color, 3);
        }
    }
    
    // Dibujar etiquetas si está activado
    if (checkShowLabels && checkShowLabels->isChecked()) {
        for (const auto& region : regions) {
            cv::putText(imageColor, region.label, 
                       cv::Point(region.boundingBox.x, region.boundingBox.y - 10),
                       cv::FONT_HERSHEY_SIMPLEX, 0.8, region.color, 2);
        }
    }
    
    // Crear máscara binaria combinando todas las regiones
    cv::Mat combinedMask = cv::Mat::zeros(sourceImage.size(), CV_8U);
    for (const auto& region : regions) {
        combinedMask |= region.mask;
    }
    
    // Guardar resultado en el contexto
    sliceContext.segmentationMask = combinedMask;
    sliceContext.segmentationOriginal = combinedMask.clone(); // Copia para morfología
    sliceContext.finalOverlay = imageColor;
    
    // Actualizar la visualización
    updateSegmentationDisplay();
}

void MainWindow::updateSegmentationDisplay()
{
    if (!sliceContext.isValid || sliceContext.originalRaw.empty()) {
        return;
    }
    
    // Imagen ANTES (preprocesada o normalizada)
    cv::Mat imageBefore;
    if (!sliceContext.preprocessed.empty()) {
        imageBefore = sliceContext.preprocessed;
    } else {
        imageBefore = Bridge::normalize16to8bit(sliceContext.originalRaw);
    }
    
    QImage qimgBefore = cvMatToQImage(imageBefore);
    if (!qimgBefore.isNull() && imageSegBeforeLabel) {
        QPixmap pixmapBefore = QPixmap::fromImage(qimgBefore);
        imageSegBeforeLabel->setPixmap(pixmapBefore);
        imageSegBeforeLabel->resize(pixmapBefore.size());
    }
    
    // Imagen DESPUÉS (segmentada)
    if (!sliceContext.finalOverlay.empty() && imageSegAfterLabel) {
        QImage qimgAfter = cvMatToQImage(sliceContext.finalOverlay);
        if (!qimgAfter.isNull()) {
            QPixmap pixmapAfter = QPixmap::fromImage(qimgAfter);
            imageSegAfterLabel->setPixmap(pixmapAfter);
            imageSegAfterLabel->resize(pixmapAfter.size());
        }
    }
}

void MainWindow::onSegmentationChanged()
{
    // Actualizar labels de valores
    if (lblMinHUValue) {
        lblMinHUValue->setText(QString::number(sliderMinHU->value()));
    }
    
    if (lblMaxHUValue) {
        lblMaxHUValue->setText(QString::number(sliderMaxHU->value()));
    }
    
    if (lblMinAreaValue) {
        lblMinAreaValue->setText(QString::number(sliderMinArea->value()));
    }
    
    if (lblMaxAreaValue) {
        lblMaxAreaValue->setText(QString::number(sliderMaxArea->value()));
    }
    
    // Aplicar segmentación si hay datos cargados
    if (datasetLoaded && sliceContext.isValid) {
        applySegmentation();
    }
}

void MainWindow::onSegPresetLungs()
{
    // Preset optimizado para pulmones (basado en pipeline_pulmones.cpp)
    
    // Bloquear señales temporalmente
    bool prevState = blockSignals(true);
    
    // Configurar umbrales HU para aire/pulmones
    if (sliderMinHU) sliderMinHU->setValue(-1000);
    if (sliderMaxHU) sliderMaxHU->setValue(-400);
    
    // Configurar filtros de área
    if (sliderMinArea) sliderMinArea->setValue(5000);   // Filtrar regiones pequeñas
    if (sliderMaxArea) sliderMaxArea->setValue(200000); // Límite superior
    
    // Activar opciones de visualización óptimas
    if (checkShowContours) checkShowContours->setChecked(true);
    if (checkShowOverlay) checkShowOverlay->setChecked(true);
    if (checkShowLabels) checkShowLabels->setChecked(true);
    if (checkFilterBorder) checkFilterBorder->setChecked(true); // Eliminar aire exterior
    
    blockSignals(prevState);
    
    // Aplicar cambios
    onSegmentationChanged();
    
    lblStatus->setText("Preset 'Pulmones Óptimo' aplicado a segmentación");
}

void MainWindow::onSegPresetBones()
{
    // 🦴 PIPELINE ESPECIALIZADO DE HUESOS - COPIA EXACTA de pipeline_huesos.cpp
    std::cout << "\n╔═══════════════════════════════════════════╗" << std::endl;
    std::cout << "║   PIPELINE ESPECIALIZADO: HUESOS 🦴      ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════╝\n" << std::endl;
    
    lblStatus->setText("🦴 Iniciando Pipeline Especializado de Huesos...");
    
    if (sliceContext.originalRaw.empty()) {
        lblStatus->setText("⚠️ No hay imagen cargada");
        return;
    }
    
    // Bloquear señales temporalmente
    bool prevState = blockSignals(true);
    
    // 1. LECTURA - usar imagen original RAW
    cv::Mat imageHU_16bit = sliceContext.originalRaw;
    if (imageHU_16bit.empty()) {
        lblStatus->setText("⚠️ Error: Imagen vacía");
        blockSignals(prevState);
        return;
    }
    std::cout << "  Dimensiones: " << imageHU_16bit.cols << "x" << imageHU_16bit.rows << std::endl;
    
    cv::Point2d imgCenter(imageHU_16bit.cols / 2.0, imageHU_16bit.rows / 2.0);
    
    // 3. SEGMENTACIÓN DE HUESOS
    std::cout << "\n→ Segmentando estructuras óseas..." << std::endl;
    
    // Parámetros para hueso cortical y esponjoso
    Segmentation::SegmentationParams boneParams;
    boneParams.minHU = 200;    // Hueso denso
    boneParams.maxHU = 3000;   // Rango completo de hueso
    boneParams.minArea = 80;   // Aumentado para reducir fragmentos pequeños
    boneParams.maxArea = 100000;
    boneParams.visualColor = cv::Scalar(0, 0, 255); // Rojo
    
    auto boneRegions = Segmentation::segmentBones(imageHU_16bit, boneParams);
    
    // Filtrar por maxArea manualmente (segmentBones no lo hace)
    std::vector<Segmentation::SegmentedRegion> filteredRegions;
    for (const auto& region : boneRegions) {
        if (region.area <= boneParams.maxArea) {
            filteredRegions.push_back(region);
        }
    }
    boneRegions = filteredRegions;
    
    std::cout << "  Total regiones óseas detectadas: " << boneRegions.size() << std::endl;
    
    // 4. CLASIFICACIÓN DE HUESOS CON CRITERIOS MEJORADOS
    std::cout << "\n→ Clasificando estructuras óseas..." << std::endl;
    
    std::vector<Segmentation::SegmentedRegion> costillas;
    std::vector<Segmentation::SegmentedRegion> columna;
    std::vector<Segmentation::SegmentedRegion> esternon;
    std::vector<Segmentation::SegmentedRegion> otros;
    
    for (const auto& region : boneRegions) {
        if (region.area < 80) continue;
        
        Segmentation::SegmentedRegion bone = region;
        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = region.centroid.y - imgCenter.y;
        double distTotal = cv::norm(region.centroid - imgCenter);
        
        // Calcular aspect ratio (relación ancho/alto) del bounding box
        double width = region.boundingBox.width;
        double height = region.boundingBox.height;
        double aspectRatio = (height > 0) ? (width / height) : 1.0;
        
        // COLUMNA: Posterior (parte inferior) y muy central horizontalmente
        if (distX < 60 && distY > 40 && region.area > 150) {
            bone.label = "Columna Vertebral";
            bone.color = cv::Scalar(0, 0, 255); // Rojo
            columna.push_back(bone);
        }
        // ESTERNÓN: Anterior (parte superior) y central
        else if (distX < 60 && distY < -20 && region.area > 200) {
            bone.label = "Esternon";
            bone.color = cv::Scalar(0, 100, 255); // Rojo claro
            esternon.push_back(bone);
        }
        // COSTILLAS: Laterales y generalmente alargadas
        else if (distX > 60 || aspectRatio > 2.0) {
            bone.label = "Costilla";
            bone.color = cv::Scalar(0, 200, 255); // Amarillo/Naranja
            costillas.push_back(bone);
        }
        // OTROS: Fragmentos no clasificados
        // CORRECCIÓN: Aumentamos área mínima y FILTRAMOS EL CENTRO
        else if (region.area > 300) { 
            // Si está muy cerca del centro (distTotal < 60), es probable que sea calcificación de aorta/corazón.
            // Solo aceptamos "otros huesos" si están lejos del centro.
            if (distTotal > 60) {
                bone.label = "Hueso (Otro)";
                bone.color = cv::Scalar(100, 100, 100); // CORRECCIÓN: Gris (para diferenciar del rojo)
                otros.push_back(bone);
            }
        }
    }
    
    std::cout << "  ✓ Costillas: " << costillas.size() << std::endl;
    std::cout << "  ✓ Columna vertebral: " << columna.size() << " segmentos" << std::endl;
    std::cout << "  ✓ Esternón: " << esternon.size() << std::endl;
    std::cout << "  ✓ Otros: " << otros.size() << std::endl;
    
    // 5. REFINAMIENTO
    std::cout << "\n→ Refinando máscaras..." << std::endl;
    
    auto refinar = [](std::vector<Segmentation::SegmentedRegion>& bones) {
        for (auto& bone : bones) {
            // Apertura suave para eliminar ruido
            bone.mask = Morphology::opening(bone.mask, cv::Size(3, 3));
            // Cierre para conectar fragmentos
            bone.mask = Morphology::closing(bone.mask, cv::Size(3, 3));
        }
    };
    
    refinar(costillas);
    refinar(columna);
    refinar(esternon);
    refinar(otros);
    
    // 6. VISUALIZACIÓN CON OVERLAY RELLENO
    std::cout << "\n→ Generando visualización..." << std::endl;
    
    cv::Mat image8bit = Bridge::normalize16to8bit(imageHU_16bit);
    cv::Mat imageColor;
    
    // FORZAR conversión a BGR
    if (image8bit.channels() == 1) {
        cv::cvtColor(image8bit, imageColor, cv::COLOR_GRAY2BGR);
    } else {
        imageColor = image8bit.clone();
    }
    
    // Combinar todas las regiones
    std::vector<Segmentation::SegmentedRegion> allBones;
    allBones.insert(allBones.end(), columna.begin(), columna.end());
    allBones.insert(allBones.end(), esternon.begin(), esternon.end());
    allBones.insert(allBones.end(), costillas.begin(), costillas.end());
    allBones.insert(allBones.end(), otros.begin(), otros.end());
    
    // Crear resultado inicial
    cv::Mat result = imageColor.clone();
    
    // PASO 1: Crear overlay negro del mismo tamaño
    cv::Mat overlay = cv::Mat::zeros(result.size(), result.type());
    
    // PASO 2: Rellenar cada región con su color EN EL OVERLAY
    for (const auto& bone : allBones) {
        overlay.setTo(bone.color, bone.mask);
    }
    
    // PASO 3: Combinar con transparencia (70% imagen + 30% overlay)
    cv::addWeighted(result, 0.7, overlay, 0.3, 0, result);
    
    // PASO 4: Dibujar contornos GRUESOS encima para mejor definición
    for (const auto& bone : allBones) {
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(bone.mask.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::drawContours(result, contours, -1, bone.color, 2);
    }
    
    // Leyenda
    int y = 30;
    cv::putText(result, "ROJO: Columna", cv::Point(10, y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
    y += 25;
    cv::putText(result, "NARANJA: Costillas", cv::Point(10, y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 200, 255), 2);
    y += 25;
    cv::putText(result, "ROJO CLARO: Esternon", cv::Point(10, y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 100, 255), 2);
    y += 25;
    cv::putText(result, "GRIS: Otros", cv::Point(10, y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(100, 100, 100), 2);
    
    // Crear máscara combinada para sliceContext
    cv::Mat combinedMask = cv::Mat::zeros(imageHU_16bit.size(), CV_8UC1);
    for (const auto& bone : allBones) {
        combinedMask |= bone.mask;
    }
    
    // Actualizar contexto
    sliceContext.segmentationMask = combinedMask;
    sliceContext.huesosRegions = allBones;
    
    sliceContext.finalOverlay = result;
    
    std::cout << "\n╔═══════════════════════════════════════════╗" << std::endl;
    std::cout << "║          SEGMENTACIÓN COMPLETADA          ║" << std::endl;
    std::cout << "║  " << allBones.size() << " estructuras óseas identificadas     ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════╝\n" << std::endl;
    
    // Actualizar UI
    if (sliderMinHU) sliderMinHU->setValue(200);
    if (sliderMaxHU) sliderMaxHU->setValue(3000);
    if (sliderMinArea) sliderMinArea->setValue(80);
    if (sliderMaxArea) sliderMaxArea->setValue(100000);
    
    if (checkShowContours) checkShowContours->setChecked(true);
    if (checkShowOverlay) checkShowOverlay->setChecked(true);
    if (checkFilterBorder) checkFilterBorder->setChecked(false);
    
    blockSignals(prevState);
    
    // Mostrar resultado
    updateSegmentationDisplay();
    
    QString statusMsg = QString("🦴 Pipeline Huesos Completado: %1 estructuras identificadas "
                               "(Costillas:%2, Columna:%3, Esternón:%4, Otros:%5)")
                        .arg(allBones.size())
                        .arg(costillas.size())
                        .arg(columna.size())
                        .arg(esternon.size())
                        .arg(otros.size());
    lblStatus->setText(statusMsg);
}

void MainWindow::onSegPresetAorta()
{
    // Preset optimizado para aorta (basado en pipeline_aorta.cpp)
    
    // Bloquear señales temporalmente
    bool prevState = blockSignals(true);
    
    // Configurar umbrales HU para arterias (rango 30-120 HU)
    // La aorta tiene densidad similar al tejido blando con contraste
    if (sliderMinHU) sliderMinHU->setValue(30);
    if (sliderMaxHU) sliderMaxHU->setValue(120);
    
    // Configurar filtros de área para aorta
    // La aorta es una estructura mediana (200-5000 px típicamente)
    if (sliderMinArea) sliderMinArea->setValue(200);
    if (sliderMaxArea) sliderMaxArea->setValue(8000);
    
    // Activar opciones de visualización
    if (checkShowContours) checkShowContours->setChecked(true);
    if (checkShowOverlay) checkShowOverlay->setChecked(true);
    if (checkShowLabels) checkShowLabels->setChecked(false); // Sin etiquetas para arterias
    if (checkFilterBorder) checkFilterBorder->setChecked(false); // No filtrar borde para aorta
    
    blockSignals(prevState);
    
    // Aplicar cambios
    onSegmentationChanged();
    
    lblStatus->setText("🩸 Preset 'Aorta Óptimo' aplicado: HU[30,120], Área[200,8000]");
}

void MainWindow::onResetSegmentation()
{
    // Bloquear señales
    bool prevState = blockSignals(true);
    
    // Resetear umbrales HU
    if (sliderMinHU) sliderMinHU->setValue(-1000);
    if (sliderMaxHU) sliderMaxHU->setValue(-400);
    
    // Resetear filtros de área
    if (sliderMinArea) sliderMinArea->setValue(1000);
    if (sliderMaxArea) sliderMaxArea->setValue(200000);
    
    // Resetear opciones de visualización
    if (checkShowContours) checkShowContours->setChecked(true);
    if (checkShowOverlay) checkShowOverlay->setChecked(true);
    if (checkShowLabels) checkShowLabels->setChecked(true);
    if (checkFilterBorder) checkFilterBorder->setChecked(true);
    
    blockSignals(prevState);
    
    // Aplicar cambios
    onSegmentationChanged();
    
    lblStatus->setText("Segmentación reseteada a valores por defecto");
}

// ============================================================================
// MORPHOLOGY METHODS
// ============================================================================

void MainWindow::applyMorphology()
{
    if (!sliceContext.isValid) {
        return;
    }

    // CRÍTICO: Siempre partir de la segmentación ORIGINAL, no de la modificada
    cv::Mat workingImage;
    
    if (!sliceContext.segmentationOriginal.empty()) {
        // IMPORTANTE: Usar la segmentación ORIGINAL, no la modificada
        workingImage = sliceContext.segmentationOriginal.clone();
    } else {
        std::cerr << "No hay máscara de segmentación original para aplicar morfología" << std::endl;
        return;
    }
    
    // Asegurarse de que es binaria de 8 bits
    if (workingImage.type() != CV_8U) {
        workingImage.convertTo(workingImage, CV_8U);
    }
    
    // Si tiene múltiples canales, convertir a escala de grises
    if (workingImage.channels() > 1) {
        cv::cvtColor(workingImage, workingImage, cv::COLOR_BGR2GRAY);
    }
    
    // Asegurar que sea binaria (0 o 255)
    cv::threshold(workingImage, workingImage, 10, 255, cv::THRESH_BINARY);

    // Obtener forma del kernel
    int shapeIdx = comboKernelShape ? comboKernelShape->currentIndex() : 0;
    Morphology::StructuringElementShape shape = static_cast<Morphology::StructuringElementShape>(shapeIdx);

    cv::Mat result = workingImage.clone();

    // Aplicar operaciones en orden
    
    // 1. Erosión
    if (checkErode && checkErode->isChecked()) {
        int kernelSize = sliderErodeKernel ? sliderErodeKernel->value() : 3;
        if (kernelSize % 2 == 0) kernelSize++;
        int iterations = sliderErodeIter ? sliderErodeIter->value() : 1;
        result = Morphology::erode(result, cv::Size(kernelSize, kernelSize), shape, iterations);
    }

    // 2. Dilatación
    if (checkDilate && checkDilate->isChecked()) {
        int kernelSize = sliderDilateKernel ? sliderDilateKernel->value() : 3;
        if (kernelSize % 2 == 0) kernelSize++;
        int iterations = sliderDilateIter ? sliderDilateIter->value() : 1;
        result = Morphology::dilate(result, cv::Size(kernelSize, kernelSize), shape, iterations);
    }

    // 3. Apertura (Opening)
    if (checkOpening && checkOpening->isChecked()) {
        int kernelSize = sliderOpeningKernel ? sliderOpeningKernel->value() : 5;
        if (kernelSize % 2 == 0) kernelSize++;
        result = Morphology::opening(result, cv::Size(kernelSize, kernelSize), shape);
    }

    // 4. Cierre (Closing)
    if (checkClosing && checkClosing->isChecked()) {
        int kernelSize = sliderClosingKernel ? sliderClosingKernel->value() : 9;
        if (kernelSize % 2 == 0) kernelSize++;
        result = Morphology::closing(result, cv::Size(kernelSize, kernelSize), shape);
    }

    // 5. Gradiente morfológico
    if (checkGradient && checkGradient->isChecked()) {
        int kernelSize = sliderGradientKernel ? sliderGradientKernel->value() : 3;
        if (kernelSize % 2 == 0) kernelSize++;
        result = Morphology::morphologicalGradient(result, cv::Size(kernelSize, kernelSize), shape);
    }

    // 6. Rellenar huecos
    if (checkFillHoles && checkFillHoles->isChecked()) {
        result = Morphology::fillHoles(result);
    }

    // 7. Eliminar bordes
    if (checkRemoveBorder && checkRemoveBorder->isChecked()) {
        result = Morphology::clearBorder(result);
    }

    // Guardar resultado en sliceContext
    sliceContext.segmentationMask = result;
    
    // Crear visualización en color
    cv::Mat colorResult;
    cv::cvtColor(result, colorResult, cv::COLOR_GRAY2BGR);
    sliceContext.finalOverlay = colorResult;

    // Actualizar display
    updateMorphologyDisplay();
}

void MainWindow::updateMorphologyDisplay()
{
    if (!imageMorphBeforeLabel || !imageMorphAfterLabel) {
        return;
    }

    // ANTES: Mostrar la segmentación original (antes de morfología)
    if (!sliceContext.segmentationOriginal.empty()) {
        cv::Mat beforeDisplay;
        if (sliceContext.segmentationOriginal.channels() == 1) {
            cv::cvtColor(sliceContext.segmentationOriginal, beforeDisplay, cv::COLOR_GRAY2BGR);
        } else {
            beforeDisplay = sliceContext.segmentationOriginal.clone();
        }
        
        QImage qimgBefore = cvMatToQImage(beforeDisplay);
        if (!qimgBefore.isNull()) {
            QPixmap pixmapBefore = QPixmap::fromImage(qimgBefore);
            imageMorphBeforeLabel->setPixmap(pixmapBefore);
            imageMorphBeforeLabel->resize(pixmapBefore.size());
        }
    }

    // DESPUÉS: Mostrar el resultado morfológico
    if (!sliceContext.segmentationMask.empty()) {
        cv::Mat afterDisplay;
        if (sliceContext.segmentationMask.channels() == 1) {
            cv::cvtColor(sliceContext.segmentationMask, afterDisplay, cv::COLOR_GRAY2BGR);
        } else {
            afterDisplay = sliceContext.segmentationMask.clone();
        }
        
        QImage qimgAfter = cvMatToQImage(afterDisplay);
        if (!qimgAfter.isNull()) {
            QPixmap pixmapAfter = QPixmap::fromImage(qimgAfter);
            imageMorphAfterLabel->setPixmap(pixmapAfter);
            imageMorphAfterLabel->resize(pixmapAfter.size());
        }
    }
}

void MainWindow::onMorphologyChanged()
{
    if (!datasetLoaded || currentSliceIndex < 0) {
        return;
    }

    applyMorphology();
    
    if (lblStatus) {
        lblStatus->setText("Operaciones morfológicas actualizadas");
    }
}

void MainWindow::onMorphPresetLungs()
{
    if (!checkOpening || !checkClosing || !checkFillHoles) {
        return;
    }

    // Bloquear señales
    bool prevState = blockSignals(true);

    // Desactivar todas las operaciones
    if (checkErode) checkErode->setChecked(false);
    if (checkDilate) checkDilate->setChecked(false);
    if (checkGradient) checkGradient->setChecked(false);
    if (checkRemoveBorder) checkRemoveBorder->setChecked(false);

    // Configuración óptima para pulmones
    // Opening para eliminar pequeñas protuberancias
    checkOpening->setChecked(true);
    if (sliderOpeningKernel) sliderOpeningKernel->setValue(5);

    // Closing para cerrar pequeños huecos
    checkClosing->setChecked(true);
    if (sliderClosingKernel) sliderClosingKernel->setValue(9);

    // Fill holes para rellenar huecos internos
    checkFillHoles->setChecked(true);

    // Kernel elíptico
    if (comboKernelShape) comboKernelShape->setCurrentIndex(0); // Elipse

    blockSignals(prevState);

    // Aplicar cambios
    onMorphologyChanged();
    
    if (lblStatus) {
        lblStatus->setText("Preset 'Pulmones Óptimo' aplicado: Opening(5) + Closing(9) + FillHoles");
    }
}

void MainWindow::onMorphPresetBones()
{
    if (!checkOpening || !checkErode || !checkRemoveBorder) {
        return;
    }

    // Bloquear señales
    bool prevState = blockSignals(true);

    // Desactivar operaciones no necesarias
    if (checkDilate) checkDilate->setChecked(false);
    if (checkClosing) checkClosing->setChecked(false);
    if (checkGradient) checkGradient->setChecked(false);
    if (checkFillHoles) checkFillHoles->setChecked(false);

    // Configuración óptima para huesos
    // Erosión suave para eliminar ruido
    if (checkErode) {
        checkErode->setChecked(true);
        if (sliderErodeKernel) sliderErodeKernel->setValue(3);
        if (sliderErodeIter) sliderErodeIter->setValue(1);
    }

    // Opening para limpiar
    checkOpening->setChecked(true);
    if (sliderOpeningKernel) sliderOpeningKernel->setValue(3);

    // Remover estructuras que tocan bordes
    checkRemoveBorder->setChecked(true);

    // Kernel rectangular
    if (comboKernelShape) comboKernelShape->setCurrentIndex(1); // Rectángulo

    blockSignals(prevState);

    // Aplicar cambios
    onMorphologyChanged();
    
    if (lblStatus) {
        lblStatus->setText("Preset 'Huesos Óptimo' aplicado: Erode(3x1) + Opening(3) + RemoveBorder");
    }
}

void MainWindow::onMorphPresetAorta()
{
    if (!checkClosing || !checkDilate) {
        return;
    }

    // Bloquear señales
    bool prevState = blockSignals(true);

    // Desactivar operaciones no necesarias
    if (checkErode) checkErode->setChecked(false);
    if (checkOpening) checkOpening->setChecked(false);
    if (checkGradient) checkGradient->setChecked(false);
    if (checkFillHoles) checkFillHoles->setChecked(false);
    if (checkRemoveBorder) checkRemoveBorder->setChecked(false);

    // Configuración óptima para aorta (basado en pipeline_aorta.cpp)
    // Closing para unir fragmentos de la aorta
    checkClosing->setChecked(true);
    if (sliderClosingKernel) sliderClosingKernel->setValue(5);

    // Dilatación para expandir ligeramente la máscara
    if (checkDilate) {
        checkDilate->setChecked(true);
        if (sliderDilateKernel) sliderDilateKernel->setValue(5);
        if (sliderDilateIter) sliderDilateIter->setValue(1);
    }

    // Kernel elíptico (apropiado para estructuras circulares como arterias)
    if (comboKernelShape) comboKernelShape->setCurrentIndex(0); // Elipse

    blockSignals(prevState);

    // Aplicar cambios
    onMorphologyChanged();
    
    if (lblStatus) {
        lblStatus->setText("🩸 Preset 'Aorta Óptimo' aplicado: Closing(5) + Dilate(5x1)");
    }
}

void MainWindow::onResetMorphology()
{
    // Bloquear señales
    bool prevState = blockSignals(true);

    // Desactivar todas las operaciones
    if (checkErode) checkErode->setChecked(false);
    if (checkDilate) checkDilate->setChecked(false);
    if (checkOpening) checkOpening->setChecked(false);
    if (checkClosing) checkClosing->setChecked(false);
    if (checkGradient) checkGradient->setChecked(false);
    if (checkFillHoles) checkFillHoles->setChecked(false);
    if (checkRemoveBorder) checkRemoveBorder->setChecked(false);

    // Resetear valores de sliders
    if (sliderErodeKernel) sliderErodeKernel->setValue(3);
    if (sliderDilateKernel) sliderDilateKernel->setValue(3);
    if (sliderOpeningKernel) sliderOpeningKernel->setValue(5);
    if (sliderClosingKernel) sliderClosingKernel->setValue(9);
    if (sliderGradientKernel) sliderGradientKernel->setValue(3);
    if (sliderErodeIter) sliderErodeIter->setValue(1);
    if (sliderDilateIter) sliderDilateIter->setValue(1);

    // Kernel elíptico por defecto
    if (comboKernelShape) comboKernelShape->setCurrentIndex(0);

    blockSignals(prevState);

    // Aplicar cambios
    onMorphologyChanged();
    
    if (lblStatus) {
        lblStatus->setText("Morfología reseteada a valores por defecto");
    }
}

// ============================================================================
// VISUALIZATION METHODS
// ============================================================================

void MainWindow::updateVisualization()
{
    if (!sliceContext.isValid || sliceContext.originalRaw.empty()) {
        if (lblFinalView) {
            lblFinalView->setText("No hay imagen cargada");
        }
        return;
    }

    // 1. Convertir imagen original de 16-bit a 8-bit BGR
    cv::Mat imgNormalized = Bridge::normalize16to8bit(sliceContext.originalRaw);
    cv::Mat imgColor;
    if (imgNormalized.channels() == 1) {
        cv::cvtColor(imgNormalized, imgColor, cv::COLOR_GRAY2BGR);
    } else {
        imgColor = imgNormalized.clone();
    }

    // 2. Crear overlay clonando la imagen base
    cv::Mat overlay = imgColor.clone();
    cv::Mat finalResult = imgColor.clone();

    // Determinar el estilo de visualización
    bool useFillStyle = radStyleFill && radStyleFill->isChecked();
    
    // Obtener opacidad (0-100 -> 0.0-1.0)
    double alpha = (sliderOpacity ? sliderOpacity->value() : 40) / 100.0;

    // 3. Aplicar capas de órganos desde vectores almacenados
    
    // CAPA: Pulmones (Azul)
    if (chkShowLungs && chkShowLungs->isChecked() && 
        !sliceContext.pulmonesRegions.empty()) {
        
        // Crear máscara combinando todas las regiones de pulmones
        cv::Mat lungsMask = cv::Mat::zeros(imgColor.size(), CV_8U);
        for (const auto& region : sliceContext.pulmonesRegions) {
            lungsMask |= region.mask;
        }
        
        if (useFillStyle) {
            // Relleno sólido con blend
            overlay.setTo(cv::Scalar(255, 0, 0), lungsMask); // BGR: Azul
            cv::addWeighted(imgColor, 1.0 - alpha, overlay, alpha, 0, finalResult);
            imgColor = finalResult.clone(); // Acumular para siguiente capa
        } else {
            // Solo contornos
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(lungsMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::drawContours(finalResult, contours, -1, cv::Scalar(255, 0, 0), 3);
        }
    }

    // CAPA: Huesos (Verde)
    if (chkShowBones && chkShowBones->isChecked() && chkShowBones->isEnabled() &&
        !sliceContext.huesosRegions.empty()) {
        
        // Crear máscara combinando todas las regiones de huesos
        cv::Mat bonesMask = cv::Mat::zeros(imgColor.size(), CV_8U);
        for (const auto& region : sliceContext.huesosRegions) {
            bonesMask |= region.mask;
        }
        
        if (useFillStyle) {
            // Relleno sólido con blend
            overlay.setTo(cv::Scalar(0, 255, 0), bonesMask); // BGR: Verde
            cv::addWeighted(imgColor, 1.0 - alpha, overlay, alpha, 0, finalResult);
            imgColor = finalResult.clone(); // Acumular para siguiente capa
        } else {
            // Solo contornos
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(bonesMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::drawContours(finalResult, contours, -1, cv::Scalar(0, 255, 0), 3);
        }
    }

    // CAPA: Aorta/Tejido Blando (Rojo)
    if (chkShowSoftTissue && chkShowSoftTissue->isChecked() && chkShowSoftTissue->isEnabled() &&
        !sliceContext.aortaRegions.empty()) {
        
        // Crear máscara combinando todas las regiones de aorta
        cv::Mat aortaMask = cv::Mat::zeros(imgColor.size(), CV_8U);
        for (const auto& region : sliceContext.aortaRegions) {
            aortaMask |= region.mask;
        }
        
        if (useFillStyle) {
            // Relleno sólido con blend
            overlay.setTo(cv::Scalar(0, 0, 255), aortaMask); // BGR: Rojo
            cv::addWeighted(imgColor, 1.0 - alpha, overlay, alpha, 0, finalResult);
            imgColor = finalResult.clone(); // Acumular para siguiente capa
        } else {
            // Solo contornos
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(aortaMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::drawContours(finalResult, contours, -1, cv::Scalar(0, 0, 255), 3);
        }
    }

    // 4. Mostrar resultado en el QLabel
    if (lblFinalView) {
        QImage qimg = cvMatToQImage(finalResult);
        if (!qimg.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(qimg);
            lblFinalView->setPixmap(pixmap);
            lblFinalView->resize(pixmap.size());
        }
    }
}

void MainWindow::onVisualizationChanged()
{
    if (!datasetLoaded || currentSliceIndex < 0) {
        return;
    }

    updateVisualization();
    
    if (lblStatus) {
        QString styleText = (radStyleFill && radStyleFill->isChecked()) ? "Relleno" : "Contornos";
        int opacity = sliderOpacity ? sliderOpacity->value() : 40;
        lblStatus->setText(QString("Visualización actualizada: %1, Opacidad: %2%")
                          .arg(styleText).arg(opacity));
    }
}

void MainWindow::onSaveClicked()
{
    if (!sliceContext.isValid || sliceContext.originalRaw.empty()) {
        QMessageBox::warning(this, "Error", 
            "No hay imagen para guardar. Por favor carga un dataset primero.");
        return;
    }

    // Obtener nombre sugerido del archivo actual
    QString suggestedName;
    if (currentSliceIndex >= 0 && currentSliceIndex < static_cast<int>(dicomFiles.size())) {
        std::string currentFile = dicomFiles[currentSliceIndex];
        fs::path filePath(currentFile);
        suggestedName = QString::fromStdString(filePath.stem().string()) + "_visualized.png";
    } else {
        suggestedName = "slice_visualized.png";
    }

    // Diálogo para guardar
    QString fileName = QFileDialog::getSaveFileName(this,
        "Guardar Imagen Final",
        suggestedName,
        "Imágenes PNG (*.png);;Imágenes JPEG (*.jpg *.jpeg);;Todos los archivos (*)");

    if (fileName.isEmpty()) {
        return;
    }

    // Regenerar la imagen final
    if (sliceContext.originalRaw.empty()) {
        QMessageBox::critical(this, "Error", "No hay imagen para guardar.");
        return;
    }

    // Convertir y aplicar overlay
    cv::Mat imgNormalized = Bridge::normalize16to8bit(sliceContext.originalRaw);
    cv::Mat imgColor;
    if (imgNormalized.channels() == 1) {
        cv::cvtColor(imgNormalized, imgColor, cv::COLOR_GRAY2BGR);
    } else {
        imgColor = imgNormalized.clone();
    }

    cv::Mat overlay = imgColor.clone();
    cv::Mat finalResult = imgColor.clone();

    bool useFillStyle = radStyleFill && radStyleFill->isChecked();
    double alpha = (sliderOpacity ? sliderOpacity->value() : 40) / 100.0;

    // Aplicar pulmones si está activo
    if (chkShowLungs && chkShowLungs->isChecked() && !sliceContext.segmentationMask.empty()) {
        cv::Mat lungsMask = sliceContext.segmentationMask.clone();
        
        if (useFillStyle) {
            overlay.setTo(cv::Scalar(255, 0, 0), lungsMask);
            cv::addWeighted(imgColor, 1.0 - alpha, overlay, alpha, 0, finalResult);
        } else {
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(lungsMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::drawContours(finalResult, contours, -1, cv::Scalar(255, 0, 0), 3);
        }
    }

    // Guardar imagen
    std::string fileNameStd = fileName.toStdString();
    bool success = cv::imwrite(fileNameStd, finalResult);

    if (success) {
        QMessageBox::information(this, "Éxito", 
            QString("Imagen guardada exitosamente en:\n%1").arg(fileName));
        lblStatus->setText("Imagen final guardada: " + fileName);
    } else {
        QMessageBox::critical(this, "Error", 
            "No se pudo guardar la imagen. Verifica la ruta y los permisos.");
    }
}

void MainWindow::updateMetrics()
{
    if (!tableMetrics || !lblHistogramROI || !lblProcessTime || !lblMemoryUsage) {
        return;
    }

    // Verificar si hay máscara de segmentación
    if (sliceContext.segmentationMask.empty() || sliceContext.originalRaw.empty()) {
        tableMetrics->setRowCount(0);
        lblHistogramROI->setText("No hay segmentación disponible");
        lblProcessTime->setText("Tiempo de Pipeline: 0 ms");
        lblMemoryUsage->setText("Memoria Estimada: 0 MB");
        return;
    }

    // ===== CÁLCULOS DE ROI =====
    cv::Mat mask = sliceContext.segmentationMask;
    cv::Mat original = sliceContext.originalRaw;

    // Calcular área (número de píxeles en la máscara)
    int area = cv::countNonZero(mask);

    // Calcular densidad media y desviación estándar dentro de la máscara
    cv::Scalar mean, stddev;
    cv::meanStdDev(original, mean, stddev, mask);

    // ===== ACTUALIZAR TABLA =====
    tableMetrics->setRowCount(0);
    tableMetrics->insertRow(0);
    
    tableMetrics->setItem(0, 0, new QTableWidgetItem("Pulmones"));
    tableMetrics->setItem(0, 1, new QTableWidgetItem(QString::number(area)));
    tableMetrics->setItem(0, 2, new QTableWidgetItem(QString::number(mean[0], 'f', 1) + " HU"));
    tableMetrics->setItem(0, 3, new QTableWidgetItem(QString::number(stddev[0], 'f', 2)));

    // ===== HISTOGRAMA DE LA ROI =====
    // Convertir imagen de 16-bit a 8-bit para el histograma
    cv::Mat original8bit = Bridge::normalize16to8bit(original);
    
    // Configuración del histograma
    int histSize = 256;
    float range[] = {0, 256}; // Rango para imágenes de 8 bits
    const float* histRange = {range};
    bool uniform = true;
    bool accumulate = false;

    cv::Mat hist;
    cv::calcHist(&original8bit, 1, 0, mask, hist, 1, &histSize, &histRange, uniform, accumulate);

    // Normalizar histograma
    double maxVal;
    cv::minMaxLoc(hist, nullptr, &maxVal);
    
    // Crear imagen del histograma
    int hist_w = 512;
    int hist_h = 300;
    int bin_w = cvRound((double) hist_w / histSize);
    
    cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(255, 255, 255));
    
    // Normalizar y dibujar
    for (int i = 1; i < histSize; i++) {
        int val1 = cvRound(hist.at<float>(i-1) * hist_h / maxVal);
        int val2 = cvRound(hist.at<float>(i) * hist_h / maxVal);
        
        cv::line(histImage,
                 cv::Point(bin_w * (i-1), hist_h - val1),
                 cv::Point(bin_w * i, hist_h - val2),
                 cv::Scalar(0, 120, 255), 2, cv::LINE_AA);
    }

    // Agregar ejes y título
    cv::putText(histImage, "Distribucion de Intensidades (ROI)",
                cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                cv::Scalar(0, 0, 0), 1, cv::LINE_AA);
    
    cv::line(histImage, cv::Point(30, hist_h - 30), cv::Point(30, 30),
             cv::Scalar(0, 0, 0), 2);
    cv::line(histImage, cv::Point(30, hist_h - 30), cv::Point(hist_w - 10, hist_h - 30),
             cv::Scalar(0, 0, 0), 2);

    // Convertir y mostrar
    QImage qImage = cvMatToQImage(histImage);
    lblHistogramROI->setPixmap(QPixmap::fromImage(qImage));

    // ===== RENDIMIENTO DEL SISTEMA =====
    // Actualizar tiempo de procesamiento
    lblProcessTime->setText(QString("Tiempo de Pipeline: %1 ms")
                            .arg(QString::number(sliceContext.processingTimeMs, 'f', 2)));

    // Calcular memoria estimada
    // Memoria = ancho * alto * bytes_por_pixel * número_de_capas
    double memoryMB = 0.0;
    
    if (!original.empty()) {
        // Original (16-bit): 2 bytes por pixel
        memoryMB += (original.cols * original.rows * 2) / (1024.0 * 1024.0);
    }
    
    if (!sliceContext.preprocessed.empty()) {
        // Preprocessed (8-bit): 1 byte por pixel
        memoryMB += (sliceContext.preprocessed.cols * sliceContext.preprocessed.rows) / (1024.0 * 1024.0);
    }
    
    if (!mask.empty()) {
        // Mask (8-bit): 1 byte por pixel
        memoryMB += (mask.cols * mask.rows) / (1024.0 * 1024.0);
    }
    
    if (!sliceContext.finalOverlay.empty()) {
        // Overlay (BGR 8-bit): 3 bytes por pixel
        memoryMB += (sliceContext.finalOverlay.cols * sliceContext.finalOverlay.rows * 3) / (1024.0 * 1024.0);
    }

    lblMemoryUsage->setText(QString("Memoria Estimada: %1 MB")
                            .arg(QString::number(memoryMB, 'f', 2)));
}
