#ifndef METRICS_CONTROLLER_H
#define METRICS_CONTROLLER_H

#include <QObject>
#include <QLabel>
#include <QTableWidget>
#include <opencv2/opencv.hpp>

// Forward declaration
struct SliceContext;

/**
 * @brief Controller for metrics calculation and display
 * 
 * Handles ROI statistics, histogram generation, and system performance metrics.
 */
class MetricsController : public QObject
{
    Q_OBJECT

public:
    explicit MetricsController(QObject* parent = nullptr);
    ~MetricsController() = default;

    // Context and widget configuration
    void setSliceContext(SliceContext* context);
    void setWidgets(
        QTableWidget* metricsTable,
        QLabel* histogramLabel,
        QLabel* processTimeLabel,
        QLabel* memoryUsageLabel
    );

    // Main operations
    void updateMetrics();

signals:
    void statusMessageChanged(const QString& message);

private:
    // Context
    SliceContext* sliceContext = nullptr;

    // Widget pointers
    QTableWidget* tableMetrics = nullptr;
    QLabel* lblHistogramROI = nullptr;
    QLabel* lblProcessTime = nullptr;
    QLabel* lblMemoryUsage = nullptr;

    // Helper methods
    cv::Mat generateHistogram(const cv::Mat& image, const cv::Mat& mask);
    double calculateMemoryUsage();
};

#endif // METRICS_CONTROLLER_H
