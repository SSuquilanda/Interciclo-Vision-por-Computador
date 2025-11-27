#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "f2_io/dicom_reader.h"
#include "utils/itk_opencv_bridge.h"
#include "f3_preprocessing/denoising.h"
#include "f4_segmentation/segmentation.h"
#include "f5_morphology/morphology.h"

namespace fs = std::filesystem;

std::string g_outputDir;

void saveProcessImage(const cv::Mat& image, const std::string& step, const std::string& description) {
    if (g_outputDir.empty()) {
        std::cerr << "⚠ Error: directorio de salida no configurado" << std::endl;
        return;
    }
    
    std::string filename = g_outputDir + "/" + step + "_" + description + ".png";
    
    if (cv::imwrite(filename, image)) {
        std::cout << "  ✓ Guardado: " << filename << std::endl;
    } else {
        std::cerr << "  ✗ Error al guardar: " << filename << std::endl;
    }
}

// FUNCIÓN PARA SEGMENTAR AORTA (reutilizable)
std::vector<Segmentation::SegmentedRegion> segmentarAorta(
    const cv::Mat& imageHU_16bit, 
    const cv::Mat& image8bit,
    const std::string& label) {
    
    cv::Point2d imgCenter(imageHU_16bit.cols / 2.0, imageHU_16bit.rows / 2.0);
    
    // Umbralización
    cv::Mat thresholdMask = Segmentation::thresholdByRange(imageHU_16bit, 30, 120);
    
    // Segmentación
    Segmentation::SegmentationParams arteryParams;
    arteryParams.minHU = 30;
    arteryParams.maxHU = 120;
    arteryParams.minArea = 200;
    arteryParams.maxArea = 8000;
    arteryParams.visualColor = cv::Scalar(0, 255, 0);
    
    auto candidates = Segmentation::segmentOrgan(imageHU_16bit, arteryParams, "Arteria");
    
    // Filtros anatómicos
    std::vector<Segmentation::SegmentedRegion> filteredArteries;
    for (const auto& region : candidates) {
        double distX = std::abs(region.centroid.x - imgCenter.x);
        double distY = region.centroid.y - imgCenter.y;
        double distTotal = cv::norm(region.centroid - imgCenter);
        
        bool esCentral = (distX < 70);
        bool esAnterior = (distY < 20);
        bool esMediano = (distTotal < 100);
        bool tamanioOk = (region.area >= 300 && region.area <= 5000);
        
        if (esCentral && esAnterior && esMediano && tamanioOk) {
            filteredArteries.push_back(region);
        }
    }
    
    // Procesamiento morfológico
    std::vector<Segmentation::SegmentedRegion> finalArteries;
    if (!filteredArteries.empty()) {
        cv::Mat combinedMask = cv::Mat::zeros(imageHU_16bit.size(), CV_8U);
        for (const auto& r : filteredArteries) {
            cv::bitwise_or(combinedMask, r.mask, combinedMask);
        }
        
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);
        cv::dilate(combinedMask, combinedMask, kernel);
        
        auto finalComponents = Segmentation::findConnectedComponents(combinedMask, 500);
        
        if (!finalComponents.empty()) {
            std::sort(finalComponents.begin(), finalComponents.end(),
                [](const auto& a, const auto& b) { return a.area > b.area; });
            
            auto& largest = finalComponents[0];
            double dist = cv::norm(largest.centroid - imgCenter);
            
            if (dist < 120.0) {
                largest.label = label;
                largest.color = cv::Scalar(0, 255, 0);
                finalArteries.push_back(largest);
            }
        }
    }
    
    return finalArteries;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <ruta_archivo.IMA>" << std::endl;
        return -1;
    }

    std::string dicomPath = argv[1];

    try {
        // Configurar directorio de salida
        fs::path dicomFilePath = fs::path(dicomPath);
        fs::path outputPath = dicomFilePath.parent_path() / "resultados_aorta";
        
        if (!fs::exists(outputPath)) {
            fs::create_directories(outputPath);
            std::cout << "📁 Directorio creado: " << outputPath.string() << "\n" << std::endl;
        } else {
            std::cout << "📁 Usando directorio existente: " << outputPath.string() << "\n" << std::endl;
        }
        
        g_outputDir = outputPath.string();

        std::cout << "╔═══════════════════════════════════════════════╗" << std::endl;
        std::cout << "║  PIPELINE: AORTA - COMPARACIÓN DENOISING 🫀  ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════════╝\n" << std::endl;

        // 1. LECTURA DICOM
        std::cout << "→ PASO 1: Lectura DICOM" << std::endl;
        DicomIO::ImagePointer itkImage = DicomIO::readDicomImage(dicomPath);
        DicomIO::displayImageStatistics(itkImage);
        
        // 2. CONVERSIÓN
        std::cout << "\n→ PASO 2: Conversión ITK->OpenCV" << std::endl;
        cv::Mat imageHU_16bit = Bridge::itkToOpenCV(itkImage);
        cv::Mat image8bit_original = Bridge::normalize16to8bit(imageHU_16bit);
        
        saveProcessImage(image8bit_original, "01", "imagen_original");

        // ═══════════════════════════════════════════════════════════
        // 3. CARGAR MODELO DE DENOISING
        // ═══════════════════════════════════════════════════════════
        std::cout << "\n→ PASO 3: Cargando modelo DnCNN..." << std::endl;
        
        Denoising::DnCNNDenoiser denoiser;
        std::string modelPath = "models/dncnn_grayscale.onnx";
        
        bool denoisingAvailable = denoiser.loadModel(modelPath);
        
        if (!denoisingAvailable) {
            std::cerr << "  Advertencia: No se pudo cargar el modelo de denoising" << std::endl;
            std::cerr << "   Ruta esperada: " << modelPath << std::endl;
            std::cerr << "   Continuando SOLO con segmentación sin denoising...\n" << std::endl;
        }

        // ═══════════════════════════════════════════════════════════
        // 4. COMPARACIÓN: SIN DENOISING vs CON DENOISING
        // ═══════════════════════════════════════════════════════════
        
        cv::Mat image8bit_denoised;
        Denoising::DenoisingComparison comparison;
        
        if (denoisingAvailable) {
            std::cout << "\n→ PASO 4: Aplicando Denoising y Comparando..." << std::endl;
            
            // Aplicar denoising
            comparison = Denoising::compareWithAndWithoutDenoising(image8bit_original, denoiser);
            image8bit_denoised = comparison.denoised;
            
            // ═══════════════════════════════════════════════════════════
            // NUEVA SECCIÓN: CALCULAR Y VISUALIZAR DIFERENCIAS
            // ═══════════════════════════════════════════════════════════
            
            // 1. Calcular diferencia absoluta
            cv::Mat diff;
            cv::absdiff(image8bit_original, image8bit_denoised, diff);
            
            // 2. Amplificar diferencias para visualización
            cv::Mat diffAmplified;
            diff.convertTo(diffAmplified, CV_8U, 5.0, 0); // Multiplicar por 5
            
            // 3. Crear mapa de calor de diferencias
            cv::Mat diffColorMap;
            cv::applyColorMap(diffAmplified, diffColorMap, cv::COLORMAP_JET);
            
            // 4. Calcular histogramas
            int histSize = 256;
            float range[] = {0, 256};
            const float* histRange = {range};
            
            cv::Mat hist_original, hist_denoised;
            cv::calcHist(&image8bit_original, 1, 0, cv::Mat(), hist_original, 1, &histSize, &histRange);
            cv::calcHist(&image8bit_denoised, 1, 0, cv::Mat(), hist_denoised, 1, &histSize, &histRange);
            
            // Normalizar histogramas
            cv::normalize(hist_original, hist_original, 0, 400, cv::NORM_MINMAX);
            cv::normalize(hist_denoised, hist_denoised, 0, 400, cv::NORM_MINMAX);
            
            // 5. Dibujar histogramas
            int hist_w = 512;
            int hist_h = 400;
            int bin_w = cvRound((double)hist_w / histSize);
            
            cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(255, 255, 255));
            
            // Dibujar histograma original (rojo)
            for(int i = 1; i < histSize; i++) {
                cv::line(histImage, 
                    cv::Point(bin_w*(i-1), hist_h - cvRound(hist_original.at<float>(i-1))),
                    cv::Point(bin_w*(i), hist_h - cvRound(hist_original.at<float>(i))),
                    cv::Scalar(0, 0, 255), 2);
            }
            
            // Dibujar histograma denoised (verde)
            for(int i = 1; i < histSize; i++) {
                cv::line(histImage, 
                    cv::Point(bin_w*(i-1), hist_h - cvRound(hist_denoised.at<float>(i-1))),
                    cv::Point(bin_w*(i), hist_h - cvRound(hist_denoised.at<float>(i))),
                    cv::Scalar(0, 255, 0), 2);
            }
            
            // Agregar leyenda
            cv::putText(histImage, "Rojo = Original", cv::Point(10, 30), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
            cv::putText(histImage, "Verde = Denoised", cv::Point(10, 60), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
            
            // 6. Crear panel de comparación mejorado
            cv::Mat panel_original, panel_denoised, panel_diff;
            
            // Convertir a color
            cv::cvtColor(image8bit_original, panel_original, cv::COLOR_GRAY2BGR);
            cv::cvtColor(image8bit_denoised, panel_denoised, cv::COLOR_GRAY2BGR);
            
            // Agregar títulos y métricas
            std::string psnr_text = "PSNR: " + std::to_string(comparison.psnr).substr(0, 6) + " dB";
            std::string snr_text = "SNR: " + std::to_string(comparison.snr).substr(0, 6) + " dB";
            std::string time_text = "Tiempo: " + std::to_string(comparison.processingTime).substr(0, 6) + " ms";
            
            cv::putText(panel_original, "ORIGINAL (Con Ruido)", 
                       cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
            
            cv::putText(panel_denoised, "DENOISED (Limpia)", 
                       cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
            cv::putText(panel_denoised, psnr_text, 
                       cv::Point(10, panel_denoised.rows - 90), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);
            cv::putText(panel_denoised, snr_text, 
                       cv::Point(10, panel_denoised.rows - 60), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);
            cv::putText(panel_denoised, time_text, 
                       cv::Point(10, panel_denoised.rows - 30), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);
            
            // Panel de diferencias
            cv::putText(diffColorMap, "DIFERENCIAS (x5 amplificado)", 
                       cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 2);
            
            double minDiff, maxDiff;
            cv::minMaxLoc(diff, &minDiff, &maxDiff);
            std::string diff_text = "Max diff: " + std::to_string((int)maxDiff) + " niveles";
            cv::putText(diffColorMap, diff_text, 
                       cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);
            
            // Comparación triple
            cv::Mat comparacion_detallada;
            cv::hconcat(panel_original, panel_denoised, comparacion_detallada);
            cv::hconcat(comparacion_detallada, diffColorMap, comparacion_detallada);
            
            // Guardar todas las visualizaciones
            saveProcessImage(comparacion_detallada, "02a", "comparacion_denoising_detallada");
            saveProcessImage(diffColorMap, "02b", "mapa_diferencias");
            saveProcessImage(diffAmplified, "02c", "diferencias_amplificadas");
            saveProcessImage(histImage, "02d", "histogramas_comparacion");
            saveProcessImage(image8bit_denoised, "03", "imagen_denoised");
            
            std::cout << "\n📊 Métricas de Denoising:" << std::endl;
            std::cout << "   PSNR: " << comparison.psnr << " dB" << std::endl;
            std::cout << "   SNR: " << comparison.snr << " dB" << std::endl;
            std::cout << "   Tiempo: " << comparison.processingTime << " ms" << std::endl;
            std::cout << "   Diferencia máxima: " << maxDiff << " niveles de gris" << std::endl;
            std::cout << "   Diferencia promedio: " << cv::mean(diff)[0] << " niveles" << std::endl;
            
            // VERIFICACIÓN: Calcular estadísticas de ruido
            cv::Scalar mean_orig, stddev_orig;
            cv::Scalar mean_den, stddev_den;
            
            cv::meanStdDev(image8bit_original, mean_orig, stddev_orig);
            cv::meanStdDev(image8bit_denoised, mean_den, stddev_den);
            
            std::cout << "\n🔬 Análisis de Ruido:" << std::endl;
            std::cout << "   Original  → Mean: " << mean_orig[0] << " | StdDev: " << stddev_orig[0] << std::endl;
            std::cout << "   Denoised  → Mean: " << mean_den[0] << " | StdDev: " << stddev_den[0] << std::endl;
            std::cout << "   Reducción de ruido: " << ((stddev_orig[0] - stddev_den[0]) / stddev_orig[0] * 100) << "%" << std::endl;
            
            // Si la reducción es < 5%, el modelo podría no estar funcionando
            if (stddev_orig[0] - stddev_den[0] < stddev_orig[0] * 0.05) {
                std::cout << "\n  ADVERTENCIA: Reducción de ruido mínima detectada" << std::endl;
                std::cout << "   Posibles causas:" << std::endl;
                std::cout << "   1. Modelo no tiene pesos pre-entrenados" << std::endl;
                std::cout << "   2. Imagen original tiene poco ruido (Quarter Dose ya es buena)" << std::endl;
                std::cout << "   3. Modelo no es adecuado para imágenes médicas CT" << std::endl;
            }
        } else {
            image8bit_denoised = image8bit_original.clone();
        }

        // ═══════════════════════════════════════════════════════════
        // 5. SEGMENTACIÓN EN AMBAS IMÁGENES
        // ═══════════════════════════════════════════════════════════
        
        std::cout << "\n→ PASO 5: Segmentando Aorta en AMBAS imágenes..." << std::endl;
        
        std::cout << "\n  [A] Segmentación SIN Denoising:" << std::endl;
        auto arterias_sin_denoising = segmentarAorta(imageHU_16bit, image8bit_original, "Aorta (Sin Denoising)");
        
        std::cout << "\n  [B] Segmentación CON Denoising:" << std::endl;
        auto arterias_con_denoising = segmentarAorta(imageHU_16bit, image8bit_denoised, "Aorta (Con Denoising)");

        // ═══════════════════════════════════════════════════════════
        // 6. VISUALIZACIÓN COMPARATIVA
        // ═══════════════════════════════════════════════════════════
        
        std::cout << "\n→ PASO 6: Generando visualización comparativa..." << std::endl;
        
        // Resultado SIN denoising
        cv::Mat imageColor_sin = Bridge::convertToColor(image8bit_original);
        cv::Mat result_sin = imageColor_sin.clone();
        
        if (!arterias_sin_denoising.empty()) {
            cv::Mat overlay = imageColor_sin.clone();
            for (const auto& artery : arterias_sin_denoising) {
                overlay.setTo(cv::Scalar(0, 255, 0), artery.mask);
            }
            cv::addWeighted(imageColor_sin, 0.6, overlay, 0.4, 0, result_sin);
            
            for (const auto& artery : arterias_sin_denoising) {
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(artery.mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
                cv::drawContours(result_sin, contours, -1, cv::Scalar(0, 255, 0), 3);
            }
        }
        
        cv::putText(result_sin, "SIN DENOISING", 
                   cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 0, 0), 2);
        
        // Resultado CON denoising
        cv::Mat imageColor_con = Bridge::convertToColor(image8bit_denoised);
        cv::Mat result_con = imageColor_con.clone();
        
        if (!arterias_con_denoising.empty()) {
            cv::Mat overlay = imageColor_con.clone();
            for (const auto& artery : arterias_con_denoising) {
                overlay.setTo(cv::Scalar(0, 255, 0), artery.mask);
            }
            cv::addWeighted(imageColor_con, 0.6, overlay, 0.4, 0, result_con);
            
            for (const auto& artery : arterias_con_denoising) {
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(artery.mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
                cv::drawContours(result_con, contours, -1, cv::Scalar(0, 255, 0), 3);
            }
        }
        
        cv::putText(result_con, "CON DENOISING", 
                   cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

        // Guardar resultados
        saveProcessImage(result_sin, "04", "segmentacion_sin_denoising");
        saveProcessImage(result_con, "05", "segmentacion_con_denoising");
        
        // Crear comparación lado a lado
        cv::Mat comparacion_final;
        cv::hconcat(result_sin, result_con, comparacion_final);
        saveProcessImage(comparacion_final, "06", "comparacion_final");


        // ═══════════════════════════════════════════════════════════
        // 7. MOSTRAR VENTANAS
        // ═══════════════════════════════════════════════════════════
        
        std::cout << "\n╔═══════════════════════════════════════════════╗" << std::endl;
        std::cout << "║          COMPARACIÓN COMPLETADA               ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════════╝\n" << std::endl;
        
        std::cout << " Resultados:" << std::endl;
        std::cout << "   SIN Denoising: " << arterias_sin_denoising.size() << " estructura(s) detectada(s)" << std::endl;
        std::cout << "   CON Denoising: " << arterias_con_denoising.size() << " estructura(s) detectada(s)" << std::endl;
        
        if (denoisingAvailable) {
            std::cout << "\n   Mejora con Denoising:" << std::endl;
            std::cout << "   - PSNR: " << comparison.psnr << " dB" << std::endl;
            std::cout << "   - SNR: " << comparison.snr << " dB" << std::endl;
            std::cout << "   - Tiempo procesamiento: " << comparison.processingTime << " ms" << std::endl;
        }
        
        std::cout << "\n Archivos guardados en: " << g_outputDir << std::endl;
        std::cout << "   • 01_imagen_original.png" << std::endl;
        
        if (denoisingAvailable) {
            std::cout << "   • 02a_comparacion_denoising_detallada.png" << std::endl;
            std::cout << "   • 02b_mapa_diferencias.png" << std::endl;
            std::cout << "   • 02c_diferencias_amplificadas.png" << std::endl;
            std::cout << "   • 02d_histogramas_comparacion.png" << std::endl;
            std::cout << "   • 03_imagen_denoised.png" << std::endl;
        }
        
        std::cout << "   • 04_segmentacion_sin_denoising.png" << std::endl;
        std::cout << "   • 05_segmentacion_con_denoising.png" << std::endl;
        std::cout << "   • 06_comparacion_final.png (LADO A LADO)" << std::endl;

        // ═══════════════════════════════════════════════════════════
        // MOSTRAR SOLO 4 VENTANAS PRINCIPALES
        // ═══════════════════════════════════════════════════════════
        
        std::cout << "\n🖼️  Mostrando ventanas de visualización..." << std::endl;
        
        // VENTANA 1: Original
        cv::imshow("1. Imagen Original CT", image8bit_original);
        
        // VENTANA 2: Comparación Denoising (solo si está disponible)
        if (denoisingAvailable) {
            // Crear panel de comparación detallada
            cv::Mat panel_original_vis, panel_denoised_vis;
            cv::cvtColor(image8bit_original, panel_original_vis, cv::COLOR_GRAY2BGR);
            cv::cvtColor(image8bit_denoised, panel_denoised_vis, cv::COLOR_GRAY2BGR);
            
            // Agregar texto
            cv::putText(panel_original_vis, "ORIGINAL", 
                       cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
            cv::putText(panel_denoised_vis, "DENOISED", 
                       cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
            
            // Agregar métricas
            std::string psnr_text = "PSNR: " + std::to_string(comparison.psnr).substr(0, 5) + " dB";
            cv::putText(panel_denoised_vis, psnr_text, 
                       cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);
            
            // Concatenar horizontalmente
            cv::Mat denoising_comparison;
            cv::hconcat(panel_original_vis, panel_denoised_vis, denoising_comparison);
            
            cv::imshow("2. Comparacion Denoising", denoising_comparison);
        }
        
        // VENTANA 3: Máscaras binarias de segmentación
        cv::Mat mascaras_binarias;
        
        // Crear máscaras binarias para visualización
        cv::Mat mask_sin = cv::Mat::zeros(imageHU_16bit.size(), CV_8U);
        cv::Mat mask_con = cv::Mat::zeros(imageHU_16bit.size(), CV_8U);
        
        if (!arterias_sin_denoising.empty()) {
            for (const auto& artery : arterias_sin_denoising) {
                cv::bitwise_or(mask_sin, artery.mask, mask_sin);
            }
        }
        
        if (!arterias_con_denoising.empty()) {
            for (const auto& artery : arterias_con_denoising) {
                cv::bitwise_or(mask_con, artery.mask, mask_con);
            }
        }
        
        // Convertir a color para visualización
        cv::Mat mask_sin_color, mask_con_color;
        cv::cvtColor(mask_sin, mask_sin_color, cv::COLOR_GRAY2BGR);
        cv::cvtColor(mask_con, mask_con_color, cv::COLOR_GRAY2BGR);
        
        // Colorear máscaras (rojo para sin denoising, verde para con denoising)
        mask_sin_color.setTo(cv::Scalar(0, 0, 255), mask_sin);
        mask_con_color.setTo(cv::Scalar(0, 255, 0), mask_con);
        
        // Agregar títulos
        cv::putText(mask_sin_color, "MASCARA SIN DENOISING", 
                   cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
        cv::putText(mask_con_color, "MASCARA CON DENOISING", 
                   cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
        
        // Concatenar máscaras
        cv::hconcat(mask_sin_color, mask_con_color, mascaras_binarias);
        cv::imshow("3. Mascaras Binarias (Rojo=Sin | Verde=Con)", mascaras_binarias);
        
        // VENTANA 4: Comparación final (segmentación sobre imagen)
        cv::imshow("4. Comparacion Final - Segmentacion", comparacion_final);
        
        std::cout << "\n💡 Ventanas mostradas:" << std::endl;
        std::cout << "   1. Imagen Original CT" << std::endl;
        if (denoisingAvailable) {
            std::cout << "   2. Comparacion Denoising" << std::endl;
        }
        std::cout << "   3. Mascaras Binarias" << std::endl;
        std::cout << "   4. Comparacion Final" << std::endl;
        
        std::cout << "\n💡 Presiona cualquier tecla para salir..." << std::endl;
        cv::waitKey(0);

    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERROR: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}