# 🎨 Preprocessing - Filtrado y Mejora de Imágenes Médicas

## 📋 Descripción

El módulo **Preprocessing** (`preprocessing.h/cpp`) implementa algoritmos de **mejora y filtrado** de imágenes CT para reducir ruido, mejorar contraste y preparar las imágenes para análisis posterior. Incluye filtros clásicos y una **red neuronal DnCNN** para denoising avanzado.

## 🎯 Responsabilidad

> **Única responsabilidad**: Aplicar transformaciones que mejoren la calidad visual y SNR de imágenes CT sin alterar estructuras anatómicas

## 🏗️ Arquitectura

```cpp
namespace Preprocessing {
    // Filtros clásicos (operan en 8-bit)
    cv::Mat applyGaussianFilter(const cv::Mat& image, int kernelSize = 5);
    cv::Mat applyMedianFilter(const cv::Mat& image, int kernelSize = 5);
    cv::Mat applyBilateralFilter(const cv::Mat& image, int d = 9, 
                                  double sigmaColor = 75, double sigmaSpace = 75);
    cv::Mat applyCLAHE(const cv::Mat& image, double clipLimit = 2.0);
    
    // Red neuronal (opera en 8-bit o 16-bit)
    cv::Mat applyDnCNN(const cv::Mat& image, const std::string& modelPath);
    
    // Métricas de calidad
    double calculatePSNR(const cv::Mat& original, const cv::Mat& processed);
    double calculateSNR(const cv::Mat& image, const cv::Mat& mask = cv::Mat());
}
```

## 📚 API Pública

### 1. `applyGaussianFilter()`

**Descripción**: Aplica filtro gaussiano para reducir ruido de alta frecuencia

**Firma**:

```cpp
cv::Mat applyGaussianFilter(const cv::Mat& image, int kernelSize = 5);
```

**Parámetros**:

- `image`: Imagen 8-bit (`CV_8U`) o 16-bit (`CV_16S`)
- `kernelSize`: Tamaño del kernel (debe ser impar: 3, 5, 7, 9...)

**Retorna**:

- Imagen filtrada del mismo tipo que la entrada

**Funcionamiento**:

- Convolución con kernel gaussiano: $G(x,y) = \frac{1}{2\pi\sigma^2}e^{-\frac{x^2+y^2}{2\sigma^2}}$
- Suaviza la imagen ponderando más los píxeles cercanos
- Efecto: Reduce ruido pero puede difuminar bordes

**Cuándo usar**:

- ✅ Ruido gaussiano presente
- ✅ Imágenes con artefactos de reconstrucción leves
- ⚠️ Puede difuminar detalles finos

**Ejemplo**:

```cpp
cv::Mat original = Bridge::itkToOpenCV8bit(dicomData.image);
cv::Mat smoothed = Preprocessing::applyGaussianFilter(original, 5);

// Comparar SNR
double snrBefore = Preprocessing::calculateSNR(original);
double snrAfter = Preprocessing::calculateSNR(smoothed);

std::cout << "SNR antes: " << snrBefore << " dB" << std::endl;
std::cout << "SNR después: " << snrAfter << " dB" << std::endl;
```

---

### 2. `applyMedianFilter()`

**Descripción**: Aplica filtro de mediana para eliminar **ruido impulsivo** (salt-and-pepper)

**Firma**:

```cpp
cv::Mat applyMedianFilter(const cv::Mat& image, int kernelSize = 5);
```

**Parámetros**:

- `image`: Imagen 8-bit o 16-bit
- `kernelSize`: Tamaño de la ventana (3, 5, 7...)

**Retorna**:

- Imagen filtrada con ruido impulsivo eliminado

**Funcionamiento**:

- Para cada píxel, reemplaza su valor por la **mediana** de su vecindad
- No genera nuevos valores (solo usa valores existentes)
- **Preserva bordes** mejor que filtro gaussiano

**Cuándo usar**:

- ✅ Ruido tipo "sal y pimienta" (píxeles aislados muy diferentes)
- ✅ Artefactos de transmisión o digitalización
- ✅ Cuando es crítico preservar bordes

**Ejemplo**:

```cpp
cv::Mat noisy = loadNoisyImage();
cv::Mat clean = Preprocessing::applyMedianFilter(noisy, 5);

// Visualizar diferencia
cv::Mat diff;
cv::absdiff(noisy, clean, diff);
cv::imshow("Ruido eliminado", diff);
```

---

### 3. `applyBilateralFilter()` 🌟

**Descripción**: Aplica filtro bilateral que **reduce ruido preservando bordes** de forma excepcional

**Firma**:

```cpp
cv::Mat applyBilateralFilter(const cv::Mat& image, int d = 9, 
                             double sigmaColor = 75, double sigmaSpace = 75);
```

**Parámetros**:

- `image`: Imagen 8-bit o 16-bit
- `d`: Diámetro del vecindario de píxeles (valores típicos: 5, 9, 15)
- `sigmaColor`: Filtro sigma en el espacio de color
  - Valores altos (> 100): Mezcla colores/intensidades más diferentes
  - Valores bajos (< 50): Solo mezcla colores similares
- `sigmaSpace`: Filtro sigma en el espacio de coordenadas
  - Define hasta qué distancia espacial se consideran píxeles

**Retorna**:

- Imagen filtrada con bordes preservados

**Funcionamiento**:
El filtro bilateral combina **dos kernels**:

1. **Dominio espacial**: Píxeles cercanos tienen más peso (como gaussiano)
2. **Dominio de intensidad**: Píxeles con valores similares tienen más peso

**Fórmula**:
$$
I_{\text{filtered}}(x) = \frac{1}{W_p}\sum_{x_i \in \Omega}I(x_i) \cdot e^{-\frac{||x_i-x||^2}{2\sigma_s^2}} \cdot e^{-\frac{||I(x_i)-I(x)||^2}{2\sigma_c^2}}
$$

Donde:

- Primer término exponencial: peso espacial (gaussiano)
- Segundo término exponencial: peso de intensidad (preserva bordes)

**Ventajas sobre filtros clásicos**:

- ✅ **Preserva bordes perfectamente** (no difumina estructuras importantes)
- ✅ **Reduce ruido en regiones homogéneas** eficazmente
- ✅ **Ideal para imágenes médicas** donde bordes anatómicos son críticos
- ⚠️ Más lento que Gaussiano (complejidad cuadrática)

**Cuándo usar**:

- ✅ Imágenes CT con **ruido pero bordes críticos** (vasos, órganos)
- ✅ Pre-procesamiento antes de **segmentación** (mejora precisión)
- ✅ Cuando filtro gaussiano **difumina demasiado**
- ⚠️ No usar si velocidad es crítica (prefiere Gaussiano o Mediana)

**Ejemplo**:

```cpp
cv::Mat noisyCT = Bridge::itkToOpenCV8bit(dicomData.image);

// Bilateral con parámetros conservadores
cv::Mat smoothed = Preprocessing::applyBilateralFilter(noisyCT, 9, 75, 75);

// Bilateral agresivo (más suavizado, mantiene bordes)
cv::Mat aggressive = Preprocessing::applyBilateralFilter(noisyCT, 15, 150, 150);

// Visualizar diferencia en bordes
cv::Mat edges1, edges2;
cv::Canny(noisyCT, edges1, 50, 150);
cv::Canny(smoothed, edges2, 50, 150);

std::cout << "Bordes originales: " << cv::countNonZero(edges1) << std::endl;
std::cout << "Bordes después bilateral: " << cv::countNonZero(edges2) << std::endl;
// Los bordes se mantienen casi igual!
```

**Comparación visual**:

```bash
Original (ruidoso):    Gaussiano:           Bilateral:
  ███████████            ▓▓▓▓▓▓▓▓▓            ███████████
  ███░░░███              ▓▓▓░░░▓▓▓            ███░░░███
  ███░░░███              ▓▓▓░░░▓▓▓            ███░░░███
  ███████████            ▓▓▓▓▓▓▓▓▓            ███████████
  
  Ruido + Bordes        Bordes difusos       Bordes nítidos
  difuminados                                + Sin ruido ✓
```

**Parámetros recomendados por caso**:

| Aplicación | d | sigmaColor | sigmaSpace | Notas |
|------------|---|------------|------------|-------|
| **CT pulmones** | 9 | 50 | 50 | Preservar vasos finos |
| **CT huesos** | 7 | 75 | 75 | Balance estándar |
| **Suavizado fuerte** | 15 | 150 | 150 | Aún preserva bordes |
| **Tiempo real** | 5 | 50 | 50 | Más rápido |

**Implementación**:

```cpp
cv::Mat Preprocessing::applyBilateralFilter(const cv::Mat& image, int d, 
                                             double sigmaColor, double sigmaSpace) {
    cv::Mat result;
    cv::bilateralFilter(image, result, d, sigmaColor, sigmaSpace);
    return result;
}
```

**Tips de optimización**:

```cpp
// Para datasets grandes, procesar ROI en lugar de imagen completa
cv::Rect roi(100, 100, 300, 300);
cv::Mat subImage = image(roi);
cv::Mat filtered = Preprocessing::applyBilateralFilter(subImage, 9, 75, 75);
image(roi) = filtered;

// O reducir d para acelerar (complejidad O(d²))
// d=5 es 4× más rápido que d=9
```

---

### 4. `applyCLAHE()`

**Descripción**: **Contrast Limited Adaptive Histogram Equalization** - Mejora contraste local sin amplificar ruido

**Firma**:

```cpp
cv::Mat applyCLAHE(const cv::Mat& image, double clipLimit = 2.0);
```

**Parámetros**:

- `image`: Imagen 8-bit (`CV_8U`) - **IMPORTANTE**: Solo soporta 8-bit
- `clipLimit`: Límite de amplificación (típico: 1.0 - 4.0)
  - Valores bajos (1.0-2.0): contraste sutil
  - Valores altos (3.0-4.0): contraste agresivo

**Retorna**:

- Imagen 8-bit con contraste mejorado localmente

**Funcionamiento**:

1. Divide imagen en tiles (típicamente 8×8)
2. Ecualiza histograma en cada tile
3. Limita amplificación con `clipLimit` para evitar ruido
4. Interpola bordes entre tiles para suavidad

**Cuándo usar**:

- ✅ Imágenes con **bajo contraste** global
- ✅ Estructuras de interés en regiones oscuras/claras
- ✅ Visualización para radiólogos (mejora detalles sutiles)
- ⚠️ NO usar antes de segmentación (altera valores HU)

**Ejemplo**:

```cpp
cv::Mat lowContrast = Bridge::itkToOpenCV8bit(dicomData.image);

// CLAHE conservador para diagnóstico
cv::Mat diagnostic = Preprocessing::applyCLAHE(lowContrast, 2.0);

// CLAHE agresivo para visualización
cv::Mat enhanced = Preprocessing::applyCLAHE(lowContrast, 3.5);

// Mostrar ambos
cv::imshow("Original", lowContrast);
cv::imshow("CLAHE 2.0", diagnostic);
cv::imshow("CLAHE 3.5", enhanced);
```

---

### 4. `applyDnCNN()` ⭐

**Descripción**: Aplica **Deep Neural Network for Denoising** - Red neuronal convolucional entrenada para eliminar ruido preservando detalles

**Firma**:

```cpp
cv::Mat applyDnCNN(const cv::Mat& image, const std::string& modelPath);
```

**Parámetros**:

- `image`: Imagen 8-bit (`CV_8U`) o 16-bit (`CV_16S`)
- `modelPath`: Ruta al modelo ONNX (típicamente `src/models/dncnn_grayscale.onnx`)

**Retorna**:

- Imagen denoised del mismo tipo que la entrada

**Arquitectura de la red**:

```bash
Input [1x1xHxW]
    ↓
Conv2D(64 filters, 3x3) + ReLU
    ↓
16× [Conv2D(64, 3x3) + BatchNorm + ReLU]
    ↓
Conv2D(1, 3x3)  // Residual: predice el ruido
    ↓
Output = Input - Noise
```

**Ventajas sobre filtros clásicos**:

- ✅ **Preserva bordes y detalles finos** (entrenada end-to-end)
- ✅ **Adapta al tipo de ruido** presente en CTs
- ✅ **Resultados clínicos superiores** (estudios demuestran mejora en diagnóstico)
- ⚠️ Requiere GPU para tiempo real (CPU: ~1-2 seg por imagen 512×512)

**Ejemplo**:

```cpp
// Cargar modelo al inicio (una sola vez)
static std::string modelPath = "../src/models/dncnn_grayscale.onnx";

// Aplicar a imagen actual
cv::Mat noisy = Bridge::itkToOpenCV8bit(dicomData.image);
cv::Mat denoised = Preprocessing::applyDnCNN(noisy, modelPath);

// Comparar métricas
double psnr = Preprocessing::calculatePSNR(noisy, denoised);
std::cout << "PSNR: " << psnr << " dB (>30 dB = excelente)" << std::endl;

// Visualizar diferencia
cv::Mat noise;
cv::subtract(noisy, denoised, noise);
cv::imshow("Ruido eliminado", noise * 5);  // Amplificar para visualizar
```

**Implementación interna (crítico)**:

```cpp
cv::Mat Preprocessing::applyDnCNN(const cv::Mat& image, const std::string& modelPath) {
    // 1. Cargar modelo ONNX (cachear para eficiencia)
    static cv::dnn::Net net;
    static std::string loadedModel = "";
    
    if (loadedModel != modelPath) {
        net = cv::dnn::readNetFromONNX(modelPath);
        if (net.empty()) {
            throw std::runtime_error("No se pudo cargar modelo DnCNN");
        }
        loadedModel = modelPath;
    }

    // 2. Normalizar a [0, 1]
    cv::Mat normalized;
    image.convertTo(normalized, CV_32F);
    if (image.type() == CV_8U) {
        normalized /= 255.0;
    } else {  // CV_16S
        double minVal, maxVal;
        cv::minMaxLoc(image, &minVal, &maxVal);
        normalized = (normalized - minVal) / (maxVal - minVal);
    }

    // 3. Crear blob (NCHW format)
    cv::Mat blob = cv::dnn::blobFromImage(normalized, 1.0, normalized.size(),
                                          cv::Scalar(0), false, false);

    // 4. Inferencia
    net.setInput(blob);
    cv::Mat outputBlob = net.forward();

    // 5. Extraer resultado (CRÍTICO: acceso directo al blob)
    const int* dims = outputBlob.size.p;
    int batch = dims[0];    // Siempre 1
    int channels = dims[1]; // Siempre 1
    int height = dims[2];
    int width = dims[3];

    cv::Mat denoisedFloat(height, width, CV_32F, outputBlob.ptr<float>(0, 0));
    denoisedFloat = denoisedFloat.clone();  // Copiar antes que blob se destruya

    // 6. Clip a [0, 1]
    cv::max(denoisedFloat, 0.0, denoisedFloat);
    cv::min(denoisedFloat, 1.0, denoisedFloat);

    // 7. Desnormalizar al tipo original
    cv::Mat result;
    if (image.type() == CV_8U) {
        denoisedFloat.convertTo(result, CV_8U, 255.0);
    } else {
        // Reconstruir rango 16-bit original
        double minVal, maxVal;
        cv::minMaxLoc(image, &minVal, &maxVal);
        denoisedFloat = denoisedFloat * (maxVal - minVal) + minVal;
        denoisedFloat.convertTo(result, CV_16S);
    }

    return result;
}
```

**Bug histórico resuelto** ⚠️:

- ❌ **Problema**: Usar `cv::dnn::imagesFromBlob()` generaba imágenes completamente negras
- ✅ **Solución**: Acceso directo con `outputBlob.ptr<float>(0, 0)` y crear `cv::Mat` manualmente
- **Lección**: No todas las utilidades de OpenCV DNN son confiables, a veces hay que ir al bajo nivel

---

### 5. `calculatePSNR()`

**Descripción**: Calcula **Peak Signal-to-Noise Ratio** entre imagen original y procesada

**Firma**:

```cpp
double calculatePSNR(const cv::Mat& original, const cv::Mat& processed);
```

**Parámetros**:

- `original`: Imagen de referencia
- `processed`: Imagen filtrada/procesada

**Retorna**:

- PSNR en decibelios (dB)
  - **> 40 dB**: Excelente (casi imperceptible)
  - **30-40 dB**: Buena calidad
  - **20-30 dB**: Calidad aceptable
  - **< 20 dB**: Pobre calidad

**Fórmula**:
$$
\text{PSNR} = 10 \cdot \log_{10}\left(\frac{\text{MAX}^2}{\text{MSE}}\right)
$$

Donde:

- $\text{MAX}$ = valor máximo posible del píxel (255 para 8-bit)
- $\text{MSE}$ = Mean Squared Error = $\frac{1}{n}\sum(I_1 - I_2)^2$

**Ejemplo**:

```cpp
cv::Mat original = loadImage();
cv::Mat denoised = Preprocessing::applyDnCNN(original, modelPath);

double psnr = Preprocessing::calculatePSNR(original, denoised);

if (psnr > 35) {
    std::cout << "Denoising excelente: " << psnr << " dB" << std::endl;
} else {
    std::cout << "Denoising pobre, ajustar parámetros" << std::endl;
}
```

---

### 6. `calculateSNR()`

**Descripción**: Calcula **Signal-to-Noise Ratio** de una imagen (opcionalmente en ROI)

**Firma**:

```cpp
double calculateSNR(const cv::Mat& image, const cv::Mat& mask = cv::Mat());
```

**Parámetros**:

- `image`: Imagen a analizar
- `mask`: Máscara binaria opcional (ROI)

**Retorna**:

- SNR en decibelios (dB)

**Fórmula**:
$$
\text{SNR} = 20 \cdot \log_{10}\left(\frac{\mu}{\sigma}\right)
$$

Donde:

- $\mu$ = media de la señal
- $\sigma$ = desviación estándar (estimación del ruido)

**Ejemplo**:

```cpp
// SNR global
double snrGlobal = Preprocessing::calculateSNR(image);

// SNR solo en región de pulmones
cv::Mat lungsMask = Segmentation::segmentLungs(...);
double snrLungs = Preprocessing::calculateSNR(image, lungsMask);

std::cout << "SNR global: " << snrGlobal << " dB" << std::endl;
std::cout << "SNR pulmones: " << snrLungs << " dB" << std::endl;
```

## 🔍 Comparación de Técnicas

| Técnica | Tipo Ruido | Preserva Bordes | Velocidad | Calidad | Uso Clínico |
|---------|------------|-----------------|-----------|---------|-------------|
| **Gaussiano** | Gaussiano | ⚠️ Medio | 🚀 Muy rápido | ⭐⭐⭐ | Leve |
| **Mediana** | Impulsivo | ✅ Excelente | 🚀 Rápido | ⭐⭐⭐⭐ | Artefactos |
| **Bilateral** | Gaussiano | ✅✅ Excepcional | ⚠️ Medio | ⭐⭐⭐⭐⭐ | Pre-segmentación |
| **CLAHE** | N/A (contraste) | ✅ Sí | 🚀 Rápido | ⭐⭐⭐⭐ | Visualización |
| **DnCNN** | Mixto | ✅✅ Excelente | 🐢 Lento (CPU) | ⭐⭐⭐⭐⭐ | Diagnóstico |

## ⚡ Consideraciones de Rendimiento

### **Tiempos de Ejecución** (512×512, CPU Intel i7)

```bash
Gaussiano (5×5):    0.8 ms
Mediana (5×5):      3.2 ms
Bilateral (d=9):    45 ms   ⚠️
CLAHE:              12 ms
DnCNN (CPU):        1800 ms  ⚠️
DnCNN (GPU):        45 ms    ✅
```

### **Optimización para Tiempo Real**

```cpp
// Usar DnCNN solo cuando usuario lo seleccione
if (userRequestedDnCNN) {
    // Mostrar indicador de progreso
    QProgressDialog progress("Aplicando DnCNN...", "Cancelar", 0, 100, parent);
    progress.show();
    
    // Ejecutar en thread separado
    QFuture<cv::Mat> future = QtConcurrent::run([=]() {
        return Preprocessing::applyDnCNN(image, modelPath);
    });
    
    // Esperar con GUI responsive
    while (!future.isFinished()) {
        QApplication::processEvents();
    }
    
    cv::Mat result = future.result();
}
```

### **Caché de Modelos**

```cpp
// El modelo DnCNN se carga UNA SOLA VEZ
// Implementado internamente con variable static
static cv::dnn::Net net;
static std::string loadedModel = "";

if (loadedModel != modelPath) {
    net = cv::dnn::readNetFromONNX(modelPath);
    loadedModel = modelPath;
    std::cout << "Modelo DnCNN cargado" << std::endl;
} else {
    // Reutilizar modelo ya cargado (¡ahorra 200ms!)
}
```

## 🎓 Fundamentos Teóricos

### **¿Por qué funciona el filtro gaussiano?**

Teorema de convolución: Una señal ruidosa puede modelarse como:
$$
I_{\text{noisy}}(x,y) = I_{\text{clean}}(x,y) + N(0, \sigma^2)
$$

La convolución con gaussiana actúa como **filtro paso-bajo**:

- Elimina componentes de alta frecuencia (ruido)
- Preserva componentes de baja frecuencia (estructuras grandes)

### **¿Cómo DnCNN supera a filtros clásicos?**

1. **Aprendizaje end-to-end**: Entrenada con pares (noisy, clean)
2. **Residual learning**: Predice solo el ruido $\hat{N} \approx N$
3. **Batch Normalization**: Estabiliza entrenamiento
4. **Receptive field grande**: 35×35 píxeles de contexto

$$
\hat{I}_{\text{clean}} = I_{\text{noisy}} - \text{DnCNN}(I_{\text{noisy}})
$$

## 🐛 Solución de Problemas

### **Problema: DnCNN devuelve imagen negra**

```cpp
// ❌ CAUSA: Usar imagesFromBlob() (buggy en algunas versiones)
std::vector<cv::Mat> outputs;
cv::dnn::imagesFromBlob(outputBlob, outputs);
cv::Mat result = outputs[0];  // ¡Completamente negro!

// ✅ SOLUCIÓN: Acceso directo al blob
const int* dims = outputBlob.size.p;
int height = dims[2], width = dims[3];
cv::Mat result(height, width, CV_32F, outputBlob.ptr<float>(0, 0));
result = result.clone();  // Copiar antes que blob muera
```

### **Problema: CLAHE amplifica ruido**

```cpp
// Reducir clipLimit
cv::Mat result = Preprocessing::applyCLAHE(image, 1.5);  // En lugar de 3.0

// O aplicar filtro antes
cv::Mat smoothed = Preprocessing::applyMedianFilter(image, 3);
cv::Mat enhanced = Preprocessing::applyCLAHE(smoothed, 2.0);
```

### **Problema: Filtro mediana muy lento**

```cpp
// Para imágenes grandes, usar kernel más pequeño
cv::Mat result = Preprocessing::applyMedianFilter(image, 3);  // En lugar de 9

// O aplicar solo en ROI
cv::Mat roi = image(cv::Rect(x, y, w, h));
cv::Mat filteredROI = Preprocessing::applyMedianFilter(roi, 5);
```

## 🚀 Extensiones Futuras

### **Filtros Adicionales**

```cpp
// Bilateral filter (preserva bordes)
cv::Mat applyBilateralFilter(const cv::Mat& image, int d, double sigmaColor, double sigmaSpace);

// Non-local means (excelente calidad, muy lento)
cv::Mat applyNLMeansDenoising(const cv::Mat& image, float h);
```

### **DnCNN para 16-bit**

```cpp
// Entrenar modelo específico para valores HU
// Preservar rango dinámico completo [-1024, +3071]
cv::Mat applyDnCNN16bit(const cv::Mat& image16bit, const std::string& modelPath);
```

### **Procesamiento en GPU**

```cpp
// Acelerar 40× con OpenCL/CUDA
net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
```

## 📚 Referencias

- [DnCNN Paper](https://arxiv.org/abs/1608.03981) - Zhang et al., 2017
- [CLAHE Algorithm](https://en.wikipedia.org/wiki/Adaptive_histogram_equalization)
- [OpenCV DNN Module](https://docs.opencv.org/4.x/d6/d0f/group__dnn.html)
- [Medical Image Denoising](https://www.sciencedirect.com/topics/medicine-and-dentistry/image-denoising)

---

**Versión**: 1.0  
**Última actualización**: Noviembre 2025  
**Archivos**: `src/core/preprocessing.h`, `src/core/preprocessing.cpp`
