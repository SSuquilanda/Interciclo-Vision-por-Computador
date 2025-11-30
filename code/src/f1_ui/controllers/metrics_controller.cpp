#include "metrics_controller.h"
#include "f1_ui/mainwindow.h"
#include "f1_ui/helpers/qt_opencv_helpers.h"
#include "utils/itk_opencv_bridge.h"
#include <QPixmap>
#include <QTableWidgetItem>

MetricsController::MetricsController(QObject* parent)
    : QObject(parent)
{
}

void MetricsController::setSliceContext(SliceContext* context)
{
    sliceContext = context;
}

void MetricsController::setWidgets(
    QTableWidget* metricsTable,
    QLabel* histogramLabel,
    QLabel* processTimeLabel,
    QLabel* memoryUsageLabel)
{
    tableMetrics = metricsTable;
    lblHistogramROI = histogramLabel;
    lblProcessTime = processTimeLabel;
    lblMemoryUsage = memoryUsageLabel;
}

void MetricsController::updateMetrics()
{
    if (!tableMetrics || !lblHistogramROI || !lblProcessTime || !lblMemoryUsage) {
        return;
    }

    // Verify if there is a segmentation mask
    if (!sliceContext || sliceContext->segmentationMask.empty() || sliceContext->originalRaw.empty()) {
        tableMetrics->setRowCount(0);
        lblHistogramROI->setText("No hay segmentación disponible");
        lblProcessTime->setText("Tiempo de Pipeline: 0 ms");
        lblMemoryUsage->setText("Memoria Estimada: 0 MB");
        return;
    }

    // ===== ROI CALCULATIONS =====
    cv::Mat mask = sliceContext->segmentationMask;
    cv::Mat original = sliceContext->originalRaw;

    // Calculate area (number of pixels in mask)
    int area = cv::countNonZero(mask);

    // Calculate mean density and standard deviation within mask
    cv::Scalar mean, stddev;
    cv::meanStdDev(original, mean, stddev, mask);

    // ===== UPDATE TABLE =====
    tableMetrics->setRowCount(0);
    tableMetrics->insertRow(0);
    
    tableMetrics->setItem(0, 0, new QTableWidgetItem("Pulmones"));
    tableMetrics->setItem(0, 1, new QTableWidgetItem(QString::number(area)));
    tableMetrics->setItem(0, 2, new QTableWidgetItem(QString::number(mean[0], 'f', 1) + " HU"));
    tableMetrics->setItem(0, 3, new QTableWidgetItem(QString::number(stddev[0], 'f', 2)));

    // ===== ROI HISTOGRAM =====
    cv::Mat histImage = generateHistogram(original, mask);

    // Convert and display
    QImage qImage = UIHelpers::cvMatToQImage(histImage);
    lblHistogramROI->setPixmap(QPixmap::fromImage(qImage));

    // ===== SYSTEM PERFORMANCE =====
    // Update processing time
    lblProcessTime->setText(QString("Tiempo de Pipeline: %1 ms")
                            .arg(QString::number(sliceContext->processingTimeMs, 'f', 2)));

    // Calculate estimated memory
    double memoryMB = calculateMemoryUsage();
    lblMemoryUsage->setText(QString("Memoria Estimada: %1 MB")
                            .arg(QString::number(memoryMB, 'f', 2)));
}

cv::Mat MetricsController::generateHistogram(const cv::Mat& image, const cv::Mat& mask)
{
    // Convert 16-bit image to 8-bit for histogram
    cv::Mat original8bit = Bridge::normalize16to8bit(image);
    
    // Histogram configuration
    int histSize = 256;
    float range[] = {0, 256}; // Range for 8-bit images
    const float* histRange = {range};
    bool uniform = true;
    bool accumulate = false;

    cv::Mat hist;
    cv::calcHist(&original8bit, 1, 0, mask, hist, 1, &histSize, &histRange, uniform, accumulate);

    // Normalize histogram
    double maxVal;
    cv::minMaxLoc(hist, nullptr, &maxVal);
    
    // Create histogram image
    int hist_w = 512;
    int hist_h = 300;
    int bin_w = cvRound((double) hist_w / histSize);
    
    cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(255, 255, 255));
    
    // Normalize and draw
    for (int i = 1; i < histSize; i++) {
        int val1 = cvRound(hist.at<float>(i-1) * hist_h / maxVal);
        int val2 = cvRound(hist.at<float>(i) * hist_h / maxVal);
        
        cv::line(histImage,
                 cv::Point(bin_w * (i-1), hist_h - val1),
                 cv::Point(bin_w * i, hist_h - val2),
                 cv::Scalar(0, 120, 255), 2, cv::LINE_AA);
    }

    // Add axes and title
    cv::putText(histImage, "Distribucion de Intensidades (ROI)",
                cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                cv::Scalar(0, 0, 0), 1, cv::LINE_AA);
    
    cv::line(histImage, cv::Point(30, hist_h - 30), cv::Point(30, 30),
             cv::Scalar(0, 0, 0), 2);
    cv::line(histImage, cv::Point(30, hist_h - 30), cv::Point(hist_w - 10, hist_h - 30),
             cv::Scalar(0, 0, 0), 2);

    return histImage;
}

double MetricsController::calculateMemoryUsage()
{
    if (!sliceContext) {
        return 0.0;
    }

    // Memory = width * height * bytes_per_pixel * number_of_layers
    double memoryMB = 0.0;
    
    if (!sliceContext->originalRaw.empty()) {
        // Original (16-bit): 2 bytes per pixel
        memoryMB += (sliceContext->originalRaw.cols * sliceContext->originalRaw.rows * 2) / (1024.0 * 1024.0);
    }
    
    if (!sliceContext->preprocessed.empty()) {
        // Preprocessed (8-bit): 1 byte per pixel
        memoryMB += (sliceContext->preprocessed.cols * sliceContext->preprocessed.rows) / (1024.0 * 1024.0);
    }
    
    if (!sliceContext->segmentationMask.empty()) {
        // Mask (8-bit): 1 byte per pixel
        memoryMB += (sliceContext->segmentationMask.cols * sliceContext->segmentationMask.rows) / (1024.0 * 1024.0);
    }
    
    if (!sliceContext->finalOverlay.empty()) {
        // Overlay (BGR 8-bit): 3 bytes per pixel
        memoryMB += (sliceContext->finalOverlay.cols * sliceContext->finalOverlay.rows * 3) / (1024.0 * 1024.0);
    }

    return memoryMB;
}
