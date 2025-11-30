#ifndef PREPROCESSING_CONTROLLER_H
#define PREPROCESSING_CONTROLLER_H

#include <QObject>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <opencv2/core.hpp>
#include <memory>

// Forward declarations
namespace Denoising {
    class DnCNNDenoiser;
}

struct SliceContext;

/**
 * @brief Controlador para la lógica de preprocesamiento (Fase 3)
 * 
 * Gestiona la aplicación de filtros tradicionales, DnCNN y presets optimizados
 * para diferentes tipos de tejidos (pulmones, huesos, aorta).
 */
class PreprocessingController : public QObject
{
    Q_OBJECT

public:
    explicit PreprocessingController(QObject *parent = nullptr);
    ~PreprocessingController();
    
    // Configuración inicial
    void setSliceContext(SliceContext* context);
    void setDnCNNDenoiser(std::shared_ptr<Denoising::DnCNNDenoiser> denoiser);
    
    // Configurar widgets UI (referencias)
    void setWidgets(
        QCheckBox* checkGaussian, QSlider* sliderGaussianKernel,
        QCheckBox* checkMedian, QSlider* sliderMedianKernel,
        QCheckBox* checkBilateral, QSlider* sliderBilateralD, QSlider* sliderBilateralSigma,
        QCheckBox* checkCLAHE, QSlider* sliderCLAHEClip, QSlider* sliderCLAHETile,
        QCheckBox* checkDnCNN,
        QLabel* imageBeforeLabel, QLabel* imageAfterLabel
    );
    
    // Métodos principales de procesamiento
    void applyPreprocessing();
    void updateDisplay();
    
    // Presets optimizados
    void applyPresetLungs();
    void applyPresetBones();
    void applyPresetAorta();
    void resetFilters();
    
    // DnCNN comparison
    void showDnCNNComparison();

signals:
    void statusMessageChanged(const QString& message);
    void preprocessingApplied();

private:
    // Referencias a datos
    SliceContext* m_sliceContext;
    std::shared_ptr<Denoising::DnCNNDenoiser> m_dncnnDenoiser;
    
    // Referencias a widgets (no los poseemos, solo referencias)
    QCheckBox* m_checkGaussian;
    QSlider* m_sliderGaussianKernel;
    
    QCheckBox* m_checkMedian;
    QSlider* m_sliderMedianKernel;
    
    QCheckBox* m_checkBilateral;
    QSlider* m_sliderBilateralD;
    QSlider* m_sliderBilateralSigma;
    
    QCheckBox* m_checkCLAHE;
    QSlider* m_sliderCLAHEClip;
    QSlider* m_sliderCLAHETile;
    
    QCheckBox* m_checkDnCNN;
    
    QLabel* m_imageBeforeLabel;
    QLabel* m_imageAfterLabel;
};

#endif // PREPROCESSING_CONTROLLER_H
