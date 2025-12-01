#include "mainwindow.h"
#include <QScrollArea>
#include <QSplitter>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QElapsedTimer>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), tabWidget(nullptr), dncnnModelLoaded(false)
{
    // Intentar cargar modelo DnCNN al inicio - probar múltiples rutas
    std::vector<std::string> possiblePaths = {
        "../src/models/dncnn_grayscale.onnx",     // Desde build/
        "src/models/dncnn_grayscale.onnx",        // Desde raíz code2/
        "models/dncnn_grayscale.onnx",            // Por si models/ está en raíz
        "./models/dncnn_grayscale.onnx"           // Directorio actual
    };
    
    std::string modelPath;
    // PRIORIDAD 1: Configurar Flask Server
    std::cout << "[INFO] ========================================" << std::endl;
    std::cout << "[INFO] Configurando DnCNN via Flask Server" << std::endl;
    std::cout << "[INFO] ========================================" << std::endl;
    std::cout << "[INFO] URL: http://localhost:5000/denoise" << std::endl;
    dncnnDenoiser.setFlaskServer("http://localhost:5000/denoise");
    dncnnModelLoaded = true; // Flask está configurado
    std::cout << "[✓] Flask server configurado exitosamente" << std::endl;
    
    // PRIORIDAD 2: Cargar modelo local como fallback
    bool fileFound = false;
    
    std::cout << "\n[INFO] Buscando modelo DnCNN local (fallback)..." << std::endl;
    for (const auto& path : possiblePaths) {
        std::cout << "  Probando: " << path << " ... ";
        if (std::filesystem::exists(path)) {
            std::cout << "✓ ENCONTRADO" << std::endl;
            modelPath = path;
            fileFound = true;
            break;
        } else {
            std::cout << "✗" << std::endl;
        }
    }
    
    if (fileFound) {
        std::cout << "[INFO] Cargando modelo local como fallback desde: " << modelPath << std::endl;
        bool localLoaded = dncnnDenoiser.loadModel(modelPath);
        if (localLoaded) {
            std::cout << "[✓] Modelo DnCNN local cargado (fallback disponible)" << std::endl;
        } else {
            std::cerr << "[!] Modelo encontrado pero falló al cargar (solo Flask disponible)" << std::endl;
        }
    } else {
        std::cout << "[!] Modelo local no encontrado (solo Flask disponible)" << std::endl;
    }
    
    setupUI();
    setWindowTitle("Medical Vision App - Clean Architecture");
    resize(1400, 900);
}

MainWindow::~MainWindow() {
    // Qt se encarga de la limpieza de memoria
}

// ============================================================================
// CONFIGURACIÓN DE LA INTERFAZ
// ============================================================================

void MainWindow::setupUI() {
    // Widget central con tabs
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);
    
    // Crear todas las pestañas
    setupTabIO();
    setupTabPreprocessing();
    setupTabSegmentation();
    setupTabMorphology();
    setupTabVisualization();
    setupTabMetrics();
    
    // Agregar tabs al widget principal
    tabWidget->addTab(tabIO, "I/O Dataset");
    tabWidget->addTab(tabPreprocessing, "Preprocesamiento");
    tabWidget->addTab(tabSegmentation, "Segmentación");
    tabWidget->addTab(tabMorphology, "Morfología");
    tabWidget->addTab(tabVisualization, "Visualización");
    tabWidget->addTab(tabMetrics, "Métricas");
}

// ============================================================================
// PESTAÑA 1: I/O DATASET
// ============================================================================

void MainWindow::setupTabIO() {
    tabIO = new QWidget();
    
    // Layout principal: Splitter horizontal (Lista | Imagen)
    QHBoxLayout* mainLayout = new QHBoxLayout(tabIO);
    QSplitter* splitter = new QSplitter(Qt::Horizontal, tabIO);
    
    // === PANEL IZQUIERDO: Lista de archivos ===
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    
    btnLoadDataset = new QPushButton("📁 Cargar Dataset DICOM");
    btnLoadDataset->setMinimumHeight(40);
    connect(btnLoadDataset, &QPushButton::clicked, this, &MainWindow::onLoadDataset);
    
    fileListWidget = new QListWidget();
    fileListWidget->setMinimumWidth(300);
    connect(fileListWidget, &QListWidget::itemClicked, this, &MainWindow::onFileListItemClicked);
    
    leftLayout->addWidget(btnLoadDataset);
    leftLayout->addWidget(new QLabel("Archivos DICOM:"));
    leftLayout->addWidget(fileListWidget);
    
    // === PANEL DERECHO: Visualización de imagen ===
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    
    // Scroll area para la imagen
    QScrollArea* scrollArea = new QScrollArea();
    labelOriginalImage = new QLabel("Carga un dataset para comenzar");
    labelOriginalImage->setAlignment(Qt::AlignCenter);
    labelOriginalImage->setMinimumSize(512, 512);
    labelOriginalImage->setStyleSheet("QLabel { background-color: #2b2b2b; color: #ffffff; }");
    scrollArea->setWidget(labelOriginalImage);
    scrollArea->setWidgetResizable(true);
    
    // Controles de navegación
    QHBoxLayout* navigationLayout = new QHBoxLayout();
    labelSliceInfo = new QLabel("Slice: 0 / 0");
    sliceSlider = new QSlider(Qt::Horizontal);
    sliceSlider->setEnabled(false);
    sliceSlider->setMinimum(0);
    sliceSlider->setMaximum(0);
    connect(sliceSlider, &QSlider::valueChanged, this, &MainWindow::onSliceChanged);
    
    navigationLayout->addWidget(new QLabel("Navegación:"));
    navigationLayout->addWidget(sliceSlider, 1);
    navigationLayout->addWidget(labelSliceInfo);
    
    rightLayout->addWidget(scrollArea);
    rightLayout->addLayout(navigationLayout);
    
    // Agregar al splitter
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1); // Lista: 1 parte
    splitter->setStretchFactor(1, 3); // Imagen: 3 partes
    
    mainLayout->addWidget(splitter);
}

// ============================================================================
// SLOTS: PESTAÑA I/O DATASET
// ============================================================================

void MainWindow::onLoadDataset() {
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        "Seleccionar Carpeta con Archivos DICOM",
        QString::fromStdString(currentDatasetPath),
        QFileDialog::ShowDirsOnly
    );
    
    if (dirPath.isEmpty()) return;
    
    currentDatasetPath = dirPath.toStdString();
    datasetFiles.clear();
    fileListWidget->clear();
    currentSlice.clear();
    
    // Buscar archivos DICOM (.dcm, .IMA, sin extensión)
    try {
        for (const auto& entry : std::filesystem::directory_iterator(currentDatasetPath)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                std::string ext = entry.path().extension().string();
                
                // Aceptar .dcm, .IMA o archivos sin extensión
                if (ext == ".dcm" || ext == ".IMA" || ext == ".ima" || ext.empty()) {
                    datasetFiles.push_back(entry.path().string());
                    fileListWidget->addItem(QString::fromStdString(filename));
                }
            }
        }
        
        // Ordenar archivos alfabéticamente
        std::sort(datasetFiles.begin(), datasetFiles.end());
        
        if (datasetFiles.empty()) {
            QMessageBox::warning(this, "Sin archivos", 
                "No se encontraron archivos DICOM en la carpeta seleccionada.");
            return;
        }
        
        // Configurar slider
        sliceSlider->setEnabled(true);
        sliceSlider->setMaximum(datasetFiles.size() - 1);
        sliceSlider->setValue(0);
        
        // Cargar primer slice
        loadSlice(0);
        
        QMessageBox::information(this, "Dataset Cargado", 
            QString("Se encontraron %1 archivos DICOM.").arg(datasetFiles.size()));
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", 
            QString("Error al cargar dataset: %1").arg(e.what()));
    }
}

void MainWindow::onSliceChanged(int value) {
    if (value >= 0 && value < static_cast<int>(datasetFiles.size())) {
        loadSlice(value);
    }
}

void MainWindow::onFileListItemClicked(QListWidgetItem* item) {
    int index = fileListWidget->row(item);
    sliceSlider->setValue(index);
}

void MainWindow::loadSlice(int index) {
    if (index < 0 || index >= static_cast<int>(datasetFiles.size())) {
        return;
    }
    
    try {
        // 1. Leer DICOM usando backend
        std::string filePath = datasetFiles[index];
        auto itkImage = DicomIO::readDicomImage(filePath);
        
        // 2. Convertir a OpenCV
        currentSlice.originalRaw = Bridge::itkToOpenCV(itkImage);
        currentSlice.original8bit = Bridge::normalize16to8bit(currentSlice.originalRaw);
        currentSlice.sliceIndex = index;
        
        // 3. Limpiar procesamiento previo (nuevo slice = nuevo inicio)
        currentSlice.preprocessed.release();
        currentSlice.mask.release();
        currentSlice.morphologyMask.release();
        
        // 4. Actualizar UI
        updateImageDisplay(labelOriginalImage, currentSlice.original8bit);
        
        // También actualizar preview en pestaña de Preprocesamiento
        updateImageDisplay(labelOriginalPreview, currentSlice.original8bit);
        
        // También actualizar entrada en pestaña de Segmentación
        updateImageDisplay(labelSegmentationInput, currentSlice.original8bit);
        
        // Limpiar imagen procesada (nuevo slice)
        labelPreprocessedImage->clear();
        labelPreprocessedImage->setText("Aplica filtros para ver resultado");
        
        // Limpiar segmentación (nuevo slice)
        labelSegmentedImage->clear();
        labelSegmentedImage->setText("Ejecuta segmentación para ver resultado");
        currentSegmentedRegions.clear();
        
        // Limpiar morfología (nuevo slice)
        labelMorphologyInput->clear();
        labelMorphologyInput->setText("Ejecuta segmentación primero");
        labelMorphologyOutput->clear();
        labelMorphologyOutput->setText("Aplica operación para ver resultado");
        
        // Actualizar visualización con imagen original (sin segmentaciones)
        onResetVisualization();
        
        // Actualizar info
        labelSliceInfo->setText(
            QString("Slice: %1 / %2 | Tamaño: %3x%4")
                .arg(index + 1)
                .arg(datasetFiles.size())
                .arg(currentSlice.original8bit.cols)
                .arg(currentSlice.original8bit.rows)
        );
        
        // Resaltar en lista
        fileListWidget->setCurrentRow(index);
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", 
            QString("Error al cargar slice: %1").arg(e.what()));
    }
}

// ============================================================================
// PESTAÑA 2: PREPROCESAMIENTO
// ============================================================================

void MainWindow::setupTabPreprocessing() {
    tabPreprocessing = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(tabPreprocessing);
    
    // === PANEL IZQUIERDO: Controles ===
    QWidget* controlPanel = new QWidget();
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);
    controlPanel->setMaximumWidth(300);
    
    // Título
    QLabel* titleLabel = new QLabel("Configuración de Filtros");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    controlLayout->addWidget(titleLabel);
    
    // === Checkbox: Red Neuronal DnCNN ===
    checkUseDnCNN = new QCheckBox("Red Neuronal (DnCNN)");
    checkUseDnCNN->setEnabled(dncnnModelLoaded);
    if (!dncnnModelLoaded) {
        checkUseDnCNN->setToolTip("Modelo no encontrado en ../models/dncnn_grayscale.onnx");
    }
    controlLayout->addWidget(checkUseDnCNN);
    
    controlLayout->addSpacing(10);
    
    // === Checkbox: Filtro Gaussiano ===
    checkUseGaussian = new QCheckBox("Filtro Gaussiano");
    controlLayout->addWidget(checkUseGaussian);
    
    QHBoxLayout* gaussianLayout = new QHBoxLayout();
    gaussianLayout->addWidget(new QLabel("Kernel:"));
    sliderGaussianKernel = new QSlider(Qt::Horizontal);
    sliderGaussianKernel->setMinimum(1);
    sliderGaussianKernel->setMaximum(15);
    sliderGaussianKernel->setValue(3);
    sliderGaussianKernel->setSingleStep(2); // Solo valores impares
    sliderGaussianKernel->setEnabled(false);
    labelGaussianKernelValue = new QLabel("3");
    labelGaussianKernelValue->setMinimumWidth(30);
    gaussianLayout->addWidget(sliderGaussianKernel);
    gaussianLayout->addWidget(labelGaussianKernelValue);
    controlLayout->addLayout(gaussianLayout);
    
    connect(checkUseGaussian, &QCheckBox::toggled, [this](bool checked) {
        sliderGaussianKernel->setEnabled(checked);
    });
    connect(sliderGaussianKernel, &QSlider::valueChanged, [this](int value) {
        // Asegurar que sea impar
        if (value % 2 == 0) value++;
        labelGaussianKernelValue->setText(QString::number(value));
    });
    
    controlLayout->addSpacing(10);
    
    // === Checkbox: Filtro Mediana ===
    checkUseMedian = new QCheckBox("Filtro Mediana");
    controlLayout->addWidget(checkUseMedian);
    
    QHBoxLayout* medianLayout = new QHBoxLayout();
    medianLayout->addWidget(new QLabel("Kernel:"));
    sliderMedianKernel = new QSlider(Qt::Horizontal);
    sliderMedianKernel->setMinimum(1);
    sliderMedianKernel->setMaximum(15);
    sliderMedianKernel->setValue(3);
    sliderMedianKernel->setSingleStep(2);
    sliderMedianKernel->setEnabled(false);
    labelMedianKernelValue = new QLabel("3");
    labelMedianKernelValue->setMinimumWidth(30);
    medianLayout->addWidget(sliderMedianKernel);
    medianLayout->addWidget(labelMedianKernelValue);
    controlLayout->addLayout(medianLayout);
    
    connect(checkUseMedian, &QCheckBox::toggled, [this](bool checked) {
        sliderMedianKernel->setEnabled(checked);
    });
    connect(sliderMedianKernel, &QSlider::valueChanged, [this](int value) {
        if (value % 2 == 0) value++;
        labelMedianKernelValue->setText(QString::number(value));
    });
    
    controlLayout->addSpacing(10);
    
    // === Checkbox: Filtro Bilateral ===
    checkUseBilateral = new QCheckBox("Filtro Bilateral");
    controlLayout->addWidget(checkUseBilateral);
    
    QHBoxLayout* bilateralDLayout = new QHBoxLayout();
    bilateralDLayout->addWidget(new QLabel("Diámetro (d):"));
    sliderBilateralD = new QSlider(Qt::Horizontal);
    sliderBilateralD->setMinimum(3);
    sliderBilateralD->setMaximum(15);
    sliderBilateralD->setValue(9);
    sliderBilateralD->setSingleStep(2);
    sliderBilateralD->setEnabled(false);
    labelBilateralDValue = new QLabel("9");
    labelBilateralDValue->setMinimumWidth(30);
    bilateralDLayout->addWidget(sliderBilateralD);
    bilateralDLayout->addWidget(labelBilateralDValue);
    controlLayout->addLayout(bilateralDLayout);
    
    QHBoxLayout* bilateralSigmaLayout = new QHBoxLayout();
    bilateralSigmaLayout->addWidget(new QLabel("Sigma Color:"));
    sliderBilateralSigmaColor = new QSlider(Qt::Horizontal);
    sliderBilateralSigmaColor->setMinimum(10);
    sliderBilateralSigmaColor->setMaximum(200);
    sliderBilateralSigmaColor->setValue(75);
    sliderBilateralSigmaColor->setSingleStep(5);
    sliderBilateralSigmaColor->setEnabled(false);
    labelBilateralSigmaColorValue = new QLabel("75");
    labelBilateralSigmaColorValue->setMinimumWidth(30);
    bilateralSigmaLayout->addWidget(sliderBilateralSigmaColor);
    bilateralSigmaLayout->addWidget(labelBilateralSigmaColorValue);
    controlLayout->addLayout(bilateralSigmaLayout);
    
    connect(checkUseBilateral, &QCheckBox::toggled, [this](bool checked) {
        sliderBilateralD->setEnabled(checked);
        sliderBilateralSigmaColor->setEnabled(checked);
    });
    connect(sliderBilateralD, &QSlider::valueChanged, [this](int value) {
        if (value % 2 == 0) value++;
        labelBilateralDValue->setText(QString::number(value));
    });
    connect(sliderBilateralSigmaColor, &QSlider::valueChanged, [this](int value) {
        labelBilateralSigmaColorValue->setText(QString::number(value));
    });
    
    controlLayout->addSpacing(10);
    
    // === Checkbox: CLAHE ===
    checkUseCLAHE = new QCheckBox("CLAHE (Mejora de Contraste)");
    controlLayout->addWidget(checkUseCLAHE);
    
    controlLayout->addSpacing(20);
    
    // === Botón Aplicar ===
    btnApplyPreprocessing = new QPushButton("🔄 Aplicar Preprocesamiento");
    btnApplyPreprocessing->setMinimumHeight(40);
    btnApplyPreprocessing->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    connect(btnApplyPreprocessing, &QPushButton::clicked, this, &MainWindow::onApplyPreprocessing);
    controlLayout->addWidget(btnApplyPreprocessing);
    
    controlLayout->addSpacing(10);
    
    // === Log de Preprocesamiento ===
    QLabel* logLabel = new QLabel("Log de Operaciones:");
    logLabel->setStyleSheet("font-weight: bold;");
    controlLayout->addWidget(logLabel);
    
    textPreprocessingLog = new QTextEdit();
    textPreprocessingLog->setReadOnly(true);
    textPreprocessingLog->setMaximumHeight(150);
    textPreprocessingLog->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; font-family: monospace; }");
    controlLayout->addWidget(textPreprocessingLog);
    
    controlLayout->addStretch();
    
    // === PANEL DERECHO: Vista Antes/Después ===
    QWidget* imagePanel = new QWidget();
    QHBoxLayout* imageLayout = new QHBoxLayout(imagePanel);
    
    // Imagen Original
    QWidget* originalWidget = new QWidget();
    QVBoxLayout* originalLayout = new QVBoxLayout(originalWidget);
    QLabel* originalTitle = new QLabel("ORIGINAL");
    originalTitle->setAlignment(Qt::AlignCenter);
    originalTitle->setStyleSheet("font-weight: bold; font-size: 12px; color: #888;");
    
    QScrollArea* scrollOriginal = new QScrollArea();
    labelOriginalPreview = new QLabel("Carga una imagen primero");
    labelOriginalPreview->setAlignment(Qt::AlignCenter);
    labelOriginalPreview->setMinimumSize(400, 400);
    labelOriginalPreview->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    scrollOriginal->setWidget(labelOriginalPreview);
    scrollOriginal->setWidgetResizable(true);
    
    originalLayout->addWidget(originalTitle);
    originalLayout->addWidget(scrollOriginal);
    
    // Imagen Procesada
    QWidget* processedWidget = new QWidget();
    QVBoxLayout* processedLayout = new QVBoxLayout(processedWidget);
    QLabel* processedTitle = new QLabel("PROCESADA");
    processedTitle->setAlignment(Qt::AlignCenter);
    processedTitle->setStyleSheet("font-weight: bold; font-size: 12px; color: #4CAF50;");
    
    QScrollArea* scrollProcessed = new QScrollArea();
    labelPreprocessedImage = new QLabel("Aplica filtros para ver resultado");
    labelPreprocessedImage->setAlignment(Qt::AlignCenter);
    labelPreprocessedImage->setMinimumSize(400, 400);
    labelPreprocessedImage->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    scrollProcessed->setWidget(labelPreprocessedImage);
    scrollProcessed->setWidgetResizable(true);
    
    processedLayout->addWidget(processedTitle);
    processedLayout->addWidget(scrollProcessed);
    
    // Agregar ambas vistas
    imageLayout->addWidget(originalWidget);
    imageLayout->addWidget(processedWidget);
    
    // Agregar paneles al layout principal
    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(imagePanel, 1);
}

// ============================================================================
// PESTAÑA 3: SEGMENTACIÓN
// ============================================================================

void MainWindow::setupTabSegmentation() {
    tabSegmentation = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(tabSegmentation);
    
    // === PANEL IZQUIERDO: Controles ===
    QWidget* controlPanel = new QWidget();
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);
    controlPanel->setMaximumWidth(320);
    
    // Título
    QLabel* titleLabel = new QLabel("Algoritmos de Segmentación");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    controlLayout->addWidget(titleLabel);
    
    controlLayout->addSpacing(10);
    
    // === Checkbox: Usar imagen preprocesada ===
    checkUsePreprocessed = new QCheckBox("Usar imagen preprocesada");
    checkUsePreprocessed->setToolTip("Si está desmarcado, se usará la imagen original de 16-bit");
    controlLayout->addWidget(checkUsePreprocessed);
    
    controlLayout->addSpacing(15);
    
    // === Botones de Segmentación por Órgano ===
    QLabel* organsLabel = new QLabel("Seleccionar Órgano:");
    organsLabel->setStyleSheet("font-weight: bold;");
    controlLayout->addWidget(organsLabel);
    
    // Botón: Pulmones
    btnSegmentLungs = new QPushButton("Segmentar PULMONES");
    btnSegmentLungs->setMinimumHeight(50);
    btnSegmentLungs->setStyleSheet(
        "QPushButton { "
        "   background-color: #2196F3; "
        "   color: white; "
        "   font-weight: bold; "
        "   font-size: 13px; "
        "   border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #1976D2; }"
    );
    connect(btnSegmentLungs, &QPushButton::clicked, this, &MainWindow::onSegmentLungs);
    controlLayout->addWidget(btnSegmentLungs);
    
    // Botón: Huesos
    btnSegmentBones = new QPushButton("Segmentar HUESOS");
    btnSegmentBones->setMinimumHeight(50);
    btnSegmentBones->setStyleSheet(
        "QPushButton { "
        "   background-color: #FF9800; "
        "   color: white; "
        "   font-weight: bold; "
        "   font-size: 13px; "
        "   border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #F57C00; }"
    );
    connect(btnSegmentBones, &QPushButton::clicked, this, &MainWindow::onSegmentBones);
    controlLayout->addWidget(btnSegmentBones);
    
    // Botón: Aorta
    btnSegmentAorta = new QPushButton("Segmentar AORTA");
    btnSegmentAorta->setMinimumHeight(50);
    btnSegmentAorta->setStyleSheet(
        "QPushButton { "
        "   background-color: #E91E63; "
        "   color: white; "
        "   font-weight: bold; "
        "   font-size: 13px; "
        "   border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #C2185B; }"
    );
    connect(btnSegmentAorta, &QPushButton::clicked, this, &MainWindow::onSegmentAorta);
    controlLayout->addWidget(btnSegmentAorta);
    
    controlLayout->addSpacing(10);
    
    // Botón: Limpiar
    btnClearSegmentation = new QPushButton("Limpiar Segmentación");
    btnClearSegmentation->setMinimumHeight(35);
    btnClearSegmentation->setStyleSheet(
        "QPushButton { "
        "   background-color: #757575; "
        "   color: white; "
        "   font-weight: bold; "
        "   border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #616161; }"
    );
    connect(btnClearSegmentation, &QPushButton::clicked, this, &MainWindow::onClearSegmentation);
    controlLayout->addWidget(btnClearSegmentation);
    
    controlLayout->addSpacing(20);
    
    // === Ajustes de Rango HU (Opcional) ===
    QGroupBox* huGroup = new QGroupBox("Ajuste de Rango HU");
    QVBoxLayout* huLayout = new QVBoxLayout(huGroup);
    
    // Checkbox para activar modo manual
    checkUseCustomHU = new QCheckBox("Usar rangos HU personalizados");
    checkUseCustomHU->setToolTip("Activa para modificar manualmente los umbrales de Hounsfield Units");
    huLayout->addWidget(checkUseCustomHU);
    
    huLayout->addSpacing(5);
    
    // Min HU
    QHBoxLayout* minHULayout = new QHBoxLayout();
    labelMinHU = new QLabel("Min HU:");
    spinMinHU = new QSpinBox();
    spinMinHU->setMinimum(-3000);
    spinMinHU->setMaximum(3000);
    spinMinHU->setValue(-1000);
    spinMinHU->setSingleStep(10);
    spinMinHU->setEnabled(false);
    spinMinHU->setMinimumWidth(100);
    minHULayout->addWidget(labelMinHU);
    minHULayout->addWidget(spinMinHU);
    minHULayout->addStretch();
    
    // Max HU
    QHBoxLayout* maxHULayout = new QHBoxLayout();
    labelMaxHU = new QLabel("Max HU:");
    spinMaxHU = new QSpinBox();
    spinMaxHU->setMinimum(-3000);
    spinMaxHU->setMaximum(3000);
    spinMaxHU->setValue(3000);
    spinMaxHU->setSingleStep(10);
    spinMaxHU->setEnabled(false);
    spinMaxHU->setMinimumWidth(100);
    maxHULayout->addWidget(labelMaxHU);
    maxHULayout->addWidget(spinMaxHU);
    maxHULayout->addStretch();
    
    huLayout->addLayout(minHULayout);
    huLayout->addLayout(maxHULayout);
    
    // Botones de presets
    QHBoxLayout* presetsLayout = new QHBoxLayout();
    QPushButton* btnPresetLungs = new QPushButton("Pulmones");
    QPushButton* btnPresetBones = new QPushButton("Huesos");
    QPushButton* btnPresetAorta = new QPushButton("Aorta");
    
    btnPresetLungs->setMaximumWidth(80);
    btnPresetBones->setMaximumWidth(80);
    btnPresetAorta->setMaximumWidth(80);
    
    presetsLayout->addWidget(btnPresetLungs);
    presetsLayout->addWidget(btnPresetBones);
    presetsLayout->addWidget(btnPresetAorta);
    presetsLayout->addStretch();
    huLayout->addLayout(presetsLayout);
    
    // Conectar checkbox para habilitar/deshabilitar
    connect(checkUseCustomHU, &QCheckBox::toggled, [this](bool checked) {
        spinMinHU->setEnabled(checked);
        spinMaxHU->setEnabled(checked);
    });
    
    // Conectar botones de presets
    connect(btnPresetLungs, &QPushButton::clicked, [this]() {
        checkUseCustomHU->setChecked(true);
        spinMinHU->setValue(-1000);
        spinMaxHU->setValue(-400);
        logMessage(textSegmentationLog, "[PRESET] Pulmones: -1000 a -400 HU");
    });
    
    connect(btnPresetBones, &QPushButton::clicked, [this]() {
        checkUseCustomHU->setChecked(true);
        spinMinHU->setValue(200);
        spinMaxHU->setValue(3000);
        logMessage(textSegmentationLog, "[PRESET] Huesos: 200 a 3000 HU");
    });
    
    connect(btnPresetAorta, &QPushButton::clicked, [this]() {
        checkUseCustomHU->setChecked(true);
        spinMinHU->setValue(120);
        spinMaxHU->setValue(400);
        logMessage(textSegmentationLog, "[PRESET] Aorta: 120 a 400 HU");
    });
    
    controlLayout->addWidget(huGroup);
    controlLayout->addSpacing(10);
    
    // === Log de Segmentación ===
    QLabel* logLabel = new QLabel("Log de Segmentación:");
    logLabel->setStyleSheet("font-weight: bold;");
    controlLayout->addWidget(logLabel);
    
    textSegmentationLog = new QTextEdit();
    textSegmentationLog->setReadOnly(true);
    textSegmentationLog->setMaximumHeight(150);
    textSegmentationLog->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; font-family: monospace; }");
    controlLayout->addWidget(textSegmentationLog);
    
    controlLayout->addStretch();
    
    // === PANEL DERECHO: Vista Entrada/Máscara ===
    QWidget* imagePanel = new QWidget();
    QHBoxLayout* imageLayout = new QHBoxLayout(imagePanel);
    
    // Imagen de Entrada
    QWidget* inputWidget = new QWidget();
    QVBoxLayout* inputLayout = new QVBoxLayout(inputWidget);
    QLabel* inputTitle = new QLabel("ENTRADA (Original/Preprocesada)");
    inputTitle->setAlignment(Qt::AlignCenter);
    inputTitle->setStyleSheet("font-weight: bold; font-size: 12px; color: #888;");
    
    QScrollArea* scrollInput = new QScrollArea();
    labelSegmentationInput = new QLabel("Carga una imagen primero");
    labelSegmentationInput->setAlignment(Qt::AlignCenter);
    labelSegmentationInput->setMinimumSize(400, 400);
    labelSegmentationInput->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    scrollInput->setWidget(labelSegmentationInput);
    scrollInput->setWidgetResizable(true);
    
    inputLayout->addWidget(inputTitle);
    inputLayout->addWidget(scrollInput);
    
    // Máscara Segmentada
    QWidget* maskWidget = new QWidget();
    QVBoxLayout* maskLayout = new QVBoxLayout(maskWidget);
    QLabel* maskTitle = new QLabel("MÁSCARA SEGMENTADA");
    maskTitle->setAlignment(Qt::AlignCenter);
    maskTitle->setStyleSheet("font-weight: bold; font-size: 12px; color: #4CAF50;");
    
    QScrollArea* scrollMask = new QScrollArea();
    labelSegmentedImage = new QLabel("Ejecuta segmentación para ver resultado");
    labelSegmentedImage->setAlignment(Qt::AlignCenter);
    labelSegmentedImage->setMinimumSize(400, 400);
    labelSegmentedImage->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    scrollMask->setWidget(labelSegmentedImage);
    scrollMask->setWidgetResizable(true);
    
    maskLayout->addWidget(maskTitle);
    maskLayout->addWidget(scrollMask);
    
    // Agregar ambas vistas
    imageLayout->addWidget(inputWidget);
    imageLayout->addWidget(maskWidget);
    
    // Agregar paneles al layout principal
    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(imagePanel, 1);
}

// ============================================================================
// PESTAÑA 4: MORFOLOGÍA
// ============================================================================

void MainWindow::setupTabMorphology() {
    tabMorphology = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(tabMorphology);
    
    // === PANEL IZQUIERDO: Controles ===
    QWidget* controlPanel = new QWidget();
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);
    controlPanel->setMaximumWidth(320);
    
    // Título
    QLabel* titleLabel = new QLabel("Operaciones Morfológicas");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    controlLayout->addWidget(titleLabel);
    
    QLabel* descLabel = new QLabel("Refina las máscaras de segmentación");
    descLabel->setStyleSheet("color: #888; font-size: 11px;");
    descLabel->setWordWrap(true);
    controlLayout->addWidget(descLabel);
    
    controlLayout->addSpacing(15);
    
    // === ComboBox: Operación Morfológica ===
    QLabel* opLabel = new QLabel("Operación:");
    opLabel->setStyleSheet("font-weight: bold;");
    controlLayout->addWidget(opLabel);
    
    comboMorphologyOperation = new QComboBox();
    comboMorphologyOperation->addItem("Erosión", "erosion");
    comboMorphologyOperation->addItem("Dilatación", "dilation");
    comboMorphologyOperation->addItem("Apertura (Opening)", "opening");
    comboMorphologyOperation->addItem("Cierre (Closing)", "closing");
    comboMorphologyOperation->setCurrentIndex(2); // Opening por defecto
    controlLayout->addWidget(comboMorphologyOperation);
    
    controlLayout->addSpacing(15);
    
    // === Slider: Tamaño de Kernel ===
    QLabel* kernelLabel = new QLabel("Tamaño de Kernel:");
    kernelLabel->setStyleSheet("font-weight: bold;");
    controlLayout->addWidget(kernelLabel);
    
    QHBoxLayout* kernelLayout = new QHBoxLayout();
    sliderMorphKernelSize = new QSlider(Qt::Horizontal);
    sliderMorphKernelSize->setMinimum(1);
    sliderMorphKernelSize->setMaximum(21);
    sliderMorphKernelSize->setValue(5);
    sliderMorphKernelSize->setSingleStep(2); // Solo valores impares
    labelMorphKernelValue = new QLabel("5x5");
    labelMorphKernelValue->setMinimumWidth(50);
    labelMorphKernelValue->setStyleSheet("font-weight: bold; color: #4CAF50;");
    kernelLayout->addWidget(sliderMorphKernelSize, 1);
    kernelLayout->addWidget(labelMorphKernelValue);
    controlLayout->addLayout(kernelLayout);
    
    connect(sliderMorphKernelSize, &QSlider::valueChanged, [this](int value) {
        // Asegurar que sea impar
        if (value % 2 == 0) value++;
        labelMorphKernelValue->setText(QString("%1x%1").arg(value));
    });
    
    controlLayout->addSpacing(10);
    
    // === Slider: Iteraciones ===
    QLabel* iterLabel = new QLabel("Iteraciones:");
    iterLabel->setStyleSheet("font-weight: bold;");
    controlLayout->addWidget(iterLabel);
    
    QHBoxLayout* iterLayout = new QHBoxLayout();
    sliderIterations = new QSlider(Qt::Horizontal);
    sliderIterations->setMinimum(1);
    sliderIterations->setMaximum(10);
    sliderIterations->setValue(1);
    labelIterationsValue = new QLabel("1");
    labelIterationsValue->setMinimumWidth(50);
    labelIterationsValue->setStyleSheet("font-weight: bold; color: #2196F3;");
    iterLayout->addWidget(sliderIterations, 1);
    iterLayout->addWidget(labelIterationsValue);
    controlLayout->addLayout(iterLayout);
    
    connect(sliderIterations, &QSlider::valueChanged, [this](int value) {
        labelIterationsValue->setText(QString::number(value));
    });
    
    controlLayout->addSpacing(20);
    
    // === Botón: Aplicar Morfología ===
    btnApplyMorphology = new QPushButton("Aplicar Operación");
    btnApplyMorphology->setMinimumHeight(45);
    btnApplyMorphology->setStyleSheet(
        "QPushButton { "
        "   background-color: #9C27B0; "
        "   color: white; "
        "   font-weight: bold; "
        "   font-size: 13px; "
        "   border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #7B1FA2; }"
    );
    connect(btnApplyMorphology, &QPushButton::clicked, this, &MainWindow::onApplyMorphology);
    controlLayout->addWidget(btnApplyMorphology);
    
    controlLayout->addSpacing(10);
    
    // === Botón: Rellenar Huecos ===
    btnFillHoles = new QPushButton("Rellenar Huecos");
    btnFillHoles->setMinimumHeight(40);
    btnFillHoles->setStyleSheet(
        "QPushButton { "
        "   background-color: #00BCD4; "
        "   color: white; "
        "   font-weight: bold; "
        "   border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #0097A7; }"
    );
    connect(btnFillHoles, &QPushButton::clicked, [this]() {
        if (!currentSlice.hasData() || currentSlice.mask.empty()) {
            QMessageBox::warning(this, "Sin máscara", "Ejecuta segmentación primero.");
            return;
        }
        
        try {
            textMorphologyLog->clear();
            logMessage(textMorphologyLog, "[INICIO] Rellenando huecos...");
            
            cv::Mat input = currentSlice.morphologyMask.empty() ? 
                           currentSlice.mask.clone() : 
                           currentSlice.morphologyMask.clone();
            
            cv::Mat filled = Morphology::fillHoles(input);
            currentSlice.morphologyMask = filled.clone();
            
            updateImageDisplay(labelMorphologyOutput, currentSlice.morphologyMask);
            logMessage(textMorphologyLog, "✓ Huecos rellenados exitosamente");
            
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Error: %1").arg(e.what()));
        }
    });
    controlLayout->addWidget(btnFillHoles);
    
    controlLayout->addSpacing(15);
    
    // === Información ===
    QGroupBox* infoGroup = new QGroupBox("Información");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    infoLayout->addWidget(new QLabel("<b>Erosión:</b> Reduce regiones blancas"));
    infoLayout->addWidget(new QLabel("<b>Dilatación:</b> Expande regiones blancas"));
    infoLayout->addWidget(new QLabel("<b>Apertura:</b> Erosión + Dilatación (elimina ruido)"));
    infoLayout->addWidget(new QLabel("<b>Cierre:</b> Dilatación + Erosión (rellena huecos pequeños)"));
    infoGroup->setStyleSheet("QGroupBox { color: #888; font-size: 10px; }");
    controlLayout->addWidget(infoGroup);
    
    controlLayout->addSpacing(10);
    
    // === Log de Morfología ===
    QLabel* logLabel = new QLabel("Log de Operaciones:");
    logLabel->setStyleSheet("font-weight: bold;");
    controlLayout->addWidget(logLabel);
    
    textMorphologyLog = new QTextEdit();
    textMorphologyLog->setReadOnly(true);
    textMorphologyLog->setMaximumHeight(120);
    textMorphologyLog->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; font-family: monospace; font-size: 10px; }");
    controlLayout->addWidget(textMorphologyLog);
    
    controlLayout->addStretch();
    
    // === PANEL DERECHO: Vista Antes/Después ===
    QWidget* imagePanel = new QWidget();
    QHBoxLayout* imageLayout = new QHBoxLayout(imagePanel);
    
    // Máscara de Entrada
    QWidget* inputWidget = new QWidget();
    QVBoxLayout* inputLayout = new QVBoxLayout(inputWidget);
    QLabel* inputTitle = new QLabel("MÁSCARA ORIGINAL");
    inputTitle->setAlignment(Qt::AlignCenter);
    inputTitle->setStyleSheet("font-weight: bold; font-size: 12px; color: #888;");
    
    QScrollArea* scrollInput = new QScrollArea();
    labelMorphologyInput = new QLabel("Ejecuta segmentación primero");
    labelMorphologyInput->setAlignment(Qt::AlignCenter);
    labelMorphologyInput->setMinimumSize(400, 400);
    labelMorphologyInput->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    scrollInput->setWidget(labelMorphologyInput);
    scrollInput->setWidgetResizable(true);
    
    inputLayout->addWidget(inputTitle);
    inputLayout->addWidget(scrollInput);
    
    // Máscara Refinada
    QWidget* outputWidget = new QWidget();
    QVBoxLayout* outputLayout = new QVBoxLayout(outputWidget);
    QLabel* outputTitle = new QLabel("MÁSCARA REFINADA");
    outputTitle->setAlignment(Qt::AlignCenter);
    outputTitle->setStyleSheet("font-weight: bold; font-size: 12px; color: #9C27B0;");
    
    QScrollArea* scrollOutput = new QScrollArea();
    labelMorphologyOutput = new QLabel("Aplica operación para ver resultado");
    labelMorphologyOutput->setAlignment(Qt::AlignCenter);
    labelMorphologyOutput->setMinimumSize(400, 400);
    labelMorphologyOutput->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    scrollOutput->setWidget(labelMorphologyOutput);
    scrollOutput->setWidgetResizable(true);
    
    outputLayout->addWidget(outputTitle);
    outputLayout->addWidget(scrollOutput);
    
    // Agregar ambas vistas
    imageLayout->addWidget(inputWidget);
    imageLayout->addWidget(outputWidget);
    
    // Agregar paneles al layout principal
    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(imagePanel, 1);
}

// ============================================================================
// PESTAÑA 5: VISUALIZACIÓN
// ============================================================================

void MainWindow::setupTabVisualization() {
    tabVisualization = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(tabVisualization);
    
    // === PANEL SUPERIOR: Controles ===
    QWidget* controlPanel = new QWidget();
    QHBoxLayout* controlLayout = new QHBoxLayout(controlPanel);
    controlPanel->setMaximumHeight(150);
    
    // === Grupo: Opciones de Visualización ===
    QGroupBox* optionsGroup = new QGroupBox("Opciones de Visualización");
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);
    
    checkShowOverlay = new QCheckBox("Mostrar Overlay de Colores");
    checkShowOverlay->setChecked(true);
    checkShowOverlay->setStyleSheet("font-weight: bold;");
    connect(checkShowOverlay, &QCheckBox::toggled, this, &MainWindow::onUpdateVisualization);
    
    checkShowContours = new QCheckBox("Mostrar Contornos");
    checkShowContours->setChecked(true);
    connect(checkShowContours, &QCheckBox::toggled, this, &MainWindow::onUpdateVisualization);
    
    checkShowLabels = new QCheckBox("Mostrar Etiquetas");
    checkShowLabels->setChecked(true);
    connect(checkShowLabels, &QCheckBox::toggled, this, &MainWindow::onUpdateVisualization);
    
    optionsLayout->addWidget(checkShowOverlay);
    optionsLayout->addWidget(checkShowContours);
    optionsLayout->addWidget(checkShowLabels);
    
    // === Grupo: Control de Opacidad ===
    QGroupBox* opacityGroup = new QGroupBox("Control de Opacidad");
    QVBoxLayout* opacityLayout = new QVBoxLayout(opacityGroup);
    
    QLabel* opacityLabel = new QLabel("Opacidad del Overlay:");
    opacityLabel->setStyleSheet("font-weight: bold;");
    
    QHBoxLayout* sliderLayout = new QHBoxLayout();
    sliderOverlayOpacity = new QSlider(Qt::Horizontal);
    sliderOverlayOpacity->setMinimum(0);
    sliderOverlayOpacity->setMaximum(100);
    sliderOverlayOpacity->setValue(30); // 30% por defecto
    labelOpacityValue = new QLabel("30%");
    labelOpacityValue->setMinimumWidth(50);
    labelOpacityValue->setStyleSheet("font-weight: bold; color: #4CAF50; font-size: 14px;");
    sliderLayout->addWidget(sliderOverlayOpacity, 1);
    sliderLayout->addWidget(labelOpacityValue);
    
    connect(sliderOverlayOpacity, &QSlider::valueChanged, [this](int value) {
        labelOpacityValue->setText(QString("%1%").arg(value));
        onUpdateVisualization();
    });
    
    opacityLayout->addWidget(opacityLabel);
    opacityLayout->addLayout(sliderLayout);
    
    // === Botones de Acción ===
    QVBoxLayout* buttonsLayout = new QVBoxLayout();
    
    btnUpdateVisualization = new QPushButton("Actualizar Vista");
    btnUpdateVisualization->setMinimumHeight(40);
    btnUpdateVisualization->setStyleSheet(
        "QPushButton { "
        "   background-color: #4CAF50; "
        "   color: white; "
        "   font-weight: bold; "
        "   border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #45a049; }"
    );
    connect(btnUpdateVisualization, &QPushButton::clicked, this, &MainWindow::onUpdateVisualization);
    
    btnResetVisualization = new QPushButton("↺ Resetear a Original");
    btnResetVisualization->setMinimumHeight(40);
    btnResetVisualization->setStyleSheet(
        "QPushButton { "
        "   background-color: #757575; "
        "   color: white; "
        "   font-weight: bold; "
        "   border-radius: 5px; "
        "}"
        "QPushButton:hover { background-color: #616161; }"
    );
    connect(btnResetVisualization, &QPushButton::clicked, this, &MainWindow::onResetVisualization);
    
    buttonsLayout->addWidget(btnUpdateVisualization);
    buttonsLayout->addWidget(btnResetVisualization);
    buttonsLayout->addStretch();
    
    // Agregar grupos al panel de control
    controlLayout->addWidget(optionsGroup);
    controlLayout->addWidget(opacityGroup);
    controlLayout->addLayout(buttonsLayout);
    controlLayout->addStretch();
    
    // === PANEL INFERIOR: Visor de Imagen ===
    QWidget* viewerPanel = new QWidget();
    QVBoxLayout* viewerLayout = new QVBoxLayout(viewerPanel);
    
    QLabel* viewerTitle = new QLabel("VISUALIZACIÓN COMPUESTA");
    viewerTitle->setAlignment(Qt::AlignCenter);
    viewerTitle->setStyleSheet("font-weight: bold; font-size: 14px; color: #4CAF50;");
    
    QScrollArea* scrollArea = new QScrollArea();
    labelVisualizationImage = new QLabel("Carga una imagen y ejecuta segmentación");
    labelVisualizationImage->setAlignment(Qt::AlignCenter);
    labelVisualizationImage->setMinimumSize(600, 600);
    labelVisualizationImage->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; }");
    scrollArea->setWidget(labelVisualizationImage);
    scrollArea->setWidgetResizable(true);
    
    viewerLayout->addWidget(viewerTitle);
    viewerLayout->addWidget(scrollArea);
    
    // Agregar paneles al layout principal
    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(viewerPanel, 1);
}

// ============================================================================
// PESTAÑA 6: MÉTRICAS
// ============================================================================

void MainWindow::setupTabMetrics() {
    tabMetrics = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(tabMetrics);
    
    // === TÍTULO Y BOTÓN ===
    QWidget* headerPanel = new QWidget();
    QHBoxLayout* headerLayout = new QHBoxLayout(headerPanel);
    
    QLabel* titleLabel = new QLabel("Dashboard de Métricas y Rendimiento");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #00BCD4;");
    
    btnCalculateMetrics = new QPushButton("⟳ Actualizar Métricas");
    btnCalculateMetrics->setMinimumHeight(40);
    btnCalculateMetrics->setStyleSheet(
        "QPushButton { "
        "   background-color: #00BCD4; "
        "   color: white; "
        "   font-weight: bold; "
        "   font-size: 13px; "
        "   border-radius: 5px; "
        "   padding: 0 20px; "
        "}"
        "QPushButton:hover { background-color: #0097A7; }"
    );
    connect(btnCalculateMetrics, &QPushButton::clicked, this, &MainWindow::onCalculateMetrics);
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(btnCalculateMetrics);
    mainLayout->addWidget(headerPanel);
    
    // === SECCIÓN 1: TABLA DE MÉTRICAS ===
    QGroupBox* tableGroup = new QGroupBox("Estadísticas de Segmentación");
    QVBoxLayout* tableLayout = new QVBoxLayout(tableGroup);
    
    tableMetrics = new QTableWidget();
    tableMetrics->setColumnCount(5);
    tableMetrics->setHorizontalHeaderLabels({
        "Estructura", 
        "Área (px)", 
        "Densidad Media (HU)", 
        "SNR (dB)", 
        "PSNR vs Original (dB)"
    });
    tableMetrics->setMinimumHeight(200);
    tableMetrics->setMaximumHeight(300);
    tableMetrics->horizontalHeader()->setStretchLastSection(true);
    tableMetrics->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableMetrics->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableMetrics->setStyleSheet(
        "QTableWidget { "
        "   background-color: #2b2b2b; "
        "   color: #d4d4d4; "
        "   gridline-color: #444; "
        "   font-size: 11px; "
        "}"
        "QHeaderView::section { "
        "   background-color: #1e1e1e; "
        "   color: #00BCD4; "
        "   font-weight: bold; "
        "   padding: 5px; "
        "   border: 1px solid #444; "
        "}"
    );
    
    tableLayout->addWidget(tableMetrics);
    mainLayout->addWidget(tableGroup);
    
    // === SECCIÓN 2: HISTOGRAMA ROI ===
    QGroupBox* histogramGroup = new QGroupBox("Histograma de Región de Interés (ROI)");
    QVBoxLayout* histogramLayout = new QVBoxLayout(histogramGroup);
    
    labelHistogram = new QLabel("Segmenta una región para ver su histograma");
    labelHistogram->setAlignment(Qt::AlignCenter);
    labelHistogram->setMinimumHeight(250);
    labelHistogram->setMaximumHeight(350);
    labelHistogram->setStyleSheet(
        "QLabel { "
        "   background-color: #2b2b2b; "
        "   color: #888; "
        "   border: 2px solid #444; "
        "   border-radius: 5px; "
        "}"
    );
    
    histogramLayout->addWidget(labelHistogram);
    mainLayout->addWidget(histogramGroup);
    
    // === SECCIÓN 3: MÉTRICAS DE RENDIMIENTO ===
    QGroupBox* performanceGroup = new QGroupBox("Rendimiento del Sistema");
    QHBoxLayout* performanceLayout = new QHBoxLayout(performanceGroup);
    
    labelPerformanceInfo = new QLabel("⏱️ Tiempo de Procesamiento: -- ms");
    labelPerformanceInfo->setStyleSheet("font-size: 12px; color: #4CAF50; font-weight: bold;");
    
    labelMemoryInfo = new QLabel("💾 Memoria Estimada: -- MB");
    labelMemoryInfo->setStyleSheet("font-size: 12px; color: #FF9800; font-weight: bold;");
    
    performanceLayout->addWidget(labelPerformanceInfo);
    performanceLayout->addStretch();
    performanceLayout->addWidget(labelMemoryInfo);
    
    mainLayout->addWidget(performanceGroup);
    mainLayout->addStretch();
}

// ============================================================================
// SLOTS: PESTAÑA PREPROCESAMIENTO
// ============================================================================

void MainWindow::onApplyPreprocessing() {
    // Validar que hay datos cargados
    if (!currentSlice.hasData()) {
        QMessageBox::warning(this, "Sin datos", "Carga un dataset primero en la pestaña I/O.");
        return;
    }
    
    textPreprocessingLog->clear();
    logMessage(textPreprocessingLog, "[INICIO] Aplicando preprocesamiento...");
    
    // Actualizar imagen original en el preview
    updateImageDisplay(labelOriginalPreview, currentSlice.original8bit);
    
    // Imagen de trabajo (empezar con la original)
    cv::Mat working = currentSlice.original8bit.clone();
    int filtersApplied = 0;
    
    try {
        // === 1. Red Neuronal DnCNN ===
        if (checkUseDnCNN->isChecked() && dncnnModelLoaded) {
            logMessage(textPreprocessingLog, "[1/4] Aplicando Red Neuronal DnCNN...");
            working = dncnnDenoiser.denoise(working);
            logMessage(textPreprocessingLog, "   ✓ DnCNN aplicado exitosamente");
            filtersApplied++;
        } else if (checkUseDnCNN->isChecked() && !dncnnModelLoaded) {
            logMessage(textPreprocessingLog, "[!] DnCNN no disponible (modelo no cargado)");
        }
        
        // === 2. Filtro Gaussiano ===
        if (checkUseGaussian->isChecked()) {
            int kernel = sliderGaussianKernel->value();
            if (kernel % 2 == 0) kernel++; // Asegurar impar
            
            logMessage(textPreprocessingLog, QString("[2/4] Aplicando Filtro Gaussiano (kernel=%1)...").arg(kernel));
            working = Preprocessing::applyGaussianFilter(working, kernel);
            logMessage(textPreprocessingLog, "   ✓ Filtro Gaussiano aplicado");
            filtersApplied++;
        }
        
        // === 3. Filtro Mediana ===
        if (checkUseMedian->isChecked()) {
            int kernel = sliderMedianKernel->value();
            if (kernel % 2 == 0) kernel++;
            
            logMessage(textPreprocessingLog, QString("[3/5] Aplicando Filtro Mediana (kernel=%1)...").arg(kernel));
            working = Preprocessing::applyMedianFilter(working, kernel);
            logMessage(textPreprocessingLog, "   ✓ Filtro Mediana aplicado");
            filtersApplied++;
        }
        
        // === 4. Filtro Bilateral ===
        if (checkUseBilateral->isChecked()) {
            int d = sliderBilateralD->value();
            if (d % 2 == 0) d++;
            double sigmaColor = sliderBilateralSigmaColor->value();
            double sigmaSpace = sigmaColor; // Usar mismo valor para sigmaSpace
            
            logMessage(textPreprocessingLog, QString("[4/5] Aplicando Filtro Bilateral (d=%1, sigmaColor=%2)...").arg(d).arg(sigmaColor));
            working = Preprocessing::applyBilateralFilter(working, d, sigmaColor, sigmaSpace);
            logMessage(textPreprocessingLog, "   ✓ Filtro Bilateral aplicado");
            filtersApplied++;
        }
        
        // === 5. CLAHE ===
        if (checkUseCLAHE->isChecked()) {
            logMessage(textPreprocessingLog, "[5/5] Aplicando CLAHE (Mejora de Contraste)...");
            working = Preprocessing::applyCLAHE(working);
            logMessage(textPreprocessingLog, "   ✓ CLAHE aplicado");
            filtersApplied++;
        }
        
        // Guardar resultado en el contexto
        currentSlice.preprocessed = working.clone();
        
        // Actualizar visualización
        updateImageDisplay(labelPreprocessedImage, currentSlice.preprocessed);
        
        // Calcular métricas
        if (filtersApplied > 0) {
            double psnr = Preprocessing::calculatePSNR(currentSlice.original8bit, currentSlice.preprocessed);
            double snr = Preprocessing::calculateSNR(currentSlice.preprocessed);
            
            logMessage(textPreprocessingLog, "\n=== MÉTRICAS ===");
            logMessage(textPreprocessingLog, QString("PSNR: %1 dB").arg(psnr, 0, 'f', 2));
            logMessage(textPreprocessingLog, QString("SNR: %1 dB").arg(snr, 0, 'f', 2));
            logMessage(textPreprocessingLog, QString("\n✓ Preprocesamiento completado (%1 filtros aplicados)").arg(filtersApplied));
        } else {
            logMessage(textPreprocessingLog, "\n[!] No se seleccionó ningún filtro");
            QMessageBox::information(this, "Sin filtros", "Selecciona al menos un filtro para aplicar.");
        }
        
    } catch (const std::exception& e) {
        QString errorMsg = QString("Error al aplicar preprocesamiento: %1").arg(e.what());
        logMessage(textPreprocessingLog, QString("\n✗ ERROR: %1").arg(e.what()));
        QMessageBox::critical(this, "Error", errorMsg);
    }
}

// ============================================================================
// SLOTS: PESTAÑA SEGMENTACIÓN
// ============================================================================

void MainWindow::onSegmentLungs() {
    if (!currentSlice.hasData()) {
        QMessageBox::warning(this, "Sin datos", "Carga un dataset primero en la pestaña I/O.");
        return;
    }
    
    textSegmentationLog->clear();
    logMessage(textSegmentationLog, "[INICIO] Segmentando PULMONES...");
    
    try {
        // IMPORTANTE: Los algoritmos de segmentación necesitan valores HU de la imagen 16-bit original
        // El preprocesamiento (CLAHE, filtros) trabaja con 8-bit y NO preserva valores HU
        // Por eso SIEMPRE usamos originalRaw para segmentar
        cv::Mat inputImage = currentSlice.originalRaw;
        cv::Mat displayImage = currentSlice.original8bit;
        
        if (checkUsePreprocessed->isChecked() && !currentSlice.preprocessed.empty()) {
            logMessage(textSegmentationLog, "[INFO] Mostrando imagen preprocesada (referencia visual únicamente)");
            logMessage(textSegmentationLog, "[INFO] Segmentación usa ORIGINAL 16-bit para valores HU correctos");
            displayImage = currentSlice.preprocessed;
        } else {
            logMessage(textSegmentationLog, "[INFO] Usando imagen ORIGINAL para segmentación y visualización");
        }
        
        // Actualizar imagen de entrada en la UI
        updateImageDisplay(labelSegmentationInput, displayImage);
        
        // Determinar rangos HU
        int minHU = -1000;  // Valores por defecto para pulmones
        int maxHU = -400;
        
        if (checkUseCustomHU->isChecked()) {
            minHU = spinMinHU->value();
            maxHU = spinMaxHU->value();
            logMessage(textSegmentationLog, QString("[INFO] Usando rangos personalizados: %1 a %2 HU").arg(minHU).arg(maxHU));
        } else {
            logMessage(textSegmentationLog, QString("[INFO] Usando rangos por defecto: %1 a %2 HU").arg(minHU).arg(maxHU));
        }
        
        // Llamar al algoritmo de segmentación
        logMessage(textSegmentationLog, "[1/3] Aplicando algoritmo de segmentación de pulmones...");
        auto lungRegions = Segmentation::segmentLungsCustom(inputImage, minHU, maxHU);
        
        logMessage(textSegmentationLog, QString("[2/3] Detectadas %1 regiones pulmonares").arg(lungRegions.size()));
        
        // Guardar regiones
        currentSegmentedRegions = lungRegions;
        
        // Combinar todas las máscaras en una sola
        cv::Mat combinedMask = cv::Mat::zeros(inputImage.size(), CV_8UC1);
        for (const auto& region : lungRegions) {
            combinedMask.setTo(255, region.mask);
            logMessage(textSegmentationLog, 
                QString("   - %1: Área=%2, Centro=(%3, %4)")
                    .arg(QString::fromStdString(region.label))
                    .arg(region.area)
                    .arg(region.centroid.x)
                    .arg(region.centroid.y)
            );
        }
        
        // Guardar en contexto
        currentSlice.mask = combinedMask.clone();
        
        // IMPORTANTE: Limpiar resultado morfológico anterior (nueva segmentación)
        currentSlice.morphologyMask.release();
        labelMorphologyOutput->clear();
        labelMorphologyOutput->setText("Aplica operación para ver resultado");
        
        // Mostrar máscara
        updateImageDisplay(labelSegmentedImage, currentSlice.mask);
        
        // También actualizar entrada en pestaña de Morfología
        updateImageDisplay(labelMorphologyInput, currentSlice.mask);
        
        // Actualizar visualización automáticamente
        onUpdateVisualization();
        
        logMessage(textSegmentationLog, QString("\n✓ Segmentación de pulmones completada (%1 regiones)").arg(lungRegions.size()));
        
    } catch (const std::exception& e) {
        QString errorMsg = QString("Error en segmentación: %1").arg(e.what());
        logMessage(textSegmentationLog, QString("\n✗ ERROR: %1").arg(e.what()));
        QMessageBox::critical(this, "Error", errorMsg);
    }
}

void MainWindow::onSegmentBones() {
    if (!currentSlice.hasData()) {
        QMessageBox::warning(this, "Sin datos", "Carga un dataset primero en la pestaña I/O.");
        return;
    }
    
    textSegmentationLog->clear();
    logMessage(textSegmentationLog, "[INICIO] Segmentando HUESOS...");
    
    try {
        // IMPORTANTE: Segmentación de huesos requiere valores HU específicos (>200 HU)
        // Siempre usar originalRaw que tiene los valores correctos
        cv::Mat inputImage = currentSlice.originalRaw;
        cv::Mat displayImage = currentSlice.original8bit;
        
        if (checkUsePreprocessed->isChecked() && !currentSlice.preprocessed.empty()) {
            logMessage(textSegmentationLog, "[INFO] Mostrando imagen preprocesada (referencia visual)");
            logMessage(textSegmentationLog, "[INFO] Segmentación usa ORIGINAL 16-bit para umbrales HU");
            displayImage = currentSlice.preprocessed;
        } else {
            logMessage(textSegmentationLog, "[INFO] Usando imagen ORIGINAL");
        }
        
        updateImageDisplay(labelSegmentationInput, displayImage);
        
        // Determinar rangos HU
        int minHU = 200;   // Valores por defecto para huesos
        int maxHU = 3000;
        
        if (checkUseCustomHU->isChecked()) {
            minHU = spinMinHU->value();
            maxHU = spinMaxHU->value();
            logMessage(textSegmentationLog, QString("[INFO] Usando rangos personalizados: %1 a %2 HU").arg(minHU).arg(maxHU));
        } else {
            logMessage(textSegmentationLog, QString("[INFO] Usando rangos por defecto: %1 a %2 HU").arg(minHU).arg(maxHU));
        }
        
        logMessage(textSegmentationLog, "[1/3] Aplicando algoritmo de segmentación de huesos...");
        auto boneRegions = Segmentation::segmentBonesCustom(inputImage, minHU, maxHU);
        
        logMessage(textSegmentationLog, QString("[2/3] Detectadas %1 regiones óseas").arg(boneRegions.size()));
        
        currentSegmentedRegions = boneRegions;
        
        // Combinar máscaras
        cv::Mat combinedMask = cv::Mat::zeros(inputImage.size(), CV_8UC1);
        for (const auto& region : boneRegions) {
            combinedMask.setTo(255, region.mask);
            logMessage(textSegmentationLog, 
                QString("   - %1: Área=%2, Centro=(%3, %4)")
                    .arg(QString::fromStdString(region.label))
                    .arg(region.area)
                    .arg(region.centroid.x)
                    .arg(region.centroid.y)
            );
        }
        
        currentSlice.mask = combinedMask.clone();
        
        // IMPORTANTE: Limpiar resultado morfológico anterior (nueva segmentación)
        currentSlice.morphologyMask.release();
        labelMorphologyOutput->clear();
        labelMorphologyOutput->setText("Aplica operación para ver resultado");
        
        updateImageDisplay(labelSegmentedImage, currentSlice.mask);
        updateImageDisplay(labelMorphologyInput, currentSlice.mask);
        onUpdateVisualization();
        
        logMessage(textSegmentationLog, QString("\n✓ Segmentación de huesos completada (%1 regiones)").arg(boneRegions.size()));
        
    } catch (const std::exception& e) {
        QString errorMsg = QString("Error en segmentación: %1").arg(e.what());
        logMessage(textSegmentationLog, QString("\n✗ ERROR: %1").arg(e.what()));
        QMessageBox::critical(this, "Error", errorMsg);
    }
}

void MainWindow::onSegmentAorta() {
    if (!currentSlice.hasData()) {
        QMessageBox::warning(this, "Sin datos", "Carga un dataset primero en la pestaña I/O.");
        return;
    }
    
    textSegmentationLog->clear();
    logMessage(textSegmentationLog, "[INICIO] Segmentando AORTA...");
    
    try {
        // IMPORTANTE: Aorta requiere umbrales HU específicos (tejido blando)
        // Usar siempre originalRaw para tener valores HU precisos
        cv::Mat inputImage = currentSlice.originalRaw;
        cv::Mat displayImage = currentSlice.original8bit;
        
        if (checkUsePreprocessed->isChecked() && !currentSlice.preprocessed.empty()) {
            logMessage(textSegmentationLog, "[INFO] Mostrando preprocesada (solo visualización)");
            logMessage(textSegmentationLog, "[INFO] Segmentación usa ORIGINAL 16-bit");
            displayImage = currentSlice.preprocessed;
        } else {
            logMessage(textSegmentationLog, "[INFO] Usando imagen ORIGINAL");
        }
        
        updateImageDisplay(labelSegmentationInput, displayImage);
        
        // Rangos HU personalizables para aorta
        int minHU = 120;
        int maxHU = 400;
        
        if (checkUseCustomHU->isChecked()) {
            minHU = spinMinHU->value();
            maxHU = spinMaxHU->value();
            logMessage(textSegmentationLog, QString("[INFO] Usando rango HU PERSONALIZADO: [%1, %2]").arg(minHU).arg(maxHU));
        } else {
            logMessage(textSegmentationLog, QString("[INFO] Usando rango HU por defecto: [%1, %2]").arg(minHU).arg(maxHU));
        }
        
        logMessage(textSegmentationLog, "[1/3] Aplicando algoritmo de segmentación de aorta...");
        auto aortaRegions = Segmentation::segmentAortaCustom(inputImage, minHU, maxHU);
        
        logMessage(textSegmentationLog, QString("[2/3] Detectadas %1 regiones de aorta").arg(aortaRegions.size()));
        
        currentSegmentedRegions = aortaRegions;
        
        // Combinar máscaras
        cv::Mat combinedMask = cv::Mat::zeros(inputImage.size(), CV_8UC1);
        for (const auto& region : aortaRegions) {
            combinedMask.setTo(255, region.mask);
            logMessage(textSegmentationLog, 
                QString("   - %1: Área=%2, Centro=(%3, %4)")
                    .arg(QString::fromStdString(region.label))
                    .arg(region.area)
                    .arg(region.centroid.x)
                    .arg(region.centroid.y)
            );
        }
        
        // CORRECCIÓN: Guardar la máscara combinada en el contexto
        currentSlice.mask = combinedMask.clone();
        
        // IMPORTANTE: Limpiar resultado morfológico anterior (nueva segmentación)
        currentSlice.morphologyMask.release();
        labelMorphologyOutput->clear();
        labelMorphologyOutput->setText("Aplica operación para ver resultado");
        
        // Actualizar displays
        updateImageDisplay(labelSegmentedImage, currentSlice.mask);
        updateImageDisplay(labelMorphologyInput, currentSlice.mask);
        onUpdateVisualization();
        
        logMessage(textSegmentationLog, QString("\n✓ Segmentación de aorta completada (%1 regiones)").arg(aortaRegions.size()));
        
    } catch (const std::exception& e) {
        QString errorMsg = QString("Error en segmentación: %1").arg(e.what());
        logMessage(textSegmentationLog, QString("\n✗ ERROR: %1").arg(e.what()));
        QMessageBox::critical(this, "Error", errorMsg);
    }
}

void MainWindow::onClearSegmentation() {
    currentSlice.mask.release();
    currentSegmentedRegions.clear();
    
    labelSegmentedImage->clear();
    labelSegmentedImage->setText("Ejecuta segmentación para ver resultado");
    
    textSegmentationLog->clear();
    logMessage(textSegmentationLog, "[INFO] Segmentación limpiada");
}

// ============================================================================
// SLOTS: PESTAÑA MORFOLOGÍA
// ============================================================================

void MainWindow::onApplyMorphology() {
    // Validar que hay máscara de segmentación
    if (!currentSlice.hasData() || currentSlice.mask.empty()) {
        QMessageBox::warning(this, "Sin máscara", 
            "Ejecuta una segmentación primero en la pestaña anterior.");
        return;
    }
    
    textMorphologyLog->clear();
    logMessage(textMorphologyLog, "[INICIO] Aplicando operación morfológica...");
    
    try {
        // Determinar imagen de entrada
        cv::Mat inputMask = currentSlice.morphologyMask.empty() ? 
                           currentSlice.mask.clone() : 
                           currentSlice.morphologyMask.clone();
        
        // Mostrar entrada
        updateImageDisplay(labelMorphologyInput, inputMask);
        
        // Obtener parámetros
        QString operation = comboMorphologyOperation->currentData().toString();
        int kernelSize = sliderMorphKernelSize->value();
        if (kernelSize % 2 == 0) kernelSize++; // Asegurar impar
        int iterations = sliderIterations->value();
        
        cv::Size kernelSizeCv(kernelSize, kernelSize);
        
        logMessage(textMorphologyLog, 
            QString("[1/2] Operación: %1, Kernel: %2x%2, Iteraciones: %3")
                .arg(comboMorphologyOperation->currentText())
                .arg(kernelSize)
                .arg(iterations)
        );
        
        cv::Mat result;
        
        // Aplicar operación seleccionada
        if (operation == "erosion") {
            logMessage(textMorphologyLog, "[2/2] Aplicando Erosión...");
            result = Morphology::erode(inputMask, kernelSizeCv, iterations);
            
        } else if (operation == "dilation") {
            logMessage(textMorphologyLog, "[2/2] Aplicando Dilatación...");
            result = Morphology::dilate(inputMask, kernelSizeCv, iterations);
            
        } else if (operation == "opening") {
            logMessage(textMorphologyLog, "[2/2] Aplicando Apertura (Opening)...");
            result = inputMask.clone();
            // Opening no tiene parámetro de iteraciones, aplicar manualmente si es necesario
            for (int i = 0; i < iterations; i++) {
                result = Morphology::opening(result, kernelSizeCv);
            }
            
        } else if (operation == "closing") {
            logMessage(textMorphologyLog, "[2/2] Aplicando Cierre (Closing)...");
            result = inputMask.clone();
            // Closing no tiene parámetro de iteraciones, aplicar manualmente si es necesario
            for (int i = 0; i < iterations; i++) {
                result = Morphology::closing(result, kernelSizeCv);
            }
            
        } else {
            throw std::runtime_error("Operación desconocida");
        }
        
        // Guardar resultado
        currentSlice.morphologyMask = result.clone();
        
        // Mostrar resultado
        updateImageDisplay(labelMorphologyOutput, currentSlice.morphologyMask);
        
        // Calcular estadísticas
        int pixelsBefore = cv::countNonZero(inputMask);
        int pixelsAfter = cv::countNonZero(result);
        double changePercent = ((pixelsAfter - pixelsBefore) / (double)pixelsBefore) * 100.0;
        
        logMessage(textMorphologyLog, "\n=== ESTADÍSTICAS ===");
        logMessage(textMorphologyLog, QString("Píxeles antes: %1").arg(pixelsBefore));
        logMessage(textMorphologyLog, QString("Píxeles después: %1").arg(pixelsAfter));
        logMessage(textMorphologyLog, QString("Cambio: %1%2%")
            .arg(changePercent > 0 ? "+" : "")
            .arg(changePercent, 0, 'f', 2)
        );
        logMessage(textMorphologyLog, "\n✓ Operación morfológica completada");
        
    } catch (const std::exception& e) {
        QString errorMsg = QString("Error en morfología: %1").arg(e.what());
        logMessage(textMorphologyLog, QString("\n✗ ERROR: %1").arg(e.what()));
        QMessageBox::critical(this, "Error", errorMsg);
    }
}

// ============================================================================
// SLOTS: PESTAÑA VISUALIZACIÓN
// ============================================================================

void MainWindow::onUpdateVisualization() {
    // Validar que hay datos cargados
    if (!currentSlice.hasData()) {
        return; // Silenciosamente no hacer nada si no hay datos
    }
    
    try {
        // Convertir imagen original a color (BGR)
        cv::Mat visualization;
        if (currentSlice.original8bit.channels() == 1) {
            cv::cvtColor(currentSlice.original8bit, visualization, cv::COLOR_GRAY2BGR);
        } else {
            visualization = currentSlice.original8bit.clone();
        }
        
        // Si hay regiones segmentadas y el overlay está activado
        if (!currentSegmentedRegions.empty() && checkShowOverlay->isChecked()) {
            // Crear overlay vacío
            cv::Mat overlay = cv::Mat::zeros(visualization.size(), visualization.type());
            
            // Dibujar cada región con su color
            for (const auto& region : currentSegmentedRegions) {
                overlay.setTo(region.color, region.mask);
            }
            
            // Obtener opacidad (0-100 -> 0.0-1.0)
            double alpha = sliderOverlayOpacity->value() / 100.0;
            double beta = 1.0 - alpha;
            
            // Combinar original con overlay usando la opacidad
            cv::addWeighted(visualization, beta, overlay, alpha, 0, visualization);
        }
        
        // Dibujar contornos si está activado
        if (!currentSegmentedRegions.empty() && checkShowContours->isChecked()) {
            for (const auto& region : currentSegmentedRegions) {
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(region.mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
                cv::drawContours(visualization, contours, -1, region.color, 2);
            }
        }
        
        // Dibujar etiquetas si está activado
        if (!currentSegmentedRegions.empty() && checkShowLabels->isChecked()) {
            for (const auto& region : currentSegmentedRegions) {
                // Fondo blanco para mejor legibilidad
                cv::putText(visualization, region.label, region.centroid, 
                           cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 3);
                // Texto en color de la región
                cv::putText(visualization, region.label, region.centroid, 
                           cv::FONT_HERSHEY_SIMPLEX, 0.6, region.color, 2);
                
                // Opcional: dibujar un pequeño círculo en el centroide
                cv::circle(visualization, region.centroid, 4, region.color, -1);
                cv::circle(visualization, region.centroid, 4, cv::Scalar(255, 255, 255), 1);
            }
        }
        
        // Mostrar resultado
        updateImageDisplay(labelVisualizationImage, visualization);
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", 
            QString("Error en visualización: %1").arg(e.what()));
    }
}

void MainWindow::onResetVisualization() {
    if (!currentSlice.hasData()) {
        QMessageBox::warning(this, "Sin datos", "Carga un dataset primero.");
        return;
    }
    
    // Mostrar solo la imagen original
    cv::Mat visualization;
    if (currentSlice.original8bit.channels() == 1) {
        cv::cvtColor(currentSlice.original8bit, visualization, cv::COLOR_GRAY2BGR);
    } else {
        visualization = currentSlice.original8bit.clone();
    }
    
    updateImageDisplay(labelVisualizationImage, visualization);
}

// ============================================================================
// SLOTS: PESTAÑA MÉTRICAS
// ============================================================================

void MainWindow::onCalculateMetrics() {
    if (!currentSlice.hasData()) {
        QMessageBox::warning(this, "Sin datos", "Carga un dataset primero en la pestaña I/O.");
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // === 1. LIMPIAR TABLA ===
    tableMetrics->setRowCount(0);
    
    // === 2. CALCULAR MÉTRICAS POR REGIÓN SEGMENTADA ===
    if (!currentSegmentedRegions.empty()) {
        for (const auto& region : currentSegmentedRegions) {
            int row = tableMetrics->rowCount();
            tableMetrics->insertRow(row);
            
            // Columna 0: Estructura
            tableMetrics->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(region.label)));
            
            // Columna 1: Área (px)
            tableMetrics->setItem(row, 1, new QTableWidgetItem(QString::number(region.area)));
            
            // Columna 2: Densidad Media HU
            double densityHU = 0.0;
            if (!currentSlice.originalRaw.empty() && !region.mask.empty()) {
                cv::Scalar meanVal = cv::mean(currentSlice.originalRaw, region.mask);
                // Convertir a HU: HU = valor_raw - 1024 (aproximado para CT)
                densityHU = meanVal[0] - 1024.0;
            }
            tableMetrics->setItem(row, 2, new QTableWidgetItem(QString::number(densityHU, 'f', 1)));
            
            // Columna 3: SNR de la región
            double snr = 0.0;
            if (!currentSlice.original8bit.empty() && !region.mask.empty()) {
                cv::Mat roi;
                currentSlice.original8bit.copyTo(roi, region.mask);
                cv::Scalar mean, stddev;
                cv::meanStdDev(roi, mean, stddev);
                if (stddev[0] > 0) {
                    snr = 20.0 * log10(mean[0] / stddev[0]);
                }
            }
            tableMetrics->setItem(row, 3, new QTableWidgetItem(QString::number(snr, 'f', 2)));
            
            // Columna 4: PSNR vs Original
            QString psnrText = "N/A";
            if (!currentSlice.preprocessed.empty()) {
                try {
                    double psnr = Preprocessing::calculatePSNR(currentSlice.original8bit, currentSlice.preprocessed);
                    psnrText = QString::number(psnr, 'f', 2);
                } catch (...) {
                    psnrText = "Error";
                }
            }
            tableMetrics->setItem(row, 4, new QTableWidgetItem(psnrText));
        }
    } else {
        // Si no hay regiones, mostrar métricas de imagen completa
        int row = tableMetrics->rowCount();
        tableMetrics->insertRow(row);
        
        tableMetrics->setItem(row, 0, new QTableWidgetItem("Imagen Completa"));
        
        int totalPixels = currentSlice.original8bit.rows * currentSlice.original8bit.cols;
        tableMetrics->setItem(row, 1, new QTableWidgetItem(QString::number(totalPixels)));
        
        // Densidad media de toda la imagen
        cv::Scalar meanVal = cv::mean(currentSlice.originalRaw);
        double densityHU = meanVal[0] - 1024.0;
        tableMetrics->setItem(row, 2, new QTableWidgetItem(QString::number(densityHU, 'f', 1)));
        
        // SNR de imagen completa
        double snr = Preprocessing::calculateSNR(currentSlice.original8bit);
        tableMetrics->setItem(row, 3, new QTableWidgetItem(QString::number(snr, 'f', 2)));
        
        // PSNR vs procesada
        QString psnrText = "N/A";
        if (!currentSlice.preprocessed.empty()) {
            double psnr = Preprocessing::calculatePSNR(currentSlice.original8bit, currentSlice.preprocessed);
            psnrText = QString::number(psnr, 'f', 2);
        }
        tableMetrics->setItem(row, 4, new QTableWidgetItem(psnrText));
    }
    
    // === 3. GENERAR HISTOGRAMA DE ROI ===
    if (!currentSlice.mask.empty() && !currentSlice.original8bit.empty()) {
        try {
            // Usar imagen 8-bit para calcular histograma (más compatible)
            int histSize = 256;
            float range[] = {0, 256};
            const float* histRange = {range};
            
            cv::Mat hist;
            cv::calcHist(&currentSlice.original8bit, 1, 0, currentSlice.mask, hist, 1, &histSize, &histRange);
            
            // Normalizar histograma
            cv::normalize(hist, hist, 0, 255, cv::NORM_MINMAX);
            
            // Crear imagen del histograma
            int histW = 512;
            int histH = 300;
            int binW = cvRound((double)histW / histSize);
            
            cv::Mat histImage(histH, histW, CV_8UC3, cv::Scalar(30, 30, 30));
            
            // Dibujar líneas del histograma
            for (int i = 1; i < histSize; i++) {
                cv::line(histImage,
                    cv::Point(binW * (i - 1), histH - cvRound(hist.at<float>(i - 1))),
                    cv::Point(binW * i, histH - cvRound(hist.at<float>(i))),
                    cv::Scalar(0, 200, 255), 2, 8, 0);
            }
            
            // Agregar etiquetas
            cv::putText(histImage, "Histograma ROI (HU)", cv::Point(10, 25), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
            cv::putText(histImage, "Intensidad ->", cv::Point(histW - 120, histH - 10), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(180, 180, 180), 1);
            cv::putText(histImage, "Frecuencia", cv::Point(10, histH / 2), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(180, 180, 180), 1);
            
            // Mostrar en el label
            QImage qimg = cvMatToQImage(histImage);
            labelHistogram->setPixmap(QPixmap::fromImage(qimg));
            
        } catch (const std::exception& e) {
            labelHistogram->setText(QString("Error generando histograma: %1").arg(e.what()));
        }
    } else {
        labelHistogram->setText("Segmenta una región para ver su histograma");
    }
    
    // === 4. CALCULAR MÉTRICAS DE RENDIMIENTO ===
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Tiempo de procesamiento
    labelPerformanceInfo->setText(QString("Tiempo de Procesamiento: %1 ms").arg(duration.count()));
    
    // Memoria estimada (bytes -> MB)
    size_t memoryBytes = 0;
    
    if (!currentSlice.originalRaw.empty()) {
        memoryBytes += currentSlice.originalRaw.total() * currentSlice.originalRaw.elemSize();
    }
    if (!currentSlice.original8bit.empty()) {
        memoryBytes += currentSlice.original8bit.total() * currentSlice.original8bit.elemSize();
    }
    if (!currentSlice.preprocessed.empty()) {
        memoryBytes += currentSlice.preprocessed.total() * currentSlice.preprocessed.elemSize();
    }
    if (!currentSlice.mask.empty()) {
        memoryBytes += currentSlice.mask.total() * currentSlice.mask.elemSize();
    }
    if (!currentSlice.morphologyMask.empty()) {
        memoryBytes += currentSlice.morphologyMask.total() * currentSlice.morphologyMask.elemSize();
    }
    
    double memoryMB = memoryBytes / (1024.0 * 1024.0);
    labelMemoryInfo->setText(QString("Memoria Estimada: %1 MB").arg(memoryMB, 0, 'f', 2));
    
    // Auto-ajustar columnas
    tableMetrics->resizeColumnsToContents();
}

// ============================================================================
// MÉTODOS AUXILIARES
// ============================================================================

void MainWindow::updateImageDisplay(QLabel* label, const cv::Mat& image) {
    if (image.empty()) {
        label->setText("Sin imagen");
        return;
    }
    
    QImage qimg = cvMatToQImage(image);
    label->setPixmap(QPixmap::fromImage(qimg));
    label->adjustSize();
}

QImage MainWindow::cvMatToQImage(const cv::Mat& mat) {
    if (mat.empty()) {
        return QImage();
    }
    
    // Convertir según el tipo
    if (mat.channels() == 1) {
        // Grayscale
        return QImage(mat.data, mat.cols, mat.rows, 
                     static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
    } else if (mat.channels() == 3) {
        // BGR -> RGB
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, 
                     static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
    } else if (mat.channels() == 4) {
        // BGRA -> RGBA
        cv::Mat rgba;
        cv::cvtColor(mat, rgba, cv::COLOR_BGRA2RGBA);
        return QImage(rgba.data, rgba.cols, rgba.rows, 
                     static_cast<int>(rgba.step), QImage::Format_RGBA8888).copy();
    }
    
    return QImage();
}

void MainWindow::logMessage(QTextEdit* textEdit, const QString& message) {
    if (textEdit) {
        textEdit->append(message);
    }
}

void MainWindow::clearLogs() {
    // Por implementar cuando se agreguen los logs
}
