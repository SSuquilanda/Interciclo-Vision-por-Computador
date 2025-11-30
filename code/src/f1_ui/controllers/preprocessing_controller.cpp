#include "preprocessing_controller.h"
#include "f1_ui/mainwindow.h"
#include "f1_ui/helpers/qt_opencv_helpers.h"
#include "f3_preprocessing/preprocessing.h"
#include "f3_preprocessing/denoising.h"
#include "utils/itk_opencv_bridge.h"

#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>

PreprocessingController::PreprocessingController(QObject *parent)
    : QObject(parent)
    , m_sliceContext(nullptr)
    , m_dncnnDenoiser(nullptr)
    , m_checkGaussian(nullptr)
    , m_sliderGaussianKernel(nullptr)
    , m_checkMedian(nullptr)
    , m_sliderMedianKernel(nullptr)
    , m_checkBilateral(nullptr)
    , m_sliderBilateralD(nullptr)
    , m_sliderBilateralSigma(nullptr)
    , m_checkCLAHE(nullptr)
    , m_sliderCLAHEClip(nullptr)
    , m_sliderCLAHETile(nullptr)
    , m_checkDnCNN(nullptr)
    , m_imageBeforeLabel(nullptr)
    , m_imageAfterLabel(nullptr)
{
}

PreprocessingController::~PreprocessingController()
{
}

void PreprocessingController::setSliceContext(SliceContext* context)
{
    m_sliceContext = context;
}

void PreprocessingController::setDnCNNDenoiser(std::shared_ptr<Denoising::DnCNNDenoiser> denoiser)
{
    m_dncnnDenoiser = denoiser;
}

void PreprocessingController::setWidgets(
    QCheckBox* checkGaussian, QSlider* sliderGaussianKernel,
    QCheckBox* checkMedian, QSlider* sliderMedianKernel,
    QCheckBox* checkBilateral, QSlider* sliderBilateralD, QSlider* sliderBilateralSigma,
    QCheckBox* checkCLAHE, QSlider* sliderCLAHEClip, QSlider* sliderCLAHETile,
    QCheckBox* checkDnCNN,
    QLabel* imageBeforeLabel, QLabel* imageAfterLabel)
{
    m_checkGaussian = checkGaussian;
    m_sliderGaussianKernel = sliderGaussianKernel;
    m_checkMedian = checkMedian;
    m_sliderMedianKernel = sliderMedianKernel;
    m_checkBilateral = checkBilateral;
    m_sliderBilateralD = sliderBilateralD;
    m_sliderBilateralSigma = sliderBilateralSigma;
    m_checkCLAHE = checkCLAHE;
    m_sliderCLAHEClip = sliderCLAHEClip;
    m_sliderCLAHETile = sliderCLAHETile;
    m_checkDnCNN = checkDnCNN;
    m_imageBeforeLabel = imageBeforeLabel;
    m_imageAfterLabel = imageAfterLabel;
}

void PreprocessingController::applyPreprocessing()
{
    if (!m_sliceContext || !m_sliceContext->isValid || m_sliceContext->originalRaw.empty()) {
        return;
    }
    
    // Normalizar imagen original de 16-bit a 8-bit
    cv::Mat normalized = Bridge::normalize16to8bit(m_sliceContext->originalRaw);
    
    // Verificar si debe usar DnCNN
    if (m_checkDnCNN && m_checkDnCNN->isChecked() && m_dncnnDenoiser && m_dncnnDenoiser->isLoaded()) {
        // Usar DnCNN en lugar de filtros tradicionales
        cv::Mat denoised = m_dncnnDenoiser->denoise(normalized);
        m_sliceContext->preprocessedDnCNN = denoised.clone();
        m_sliceContext->preprocessed = denoised;
    } else {
        // Aplicar filtros tradicionales en secuencia
        cv::Mat current = normalized.clone();
        
        // 1. Filtro Gaussiano
        if (m_checkGaussian && m_checkGaussian->isChecked()) {
            int kernelSize = m_sliderGaussianKernel->value();
            if (kernelSize % 2 == 0) kernelSize++;
            current = Preprocessing::applyGaussianBlur(current, kernelSize);
        }
        
        // 2. Filtro Mediana
        if (m_checkMedian && m_checkMedian->isChecked()) {
            int kernelSize = m_sliderMedianKernel->value();
            if (kernelSize % 2 == 0) kernelSize++;
            current = Preprocessing::applyMedianFilter(current, kernelSize);
        }
        
        // 3. Filtro Bilateral
        if (m_checkBilateral && m_checkBilateral->isChecked()) {
            int d = m_sliderBilateralD->value();
            double sigma = m_sliderBilateralSigma->value();
            current = Preprocessing::applyBilateralFilter(current, d, sigma, sigma);
        }
        
        // 4. CLAHE (mejora de contraste)
        if (m_checkCLAHE && m_checkCLAHE->isChecked()) {
            double clipLimit = m_sliderCLAHEClip->value() / 10.0;
            int tileSize = m_sliderCLAHETile->value();
            current = Preprocessing::applyCLAHE(current, clipLimit, cv::Size(tileSize, tileSize));
        }
        
        m_sliceContext->preprocessed = current;
    }
    
    // Actualizar la visualización
    updateDisplay();
    emit preprocessingApplied();
}

void PreprocessingController::updateDisplay()
{
    if (!m_sliceContext || !m_sliceContext->isValid || m_sliceContext->originalRaw.empty()) {
        return;
    }
    
    // Imagen ANTES (normalizada)
    cv::Mat imageBefore = Bridge::normalize16to8bit(m_sliceContext->originalRaw);
    UIHelpers::displayImageInLabel(imageBefore, m_imageBeforeLabel);
    
    // Imagen DESPUÉS (procesada)
    if (!m_sliceContext->preprocessed.empty() && m_imageAfterLabel) {
        UIHelpers::displayImageInLabel(m_sliceContext->preprocessed, m_imageAfterLabel);
    }
}

void PreprocessingController::applyPresetLungs()
{
    // Preset optimizado para pulmones (basado en pipeline_pulmones.cpp)
    // NOTA: pipeline_pulmones.cpp NO usa preprocesamiento, solo CLAHE para visualización
    
    // Desactivar todos los filtros primero
    if (m_checkGaussian) m_checkGaussian->setChecked(false);
    if (m_checkMedian) m_checkMedian->setChecked(false);
    if (m_checkBilateral) m_checkBilateral->setChecked(false);
    if (m_checkCLAHE) m_checkCLAHE->setChecked(false);
    if (m_checkDnCNN) m_checkDnCNN->setChecked(false);
    
    // Configuración óptima para pulmones:
    // - Solo CLAHE para mejorar contraste aire-tejido
    // - SIN bilateral: la segmentación por HU es suficientemente precisa
    
    if (m_checkCLAHE && m_sliderCLAHEClip && m_sliderCLAHETile) {
        m_checkCLAHE->setChecked(true);
        m_sliderCLAHEClip->setValue(20);  // 2.0 clip limit (moderado)
        m_sliderCLAHETile->setValue(8);   // 8x8 tiles
    }
    
    applyPreprocessing();
    emit statusMessageChanged("🫁 Preset 'Pulmones Óptimo' aplicado: CLAHE únicamente");
}

void PreprocessingController::applyPresetBones()
{
    // Preset optimizado para huesos (basado en pipeline_huesos.cpp)
    
    // Desactivar todos los filtros primero
    if (m_checkGaussian) m_checkGaussian->setChecked(false);
    if (m_checkMedian) m_checkMedian->setChecked(false);
    if (m_checkBilateral) m_checkBilateral->setChecked(false);
    if (m_checkCLAHE) m_checkCLAHE->setChecked(false);
    if (m_checkDnCNN) m_checkDnCNN->setChecked(false);
    
    // Configuración óptima para huesos:
    // - Mediana para eliminar ruido sin perder bordes duros
    // - CLAHE fuerte para realzar estructuras óseas
    
    if (m_checkMedian && m_sliderMedianKernel) {
        m_checkMedian->setChecked(true);
        m_sliderMedianKernel->setValue(5); // Kernel moderado
    }
    
    if (m_checkCLAHE && m_sliderCLAHEClip && m_sliderCLAHETile) {
        m_checkCLAHE->setChecked(true);
        m_sliderCLAHEClip->setValue(30);  // 3.0 clip limit (fuerte para huesos)
        m_sliderCLAHETile->setValue(8);   // 8x8 tiles
    }
    
    applyPreprocessing();
    emit statusMessageChanged("Preset 'Huesos Óptimo' aplicado: Mediana + CLAHE fuerte");
}

void PreprocessingController::applyPresetAorta()
{
    // Preset optimizado para aorta/arterias (basado en pipeline_aorta.cpp)
    // NOTA: pipeline_aorta.cpp usa DnCNN opcional, no filtros tradicionales
    
    // Desactivar todos los filtros primero
    if (m_checkGaussian) m_checkGaussian->setChecked(false);
    if (m_checkMedian) m_checkMedian->setChecked(false);
    if (m_checkBilateral) m_checkBilateral->setChecked(false);
    if (m_checkCLAHE) m_checkCLAHE->setChecked(false);
    if (m_checkDnCNN) m_checkDnCNN->setChecked(false);
    
    // Configuración óptima para arterias/aorta:
    // - Solo CLAHE muy suave para mejorar contraste vascular
    // - La segmentación por HU [30-120] es muy precisa para vasos con contraste
    // - Usar DnCNN manualmente si se necesita denoising
    
    if (m_checkCLAHE && m_sliderCLAHEClip && m_sliderCLAHETile) {
        m_checkCLAHE->setChecked(true);
        m_sliderCLAHEClip->setValue(15);  // 1.5 clip limit (suave)
        m_sliderCLAHETile->setValue(8);   // 8x8 tiles
    }
    
    applyPreprocessing();
    emit statusMessageChanged("🩸 Preset 'Aorta Óptimo' aplicado: CLAHE suave únicamente");
}

void PreprocessingController::resetFilters()
{
    // Desactivar todos los checkboxes
    if (m_checkGaussian) m_checkGaussian->setChecked(false);
    if (m_checkMedian) m_checkMedian->setChecked(false);
    if (m_checkBilateral) m_checkBilateral->setChecked(false);
    if (m_checkCLAHE) m_checkCLAHE->setChecked(false);
    
    // Resetear sliders a valores por defecto
    if (m_sliderGaussianKernel) m_sliderGaussianKernel->setValue(5);
    if (m_sliderMedianKernel) m_sliderMedianKernel->setValue(5);
    if (m_sliderBilateralD) m_sliderBilateralD->setValue(9);
    if (m_sliderBilateralSigma) m_sliderBilateralSigma->setValue(75);
    if (m_sliderCLAHEClip) m_sliderCLAHEClip->setValue(20);
    if (m_sliderCLAHETile) m_sliderCLAHETile->setValue(8);
    
    applyPreprocessing();
    emit statusMessageChanged("Filtros reseteados a valores por defecto");
}

void PreprocessingController::showDnCNNComparison()
{
    if (!m_sliceContext || !m_sliceContext->isValid || m_sliceContext->originalRaw.empty()) {
        QMessageBox::warning(nullptr, "Error", 
            "No hay imagen cargada. Por favor carga un dataset primero.");
        return;
    }
    
    if (!m_dncnnDenoiser || !m_dncnnDenoiser->isLoaded()) {
        QMessageBox::warning(nullptr, "Error", 
            "El modelo DnCNN no está cargado.");
        return;
    }
    
    // Obtener imagen original normalizada
    cv::Mat original8bit = Bridge::normalize16to8bit(m_sliceContext->originalRaw);
    
    // Aplicar DnCNN
    Denoising::DenoisingComparison comparison = 
        Denoising::compareWithAndWithoutDenoising(original8bit, *m_dncnnDenoiser);
    
    // Crear visualización de comparación
    cv::Mat visualComparison = Denoising::visualizeComparison(comparison);
    
    // Mostrar en ventana modal
    QDialog *comparisonDialog = new QDialog();
    comparisonDialog->setWindowTitle("Comparación: DnCNN vs Original");
    comparisonDialog->resize(visualComparison.cols + 40, visualComparison.rows + 120);
    
    QVBoxLayout *layout = new QVBoxLayout(comparisonDialog);
    
    // Label con la imagen
    QLabel *imageLabel = new QLabel();
    QImage qImage = UIHelpers::cvMatToQImage(visualComparison);
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
    QObject::connect(closeBtn, &QPushButton::clicked, comparisonDialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    comparisonDialog->exec();
    
    emit statusMessageChanged(QString("Comparación DnCNN: PSNR=%1dB, SNR=%2dB")
                      .arg(comparison.psnr, 0, 'f', 1)
                      .arg(comparison.snr, 0, 'f', 1));
}
