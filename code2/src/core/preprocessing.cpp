#include "preprocessing.h"
#include <opencv2/dnn.hpp> // Importante para la IA
#include <iostream>
#include <fstream>

namespace Preprocessing {

    cv::Mat convertToGrayscale(const cv::Mat& image) {
        cv::Mat gray;
        if (image.channels() == 3) cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        else gray = image.clone();
        return gray;
    }

    cv::Mat normalizeImage(const cv::Mat& image) {
        cv::Mat norm;
        cv::normalize(image, norm, 0, 255, cv::NORM_MINMAX, CV_8U);
        return norm;
    }

    cv::Mat applyGaussianFilter(const cv::Mat& image, int kernelSize, double sigma) {
        cv::Mat result;
        if (kernelSize % 2 == 0) kernelSize++; // Asegurar impar
        cv::GaussianBlur(image, result, cv::Size(kernelSize, kernelSize), sigma);
        return result;
    }

    cv::Mat applyMedianFilter(const cv::Mat& image, int kernelSize) {
        cv::Mat result;
        if (kernelSize % 2 == 0) kernelSize++;
        cv::medianBlur(image, result, kernelSize);
        return result;
    }

    cv::Mat applyBilateralFilter(const cv::Mat& image, int d, double sigmaColor, double sigmaSpace) {
        cv::Mat result;
        cv::bilateralFilter(image, result, d, sigmaColor, sigmaSpace);
        return result;
    }

    cv::Mat applyCLAHE(const cv::Mat& image, double clipLimit, cv::Size tileGridSize) {
        cv::Mat result;
        auto clahe = cv::createCLAHE(clipLimit, tileGridSize);
        clahe->apply(image, result);
        return result;
    }

    // Implementar DnCNN

    // Callback para recibir datos de CURL
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    DnCNNDenoiser::DnCNNDenoiser() : modelLoaded(false), useFlaskServer(false), flaskServerUrl("http://localhost:5000/denoise") {}

    void DnCNNDenoiser::setFlaskServer(const std::string& url) {
        flaskServerUrl = url;
        useFlaskServer = true;
        std::cout << "[INFO] DnCNN configurado para usar Flask server: " << url << std::endl;
    }

    bool DnCNNDenoiser::loadModel(const std::string& onnxPath) {
        try {
            std::ifstream f(onnxPath.c_str());
            if (!f.good()) {
                std::cerr << "[Error] Archivo ONNX no encontrado: " << onnxPath << std::endl;
                return false;
            }

            net = cv::dnn::readNetFromONNX(onnxPath);
            if (net.empty()) return false;

            // Intentar usar aceleración si está disponible
            net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
            
            modelLoaded = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[Excepción DnCNN]: " << e.what() << std::endl;
            modelLoaded = false;
            return false;
        }
    }

    cv::Mat DnCNNDenoiser::denoiseViaFlask(const cv::Mat& noisyImage) {
        try {
            // Codificar imagen a PNG en memoria
            std::vector<uchar> buffer;
            cv::imencode(".png", noisyImage, buffer);
            
            CURL* curl = curl_easy_init();
            if (!curl) {
                std::cerr << "[ERROR] No se pudo inicializar CURL" << std::endl;
                return noisyImage.clone();
            }
            
            // Preparar form data
            curl_mime* form = curl_mime_init(curl);
            curl_mimepart* field = curl_mime_addpart(form);
            curl_mime_name(field, "image");
            curl_mime_data(field, (const char*)buffer.data(), buffer.size());
            curl_mime_filename(field, "image.png");
            curl_mime_type(field, "image/png");
            
            // Configurar request
            std::string responseBuffer;
            curl_easy_setopt(curl, CURLOPT_URL, flaskServerUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
            
            // Realizar request
            CURLcode res = curl_easy_perform(curl);
            
            curl_mime_free(form);
            curl_easy_cleanup(curl);
            
            if (res != CURLE_OK) {
                std::cerr << "[ERROR] Flask request falló: " << curl_easy_strerror(res) << std::endl;
                return noisyImage.clone();
            }
            
            // Decodificar respuesta
            std::vector<uchar> responseData(responseBuffer.begin(), responseBuffer.end());
            cv::Mat denoised = cv::imdecode(responseData, cv::IMREAD_GRAYSCALE);
            
            if (denoised.empty()) {
                std::cerr << "[ERROR] No se pudo decodificar la respuesta del servidor" << std::endl;
                return noisyImage.clone();
            }
            
            return denoised;
            
        } catch (const std::exception& e) {
            std::cerr << "[Excepción Flask]: " << e.what() << std::endl;
            return noisyImage.clone();
        }
    }

    cv::Mat DnCNNDenoiser::denoiseViaOpenCV(const cv::Mat& noisyImage) {
        if (!modelLoaded) return noisyImage.clone();

        try {
            // Convertir a Float32 y normalizar [0,1]
            cv::Mat inputFloat;
            int originalType = noisyImage.type();
            
            if (noisyImage.type() == CV_8U) {
                noisyImage.convertTo(inputFloat, CV_32F, 1.0 / 255.0);
            } else if (noisyImage.type() == CV_16S || noisyImage.type() == CV_16U) {
                // Para imágenes CT (16-bit), normalizar a [0, 1]
                double minVal, maxVal;
                cv::minMaxLoc(noisyImage, &minVal, &maxVal);
                noisyImage.convertTo(inputFloat, CV_32F, 1.0/(maxVal - minVal), -minVal/(maxVal - minVal));
            } else {
                inputFloat = noisyImage.clone();
            }
            
            // Asegurar single-channel
            if (inputFloat.channels() > 1) {
                cv::cvtColor(inputFloat, inputFloat, cv::COLOR_BGR2GRAY);
            }

            // Crear Blob (NCHW: 1 x 1 x H x W)
            cv::Mat blob = cv::dnn::blobFromImage(
                inputFloat,
                1.0,                    
                inputFloat.size(),      
                cv::Scalar(0),          
                false,                  
                false                   
            );
            
            // Inferencia
            net.setInput(blob);
            cv::Mat outputBlob = net.forward();

            // Extraer imagen del blob
            const int* dims = outputBlob.size.p;
            int height = dims[2];
            int width = dims[3];
            
            cv::Mat denoisedFloat = cv::Mat(height, width, CV_32F, outputBlob.ptr<float>(0, 0));
            denoisedFloat = denoisedFloat.clone();
            
            // Clamp a [0, 1]
            cv::max(denoisedFloat, 0.0, denoisedFloat);
            cv::min(denoisedFloat, 1.0, denoisedFloat);

            // Convertir al tipo original
            cv::Mat result;
            
            if (originalType == CV_8U) {
                denoisedFloat.convertTo(result, CV_8U, 255.0);
            } else if (originalType == CV_16S || originalType == CV_16U) {
                double minVal, maxVal;
                cv::minMaxLoc(noisyImage, &minVal, &maxVal);
                denoisedFloat.convertTo(result, originalType, (maxVal - minVal), minVal);
            } else {
                result = denoisedFloat;
            }
            
            return result;
            
        } catch (const std::exception& e) {
            std::cerr << "[Excepción OpenCV DNN]: " << e.what() << std::endl;
            return noisyImage.clone();
        }
    }

    cv::Mat DnCNNDenoiser::denoise(const cv::Mat& noisyImage) {
        // Prioridad 1: Intentar Flask Server
        if (useFlaskServer) {
            std::cout << "[INFO] Intentando denoising via Flask server..." << std::endl;
            cv::Mat result = denoiseViaFlask(noisyImage);
            
            // Si Flask funcionó, retornar resultado
            if (!result.empty() && (result.data != noisyImage.data)) {
                std::cout << "[✓] Denoising exitoso via Flask" << std::endl;
                return result;
            }
            
            // Si Flask falló, intentar fallback
            std::cerr << "[!] Flask server no disponible, intentando fallback a OpenCV DNN..." << std::endl;
        }
        
        // Prioridad 2: Fallback a OpenCV DNN local
        if (modelLoaded) {
            std::cout << "[INFO] Usando OpenCV DNN local como fallback..." << std::endl;
            cv::Mat result = denoiseViaOpenCV(noisyImage);
            std::cout << "[✓] Denoising exitoso via OpenCV DNN" << std::endl;
            return result;
        }
        
        // Si nada funciona, retornar imagen original
        std::cerr << "[!] Ni Flask ni OpenCV DNN disponibles, retornando imagen original" << std::endl;
        return noisyImage.clone();
    }

    // Aplicar DnCNN fácilmente

    cv::Mat applyDnCNN(const cv::Mat& noisyImage, const std::string& modelPath) {
        static DnCNNDenoiser denoiser;
        static std::string lastModelPath = "";
        
        // Cargar el modelo solo si es diferente o no está cargado
        if (!denoiser.isLoaded() || lastModelPath != modelPath) {
            if (!denoiser.loadModel(modelPath)) {
                std::cerr << "Error: No se pudo cargar DnCNN. Retornando imagen original." << std::endl;
                return noisyImage.clone();
            }
            lastModelPath = modelPath;
        }
        
        return denoiser.denoise(noisyImage);
    }

    // Métricas

    double calculatePSNR(const cv::Mat& I1, const cv::Mat& I2) {
        cv::Mat s1;
        cv::absdiff(I1, I2, s1);       // |I1 - I2|
        s1.convertTo(s1, CV_32F);      // No se puede elevar al cuadrado en 8-bit
        s1 = s1.mul(s1);               // (I1 - I2)^2

        cv::Scalar s = cv::sum(s1);    // Suma de elementos
        double sse = s.val[0] + s.val[1] + s.val[2]; // Suma de canales

        if(sse <= 1e-10) return 0; // Son iguales

        double mse = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }

    double calculateSNR(const cv::Mat& image) {
        cv::Scalar mean, stddev;
        cv::meanStdDev(image, mean, stddev);
        // SNR = Media / Desviación Estándar
        if (stddev.val[0] == 0) return 0;
        return 20.0 * log10(mean.val[0] / stddev.val[0]);
    }

}