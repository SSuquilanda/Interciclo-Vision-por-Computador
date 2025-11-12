#include "dataset_explorer.h"
#include "itkStatisticsImageFilter.h"
#include "../utils/itk_opencv_bridge.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

namespace DatasetExplorer {

std::vector<std::string> getDicomFileList(const std::string& folderPath) {
    std::vector<std::string> files;
    
    if (!fs::exists(folderPath)) {
        std::cerr << "Error: La carpeta no existe: " << folderPath << std::endl;
        return files;
    }
    
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".IMA" || ext == ".dcm" || ext == ".DCM") {
                files.push_back(entry.path().string());
            }
        }
    }
    
    // Ordenar alfabéticamente
    std::sort(files.begin(), files.end());
    
    return files;
}

SliceInfo calculateSliceStats(DicomIO::ImagePointer image, 
                               const std::string& filename, 
                               int sliceNumber) {
    SliceInfo info;
    info.sliceNumber = sliceNumber;
    info.filename = filename;
    
    using StatsFilterType = itk::StatisticsImageFilter<DicomIO::ImageType>;
    auto statsFilter = StatsFilterType::New();
    statsFilter->SetInput(image);
    statsFilter->Update();
    
    info.mean = statsFilter->GetMean();
    info.stdDev = statsFilter->GetSigma();
    info.min = statsFilter->GetMinimum();
    info.max = statsFilter->GetMaximum();
    
    // Calcular SNR (Signal-to-Noise Ratio)
    // SNR = mean / stdDev (simplificado)
    info.snr = (info.stdDev > 0) ? (info.mean / info.stdDev) : 0.0;
    
    return info;
}

double calculatePSNR(const cv::Mat& img1, const cv::Mat& img2) {
    if (img1.size() != img2.size() || img1.type() != img2.type()) {
        std::cerr << "Error: Las imágenes deben tener el mismo tamaño y tipo" << std::endl;
        return 0.0;
    }
    
    cv::Mat diff;
    cv::absdiff(img1, img2, diff);
    diff.convertTo(diff, CV_32F);
    diff = diff.mul(diff);  // Elevar al cuadrado
    
    cv::Scalar s = cv::sum(diff);
    double sse = s.val[0];  // Sum of Squared Errors
    
    if (sse <= 1e-10) {
        return 0.0;  // Imágenes idénticas
    }
    
    double mse = sse / (double)(img1.channels() * img1.total());
    
    // Encontrar el valor máximo posible
    double maxVal;
    cv::minMaxLoc(img1, nullptr, &maxVal);
    
    double psnr = 10.0 * std::log10((maxVal * maxVal) / mse);
    
    return psnr;
}

DoseComparison compareSlices(DicomIO::ImagePointer fdImage, 
                              DicomIO::ImagePointer qdImage,
                              const SliceInfo& fdInfo,
                              const SliceInfo& qdInfo) {
    DoseComparison comparison;
    comparison.fullDose = fdInfo;
    comparison.quarterDose = qdInfo;
    
    comparison.meanDifference = std::abs(fdInfo.mean - qdInfo.mean);
    comparison.stdDevDifference = std::abs(fdInfo.stdDev - qdInfo.stdDev);
    
    // Convertir a OpenCV para calcular PSNR
    cv::Mat fdMat = Bridge::itkToOpenCV(fdImage);
    cv::Mat qdMat = Bridge::itkToOpenCV(qdImage);
    
    comparison.psnr = calculatePSNR(fdMat, qdMat);
    
    return comparison;
}

std::vector<int> identifyRepresentativeSlices(
    const std::vector<DoseComparison>& comparisons, 
    int numSlices) {
    
    std::vector<int> indices;
    
    if (comparisons.empty()) {
        return indices;
    }
    
    int totalSlices = comparisons.size();
    
    // Seleccionar slices distribuidos uniformemente  aquellos con características interesantes
    
    // Slice del inicio (primeros 10%)
    indices.push_back(totalSlices * 0.05);
    
    // Slice con mayor diferencia de ruido (más interesante para análisis)
    auto maxStdDevDiff = std::max_element(comparisons.begin(), comparisons.end(),
        [](const DoseComparison& a, const DoseComparison& b) {
            return a.stdDevDifference < b.stdDevDifference;
        });
    indices.push_back(std::distance(comparisons.begin(), maxStdDevDiff));
    
    // Slice del medio (50%)
    indices.push_back(totalSlices / 2);

    // Slice con menor PSNR (mayor diferencia visual)
    auto minPSNR = std::min_element(comparisons.begin(), comparisons.end(),
        [](const DoseComparison& a, const DoseComparison& b) {
            return a.psnr < b.psnr;
        });
    indices.push_back(std::distance(comparisons.begin(), minPSNR));
    
    // Slice del final (últimos 10%)
    indices.push_back(totalSlices * 0.95);
    
    // Eliminar duplicados y ordenar
    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
    
    // Limitar al número solicitado
    if (indices.size() > static_cast<size_t>(numSlices)) {
        indices.resize(numSlices);
    }
    
    return indices;
}

void saveComparisonReport(const std::vector<DoseComparison>& comparisons,
                          const std::string& outputPath) {
    std::ofstream file(outputPath);
    
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo crear el archivo " << outputPath << std::endl;
        return;
    }
    
    // Escribir encabezados
    file << "SliceNumber,";
    file << "FD_Mean,FD_StdDev,FD_Min,FD_Max,FD_SNR,";
    file << "QD_Mean,QD_StdDev,QD_Min,QD_Max,QD_SNR,";
    file << "MeanDiff,StdDevDiff,PSNR\n";
    
    // Escribir datos
    file << std::fixed << std::setprecision(4);
    for (const auto& comp : comparisons) {
        file << comp.fullDose.sliceNumber << ",";
        file << comp.fullDose.mean << "," << comp.fullDose.stdDev << ",";
        file << comp.fullDose.min << "," << comp.fullDose.max << ",";
        file << comp.fullDose.snr << ",";
        file << comp.quarterDose.mean << "," << comp.quarterDose.stdDev << ",";
        file << comp.quarterDose.min << "," << comp.quarterDose.max << ",";
        file << comp.quarterDose.snr << ",";
        file << comp.meanDifference << "," << comp.stdDevDifference << ",";
        file << comp.psnr << "\n";
    }
    
    file.close();
    std::cout << "Reporte guardado en: " << outputPath << std::endl;
}

void displaySummaryStatistics(const std::vector<DoseComparison>& comparisons) {
    if (comparisons.empty()) {
        std::cout << "No hay datos para mostrar." << std::endl;
        return;
    }
    
    // Calcular promedios
    double avgMeanDiff = 0.0;
    double avgStdDevDiff = 0.0;
    double avgPSNR = 0.0;
    double avgFD_SNR = 0.0;
    double avgQD_SNR = 0.0;
    
    for (const auto& comp : comparisons) {
        avgMeanDiff += comp.meanDifference;
        avgStdDevDiff += comp.stdDevDifference;
        avgPSNR += comp.psnr;
        avgFD_SNR += comp.fullDose.snr;
        avgQD_SNR += comp.quarterDose.snr;
    }
    
    int n = comparisons.size();
    avgMeanDiff /= n;
    avgStdDevDiff /= n;
    avgPSNR /= n;
    avgFD_SNR /= n;
    avgQD_SNR /= n;
    
    // Encontrar valores extremos
    auto minPSNR = std::min_element(comparisons.begin(), comparisons.end(),
        [](const DoseComparison& a, const DoseComparison& b) {
            return a.psnr < b.psnr;
        });
    
    auto maxPSNR = std::max_element(comparisons.begin(), comparisons.end(),
        [](const DoseComparison& a, const DoseComparison& b) {
            return a.psnr < b.psnr;
        });
    
    // Mostrar resultados
    std::cout << "\nRESUMEN ESTADÍSTICO: FULL DOSE vs QUARTER DOSE\n";
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total de slices analizados: " << n << "\n\n";
    
    std::cout << "--- Diferencias Promedio ---\n";
    std::cout << "  Diferencia en Media (HU): " << avgMeanDiff << "\n";
    std::cout << "  Diferencia en Desv. Estándar: " << avgStdDevDiff << "\n\n";
    
    std::cout << "--- Signal-to-Noise Ratio (SNR) ---\n";
    std::cout << "  Full Dose SNR promedio: " << avgFD_SNR << "\n";
    std::cout << "  Quarter Dose SNR promedio: " << avgQD_SNR << "\n";
    std::cout << "  Degradación de SNR: " << std::setprecision(1) 
              << ((avgFD_SNR - avgQD_SNR) / avgFD_SNR * 100.0) << "%\n\n";
    
    std::cout << "--- Peak Signal-to-Noise Ratio (PSNR) ---\n";
    std::cout << "  PSNR promedio: " << avgPSNR << " dB\n";
    std::cout << "  PSNR mínimo: " << minPSNR->psnr << " dB (Slice " 
              << minPSNR->fullDose.sliceNumber << ")\n";
    std::cout << "  PSNR máximo: " << maxPSNR->psnr << " dB (Slice " 
              << maxPSNR->fullDose.sliceNumber << ")\n\n";
    
    // Interpretación
    std::cout << "Interpretación:\n";
    if (avgPSNR > 40) {
        std::cout << "Bien: Quarter Dose mantiene alta calidad\n";
    } else if (avgPSNR > 30) {
        std::cout << "Regular: Quarter Dose tiene calidad aceptable\n";
    } else {
        std::cout << "Mal: Quarter Dose muestra degradación significativa\n";
    }
    
    std::cout << "\n";
}

} // namespace DatasetExplorer
