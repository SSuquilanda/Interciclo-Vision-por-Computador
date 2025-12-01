# 🔍 Segmentation - Segmentación de Órganos en CT

## 📋 Descripción

El módulo **Segmentation** (`segmentation.h/cpp`) implementa algoritmos para **identificar y delimitar órganos** en imágenes de tomografía computarizada (CT) usando los valores de Hounsfield Units (HU). Proporciona segmentación automática de pulmones, huesos y arterias principales (aorta).

## 🎯 Responsabilidad

> **Única responsabilidad**: Generar máscaras binarias que delimiten estructuras anatómicas específicas basándose en propiedades radiológicas

## 🏗️ Arquitectura

```cpp
namespace Segmentation {
    struct SegmentedRegion {
        cv::Mat mask;              // Máscara binaria (255 = órgano, 0 = fondo)
        std::string label;         // Nombre del órgano ("Pulmones", "Huesos", etc.)
        double area;               // Área en píxeles
        double meanHU;             // Densidad media en Hounsfield Units
        cv::Rect boundingBox;      // Bounding box de la región
        std::vector<std::vector<cv::Point>> contours;  // Contornos
    };

    // Segmentación por órgano
    std::vector<SegmentedRegion> segmentLungs(const itk::Image<short, 2>::Pointer& image);
    std::vector<SegmentedRegion> segmentBones(const itk::Image<short, 2>::Pointer& image);
    std::vector<SegmentedRegion> segmentAorta(const itk::Image<short, 2>::Pointer& image);
}
```

## 📚 API Pública

### 1. `segmentLungs()` 🫁

**Descripción**: Segmenta tejido pulmonar usando umbrales de HU característicos del aire

**Firma**:

```cpp
std::vector<SegmentedRegion> segmentLungs(const itk::Image<short, 2>::Pointer& image);
```

**Parámetros**:

- `image`: Imagen ITK 16-bit con valores HU originales

**Retorna**:

- Vector de regiones (típicamente 2: pulmón derecho + pulmón izquierdo)

**Rango de HU utilizado**:

```cpp
constexpr int MIN_HU_LUNG = -1000;  // Aire en alvéolos
constexpr int MAX_HU_LUNG = -400;   // Tejido pulmonar ventilado
```

**Justificación radiológica**:

- **-1000 HU**: Aire puro (alvéolos llenos de aire)
- **-500 HU**: Tejido pulmonar normal (mezcla aire + parénquima)
- **-400 HU**: Límite superior (consolidaciones se vuelven más densas)

**Algoritmo**:

```bash
1. Convertir ITK → OpenCV 16-bit (preservar HU)
2. Umbralizar: -1000 ≤ HU ≤ -400
3. Operaciones morfológicas:
   - Close (kernel 7×7): Rellenar pequeños huecos
   - Open (kernel 5×5): Eliminar ruido
4. Análisis de componentes conectadas
5. Filtrar regiones pequeñas (< 500 píxeles)
6. Calcular métricas (área, HU media, contornos)
7. Retornar SegmentedRegions
```

**Ejemplo de uso**:

```cpp
auto dicomData = DicomIO::readDicomImage("chest_ct.dcm");
auto lungRegions = Segmentation::segmentLungs(dicomData.image);

std::cout << "Pulmones detectados: " << lungRegions.size() << std::endl;

for (const auto& region : lungRegions) {
    std::cout << "  " << region.label << std::endl;
    std::cout << "    Área: " << region.area << " px²" << std::endl;
    std::cout << "    Densidad: " << region.meanHU << " HU" << std::endl;
    
    // Visualizar
    cv::imshow(region.label, region.mask);
}
```

**Casos especiales**:

- **Paciente intubado**: Puede detectar vías aéreas como "pulmones"
- **Neumonía/consolidación**: Región afectada puede no segmentarse (HU > -400)
- **Derrame pleural**: Fluido no se incluye (HU cercano a agua, 0 HU)

---

### 2. `segmentBones()` 🦴

**Descripción**: Segmenta tejido óseo usando umbrales de alta densidad

**Firma**:

```cpp
std::vector<SegmentedRegion> segmentBones(const itk::Image<short, 2>::Pointer& image);
```

**Parámetros**:

- `image`: Imagen ITK 16-bit con valores HU

**Retorna**:

- Vector de regiones óseas (costillas, vértebras, esternón, etc.)

**Rango de HU utilizado**:

```cpp
constexpr int MIN_HU_BONE = 200;    // Hueso trabecular
constexpr int MAX_HU_BONE = 3000;   // Hueso cortical denso
```

**Justificación radiológica**:

- **+200 HU**: Hueso esponjoso/trabecular (vértebras)
- **+700 HU**: Hueso compacto típico (costillas)
- **+1000-3000 HU**: Hueso cortical muy denso (cráneo, fémur)

**Algoritmo**:

```bash
1. Convertir ITK → OpenCV 16-bit
2. Umbralizar: 200 ≤ HU ≤ 3000
3. Morfología:
   - Close (kernel 5×5): Conectar fragmentos de mismo hueso
   - Open (kernel 3×3): Eliminar píxeles aislados
4. Análisis de componentes conectadas
5. Filtrar regiones diminutas (< 100 píxeles)
6. Etiquetar por posición anatómica si es posible
7. Retornar SegmentedRegions
```

**Ejemplo de uso**:

```cpp
auto dicomData = DicomIO::readDicomImage("spine_ct.dcm");
auto boneRegions = Segmentation::segmentBones(dicomData.image);

std::cout << "Estructuras óseas: " << boneRegions.size() << std::endl;

// Crear máscara combinada de todos los huesos
cv::Mat allBones = cv::Mat::zeros(boneRegions[0].mask.size(), CV_8U);
for (const auto& region : boneRegions) {
    cv::bitwise_or(allBones, region.mask, allBones);
}

cv::imshow("Todos los huesos", allBones);
```

**Casos especiales**:

- **Calcificaciones arteriales**: Pueden segmentarse como "hueso" (HU > 400)
- **Prótesis metálicas**: Exceden rango (HU > 3000), aparecen blancas
- **Osteoporosis**: Hueso menos denso, puede caer bajo umbral mínimo

---

### 3. `segmentAorta()` 🫀

**Descripción**: Segmenta arteria aorta y grandes vasos con medio de contraste

**Firma**:

```cpp
std::vector<SegmentedRegion> segmentAorta(const itk::Image<short, 2>::Pointer& image);
```

**Parámetros**:

- `image`: Imagen ITK 16-bit (típicamente CT con contraste IV)

**Retorna**:

- Vector de regiones vasculares (aorta, arterias pulmonares principales)

**Rango de HU utilizado**:

```cpp
constexpr int MIN_HU_AORTA = 120;   // Contraste moderado
constexpr int MAX_HU_AORTA = 400;   // Contraste intenso
```

**Justificación radiológica**:

- **Sin contraste**: Sangre ≈ +40-60 HU (difícil de segmentar)
- **Con contraste yodado**: +120 a +400 HU
  - **+150 HU**: Realce arterial típico
  - **+250 HU**: Realce intenso (fase arterial)
  - **+350-400 HU**: Contraste muy concentrado

⚠️ **IMPORTANTE**: Esta segmentación **REQUIERE** contraste intravenoso. Sin contraste, detectará tejidos densos incorrectos.

**Algoritmo**:

```bash
1. Convertir ITK → OpenCV 16-bit
2. Umbralizar: 120 ≤ HU ≤ 400
3. Morfología:
   - Close (kernel 7×7): Conectar vasos fragmentados
   - Open (kernel 5×5): Eliminar artefactos
4. Análisis de componentes conectadas
5. Filtrar por:
   - Área: > 50 píxeles (vasos pequeños son artefactos)
   - Forma: Circularidad o tubularidad
   - Posición: Central en tórax (para aorta)
6. Retornar SegmentedRegions
```

**Ejemplo de uso**:

```cpp
auto dicomData = DicomIO::readDicomImage("angioCT.dcm");

// Verificar si hay contraste
if (dicomData.metadata.find("ContrastAgent") == dicomData.metadata.end()) {
    std::cerr << "⚠️ ADVERTENCIA: Estudio sin contraste, segmentación imprecisa" << std::endl;
}

auto vascularRegions = Segmentation::segmentAorta(dicomData.image);

for (const auto& region : vascularRegions) {
    std::cout << region.label << " - Área: " << region.area << " px²" << std::endl;
    
    // Calcular diámetro aproximado
    double radius = std::sqrt(region.area / M_PI);
    double diameter_mm = radius * 2 * 0.5;  // Asumiendo spacing 0.5mm
    std::cout << "  Diámetro aprox: " << diameter_mm << " mm" << std::endl;
}
```

**Casos especiales**:

- **Sin contraste**: Segmentará huesos y calcificaciones (falsos positivos)
- **Contraste en fase venosa**: Realce más bajo, puede fallar detección
- **Disección aórtica**: Flap intimal puede no capturarse correctamente

---

## 🔍 Implementación Interna

### **Pipeline Común**

Todas las funciones de segmentación siguen este patrón:

```cpp
std::vector<SegmentedRegion> Segmentation::segmentOrgan(
    const itk::Image<short, 2>::Pointer& itkImage,
    int minHU, int maxHU,
    const std::string& organName
) {
    // 1. ITK → OpenCV (16-bit, preservar HU)
    cv::Mat raw16bit = Bridge::itkToOpenCV16bit(itkImage);

    // 2. Umbralización por rango HU
    cv::Mat binaryMask;
    cv::inRange(raw16bit, minHU, maxHU, binaryMask);

    // 3. Limpieza morfológica
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7));
    cv::morphologyEx(binaryMask, binaryMask, cv::MORPH_CLOSE, kernel);
    
    kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(binaryMask, binaryMask, cv::MORPH_OPEN, kernel);

    // 4. Análisis de componentes conectadas
    cv::Mat labels, stats, centroids;
    int numLabels = cv::connectedComponentsWithStats(binaryMask, labels, stats, centroids);

    // 5. Procesar cada componente
    std::vector<SegmentedRegion> regions;
    for (int i = 1; i < numLabels; ++i) {  // Saltar label 0 (fondo)
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        
        // Filtrar regiones pequeñas
        if (area < minAreaThreshold) continue;

        // Crear región
        SegmentedRegion region;
        region.mask = (labels == i);
        region.label = organName + " " + std::to_string(i);
        region.area = area;
        
        // Calcular densidad media
        cv::Scalar meanHU = cv::mean(raw16bit, region.mask);
        region.meanHU = meanHU[0];
        
        // Extraer bounding box
        region.boundingBox = cv::Rect(
            stats.at<int>(i, cv::CC_STAT_LEFT),
            stats.at<int>(i, cv::CC_STAT_TOP),
            stats.at<int>(i, cv::CC_STAT_WIDTH),
            stats.at<int>(i, cv::CC_STAT_HEIGHT)
        );
        
        // Encontrar contornos
        cv::findContours(region.mask, region.contours, 
                        cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        regions.push_back(region);
    }

    return regions;
}
```

### **Tabla de Umbrales HU**

| Estructura | Min HU | Max HU | Color Visual | Notas |
|------------|--------|--------|--------------|-------|
| **Aire** | -1000 | -1000 | Negro | Pulmones, tráquea |
| **Pulmones** | -1000 | -400 | Negro-Gris oscuro | Incluye aire alveolar |
| **Grasa** | -120 | -70 | Gris oscuro | Tejido adiposo |
| **Agua/Sangre** | -10 | +40 | Gris medio | Sin contraste |
| **Tejido blando** | +20 | +80 | Gris medio-claro | Músculo, órganos |
| **Contraste (vasos)** | +120 | +400 | Gris claro-Blanco | Requiere contraste IV |
| **Hueso trabecular** | +200 | +700 | Blanco | Vértebras |
| **Hueso cortical** | +700 | +3000 | Blanco brillante | Costillas, cráneo |
| **Metal** | +3000 | +30000 | Blanco puro | Prótesis, clips |

## ⚡ Optimizaciones

### **Procesamiento Multi-Órgano Eficiente**

```cpp
// ❌ INEFICIENTE: Convertir múltiples veces
auto lungs = Segmentation::segmentLungs(itkImage);  // ITK→OpenCV
auto bones = Segmentation::segmentBones(itkImage);  // ITK→OpenCV otra vez
auto aorta = Segmentation::segmentAorta(itkImage);  // ITK→OpenCV otra vez

// ✅ EFICIENTE: Convertir una sola vez
cv::Mat raw16bit = Bridge::itkToOpenCV16bit(itkImage);

// Modificar funciones para aceptar cv::Mat directamente
auto lungs = Segmentation::segmentLungsFromCV(raw16bit);
auto bones = Segmentation::segmentBonesFromCV(raw16bit);
auto aorta = Segmentation::segmentAortaFromCV(raw16bit);
```

### **Caché de Segmentaciones**

```cpp
// En la UI, cachear resultados
std::map<int, std::vector<SegmentedRegion>> cachedSegmentations;
int currentSliceIndex = 42;

if (cachedSegmentations.find(currentSliceIndex) == cachedSegmentations.end()) {
    // Primera vez, segmentar
    auto regions = Segmentation::segmentLungs(itkImage);
    cachedSegmentations[currentSliceIndex] = regions;
} else {
    // Reutilizar resultado cacheado
    auto regions = cachedSegmentations[currentSliceIndex];
}
```

## 🎯 Validación Clínica

### **Verificación de Resultados**

```cpp
void validateSegmentation(const std::vector<SegmentedRegion>& regions, 
                          const std::string& organType) {
    if (organType == "Pulmones") {
        // Debe haber 1-2 regiones (unilateral o bilateral)
        if (regions.size() > 3) {
            std::cerr << "⚠️ Demasiadas regiones pulmonares detectadas" << std::endl;
        }
        
        // Densidad debe estar en rango esperado
        for (const auto& r : regions) {
            if (r.meanHU < -900 || r.meanHU > -300) {
                std::cerr << "⚠️ Densidad pulmonar anormal: " << r.meanHU << " HU" << std::endl;
            }
        }
        
        // Área típica: 10000-50000 px² (dependiendo de resolución)
        for (const auto& r : regions) {
            if (r.area < 5000) {
                std::cerr << "⚠️ Pulmón muy pequeño, posible artefacto" << std::endl;
            }
        }
    }
    
    // ... validaciones similares para huesos y aorta ...
}
```

### **Métricas de Calidad**

```cpp
double calculateDiceCoefficient(const cv::Mat& segmentation, 
                                 const cv::Mat& groundTruth) {
    cv::Mat intersection;
    cv::bitwise_and(segmentation, groundTruth, intersection);
    
    int intersectionPixels = cv::countNonZero(intersection);
    int seg1Pixels = cv::countNonZero(segmentation);
    int seg2Pixels = cv::countNonZero(groundTruth);
    
    return 2.0 * intersectionPixels / (seg1Pixels + seg2Pixels);
}

// Dice > 0.9 = Excelente
// Dice 0.7-0.9 = Buena
// Dice < 0.7 = Pobre
```

## 🐛 Casos Límite y Soluciones

### **Problema 1: Segmentación ruidosa**

```cpp
// Aumentar tamaño de kernel morfológico
cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9));  // En lugar de 5×5
cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
```

### **Problema 2: Regiones fragmentadas**

```cpp
// Usar CLOSE más agresivo
cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11));
cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
```

### **Problema 3: Falsos positivos**

```cpp
// Filtrar por características geométricas
for (auto& region : regions) {
    // Calcular circularidad
    double perimeter = cv::arcLength(region.contours[0], true);
    double circularity = 4 * M_PI * region.area / (perimeter * perimeter);
    
    // Aorta debe ser circular (circularity ≈ 1.0)
    if (circularity < 0.5) {
        regions.erase(std::remove(regions.begin(), regions.end(), region), regions.end());
    }
}
```

## 🚀 Extensiones Futuras

### **Machine Learning para Segmentación**

```cpp
// U-Net para segmentación automática
cv::Mat segmentLungsUNet(const cv::Mat& image, const std::string& modelPath) {
    cv::dnn::Net net = cv::dnn::readNetFromONNX(modelPath);
    cv::Mat blob = cv::dnn::blobFromImage(image, 1.0/255.0);
    net.setInput(blob);
    cv::Mat segmentation = net.forward();
    return segmentation;
}
```

### **Segmentación 3D**

```cpp
// Procesar volumen completo
std::vector<SegmentedRegion3D> segmentLungs3D(
    const itk::Image<short, 3>::Pointer& volume
);
```

### **Atlas-Based Segmentation**

```cpp
// Registrar atlas anatómico y propagar labels
cv::Mat segmentWithAtlas(const cv::Mat& image, const cv::Mat& atlas);
```

### **Segmentación Interactiva**

```cpp
// GrabCut con seeds del usuario
cv::Mat interactiveSegmentation(const cv::Mat& image, 
                                 const std::vector<cv::Point>& foregroundSeeds,
                                 const std::vector<cv::Point>& backgroundSeeds);
```

## 🎛️ Segmentación con Rangos HU Personalizados (Avanzado)

### **Funciones Custom**

Para casos especiales donde los rangos HU por defecto no son óptimos (patologías, artefactos, densidades atípicas), el módulo proporciona versiones parametrizadas:

```cpp
// Pulmones con rango HU personalizado
std::vector<SegmentedRegion> segmentLungsCustom(const cv::Mat& image, int minHU, int maxHU);

// Huesos con rango HU personalizado
std::vector<SegmentedRegion> segmentBonesCustom(const cv::Mat& image, int minHU, int maxHU);

// Aorta con rango HU personalizado
std::vector<SegmentedRegion> segmentAortaCustom(const cv::Mat& image, int minHU, int maxHU);
```

**Parámetros**:

- `image`: Imagen 16-bit (CV_16S) con valores HU originales
- `minHU`: Umbral mínimo en Hounsfield Units
- `maxHU`: Umbral máximo en Hounsfield Units

**Casos de uso clínicos**:

| Condición | Órgano | Rango Custom | Motivo |
|-----------|--------|--------------|--------|
| Enfisema severo | Pulmones | `-1000` a `-600` | Tejido destruido con más aire |
| Consolidación pulmonar | Pulmones | `-600` a `-100` | Tejido más denso (neumonía) |
| Osteoporosis | Huesos | `100` a `400` | Hueso desmineralizado |
| Calcificaciones vasculares | Aorta | `130` a `600` | Placas calcificadas |

**Ejemplo de uso**:

```cpp
// Detectar consolidaciones pulmonares (más densas que aire normal)
int minHU = -600;  // Menos aire que pulmón sano
int maxHU = -100;  // Más denso que aire, menos que agua
auto consolidations = Segmentation::segmentLungsCustom(image, minHU, maxHU);

std::cout << "Regiones consolidadas: " << consolidations.size() << std::endl;
```

**Interfaz de usuario**:

La aplicación GUI proporciona controles para ajustar rangos HU dinámicamente:

- **Checkbox**: "Usar rangos HU personalizados"
- **SpinBoxes**: Ajuste fino de `minHU` y `maxHU` (rango: -3000 a +3000 HU)
- **Botones preset**: Valores predefinidos para órganos comunes
  - 🫁 **Pulmones**: -1000 a -400 HU
  - 🦴 **Huesos**: 200 a 3000 HU
  - ❤️ **Aorta**: 120 a 400 HU

**Ventajas**:

- ✅ Adaptabilidad a patologías específicas
- ✅ Investigación y análisis fino de tejidos
- ✅ Experimentación con diferentes umbrales
- ✅ Ajuste para artefactos o ruido específico

**Advertencias**:

- ⚠️ Rangos muy amplios pueden incluir múltiples tejidos
- ⚠️ Rangos muy estrechos pueden fragmentar el órgano
- ⚠️ Se recomienda conocimiento radiológico para ajustar valores

---

## 🧪 Testing

### **Test de Rangos HU**

```cpp
void testHUThresholds() {
    // Crear imagen sintética con valores HU conocidos
    cv::Mat testImage(512, 512, CV_16S);
    
    // Región pulmonar (-600 HU)
    testImage(cv::Rect(100, 100, 200, 200)) = -600;
    
    // Región ósea (+800 HU)
    testImage(cv::Rect(300, 300, 100, 100)) = 800;
    
    // Convertir a ITK
    auto itkImage = Bridge::openCVToITK(testImage);
    
    // Segmentar
    auto lungs = Segmentation::segmentLungs(itkImage);
    auto bones = Segmentation::segmentBones(itkImage);
    
    // Verificar
    assert(lungs.size() == 1);
    assert(bones.size() == 1);
    assert(std::abs(lungs[0].meanHU - (-600)) < 10);
    assert(std::abs(bones[0].meanHU - 800) < 10);
    
    std::cout << "✓ Test de umbrales HU pasado" << std::endl;
}

void testCustomHURanges() {
    cv::Mat testImage(512, 512, CV_16S);
    testImage.setTo(-700); // Enfisema severo
    
    // Con rango default (-1000 a -400) debería detectar
    auto lungs_default = Segmentation::segmentLungs(testImage);
    
    // Con rango custom (-1000 a -600) debería detectar mejor
    auto lungs_custom = Segmentation::segmentLungsCustom(testImage, -1000, -600);
    
    assert(!lungs_custom.empty());
    std::cout << "✓ Test de rangos custom pasado" << std::endl;
}
```

## 📚 Referencias

- [Hounsfield Scale](https://radiopaedia.org/articles/hounsfield-unit)
- [CT Lung Segmentation](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6429418/)
- [Aortic Segmentation in CTA](https://www.sciencedirect.com/topics/medicine-and-dentistry/computed-tomography-angiography)
- [OpenCV Connected Components](https://docs.opencv.org/4.x/d3/dc0/group__imgproc__shape.html#ga107a78bf7cd25dec05fb4dfc5c9e765f)

---

**Versión**: 1.0  
**Última actualización**: Noviembre 2025  
**Archivos**: `src/core/segmentation.h`, `src/core/segmentation.cpp`
