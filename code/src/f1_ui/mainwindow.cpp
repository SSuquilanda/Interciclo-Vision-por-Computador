#include "f1_ui/mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QLineEdit>
#include <QDir>
#include <QMenu>

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
    , datasetLoaded(false)
{
    setupUI();
    
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
    QVBoxLayout *mainLayout = new QVBoxLayout(ioWidget);
    
    // Sección de carga de dataset
    QGroupBox *loadGroup = new QGroupBox("Cargar Dataset");
    QVBoxLayout *loadLayout = new QVBoxLayout(loadGroup);
    
    QHBoxLayout *pathLayout = new QHBoxLayout();
    QLabel *lblPath = new QLabel("Ruta del dataset:");
    QLineEdit *txtPath = new QLineEdit();
    txtPath->setReadOnly(true);
    txtPath->setPlaceholderText("Seleccione una carpeta con archivos DICOM...");
    QPushButton *btnBrowse = new QPushButton("Examinar...");
    pathLayout->addWidget(lblPath);
    pathLayout->addWidget(txtPath, 1);
    pathLayout->addWidget(btnBrowse);
    loadLayout->addLayout(pathLayout);
    
    QPushButton *btnLoadDataset = new QPushButton("Cargar Dataset");
    btnLoadDataset->setMinimumHeight(35);
    loadLayout->addWidget(btnLoadDataset);
    
    mainLayout->addWidget(loadGroup);
    
    // Sección de información del dataset
    QGroupBox *infoGroup = new QGroupBox("Información del Dataset");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    
    logOutput = new QTextEdit();
    logOutput->setReadOnly(true);
    logOutput->setPlaceholderText("La información del dataset aparecerá aquí...");
    logOutput->setMaximumHeight(150);
    infoLayout->addWidget(logOutput);
    
    mainLayout->addWidget(infoGroup);
    
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
    
    mainLayout->addWidget(exportGroup);
    
    mainLayout->addStretch();
    
    return ioWidget;
}

QWidget* MainWindow::createPreprocessingTab()
{
    QWidget *preprocessWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(preprocessWidget);
    
    QGroupBox *group = new QGroupBox("Fase 3: Preprocesamiento (Próximamente)");
    QVBoxLayout *groupLayout = new QVBoxLayout(group);
    
    QLabel *label = new QLabel(
        "Esta sección incluirá:<br><br>"
        "• Filtros de suavizado (Gaussian, Median, Bilateral)<br>"
        "• Mejora de contraste (CLAHE, Histogram Equalization)<br>"
        "• Reducción de ruido<br>"
        "• Normalización de intensidades<br>"
        "• Redimensionado de imágenes"
    );
    label->setWordWrap(true);
    groupLayout->addWidget(label);
    
    layout->addWidget(group);
    layout->addStretch();
    
    return preprocessWidget;
}

QWidget* MainWindow::createSegmentationTab()
{
    QWidget *segmentWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(segmentWidget);
    
    QGroupBox *group = new QGroupBox("Fase 4: Segmentación (Próximamente)");
    QVBoxLayout *groupLayout = new QVBoxLayout(group);
    
    QLabel *label = new QLabel(
        "Esta sección incluirá:<br><br>"
        "• Umbralización (Global, Adaptativa, Otsu)<br>"
        "• Detección de bordes (Canny, Sobel)<br>"
        "• Segmentación por regiones<br>"
        "• Watershed<br>"
        "• Extracción de contornos"
    );
    label->setWordWrap(true);
    groupLayout->addWidget(label);
    
    layout->addWidget(group);
    layout->addStretch();
    
    return segmentWidget;
}

QWidget* MainWindow::createMorphologyTab()
{
    QWidget *morphWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(morphWidget);
    
    QGroupBox *group = new QGroupBox("Fase 5: Operaciones Morfológicas (Próximamente)");
    QVBoxLayout *groupLayout = new QVBoxLayout(group);
    
    QLabel *label = new QLabel(
        "Esta sección incluirá:<br><br>"
        "• Erosión y Dilatación<br>"
        "• Apertura y Cierre<br>"
        "• Gradiente morfológico<br>"
        "• Top-hat y Black-hat<br>"
        "• Detección de esqueleto"
    );
    label->setWordWrap(true);
    groupLayout->addWidget(label);
    
    layout->addWidget(group);
    layout->addStretch();
    
    return morphWidget;
}

QWidget* MainWindow::createVisualizationTab()
{
    QWidget *vizWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(vizWidget);
    
    QGroupBox *group = new QGroupBox("Fase 6: Visualización (Próximamente)");
    QVBoxLayout *groupLayout = new QVBoxLayout(group);
    
    QLabel *label = new QLabel(
        "Esta sección incluirá:<br><br>"
        "• Visualización 2D de slices<br>"
        "• Reconstrucción 3D<br>"
        "• Renderizado volumétrico<br>"
        "• Comparación lado a lado (FD vs QD)<br>"
        "• Mapas de color y overlays"
    );
    label->setWordWrap(true);
    groupLayout->addWidget(label);
    
    layout->addWidget(group);
    layout->addStretch();
    
    return vizWidget;
}

QWidget* MainWindow::createMetricsTab()
{
    QWidget *metricsWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(metricsWidget);
    
    QGroupBox *group = new QGroupBox("Fase 8: Métricas y Evaluación (Próximamente)");
    QVBoxLayout *groupLayout = new QVBoxLayout(group);
    
    QLabel *label = new QLabel(
        "Esta sección incluirá:<br><br>"
        "• PSNR (Peak Signal-to-Noise Ratio)<br>"
        "• SSIM (Structural Similarity Index)<br>"
        "• MSE (Mean Squared Error)<br>"
        "• SNR (Signal-to-Noise Ratio)<br>"
        "• Histogramas comparativos<br>"
        "• Reportes estadísticos"
    );
    label->setWordWrap(true);
    groupLayout->addWidget(label);
    
    layout->addWidget(group);
    layout->addStretch();
    
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
        datasetLoaded = true;
        actionExportSlices->setEnabled(true);
        
        lblStatus->setText("Dataset cargado: " + dir);
        
        QMessageBox::information(
            this,
            "Dataset Cargado",
            "Dataset cargado correctamente.\nRuta: " + dir
        );
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
}
