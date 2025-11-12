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
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>

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

private:
    // Métodos de inicialización
    void setupUI();
    void createMenuBar();
    void createToolBar();
    void createStatusBar();
    void createTabs();
    
    // Creación de pestañas individuales
    QWidget* createWelcomeTab();
    QWidget* createIOTab();
    QWidget* createPreprocessingTab();
    QWidget* createSegmentationTab();
    QWidget* createMorphologyTab();
    QWidget* createVisualizationTab();
    QWidget* createMetricsTab();

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
    
    // Variables de estado
    QString currentDatasetPath;
    bool datasetLoaded;
};

#endif // MAINWINDOW_H
