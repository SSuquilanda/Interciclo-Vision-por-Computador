#ifndef QT_OPENCV_HELPERS_H
#define QT_OPENCV_HELPERS_H

#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace UIHelpers {

/**
 * @brief Convierte cv::Mat a QImage para visualización en Qt
 * @param mat Imagen OpenCV (CV_8UC1 o CV_8UC3)
 * @return QImage convertida, o QImage vacía si falla
 */
QImage cvMatToQImage(const cv::Mat& mat);

/**
 * @brief Muestra una imagen OpenCV en un QLabel
 * @param image Imagen OpenCV a mostrar
 * @param label QLabel donde se mostrará la imagen
 * @param errorMessage Mensaje a mostrar si hay error (opcional)
 */
void displayImageInLabel(const cv::Mat& image, QLabel* label, 
                        const QString& errorMessage = "Error al mostrar imagen");

/**
 * @brief Normaliza una imagen de 16-bit a 8-bit para visualización
 * @param image16 Imagen de entrada (CV_16U o CV_16S)
 * @return Imagen normalizada (CV_8U)
 */
cv::Mat normalize16To8(const cv::Mat& image16);

/**
 * @brief Aplica un mapa de colores a una imagen en escala de grises
 * @param grayImage Imagen en escala de grises (CV_8U)
 * @param colormap Tipo de mapa (cv::ColormapTypes)
 * @return Imagen con mapa de colores aplicado (CV_8UC3)
 */
cv::Mat applyColormap(const cv::Mat& grayImage, int colormap = cv::COLORMAP_JET);

/**
 * @brief Crea una imagen lado a lado (antes/después)
 * @param imageBefore Imagen "antes"
 * @param imageAfter Imagen "después"
 * @param addLabels Si debe agregar texto "ANTES" y "DESPUÉS"
 * @return Imagen combinada
 */
cv::Mat createSideBySideComparison(const cv::Mat& imageBefore, 
                                   const cv::Mat& imageAfter,
                                   bool addLabels = true);

} // namespace UIHelpers

#endif // QT_OPENCV_HELPERS_H
