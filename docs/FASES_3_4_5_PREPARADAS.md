# Estructura de Fases 3, 4 y 5 - Preparada para Implementación

## Resumen

Se han creado los archivos cabecera (.h) y de implementación (.cpp) para las **Fases 3, 4 y 5** del proyecto, con las declaraciones de todos los métodos recomendados según la guía `guia_fases_3_4_5.md`.

Los archivos están listos con implementaciones básicas y placeholders, preparados para ser completados según los órganos específicos que se vayan a segmentar (según consulta con radiólogo).

## Archivos Creados

### Fase 3: Preprocesamiento

- **`src/f3_preprocessing/preprocessing.h`** (174 líneas)
- **`src/f3_preprocessing/preprocessing.cpp`** (167 líneas)

### Fase 4: Segmentación

- **`src/f4_segmentation/segmentation.h`** (287 líneas)
- **`src/f4_segmentation/segmentation.cpp`** (399 líneas)

### Fase 5: Morfología

- **`src/f5_morphology/morphology.h`** (390 líneas)
- **`src/f5_morphology/morphology.cpp`** (335 líneas)

## Fase 3: Preprocesamiento (`f3_preprocessing`)

### Fase 3: Métodos Implementados

#### Conversión y Normalización

- `cv::Mat convertToGrayscale(const cv::Mat& image)` - Conversión a escala de grises
- `cv::Mat normalizeImage(const cv::Mat& image, double alpha, double beta)` - Normalización de intensidades

#### Mejora de Contraste

- `cv::Mat equalizeHistogram(const cv::Mat& image)` - Ecualización de histograma
- `cv::Mat applyCLAHE(const cv::Mat& image, double clipLimit, cv::Size tileGridSize)` - CLAHE adaptativo

#### Filtrado

- `cv::Mat applyGaussianBlur(const cv::Mat& image, cv::Size kernelSize, double sigma)` - Filtro Gaussiano
- `cv::Mat applyMedianFilter(const cv::Mat& image, int kernelSize)` - Filtro de mediana
- `cv::Mat applyBilateralFilter(const cv::Mat& image, int d, double sigmaColor, double sigmaSpace)` - Filtro bilateral

#### Detección de Bordes

- `cv::Mat detectEdgesCanny(const cv::Mat& image, double threshold1, double threshold2)` - Canny
- `cv::Mat detectEdgesSobel(const cv::Mat& image, int dx, int dy, int ksize)` - Sobel
- `cv::Mat applyLaplacian(const cv::Mat& image, int ksize)` - Laplaciano

#### Pipeline Completo

- `cv::Mat preprocessCTImage(const cv::Mat& image, const PreprocessingParams& params)` - Pipeline configurable

### Fase 3: Uso Recomendado

```cpp
#include "f3_preprocessing/preprocessing.h"

// Ejemplo básico
cv::Mat preprocessed = Preprocessing::preprocessCTImage(image, params);

// Ejemplo personalizado
cv::Mat enhanced = Preprocessing::applyCLAHE(image, 2.0, cv::Size(8, 8));
cv::Mat filtered = Preprocessing::applyBilateralFilter(enhanced, 5, 50, 50);
```

## Fase 4: Segmentación (`f4_segmentation`)

### Estructuras de Datos

```cpp
struct SegmentedRegion {
    cv::Mat mask;              // Máscara binaria
    cv::Rect boundingBox;      // Caja delimitadora
    double area;               // Área en píxeles
    cv::Point2d centroid;      // Centroide
    std::string label;         // Etiqueta del órgano
    cv::Scalar color;          // Color de visualización
};

struct SegmentationParams {
    double minHU;              // Rango HU mínimo
    double maxHU;              // Rango HU máximo
    double minArea;            // Área mínima de regiones
    double maxArea;            // Área máxima de regiones
    cv::Scalar visualColor;    // Color para visualización
};
```

### Fse 4: Métodos Implementados

#### Umbralización

- `cv::Mat thresholdOtsu(const cv::Mat& image)` - Otsu automático
- `cv::Mat thresholdManual(const cv::Mat& image, double threshold, double maxValue)` - Umbral manual
- `cv::Mat thresholdByRange(const cv::Mat& image, double minVal, double maxVal)` - Por rango (ideal para HU)
- `cv::Mat thresholdAdaptive(const cv::Mat& image, int blockSize, double C)` - Adaptativo

#### Segmentación Avanzada

- `cv::Mat segmentWatershed(const cv::Mat& image, cv::Mat& markers)` - Watershed
- `cv::Mat segmentKMeans(const cv::Mat& image, int K, int attempts)` - K-means clustering
- `cv::Mat segmentRegionGrowing(const cv::Mat& image, const std::vector<cv::Point>& seedPoints, double threshold)` - Crecimiento de regiones
- `std::vector<SegmentedRegion> findConnectedComponents(const cv::Mat& binaryImage, int minArea)` - Componentes conectados

#### Segmentación Específica de CT (Preparadas para implementar)

- `std::vector<SegmentedRegion> segmentLungs(const cv::Mat& image, const SegmentationParams& params)`
  - Parámetros por defecto: HU -1000 a -400
  
- `SegmentedRegion segmentHeart(const cv::Mat& image, const SegmentationParams& params)`
  - Parámetros por defecto: HU 0 a 100
  
- `std::vector<SegmentedRegion> segmentBones(const cv::Mat& image, const SegmentationParams& params)`
  - Parámetros por defecto: HU >200

- `std::vector<SegmentedRegion> segmentOrgan(const cv::Mat& image, const SegmentationParams& params, const std::string& organName)` - Genérica configurable

#### Refinamiento

- `std::vector<SegmentedRegion> filterRegionsByArea(const std::vector<SegmentedRegion>& regions, double minArea, double maxArea)`
- `std::vector<SegmentedRegion> filterRegionsByPosition(const std::vector<SegmentedRegion>& regions, const cv::Rect& roi)`
- `cv::Mat combineMasks(const std::vector<cv::Mat>& masks)`

#### Visualización

- `cv::Mat applyColorMap(const cv::Mat& mask, int colormapType)`
- `cv::Mat overlaySegmentations(const cv::Mat& image, const std::vector<SegmentedRegion>& regions, double alpha)`
- `cv::Mat drawSegmentationContours(const cv::Mat& image, const std::vector<SegmentedRegion>& regions, int thickness)`

#### Utilidades

- `SegmentationParams getDefaultLungParams()`
- `SegmentationParams getDefaultHeartParams()`
- `SegmentationParams getDefaultBoneParams()`
- `void saveSegmentationMasks(const std::vector<SegmentedRegion>& regions, const std::string& outputPath)`

### Fase 4: Uso Recomendado

```cpp
#include "f4_segmentation/segmentation.h"

// Ejemplo: Segmentar pulmones
auto lungParams = Segmentation::getDefaultLungParams();
auto lungs = Segmentation::segmentLungs(ctImage, lungParams);

// Visualizar resultados
cv::Mat overlay = Segmentation::overlaySegmentations(ctImage, lungs, 0.3);
cv::imshow("Pulmones Segmentados", overlay);

// Guardar máscaras
Segmentation::saveSegmentationMasks(lungs, "output/lungs");
```

## Fase 5: Morfología (`f5_morphology`)

### Fse 5: Estructuras de Datos

```cpp
enum StructuringElementShape {
    MORPH_RECT,      // Rectangular
    MORPH_ELLIPSE,   // Elíptico (circular)
    MORPH_CROSS      // Cruz
};

struct MorphParams {
    StructuringElementShape shape = MORPH_ELLIPSE;
    cv::Size kernelSize = cv::Size(5, 5);
    int iterations = 1;
    cv::Point anchor = cv::Point(-1, -1);
};
```

### Métodos Implementados

#### Operaciones Básicas

- `cv::Mat erode(const cv::Mat& image, cv::Size kernelSize, StructuringElementShape shape, int iterations)`
- `cv::Mat dilate(const cv::Mat& image, cv::Size kernelSize, StructuringElementShape shape, int iterations)`
- `cv::Mat opening(const cv::Mat& image, cv::Size kernelSize, StructuringElementShape shape)`
- `cv::Mat closing(const cv::Mat& image, cv::Size kernelSize, StructuringElementShape shape)`

#### Operaciones Avanzadas

- `cv::Mat morphologicalGradient(const cv::Mat& image, cv::Size kernelSize, StructuringElementShape shape)` - Resalta bordes
- `cv::Mat topHat(const cv::Mat& image, cv::Size kernelSize, StructuringElementShape shape)` - Estructuras brillantes
- `cv::Mat blackHat(const cv::Mat& image, cv::Size kernelSize, StructuringElementShape shape)` - Estructuras oscuras
- `cv::Mat hitOrMiss(const cv::Mat& image, const cv::Mat& kernel1, const cv::Mat& kernel2)` - Detección de patrones

#### Esqueletización

- `cv::Mat skeletonize(const cv::Mat& image)` - Esqueleto de objetos
- `cv::Mat thinning(const cv::Mat& image, int iterations)` - Adelgazamiento

#### Reconstrucción Morfológica

- `cv::Mat morphologicalReconstruction(const cv::Mat& marker, const cv::Mat& mask, int iterations)` - Reconstrucción geodésica
- `cv::Mat fillHoles(const cv::Mat& image)` - Relleno de huecos
- `cv::Mat clearBorder(const cv::Mat& image)` - Elimina objetos en bordes

#### Refinamiento de Segmentaciones

- `cv::Mat cleanMask(const cv::Mat& mask, const MorphParams& params, int minObjectSize)` - Limpieza completa
- `cv::Mat smoothContours(const cv::Mat& mask, cv::Size kernelSize)` - Suavizado de contornos
- `cv::Mat separateObjects(const cv::Mat& mask, cv::Size erosionSize)` - Separación de objetos conectados
- `cv::Mat refineSegmentation(const cv::Mat& mask, const MorphParams& params, int minObjectSize, bool removeBorderObjects)` - **Pipeline completo**

#### Operaciones con Máscaras Múltiples

- `cv::Mat intersectMasks(const std::vector<cv::Mat>& masks)` - Intersección (AND)
- `cv::Mat unionMasks(const std::vector<cv::Mat>& masks)` - Unión (OR)
- `cv::Mat subtractMasks(const cv::Mat& mask1, const cv::Mat& mask2)` - Diferencia

#### Análisis Morfométrico

- `std::vector<double> calculateAreas(const cv::Mat& mask)` - Áreas de objetos
- `std::vector<double> calculatePerimeters(const cv::Mat& mask)` - Perímetros
- `std::vector<double> calculateCircularity(const cv::Mat& mask)` - Circularidad (4π·área/perímetro²)

#### Fase 5: Utilidades

- `cv::Mat invertMask(const cv::Mat& mask)`
- `std::vector<std::vector<cv::Point>> maskToContours(const cv::Mat& mask)`
- `void visualizeMorphOperation(const cv::Mat& original, const cv::Mat& processed, const std::string& windowName)`

### Fase 5: Uso Recomendado

```cpp
#include "f5_morphology/morphology.h"

// Ejemplo: Refinamiento completo de máscara
MorphParams params;
params.kernelSize = cv::Size(5, 5);
params.shape = Morphology::MORPH_ELLIPSE;

cv::Mat refined = Morphology::refineSegmentation(
    mask, 
    params, 
    100,    // minObjectSize
    true    // removeBorderObjects
);

// Análisis morfométrico
auto areas = Morphology::calculateAreas(refined);
auto circularities = Morphology::calculateCircularity(refined);
```

## Integración en CMakeLists.txt

Los archivos ya están integrados en el sistema de compilación:

```cmake
set(COMMON_SOURCES
    src/f2_io/dicom_reader.cpp
    src/f2_io/dataset_explorer.cpp
    src/f3_preprocessing/preprocessing.cpp     # NUEVO
    src/f4_segmentation/segmentation.cpp       # NUEVO
    src/f5_morphology/morphology.cpp           # NUEVO
    src/utils/itk_opencv_bridge.cpp
    src/f6_visualization/visualization.cpp
)
```

## Estado de Compilación

```bash
cd code/build
cmake ..
make -j$(nproc)
```

**Resultado:** **Compilación exitosa** (ambos ejecutables `MyApp` y `ExploreDataset`)

## Próximos Pasos

### 1. Consultar con Radiólogo

Antes de implementar los métodos específicos de segmentación de órganos, se debe consultar con un radiólogo para determinar:

- ¿Qué órganos/estructuras se deben segmentar?
- ¿Qué rangos HU específicos usar para cada estructura?
- ¿Qué criterios de área y posición usar?
- ¿Qué características morfológicas son importantes?

### 2. Implementación Específica

Una vez definidos los órganos objetivo, completar las implementaciones en:

- **`segmentLungs()`** - Segmentación de pulmones
- **`segmentHeart()`** - Segmentación del corazón
- **`segmentBones()`** - Segmentación de estructuras óseas
- **`segmentOrgan()`** - Método genérico para otros órganos

### 3. Ajuste de Parámetros

Refinar los parámetros por defecto basándose en:

- Valores HU específicos del dataset L291
- Tamaño típico de órganos en las imágenes
- Características del ruido y artefactos presentes

### 4. Validación

- Crear programa de prueba para cada fase
- Validar resultados con imágenes representativas
- Comparar FD vs QD en cada etapa del pipeline

## Documentación de Referencia

### Valores HU Típicos en CT

| Estructura | Rango HU |
|------------|----------|
| Aire/Pulmones | -1000 a -400 |
| Tejido Adiposo | -120 a -90 |
| Agua | 0 |
| Tejido Blando | 20 a 70 |
| Músculo | 10 a 40 |
| Sangre | 30 a 45 |
| Hueso Trabecular | 200 a 700 |
| Hueso Cortical | 700 a 3000 |

### Pipeline Recomendado

```bash
CT DICOM → Preprocesamiento → Segmentación → Morfología → Resultados
           (Fase 3)           (Fase 4)       (Fase 5)
           
           - CLAHE            - Threshold     - Limpieza
           - Bilateral Filter - K-means       - Relleno huecos
           - Normalización    - Watershed     - Suavizado
                              - Componentes   - Análisis
```

## Referencias

- `docs/guia_fases_3_4_5.md` - Guía de implementación
- `docs/TODO.md` - Lista completa de tareas del proyecto
- OpenCV Documentation: <https://docs.opencv.org/>
- ITK Documentation: <https://itk.org/Doxygen/html/>

---

**Fecha de creación:** $(date)
**Estado:** Estructura preparada, pendiente implementación específica según consulta radiólogo
**Compilación:** Exitosa
