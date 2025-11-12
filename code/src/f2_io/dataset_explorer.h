#ifndef DATASET_EXPLORER_H
#define DATASET_EXPLORER_H

#include <string>
#include <vector>
#include <map>
#include "dicom_reader.h"
#include "opencv2/core.hpp"

namespace DatasetExplorer {

// Estructura para almacenar información de un slice
struct SliceInfo {
    int sliceNumber;
    std::string filename;
    double mean;
    double stdDev;
    double min;
    double max;
    double snr;  // Signal-to-Noise Ratio
};

// Estructura para comparar Full Dose vs Quarter Dose
struct DoseComparison {
    SliceInfo fullDose;
    SliceInfo quarterDose;
    double meanDifference;
    double stdDevDifference;
    double psnr;  // Peak Signal-to-Noise Ratio
};

// Obtiene la lista de archivos DICOM en una carpeta
std::vector<std::string> getDicomFileList(const std::string& folderPath);

// Calcula estadísticas de un slice
SliceInfo calculateSliceStats(DicomIO::ImagePointer image, 
                               const std::string& filename, 
                               int sliceNumber);

// Compara dos slices (Full Dose vs Quarter Dose)
DoseComparison compareSlices(DicomIO::ImagePointer fdImage, 
                              DicomIO::ImagePointer qdImage,
                              const SliceInfo& fdInfo,
                              const SliceInfo& qdInfo);

// Calcula PSNR entre dos imágenes
double calculatePSNR(const cv::Mat& img1, const cv::Mat& img2);

// Identifica los slices más representativos del dataset
std::vector<int> identifyRepresentativeSlices(
    const std::vector<DoseComparison>& comparisons, 
    int numSlices = 5);

// Guarda un reporte CSV con las estadísticas
void saveComparisonReport(const std::vector<DoseComparison>& comparisons,
                          const std::string& outputPath);

// Muestra estadísticas resumidas en consola
void displaySummaryStatistics(const std::vector<DoseComparison>& comparisons);

} // namespace DatasetExplorer

#endif // DATASET_EXPLORER_H
