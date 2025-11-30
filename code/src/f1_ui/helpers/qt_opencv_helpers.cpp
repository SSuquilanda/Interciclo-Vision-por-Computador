#include "qt_opencv_helpers.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>

namespace UIHelpers {

QImage cvMatToQImage(const cv::Mat& mat)
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

void displayImageInLabel(const cv::Mat& image, QLabel* label, const QString& errorMessage)
{
    if (!label) {
        return;
    }
    
    if (image.empty()) {
        label->setText(errorMessage);
        return;
    }
    
    QImage qimg = cvMatToQImage(image);
    if (qimg.isNull()) {
        label->setText("Error al convertir la imagen");
        return;
    }
    
    // Mostrar la imagen en el label
    QPixmap pixmap = QPixmap::fromImage(qimg);
    label->setPixmap(pixmap);
    label->resize(pixmap.size());
}

cv::Mat normalize16To8(const cv::Mat& image16)
{
    if (image16.empty()) {
        return cv::Mat();
    }
    
    cv::Mat image8;
    
    // Normalizar de 16-bit a 8-bit
    if (image16.type() == CV_16U || image16.type() == CV_16S) {
        double minVal, maxVal;
        cv::minMaxLoc(image16, &minVal, &maxVal);
        
        if (maxVal > minVal) {
            image16.convertTo(image8, CV_8U, 255.0 / (maxVal - minVal), 
                            -minVal * 255.0 / (maxVal - minVal));
        } else {
            image16.convertTo(image8, CV_8U, 1.0, 0.0);
        }
    } else if (image16.type() == CV_8U) {
        // Ya está en 8-bit
        image8 = image16.clone();
    } else {
        // Tipo no soportado, intentar conversión directa
        image16.convertTo(image8, CV_8U);
    }
    
    return image8;
}

cv::Mat applyColormap(const cv::Mat& grayImage, int colormap)
{
    if (grayImage.empty()) {
        return cv::Mat();
    }
    
    cv::Mat grayU8;
    
    // Asegurar que sea CV_8U
    if (grayImage.type() != CV_8U) {
        grayU8 = normalize16To8(grayImage);
    } else {
        grayU8 = grayImage;
    }
    
    cv::Mat colorMapped;
    cv::applyColorMap(grayU8, colorMapped, colormap);
    
    return colorMapped;
}

cv::Mat createSideBySideComparison(const cv::Mat& imageBefore, 
                                   const cv::Mat& imageAfter,
                                   bool addLabels)
{
    if (imageBefore.empty() || imageAfter.empty()) {
        return cv::Mat();
    }
    
    // Asegurar que ambas imágenes tengan el mismo tamaño
    cv::Mat before = imageBefore.clone();
    cv::Mat after = imageAfter.clone();
    
    if (before.size() != after.size()) {
        cv::resize(after, after, before.size());
    }
    
    // Convertir a color si es necesario
    if (before.channels() == 1) {
        cv::cvtColor(before, before, cv::COLOR_GRAY2BGR);
    }
    if (after.channels() == 1) {
        cv::cvtColor(after, after, cv::COLOR_GRAY2BGR);
    }
    
    // Crear imagen concatenada horizontalmente
    cv::Mat combined;
    cv::hconcat(before, after, combined);
    
    // Agregar etiquetas si se solicita
    if (addLabels) {
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        double fontScale = 1.0;
        int thickness = 2;
        cv::Scalar color(0, 255, 255); // Amarillo en BGR
        
        // Etiqueta "ANTES"
        cv::Point textPosBefore(20, 40);
        cv::putText(combined, "ANTES", textPosBefore, fontFace, 
                   fontScale, cv::Scalar(0, 0, 0), thickness + 2);
        cv::putText(combined, "ANTES", textPosBefore, fontFace, 
                   fontScale, color, thickness);
        
        // Etiqueta "DESPUÉS"
        cv::Point textPosAfter(before.cols + 20, 40);
        cv::putText(combined, "DESPUES", textPosAfter, fontFace, 
                   fontScale, cv::Scalar(0, 0, 0), thickness + 2);
        cv::putText(combined, "DESPUES", textPosAfter, fontFace, 
                   fontScale, color, thickness);
        
        // Línea divisoria vertical
        cv::line(combined, cv::Point(before.cols, 0), 
                cv::Point(before.cols, combined.rows), 
                cv::Scalar(255, 255, 0), 2);
    }
    
    return combined;
}

} // namespace UIHelpers
